// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "Globals.h"
#include "../DisplayAttributeInfo.h"
#include "TfInputProcessorProfile.h"

// //+---------------------------------------------------------------------------
// //
// // The registry key and values
// //
// //----------------------------------------------------------------------------
// 
// //+---------------------------------------------------------------------------
// //
// // DisplayAttribute
// //
// //----------------------------------------------------------------------------
// 
// //+---------------------------------------------------------------------------
// //
// // ctor
// //
// //----------------------------------------------------------------------------
// 
// CDisplayAttributeInfo::CDisplayAttributeInfo()
// {
//     DllAddRef();
// 
//     _pguid = nullptr;
//     _pDisplayAttribute = nullptr;
//     _pValueName = nullptr;
// 
//     _refCount = 1;
// }
// 
// //+---------------------------------------------------------------------------
// //
// // dtor
// //
// //----------------------------------------------------------------------------
// 
// CDisplayAttributeInfo::~CDisplayAttributeInfo()
// {
//     DllRelease();
// }
// 
// //+---------------------------------------------------------------------------
// //
// // QueryInterface
// //
// //----------------------------------------------------------------------------
// 
// STDAPI CDisplayAttributeInfo::QueryInterface(REFIID riid, _Outptr_ void **ppvObj)
// {
//     if (ppvObj == nullptr)
//         return E_INVALIDARG;
// 
//     *ppvObj = nullptr;
// 
//     if (IsEqualIID(riid, IID_IUnknown) ||
//         IsEqualIID(riid, IID_ITfDisplayAttributeInfo))
//     {
//         *ppvObj = (ITfDisplayAttributeInfo *)this;
//     }
// 
//     if (*ppvObj)
//     {
//         AddRef();
//         return S_OK;
//     }
// 
//     return E_NOINTERFACE;
// }
// 
// //+---------------------------------------------------------------------------
// //
// // AddRef
// //
// //----------------------------------------------------------------------------
// 
// ULONG CDisplayAttributeInfo::AddRef(void)
// {
//     return ++_refCount;
// }
// 
// //+---------------------------------------------------------------------------
// //
// // Release
// //
// //----------------------------------------------------------------------------
// 
// ULONG CDisplayAttributeInfo::Release(void)
// {
//     LONG cr = --_refCount;
// 
//     assert(_refCount >= 0);
// 
//     if (_refCount == 0)
//     {
//         delete this;
//     }
// 
//     return cr;
// }
// 
// //+---------------------------------------------------------------------------
// //
// // ITfDisplayAttributeInfo::GetGUID
// //
// //----------------------------------------------------------------------------
// 
// STDAPI CDisplayAttributeInfo::GetGUID(_Out_ GUID *pguid)
// {
//     if (pguid == nullptr)
//         return E_INVALIDARG;
// 
//     if (_pguid == nullptr)
//         return E_FAIL;
// 
//     *pguid = *_pguid;
// 
//     return S_OK;
// }
// 
// //+---------------------------------------------------------------------------
// //
// // ITfDisplayAttributeInfo::GetDescription
// //
// //----------------------------------------------------------------------------
// 
// STDAPI CDisplayAttributeInfo::GetDescription(_Out_ BSTR *pbstrDesc)
// {
//     BSTR tempDesc;
// 
//     if (pbstrDesc == nullptr)
//     {
//         return E_INVALIDARG;
//     }
// 
//     *pbstrDesc = nullptr;
// 
//     if ((tempDesc = SysAllocString(_pDescription)) == nullptr)
//     {
//         return E_OUTOFMEMORY;
//     }
// 
//     *pbstrDesc = tempDesc;
// 
//     return S_OK;
// }
// 
// //+---------------------------------------------------------------------------
// //
// // ITfDisplayAttributeInfo::GetAttributeInfo
// //
// //----------------------------------------------------------------------------
// 
// STDAPI CDisplayAttributeInfo::GetAttributeInfo(_Out_ TF_DISPLAYATTRIBUTE *ptfDisplayAttr)
// {
//     if (ptfDisplayAttr == nullptr)
//     {
//         return E_INVALIDARG;
//     }
// 
//     // return the default display attribute.
//     *ptfDisplayAttr = *_pDisplayAttribute;
// 
//     return S_OK;
// }
// 
// //+---------------------------------------------------------------------------
// //
// // ITfDisplayAttributeInfo::SetAttributeInfo
// //
// //----------------------------------------------------------------------------
// 
// STDAPI CDisplayAttributeInfo::SetAttributeInfo(_In_ const TF_DISPLAYATTRIBUTE *ptfDisplayAttr)
// {
//     ptfDisplayAttr;
// 
//     return E_NOTIMPL;
// }
// 
// //+---------------------------------------------------------------------------
// //
// // ITfDisplayAttributeInfo::Reset
// //
// //----------------------------------------------------------------------------
// 
// STDAPI CDisplayAttributeInfo::Reset()
// {
//     return SetAttributeInfo(_pDisplayAttribute);
// }
