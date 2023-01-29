// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "../WindowsImeLib.h"
#include "TipCandidateString.h"

HRESULT CTipCandidateString::CreateInstance(_Outptr_ CTipCandidateString **ppobj)
{
    RETURN_IF_FAILED(Microsoft::WRL::MakeAndInitialize<CTipCandidateString>(ppobj));
    return S_OK;
}

HRESULT CTipCandidateString::CreateInstance(REFIID riid, _Outptr_ void **ppvObj)
{
    wil::com_ptr<CTipCandidateString> candidateString;
    RETURN_IF_FAILED(CreateInstance(&candidateString));
    return candidateString->QueryInterface(riid, ppvObj);
}

// ITfCandidateString methods
STDMETHODIMP CTipCandidateString::GetString(BSTR *pbstr)
{
    *pbstr = SysAllocString(_candidateStr->c_str());
    return S_OK;
}

STDMETHODIMP CTipCandidateString::GetIndex(_Out_ ULONG *pnIndex)
{
    if (pnIndex == nullptr)
    {
        return E_POINTER;
    }

    *pnIndex = _index;
    return S_OK;
}

shared_wstring CTipCandidateString::GetUnderlyingString()
{
    return _candidateStr;
}

HRESULT CTipCandidateString::SetIndex(ULONG uIndex)
{
    _index = uIndex;
    return S_OK;
}

HRESULT CTipCandidateString::SetString(const shared_wstring& candidateString)
{
    _candidateStr = candidateString;
    return S_OK;
}
