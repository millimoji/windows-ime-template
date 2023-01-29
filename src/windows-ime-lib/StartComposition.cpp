// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "Globals.h"
#include "EditSession.h"
#include "WindowsIME.h"

// //+---------------------------------------------------------------------------
// //
// // CStartCompositinoEditSession
// //
// //----------------------------------------------------------------------------
// 
// class CStartCompositionEditSession : public CEditSessionBase
// {
// public:
//     CStartCompositionEditSession(_In_ CWindowsIME *pTextService, _In_ ITfContext *pContext) : CEditSessionBase(pTextService, pContext)
//     {
//     }
// 
//     // ITfEditSession
//     STDMETHODIMP DoEditSession(TfEditCookie ec);
// };

//+---------------------------------------------------------------------------
//
// ITfEditSession::DoEditSession
//
//----------------------------------------------------------------------------

// STDAPI CStartCompositionEditSession::DoEditSession(TfEditCookie ec)
HRESULT CompositionBuffer::_StartComposition(TfEditCookie ec, _In_ ITfContext *pContext)
{
    wil::com_ptr<ITfInsertAtSelection> pInsertAtSelection;
    RETURN_IF_FAILED(pContext->QueryInterface(IID_PPV_ARGS(&pInsertAtSelection)));

    wil::com_ptr<ITfRange> pRangeInsert;
    RETURN_IF_FAILED(pInsertAtSelection->InsertTextAtSelection(ec, TF_IAS_QUERYONLY, NULL, 0, &pRangeInsert));

    wil::com_ptr<ITfContextComposition> pContextComposition;
    RETURN_IF_FAILED(pContext->QueryInterface(IID_PPV_ARGS(&pContextComposition)));

    wil::com_ptr<ITfComposition> pComposition;
    RETURN_IF_FAILED(pContextComposition->StartComposition(ec, pRangeInsert.get(), m_framework->GetCompositionSink(), &pComposition));
    RETURN_HR_IF(S_OK /* ? */, !pComposition);

    _SetComposition(pComposition.get());

    // set selection to the adjusted range
    TF_SELECTION tfSelection = {};
    tfSelection.range = pRangeInsert.get();
    tfSelection.style.ase = TF_AE_NONE;
    tfSelection.style.fInterimChar = FALSE;

    RETURN_IF_FAILED(pContext->SetSelection(ec, 1, &tfSelection));

    _SaveCompositionContext(pContext);

    LOG_IF_FAILED(m_framework->_StartLayoutTracking(pContext, ec, pRangeInsert.get()));

    return S_OK;
}

//////////////////////////////////////////////////////////////////////
//
// CWindowsIME class
//
//////////////////////////////////////////////////////////////////////

//+---------------------------------------------------------------------------
//
// _StartComposition
//
// this starts the new (std::nothrow) composition at the selection of the current 
// focus context.
//----------------------------------------------------------------------------

// void CWindowsIME::_StartComposition(_In_ ITfContext *pContext)
// {
//     CStartCompositionEditSession* pStartCompositionEditSession = new (std::nothrow) CStartCompositionEditSession(this, pContext);
// 
//     if (nullptr != pStartCompositionEditSession)
//     {
//         HRESULT hr = S_OK;
//         pContext->RequestEditSession(_tfClientId, pStartCompositionEditSession, TF_ES_SYNC | TF_ES_READWRITE, &hr);
// 
//         pStartCompositionEditSession->Release();
//     }
// }
// 
// //+---------------------------------------------------------------------------
// //
// // _SaveCompositionContext
// //
// // this saves the context _pComposition belongs to; we need this to clear
// // text attribute in case composition has not been terminated on 
// // deactivation
// //----------------------------------------------------------------------------
// 
// void CompositionBuffer::_SaveCompositionContext(_In_ ITfContext *pContext)
// {
//     assert(_pContext == nullptr);
// 
// //    pContext->AddRef();
//     _pContext = pContext;
// } 
