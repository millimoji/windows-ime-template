// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


#pragma once

//+---------------------------------------------------------------------------
//
// CDisplayAttributeInfo class
//
//----------------------------------------------------------------------------

class CDisplayAttributeInfo : 
    public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>,
                                        ITfDisplayAttributeInfo,
                                        Microsoft::WRL::FtmBase>
{
public:
    CDisplayAttributeInfo() {}
    ~CDisplayAttributeInfo() {}

    HRESULT RuntimeClassInitialize(const GUID& guid, const TF_DISPLAYATTRIBUTE& displayAttribute, const shared_wstring& description, const TfGuidAtom& tfGuidAtom)
    {
        m_guid = guid;
        m_displayAttribute = displayAttribute;
        m_description = description;
        m_tfGuidAtom = tfGuidAtom;
        return S_OK;
    }

    // ITfDisplayAttributeInfo
    STDMETHODIMP GetGUID(_Out_ GUID *pguid) override { *pguid = m_guid; return S_OK; }
    STDMETHODIMP GetDescription(_Out_ BSTR *pbstrDesc) override { *pbstrDesc = SysAllocStringLen(m_description->c_str(), static_cast<UINT>(m_description->length())); return S_OK; }
    STDMETHODIMP GetAttributeInfo(_Out_ TF_DISPLAYATTRIBUTE *ptfDisplayAttr) override { *ptfDisplayAttr = m_displayAttribute; return S_OK; }
    STDMETHODIMP SetAttributeInfo(_In_ const TF_DISPLAYATTRIBUTE*) override { return E_NOTIMPL; }
    STDMETHODIMP Reset() override { return E_NOTIMPL; }

protected:
    GUID m_guid = {};
    TF_DISPLAYATTRIBUTE m_displayAttribute = {};
    shared_wstring m_description;
    TfGuidAtom m_tfGuidAtom = {};
};

// //+---------------------------------------------------------------------------
// //
// // CDisplayAttributeInfoInput class
// //
// //----------------------------------------------------------------------------
// 
// class CDisplayAttributeInfoInput : public CDisplayAttributeInfo
// {
// public:
//     CDisplayAttributeInfoInput()
//     {
//         _pguid = &WindowsImeLib::g_processorFactory->GetConstantProvider()->DisplayAttributeInput();
//         _pDisplayAttribute = &_s_DisplayAttribute;
//         _pDescription = _s_szDescription;
//         _pValueName = _s_szValueName;
//     }
// 
//     static const TF_DISPLAYATTRIBUTE _s_DisplayAttribute;
//     static const WCHAR _s_szDescription[];
//     static const WCHAR _s_szValueName[];
// };
// 
// //+---------------------------------------------------------------------------
// //
// // CDisplayAttributeInfoConverted class
// //
// //----------------------------------------------------------------------------
// 
// class CDisplayAttributeInfoConverted : public CDisplayAttributeInfo
// {
// public:
//     CDisplayAttributeInfoConverted()
//     {
//         _pguid = &WindowsImeLib::g_processorFactory->GetConstantProvider()->DisplayAttributeConverted();
//         _pDisplayAttribute = &_s_DisplayAttribute;
//         _pDescription = _s_szDescription;
//         _pValueName = _s_szValueName;
//     }
// 
//     static const TF_DISPLAYATTRIBUTE _s_DisplayAttribute;
//     static const WCHAR _s_szDescription[];
//     static const WCHAR _s_szValueName[];
// };
