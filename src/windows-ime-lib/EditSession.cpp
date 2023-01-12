// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "EditSession.h"
#include "WindowsIME.h"

//+---------------------------------------------------------------------------
//
// ctor
//
//----------------------------------------------------------------------------

CEditSessionBase::CEditSessionBase(_In_ CWindowsIME *pTextService, _In_ ITfContext *pContext)
{
    _refCount = 1;
    _pContext = pContext;
    _pContext->AddRef();

    _pTextService = pTextService;
    _pTextService->AddRef();
}

//+---------------------------------------------------------------------------
//
// dtor
//
//----------------------------------------------------------------------------

CEditSessionBase::~CEditSessionBase()
{
    _pContext->Release();
    _pTextService->Release();
}

//+---------------------------------------------------------------------------
//
// QueryInterface
//
//----------------------------------------------------------------------------

STDAPI CEditSessionBase::QueryInterface(REFIID riid, _Outptr_ void **ppvObj)
{
    if (ppvObj == nullptr)
    {
        return E_INVALIDARG;
    }

    *ppvObj = nullptr;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ITfEditSession))
    {
        *ppvObj = (ITfLangBarItemButton *)this;
    }

    if (*ppvObj)
    {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

//+---------------------------------------------------------------------------
//
// AddRef
//
//----------------------------------------------------------------------------

STDAPI_(ULONG) CEditSessionBase::AddRef(void)
{
    return ++_refCount;
}

//+---------------------------------------------------------------------------
//
// Release
//
//----------------------------------------------------------------------------

STDAPI_(ULONG) CEditSessionBase::Release(void)
{
    LONG cr = --_refCount;

    assert(_refCount >= 0);

    if (_refCount == 0)
    {
        delete this;
    }

    return cr;
}

HRESULT CWindowsIME::_SubmitEditSessionTask(_In_ ITfContext* context, const std::function<HRESULT (TfEditCookie ec, WindowsImeLib::IWindowsIMECompositionBuffer* textService)>& editSesisonTask, DWORD tfEsFlags)
{
    wil::com_ptr<CEditSessionTask> editSessionTaskObj;
    RETURN_IF_FAILED(Microsoft::WRL::MakeAndInitialize<CEditSessionTask>(&editSessionTaskObj, editSesisonTask, m_compositionBuffer.get()));

    auto editSession = editSessionTaskObj.query<ITfEditSession>();

    HRESULT hr = S_OK;
    RETURN_IF_FAILED(context->RequestEditSession(_tfClientId, editSession.get(), tfEsFlags, &hr));

    return S_OK;
}

