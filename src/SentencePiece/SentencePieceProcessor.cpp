#include "pch.h"
#include "SentencePiece_h.h"
#include "SentencePieceProcessor.h"
#include <sentencepiece_processor.h>

HRESULT SentencePieceProcessor::RuntimeClassInitialize() try
{
	m_processor.reset(new sentencepiece::SentencePieceProcessor());
	return S_OK;
}
CATCH_RETURN();

HRESULT SentencePieceProcessor::Load(/* [in] */ LPCWSTR  fileName) try
{
	const auto& utf8FileName = toUtf8(fileName);
	const auto& result = m_processor->Load(utf8FileName);
	return result.ok() ? S_OK : E_FAIL;
}
CATCH_RETURN();

HRESULT SentencePieceProcessor::Encode64(/* [in] */ LPCWSTR  text, /* [out] */ UINT* tokenLen, /* [size_is][size_is][out] */ __int64** tokens) try
{
	const auto& utf8Text = toUtf8(text);

	std::vector<int> tokenVector;
	const auto& result = m_processor->Encode(utf8Text, &tokenVector);
	RETURN_HR_IF(E_FAIL, !result.ok());

	int64_t* resultBuffer = reinterpret_cast<int64_t*>(CoTaskMemAlloc(tokenVector.size() * sizeof(int64_t)));
	RETURN_HR_IF(E_OUTOFMEMORY, resultBuffer == nullptr);

	for (auto i = 0ULL; i < tokenVector.size(); ++i) {
		resultBuffer[i] = tokenVector[i];
	}
	*tokenLen = static_cast<UINT>(tokenVector.size());
	*tokens = resultBuffer;
	return S_OK;
}
CATCH_RETURN();

HRESULT SentencePieceProcessor::Decode64(/* [in] */ UINT tokenLen, /* [size_is][in] */ __int64* tokens, /* [out] */ BSTR* text) try
{
	std::vector<int> tokensInt;
	tokensInt.resize(tokenLen);
	for (auto i = 0U; i < tokenLen; ++i) {
		tokensInt[i] = static_cast<int>(tokens[i]);
	}

	std::string decodedText;
	const auto& result = m_processor->Decode(tokensInt, &decodedText);
	RETURN_HR_IF(E_FAIL, !result.ok());

	*text = toUtf16(decodedText);
	return S_OK;
}
CATCH_RETURN();

HRESULT SentencePieceProcessor::unk_id(/* [retval][out] */ INT* id) try
{
	*id = m_processor->unk_id();
	return S_OK;
}
CATCH_RETURN();

HRESULT SentencePieceProcessor::bos_id(/* [retval][out] */ INT* id) try
{
	*id = m_processor->bos_id();
	return S_OK;
}
CATCH_RETURN();

HRESULT SentencePieceProcessor::eos_id(/* [retval][out] */ INT* id) try
{
	*id = m_processor->eos_id();
	return S_OK;
}
CATCH_RETURN();

HRESULT SentencePieceProcessor::pad_id(/* [retval][out] */ INT* id) try
{
	*id = m_processor->pad_id();
	return S_OK;
}
CATCH_RETURN();

std::string SentencePieceProcessor::toUtf8(std::wstring_view src)
{
	const auto requiredSize = WideCharToMultiByte(CP_UTF8, 0, src.data(), static_cast<int>(src.length()), nullptr, 0, nullptr, nullptr);
	std::string utf8(requiredSize, ' ');
	WideCharToMultiByte(CP_UTF8, 0, src.data(), static_cast<int>(src.length()), utf8.data(), requiredSize, nullptr, nullptr);
	return utf8;
}

BSTR SentencePieceProcessor::toUtf16(std::string_view src)
{
	const auto requiredSize = MultiByteToWideChar(CP_UTF8, 0, src.data(), static_cast<int>(src.length()), nullptr, 0);
	BSTR bstr = SysAllocStringLen(nullptr, requiredSize);
	std::wstring utf16(requiredSize, L' ');
	MultiByteToWideChar(CP_UTF8, 0, src.data(), static_cast<int>(src.length()), bstr, requiredSize);
	return bstr;
}

CoCreatableClass(SentencePieceProcessor);
ActivatableClass(SentencePieceProcessor);

