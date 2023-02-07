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

HRESULT CompositionBuffer::_StartComposition()
{
    m_isComposing = true;

    return m_framework->_SubmitEditSessionTask(m_workingContext.get(), [this, pContext = m_workingContext](TfEditCookie ec) -> HRESULT
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

        // set selection to the adjusted range
        TF_SELECTION tfSelection = {};
        tfSelection.range = pRangeInsert.get();
        tfSelection.style.ase = TF_AE_NONE;
        tfSelection.style.fInterimChar = FALSE;

        RETURN_IF_FAILED(pContext->SetSelection(ec, 1, &tfSelection));

        _SetComposition(pComposition.get());
        _SaveCompositionContext(pContext.get());

        LOG_IF_FAILED(m_framework->_StartLayoutTracking(pContext.get(), ec, pRangeInsert.get()));
        return S_OK;
    },
    TF_ES_ASYNCDONTCARE | TF_ES_READWRITE);
}

//////////////////////////////////////////////////////////////////////
//
// CWindowsIME class
//
//////////////////////////////////////////////////////////////////////
