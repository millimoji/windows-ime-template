// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "Globals.h"
#include "../Compartment.h"

//////////////////////////////////////////////////////////////////////
//
// CCompartment
//
//////////////////////////////////////////////////////////////////////

//+---------------------------------------------------------------------------
// ctor
//----------------------------------------------------------------------------

CCompartment::CCompartment(_In_ IUnknown* punk, TfClientId tfClientId, _In_ REFGUID guidCompartment)
{
    _guidCompartment = guidCompartment;

    _punk = punk;

    _tfClientId = tfClientId;
}

//+---------------------------------------------------------------------------
// dtor
//----------------------------------------------------------------------------

CCompartment::~CCompartment()
{
}

//+---------------------------------------------------------------------------
// _GetCompartment
//----------------------------------------------------------------------------

HRESULT CCompartment::_GetCompartment(_Outptr_ ITfCompartment **ppCompartment)
{
    wil::com_ptr<ITfCompartmentMgr> pCompartmentMgr;
    RETURN_IF_FAILED(_punk->QueryInterface(IID_ITfCompartmentMgr, (void **)&pCompartmentMgr));
    RETURN_IF_FAILED(pCompartmentMgr->GetCompartment(_guidCompartment, ppCompartment));
    return S_OK;
}

//+---------------------------------------------------------------------------
// _GetCompartmentBOOL
//----------------------------------------------------------------------------

HRESULT CCompartment::_GetCompartmentBOOL(_Out_ BOOL &flag)
{
    flag = FALSE;

    DWORD dw;
    RETURN_IF_FAILED(_GetCompartmentDWORD(dw));

    flag = (BOOL)dw;
    return S_OK;
}

//+---------------------------------------------------------------------------
// _SetCompartmentBOOL
//----------------------------------------------------------------------------

HRESULT CCompartment::_SetCompartmentBOOL(_In_ BOOL flag)
{
    return _SetCompartmentDWORD(flag ? 1 : 0);
}

//+---------------------------------------------------------------------------
// _GetCompartmentDWORD
//----------------------------------------------------------------------------

HRESULT CCompartment::_GetCompartmentDWORD(_Out_ DWORD &dw)
{
    dw = 0;

    wil::com_ptr<ITfCompartment> pCompartment;
    RETURN_IF_FAILED(_GetCompartment(&pCompartment));

    VARIANT var = {};
    auto clearVar = wil::scope_exit([&]() { VariantClear(&var); });
    RETURN_IF_FAILED(pCompartment->GetValue(&var));

    RETURN_HR_IF(S_FALSE, var.vt != VT_I4);

    dw = (DWORD)var.lVal;
    return S_OK;
}

//+---------------------------------------------------------------------------
// _SetCompartmentDWORD
//----------------------------------------------------------------------------

HRESULT CCompartment::_SetCompartmentDWORD(_In_ DWORD dw)
{
    wil::com_ptr<ITfCompartment> pCompartment;
    RETURN_IF_FAILED(_GetCompartment(&pCompartment));

    VARIANT var = {};
    var.vt = VT_I4;
    var.lVal = dw;
    RETURN_IF_FAILED(pCompartment->SetValue(_tfClientId, &var));
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _ClearCompartment
//
//----------------------------------------------------------------------------

HRESULT CCompartment::_ClearCompartment()
{
    if (IsEqualGUID(_guidCompartment, GUID_COMPARTMENT_KEYBOARD_OPENCLOSE))
    {
        return S_FALSE;
    }

    wil::com_ptr<ITfCompartmentMgr> pCompartmentMgr;
    RETURN_IF_FAILED(_punk->QueryInterface(IID_ITfCompartmentMgr, (void **)&pCompartmentMgr));
    RETURN_IF_FAILED(pCompartmentMgr->ClearCompartment(_tfClientId, _guidCompartment));
    return S_OK;
}

//////////////////////////////////////////////////////////////////////
//
// CCompartmentEventSink
//
//////////////////////////////////////////////////////////////////////

//+---------------------------------------------------------------------------
// ctor
//----------------------------------------------------------------------------

HRESULT CCompartmentEventSink::RuntimeClassInitialize(_In_ CESCALLBACK pfnCallback, _In_ void *pv)
{
    _pfnCallback = pfnCallback;
    _pv = pv;
    return S_OK;
}

//+---------------------------------------------------------------------------
// dtor
//----------------------------------------------------------------------------

CCompartmentEventSink::~CCompartmentEventSink()
{
}

//+---------------------------------------------------------------------------
//
// OnChange
//
//----------------------------------------------------------------------------

STDAPI CCompartmentEventSink::OnChange(_In_ REFGUID guidCompartment)
{
    return _pfnCallback(_pv, guidCompartment);
}

//+---------------------------------------------------------------------------
//
// _Advise
//
//----------------------------------------------------------------------------

HRESULT CCompartmentEventSink::_Advise(_In_ IUnknown *punk, _In_ REFGUID guidCompartment)
{
    wil::com_ptr<ITfCompartmentMgr> pCompartmentMgr;
    RETURN_IF_FAILED(punk->QueryInterface(IID_PPV_ARGS(&pCompartmentMgr)));

    RETURN_IF_FAILED(pCompartmentMgr->GetCompartment(guidCompartment, &_pCompartment));

    wil::com_ptr<ITfSource> pSource;
    RETURN_IF_FAILED(_pCompartment->QueryInterface(IID_ITfSource, (void **)&pSource));
    RETURN_IF_FAILED(pSource->AdviseSink(IID_ITfCompartmentEventSink, static_cast<ITfCompartmentEventSink*>(this), &_dwCookie));

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _Unadvise
//
//----------------------------------------------------------------------------

HRESULT CCompartmentEventSink::_Unadvise()
{
    wil::com_ptr<ITfSource> pSource;
    RETURN_IF_FAILED(_pCompartment->QueryInterface(IID_PPV_ARGS(&pSource)));
    RETURN_IF_FAILED(pSource->UnadviseSink(_dwCookie));
    _dwCookie = 0;
    return S_OK;
}
