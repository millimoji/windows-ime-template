// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "Globals.h"
#include "WindowsIME.h"
#include "../DisplayAttributeInfo.h"
#include "EnumDisplayAttributeInfo.h"

//+---------------------------------------------------------------------------
//
// ITfDisplayAttributeProvider::EnumDisplayAttributeInfo
//
//----------------------------------------------------------------------------

STDAPI CWindowsIME::EnumDisplayAttributeInfo(__RPC__deref_out_opt IEnumTfDisplayAttributeInfo **ppEnum)
{
    auto activity = WindowsImeLibTelemetry::ITfDisplayAttributeProvider_EnumDisplayAttributeInfo();

    if (m_inprocClient) {
        const auto list = m_inprocClient->GetDisplayAttributeInfoList();
        RETURN_IF_FAILED(Microsoft::WRL::MakeAndInitialize<CEnumDisplayAttributeInfo>(ppEnum, list));
    } else {
        RETURN_IF_FAILED(Microsoft::WRL::MakeAndInitialize<CEnumDisplayAttributeInfo>(ppEnum,
            std::make_shared<std::vector<std::pair<TfGuidAtom, wil::com_ptr<ITfDisplayAttributeInfo>>>>()));
    }

//    CEnumDisplayAttributeInfo* pAttributeEnum = nullptr;
//
//    if (ppEnum == nullptr)
//    {
//        return E_INVALIDARG;
//    }
//
//    *ppEnum = nullptr;
//
//    pAttributeEnum = new (std::nothrow) CEnumDisplayAttributeInfo();
//    if (pAttributeEnum == nullptr)
//    {
//        return E_OUTOFMEMORY;
//    }
//
//    *ppEnum = pAttributeEnum;

    activity.Stop();
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfDisplayAttributeProvider::GetDisplayAttributeInfo
//
//----------------------------------------------------------------------------

STDAPI CWindowsIME::GetDisplayAttributeInfo(__RPC__in REFGUID guidInfo, __RPC__deref_out_opt ITfDisplayAttributeInfo **ppInfo)
{
    auto activity = WindowsImeLibTelemetry::ITfDisplayAttributeProvider_GetDisplayAttributeInfo();

    RETURN_HR_IF(E_INVALIDARG, ppInfo == nullptr);
    *ppInfo = nullptr;

    if (m_inprocClient) {
        for (const auto& pairEnttry: *m_inprocClient->GetDisplayAttributeInfoList()) {
            GUID guid;
            if(SUCCEEDED_LOG(pairEnttry.second->GetGUID(&guid)) && IsEqualGUID(guid, guidInfo)) {
                LOG_IF_FAILED(pairEnttry.second->QueryInterface(IID_PPV_ARGS(ppInfo)));
                break;
            }
        }
    }
    if (*ppInfo == nullptr) {
        RETURN_HR(HRESULT_FROM_WIN32(ERROR_NO_DATA));
    }

//    // Which display attribute GUID?
//    if (IsEqualGUID(guidInfo, WindowsImeLib::g_processorFactory->GetConstantProvider()->DisplayAttributeInput()))
//    {
//        *ppInfo = new (std::nothrow) CDisplayAttributeInfoInput();
//        if ((*ppInfo) == nullptr)
//        {
//            return E_OUTOFMEMORY;
//        }
//    }
//    else if (IsEqualGUID(guidInfo, WindowsImeLib::g_processorFactory->GetConstantProvider()->DisplayAttributeConverted()))
//    {
//        *ppInfo = new (std::nothrow) CDisplayAttributeInfoConverted();
//        if ((*ppInfo) == nullptr)
//        {
//            return E_OUTOFMEMORY;
//        }
//    }
//    else
//    {
//        return E_INVALIDARG;
//    }

    activity.Stop();
    return S_OK;
}
