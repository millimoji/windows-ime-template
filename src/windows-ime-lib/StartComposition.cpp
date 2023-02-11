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

//+---------------------------------------------------------------------------
//
// ITfEditSession::DoEditSession
//
//----------------------------------------------------------------------------

HRESULT CompositionBuffer::_StartComposition()
{
    wil::com_ptr<CEditSessionTask> task;
    RETURN_IF_FAILED(Microsoft::WRL::MakeAndInitialize<CEditSessionTask>(&task,
        [this](TfEditCookie ec) -> HRESULT
        {
            wil::com_ptr<ITfInsertAtSelection> pInsertAtSelection;
            RETURN_IF_FAILED(m_workingContext->QueryInterface(IID_PPV_ARGS(&pInsertAtSelection)));

            wil::com_ptr<ITfRange> pRangeInsert;
            RETURN_IF_FAILED(pInsertAtSelection->InsertTextAtSelection(ec, TF_IAS_QUERYONLY, NULL, 0, &pRangeInsert));

            wil::com_ptr<ITfContextComposition> pContextComposition;
            RETURN_IF_FAILED(m_workingContext->QueryInterface(IID_PPV_ARGS(&pContextComposition)));

            wil::com_ptr<ITfComposition> pComposition;
            RETURN_IF_FAILED(pContextComposition->StartComposition(ec, pRangeInsert.get(), m_framework->GetCompositionSink(), &pComposition));
            RETURN_HR_IF(S_OK /* ? */, !pComposition);

            // set selection to the adjusted range
            TF_SELECTION tfSelection = {};
            tfSelection.range = pRangeInsert.get();
            tfSelection.style.ase = TF_AE_NONE;
            tfSelection.style.fInterimChar = FALSE;

            RETURN_IF_FAILED(m_workingContext->SetSelection(ec, 1, &tfSelection));

            _SaveCompositionAndContext(pComposition.get(), m_workingContext.get());

            LOG_IF_FAILED(m_framework->_StartLayoutTracking(m_workingContext.get(), ec, pRangeInsert.get()));
            return S_OK;
        }));
    m_listTasks.emplace_back(task);
    m_isComposing = true;
    return S_OK;
}

//////////////////////////////////////////////////////////////////////
//
// CWindowsIME class
//
//////////////////////////////////////////////////////////////////////
