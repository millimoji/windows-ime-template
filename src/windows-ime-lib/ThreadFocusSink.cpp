// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "WindowsIME.h"
#include "CandidateListUIPresenter.h"

//+---------------------------------------------------------------------------
//
// ITfTextLayoutSink::OnSetThreadFocus
//
//----------------------------------------------------------------------------

STDAPI CWindowsIME::OnSetThreadFocus() try
{
    auto activity = WindowsImeLibTelemetry::ITfThreadFocusSink_OnSetThreadFocus();

    if (m_singletonProcessor)
    {
        m_singletonProcessor->SetFocus(true);
    }

    if (m_candidateListView->IsCreated())
    {
        ITfDocumentMgr* pCandidateListDocumentMgr = nullptr;
        ITfContext* pTfContext = m_textLayoutSink._pContextDocument.get();

        if ((nullptr != pTfContext) && SUCCEEDED(pTfContext->GetDocumentMgr(&pCandidateListDocumentMgr)))
        {
            if (pCandidateListDocumentMgr == _pDocMgrLastFocused.get())
            {
                m_candidateListView->OnSetThreadFocus();
                m_singletonProcessor->CandidateListViewInternal_OnSetThreadFocus();
            }

            pCandidateListDocumentMgr->Release();
        }
    }

    activity.Stop();
    return S_OK;
}
CATCH_RETURN()

//+---------------------------------------------------------------------------
//
// ITfTextLayoutSink::OnKillThreadFocus
//
//----------------------------------------------------------------------------

STDAPI CWindowsIME::OnKillThreadFocus() try
{
    auto activity = WindowsImeLibTelemetry::ITfThreadFocusSink_OnKillThreadFocus();

    if (m_singletonProcessor)
    {
        m_singletonProcessor->SetFocus(false);
    }

    if (m_candidateListView->IsCreated())
    {
        ITfDocumentMgr* pCandidateListDocumentMgr = nullptr;
        ITfContext* pTfContext = m_textLayoutSink._pContextDocument.get();

        if ((nullptr != pTfContext) && SUCCEEDED(pTfContext->GetDocumentMgr(&pCandidateListDocumentMgr)))
        {
//            if (_pDocMgrLastFocused)
//            {
//                _pDocMgrLastFocused->Release();
//                _pDocMgrLastFocused = nullptr;
//            }
            _pDocMgrLastFocused = pCandidateListDocumentMgr;
//            if (_pDocMgrLastFocused)
//            {
//                _pDocMgrLastFocused->AddRef();
//            }
        }
        m_candidateListView->OnKillThreadFocus();
        m_singletonProcessor->CandidateListViewInternal_OnKillThreadFocus();
    }

    activity.Stop();
    return S_OK;
}
CATCH_RETURN()

BOOL CWindowsIME::_InitThreadFocusSink()
{
    ITfSource* pSource = nullptr;

    if (FAILED(_pThreadMgr->QueryInterface(IID_ITfSource, (void **)&pSource)))
    {
        return FALSE;
    }

    if (FAILED(pSource->AdviseSink(IID_ITfThreadFocusSink, (ITfThreadFocusSink *)this, &_dwThreadFocusSinkCookie)))
    {
        pSource->Release();
        return FALSE;
    }

    pSource->Release();

    return TRUE;
}

void CWindowsIME::_UninitThreadFocusSink()
{
    ITfSource* pSource = nullptr;

    if (FAILED(_pThreadMgr->QueryInterface(IID_ITfSource, (void **)&pSource)))
    {
        return;
    }

    if (FAILED(pSource->UnadviseSink(_dwThreadFocusSinkCookie)))
    {
        pSource->Release();
        return;
    }

    pSource->Release();
}