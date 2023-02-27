// (C) 2023 millimoji@gmail.com
#include "pch.h"
#include "SentencePieceHelper.h"
#include <sentencepiece_processor.h>

namespace Ribbon::Prediction
{

struct SentencePieceHelperImpl : public SentencePieceHelper, public std::enable_shared_from_this<SentencePieceHelperImpl>
{
private: // SentencePieceHelper
    void Initialize(const std::wstring_view modelFile) noexcept override try {
        m_modelFileNmae = modelFile;
    }
    catch (...) {}

    std::vector<int64_t> Encode64(std::wstring_view sourceText) noexcept override try {
        EnsureModelLoaded();
        const auto utf8Text = ToUtf8(sourceText);

        std::vector<int> tokens;
        const auto& status = m_processor->Encode(utf8Text, &tokens);
        if (!status.ok()) throw std::runtime_error("SentencePiece: failed to Encode");

        std::vector<int64_t> tokens64(tokens.size());
        std::transform(tokens.begin(), tokens.end(), tokens64.begin(),
            [](int token) -> int64_t { return static_cast<int64_t>(token); });
        return tokens64;
    }
    catch (...) { return std::vector<__int64>(); }

    std::wstring Decode64(const std::vector<int64_t>& tokens) noexcept override try {
        EnsureModelLoaded();
        std::vector<int> tokens32(tokens.size());
        std::transform(tokens.begin(), tokens.end(), tokens32.begin(),
            [](int64_t token) -> int { return static_cast<int>(token); });

        std::string decodedText;
        const auto& status = m_processor->Decode(tokens32, &decodedText);
        if (!status.ok()) throw std::runtime_error("SentencePiece: failed to Dencode");

        return ToUtf16(decodedText);
    }
    catch (...) { return std::wstring(); }

    int unk_id() noexcept override { EnsureModelLoaded(); return m_processor->unk_id(); }
    int bos_id() noexcept override { EnsureModelLoaded(); return m_processor->bos_id(); }
    int eos_id() noexcept override { EnsureModelLoaded(); return m_processor->eos_id(); }
    int pad_id() noexcept override { EnsureModelLoaded(); return m_processor->pad_id(); }

private:
    void EnsureModelLoaded() {
        if (!m_processor) {
            m_processor.reset(new sentencepiece::SentencePieceProcessor());
            const auto& utf8FileName = ToUtf8(m_modelFileNmae);
            const auto& status = m_processor->Load(utf8FileName);
            if (!status.ok()) throw std::runtime_error("SentencePiece: failed to load model file");
        }
    }

    static std::string ToUtf8(const std::wstring_view src)
    {
        const auto requiredSize = WideCharToMultiByte(CP_UTF8, 0, src.data(), static_cast<int>(src.length()), nullptr, 0, nullptr, nullptr);
        std::string utf8(requiredSize, ' ');
        WideCharToMultiByte(CP_UTF8, 0, src.data(), static_cast<int>(src.length()), utf8.data(), requiredSize, nullptr, nullptr);
        return utf8;
    }

    static std::wstring ToUtf16(const std::string_view src)
    {
        const auto requiredSize = MultiByteToWideChar(CP_UTF8, 0, src.data(), static_cast<int>(src.length()), nullptr, 0);
        std::wstring utf16(requiredSize, L' ');
        MultiByteToWideChar(CP_UTF8, 0, src.data(), static_cast<int>(src.length()), utf16.data(), requiredSize);
        return utf16;
    }

private:
    std::wstring m_modelFileNmae;
    std::unique_ptr<sentencepiece::SentencePieceProcessor> m_processor;
};

} // namespace Ribbon::Prediction

/* static */
std::shared_ptr<SentencePieceHelper> SentencePieceHelper::CreateInstance() noexcept try
{
    return std::make_shared<Ribbon::Prediction::SentencePieceHelperImpl>();
}
catch (...) { return std::shared_ptr<SentencePieceHelper>(); }
