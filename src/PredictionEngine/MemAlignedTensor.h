// (C) 2023 millimoji@gmail.com
#pragma once
#include <cstdlib>
#include <stdexcept>
#include <immintrin.h>

class MemAlignedTensor
{
private:
	static constexpr uint32_t alignmentSize = 64;

public:
	MemAlignedTensor() = default;
	MemAlignedTensor(const MemAlignedTensor&) = delete;
	MemAlignedTensor(MemAlignedTensor&& src) noexcept {
		m_row = src.m_row; m_column = src.m_column; m_body = src.m_body; src.m_body = nullptr;
	}
	~MemAlignedTensor() { _aligned_free(m_body); }

	MemAlignedTensor& operator = (const MemAlignedTensor&) = delete;
	MemAlignedTensor& operator = (MemAlignedTensor&& src) noexcept {
		m_row = src.m_row; m_column = src.m_column; m_body = src.m_body; src.m_body = nullptr;
		return *this;
	}

	MemAlignedTensor(int64_t row, int64_t column, const float* src) {
		Copy(row, column, src);
	}
	std::tuple<float*, int, int> GetBuffer() {
		return std::make_tuple(m_body, m_row, m_column);
	}
	winrt::array_view<float> GetArrayView() {
		return winrt::array_view<float>(m_body, m_body + m_row * m_column);
	}
	float* Reserve(int64_t row, int64_t column) {
		if ((row * column) != (m_row * m_column)) {
			void* pv = _aligned_malloc(row * column * sizeof(float), alignmentSize);
			if (pv == nullptr) throw std::bad_alloc();
			_aligned_free(m_body);
			m_body = reinterpret_cast<float*>(pv);
		}
		m_row = static_cast<int>(row); m_column = static_cast<int>(column);
		return m_body;
	}
	void Copy(int64_t row, int64_t column, const float* src) {
		Reserve(row, column);
		if (src != nullptr) {
			memcpy(m_body, src, row * column * sizeof(float));
		}
	}

	// intel avx code
	std::tuple<int64_t, float> GetIndexFromLogits() {
#ifdef _M_X64
		int cosSimIdx = -1;
		float cosSimScore = -FLT_MAX;

		__m256 tmpBuf[4];
		auto tmpSumVec = _mm256_setzero_ps();
		for (auto i = 0; i < m_column; i += 32) {
			const auto v1 = tmpBuf[0] = _mm256_exp_ps(*(reinterpret_cast<__m256*>(m_body + i + 0)));
			const auto v2 = tmpBuf[1] = _mm256_exp_ps(*(reinterpret_cast<__m256*>(m_body + i + 8)));
			const auto v3 = tmpBuf[2] = _mm256_exp_ps(*(reinterpret_cast<__m256*>(m_body + i + 16)));
			const auto v4 = tmpBuf[3] = _mm256_exp_ps(*(reinterpret_cast<__m256*>(m_body + i + 24)));
			const auto v12 = _mm256_add_ps(v1, v2);
			const auto v34 = _mm256_add_ps(v3, v4);
			const auto v1234 = _mm256_add_ps(v12, v34);
			tmpSumVec = _mm256_add_ps(tmpSumVec, v1234);

			const auto maxV12 = _mm256_max_ps(v1, v2);
			const auto maxV34 = _mm256_max_ps(v3, v4);
			const auto maxV1234 = _mm256_max_ps(maxV12, maxV34);

			if (cosSimScore < HorizontalMax(maxV1234)) {
				for (int j = 0; j < 8 * 4; ++j) {
					if (cosSimScore < tmpBuf[0].m256_f32[j]) {
						cosSimScore = tmpBuf[0].m256_f32[j];
						cosSimIdx = i + j;
					}
				}
			}
		}
		const auto sumf = HorizontalAdd(tmpSumVec);

		return std::make_tuple(cosSimIdx, cosSimScore / sumf);
#else
		assert(false); // not impl
#endif
	}

private:
	float HorizontalMax(const __m256& x) {
		const __m128 hiQuad = _mm256_extractf128_ps(x, 1);       // hiQuad = ( x7, x6, x5, x4 )
		const __m128 loQuad = _mm256_castps256_ps128(x);         // loQuad = ( x3, x2, x1, x0 )
		const __m128 maxQuad = _mm_max_ps(loQuad, hiQuad);      // sumQuad = ( x3 + x7, x2 + x6, x1 + x5, x0 + x4 )
		const __m128 loDual = maxQuad;                                  // loDual = ( -, -, x1 + x5, x0 + x4 )
		const __m128 hiDual = _mm_movehl_ps(maxQuad, maxQuad);          // hiDual = ( -, -, x3 + x7, x2 + x6 )
		const __m128 maxDual = _mm_max_ps(loDual, hiDual);      // sumDual = ( -, -, x1 + x3 + x5 + x7, x0 + x2 + x4 + x6 )
		const __m128 lo = maxDual;                                      // lo = ( -, -, -, x0 + x2 + x4 + x6 )
		const __m128 hi = _mm_shuffle_ps(maxDual, maxDual, 0x1); // hi = ( -, -, -, x1 + x3 + x5 + x7 )
		const __m128 maxS = _mm_max_ps(lo, hi);                  // sum = ( -, -, -, x0 + x1 + x2 + x3 + x4 + x5 + x6 + x7 )	}
		return _mm_cvtss_f32(maxS);
	}
	float HorizontalAdd(const __m256& x) {
		const __m128 hiQuad = _mm256_extractf128_ps(x, 1);       // hiQuad = ( x7, x6, x5, x4 )
		const __m128 loQuad = _mm256_castps256_ps128(x);         // loQuad = ( x3, x2, x1, x0 )
		const __m128 sumQuad = _mm_add_ps(loQuad, hiQuad);      // sumQuad = ( x3 + x7, x2 + x6, x1 + x5, x0 + x4 )
		const __m128 loDual = sumQuad;                                  // loDual = ( -, -, x1 + x5, x0 + x4 )
		const __m128 hiDual = _mm_movehl_ps(sumQuad, sumQuad);          // hiDual = ( -, -, x3 + x7, x2 + x6 )
		const __m128 sumDual = _mm_add_ps(loDual, hiDual);      // sumDual = ( -, -, x1 + x3 + x5 + x7, x0 + x2 + x4 + x6 )
		const __m128 lo = sumDual;                                      // lo = ( -, -, -, x0 + x2 + x4 + x6 )
		const __m128 hi = _mm_shuffle_ps(sumDual, sumDual, 0x1); // hi = ( -, -, -, x1 + x3 + x5 + x7 )
		const __m128 sum = _mm_add_ss(lo, hi);                  // sum = ( -, -, -, x0 + x1 + x2 + x3 + x4 + x5 + x6 + x7 )	}
		return _mm_cvtss_f32(sum);
	}

private:
	int m_row = 0;
	int m_column = 0;
	float* m_body = nullptr;
};
