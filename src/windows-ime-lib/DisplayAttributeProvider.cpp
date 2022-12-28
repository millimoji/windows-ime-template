// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "Globals.h"
#include "WindowsIME.h"
#include "DisplayAttributeInfo.h"
#include "EnumDisplayAttributeInfo.h"

//+---------------------------------------------------------------------------
//
// ITfDisplayAttributeProvider::EnumDisplayAttributeInfo
//
//----------------------------------------------------------------------------

STDAPI CWindowsIME::EnumDisplayAttributeInfo(__RPC__deref_out_opt IEnumTfDisplayAttributeInfo **ppEnum)
{
    CEnumDisplayAttributeInfo* pAttributeEnum = nullptr;

    if (ppEnum == nullptr)
    {
        return E_INVALIDARG;
    }

    *ppEnum = nullptr;

    pAttributeEnum = new (std::nothrow) CEnumDisplayAttributeInfo();
    if (pAttributeEnum == nullptr)
    {
        return E_OUTOFMEMORY;
    }

    *ppEnum = pAttributeEnum;

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfDisplayAttributeProvider::GetDisplayAttributeInfo
//
//----------------------------------------------------------------------------

STDAPI CWindowsIME::GetDisplayAttributeInfo(__RPC__in REFGUID guidInfo, __RPC__deref_out_opt ITfDisplayAttributeInfo **ppInfo)
{
    if (ppInfo == nullptr)
    {
        return E_INVALIDARG;
    }

    *ppInfo = nullptr;

    // Which display attribute GUID?
    if (IsEqualGUID(guidInfo, WindowsImeLib::g_processorFactory->GetConstantProvider()->DisplayAttributeInput()))
    {
        *ppInfo = new (std::nothrow) CDisplayAttributeInfoInput();
        if ((*ppInfo) == nullptr)
        {
            return E_OUTOFMEMORY;
        }
    }
    else if (IsEqualGUID(guidInfo, WindowsImeLib::g_processorFactory->GetConstantProvider()->DisplayAttributeConverted()))
    {
        *ppInfo = new (std::nothrow) CDisplayAttributeInfoConverted();
        if ((*ppInfo) == nullptr)
        {
            return E_OUTOFMEMORY;
        }
    }
    else
    {
        return E_INVALIDARG;
    }


    return S_OK;
}
