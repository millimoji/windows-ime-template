#pragma once

#include <wrl/module.h>
#include <wrl/implements.h>
#include "SentencePiece_h.h"

namespace sentencepiece {
    class SentencePieceProcessor;
}

#define RuntimeClass_Internal_SentencePiece_SentencePieceProcessor L"Internal.SentencePiece.SentencePieceProcessor"

class SentencePieceProcessor :
    public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::WinRtClassicComMix>,
    ISentencePieceProcessor,
    Microsoft::WRL::FtmBase>
{
    InspectableClass(RuntimeClass_Internal_SentencePiece_SentencePieceProcessor, TrustLevel::BaseTrust);

public:
    HRESULT RuntimeClassInitialize();
private:
    IFACEMETHODIMP Load(/* [in] */ LPCWSTR fileName) override;
    IFACEMETHODIMP Encode64(/* [in] */ LPCWSTR text, /* [out] */ UINT* tokenLen, /* [size_is][size_is][out] */ __int64** tokens) override;
    IFACEMETHODIMP Decode64(/* [in] */ UINT tokenLen, /* [size_is][in] */ __int64* tokens, /* [out] */ BSTR* text) override;
    IFACEMETHODIMP unk_id(/* [retval][out] */ INT* id) override;
    IFACEMETHODIMP bos_id(/* [retval][out] */ INT* id) override;
    IFACEMETHODIMP eos_id(/* [retval][out] */ INT* id) override;
    IFACEMETHODIMP pad_id(/* [retval][out] */ INT* id) override;

private:
    std::string toUtf8(std::wstring_view src);
    BSTR toUtf16(std::string_view src);

private:
    std::unique_ptr<sentencepiece::SentencePieceProcessor> m_processor;
};
