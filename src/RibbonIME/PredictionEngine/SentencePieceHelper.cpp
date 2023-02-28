// Copyright (c) millimoji@gmail.com
#include <pch.h>
#include "../PlatformService.h"
#include "SentencePieceHelper.h"
#include "../../SentencePiece/SentencePiece_h.h"

struct SentencePieceHelperImpl : public SentencePieceHelper, std::enable_shared_from_this<SentencePieceHelperImpl> {

    ~SentencePieceHelperImpl() {
        m_processor.reset();
        FreeLibrary(m_sentencePieceHelper);
    }
    void Initialize(const wchar_t* modelFile) noexcept override {
        const auto platformService = PlatformService::GetInstance();
        const auto myPath = platformService->GetThisModulePath();
        const auto pathAndFilename = platformService->SplitPathAndFile(myPath.c_str());
        const auto internalSentencePiece = std::get<0>(pathAndFilename) + L"Internal.SentencePiece.dll";
        platformService->ComlessCoCreateInstance(internalSentencePiece.c_str(),
            &m_sentencePieceHelper, __uuidof(SentencePieceProcessor), IID_PPV_ARGS(&m_processor));
        THROW_IF_FAILED(m_processor->Load(modelFile));

        THROW_IF_FAILED(m_processor->unk_id(&m_unk_id));
        THROW_IF_FAILED(m_processor->bos_id(&m_bos_id));
        THROW_IF_FAILED(m_processor->eos_id(&m_eos_id));
        THROW_IF_FAILED(m_processor->pad_id(&m_pad_id));
    }
    std::vector<int64_t> Encode64(const wchar_t* sourceText) noexcept override {
        UINT tokenLen = {};
        int64_t* tokenRawPtr = {};
        THROW_IF_FAILED(m_processor->Encode64(sourceText, &tokenLen, &tokenRawPtr));
        wil::unique_cotaskmem_ptr<__int64> tokenPtr(tokenRawPtr);
        auto resultBuf = std::vector<int64_t>(tokenLen, 0LL);
        memcpy(resultBuf.data(), tokenRawPtr, sizeof(int64_t) * tokenLen);
        return resultBuf;
    }
    std::wstring Decode64(const std::vector<int64_t>& tokens) noexcept override {
        wil::unique_bstr bstrText;
        THROW_IF_FAILED(m_processor->Decode64(static_cast<UINT>(tokens.size()), const_cast<int64_t*>(tokens.data()), &bstrText));
        return std::wstring(bstrText.get());
    }
    int unk_id() noexcept override { return m_unk_id; }
    int bos_id() noexcept override { return m_bos_id; }
    int eos_id() noexcept override { return m_eos_id; }
    int pad_id() noexcept override { return m_pad_id; }
private:
    HMODULE m_sentencePieceHelper = {};
    std::wstring m_modelFileName;
    wil::com_ptr<ISentencePieceProcessor> m_processor;
    int m_unk_id;
    int m_bos_id;
    int m_eos_id;
    int m_pad_id;
};

std::shared_ptr<SentencePieceHelper> SentencePieceHelper::CreateInstance() {
    return std::make_shared<SentencePieceHelperImpl>();
}
