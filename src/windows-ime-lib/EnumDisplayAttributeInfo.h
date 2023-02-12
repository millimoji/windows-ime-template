// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

//+---------------------------------------------------------------------------
//
// CEnumDisplayAttributeInfo
//
//----------------------------------------------------------------------------

class CEnumDisplayAttributeInfo :
    public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>,
                                        IEnumTfDisplayAttributeInfo,
                                        Microsoft::WRL::FtmBase>
{
public:
    CEnumDisplayAttributeInfo() {}
    ~CEnumDisplayAttributeInfo() {}
    HRESULT RuntimeClassInitialize(const std::shared_ptr<std::vector<std::pair<TfGuidAtom, wil::com_ptr<ITfDisplayAttributeInfo>>>>& list);

    // IEnumTfDisplayAttributeInfo
    STDMETHODIMP Clone(_Out_ IEnumTfDisplayAttributeInfo **ppEnum);
    STDMETHODIMP Next(ULONG ulCount, __RPC__out_ecount_part(ulCount, *pcFetched) ITfDisplayAttributeInfo **rgInfo, __RPC__out ULONG *pcFetched);
    STDMETHODIMP Reset();
    STDMETHODIMP Skip(ULONG ulCount);

private:
    LONG _index = 0; // next display attribute to enum
    std::shared_ptr<std::vector<std::pair<TfGuidAtom, wil::com_ptr<ITfDisplayAttributeInfo>>>> m_list;
};
