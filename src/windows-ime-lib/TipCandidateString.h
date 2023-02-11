// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include "Private.h"

class CTipCandidateString :
    public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>,
                                        ITfCandidateString,
                                        Microsoft::WRL::FtmBase>
{
public:
    CTipCandidateString() {}
    virtual ~CTipCandidateString() {}

public:
    static HRESULT CreateInstance(_Outptr_ CTipCandidateString **ppobj);
    static HRESULT CreateInstance(REFIID riid, _Outptr_ void **ppvObj);

    // ITfCandidateString methods
    virtual STDMETHODIMP GetString(BSTR *pbstr);
    virtual STDMETHODIMP GetIndex(_Out_ ULONG *pnIndex);

    HRESULT SetIndex(ULONG uIndex);
    HRESULT SetString(const shared_wstring& candidateStr);
    shared_wstring GetUnderlyingString();

protected:
    int _index = 0;
    shared_wstring _candidateStr;
};