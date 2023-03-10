// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "Globals.h"
#include "WindowsIME.h"
#include "CandidateListUIPresenter.h"

//+---------------------------------------------------------------------------
//
// ITfThreadMgrEventSink::OnInitDocumentMgr
//
// Sink called by the framework just before the first context is pushed onto
// a document.
//----------------------------------------------------------------------------

STDAPI CWindowsIME::OnInitDocumentMgr(_In_ ITfDocumentMgr*)
{
    auto activity = WindowsImeLibTelemetry::ITfThreadMgrEventSink_OnInitDocumentMgr();
    activity.Stop();
    return E_NOTIMPL;
}

//+---------------------------------------------------------------------------
//
// ITfThreadMgrEventSink::OnUninitDocumentMgr
//
// Sink called by the framework just after the last context is popped off a
// document.
//----------------------------------------------------------------------------

STDAPI CWindowsIME::OnUninitDocumentMgr(_In_ ITfDocumentMgr*)
{
    auto activity = WindowsImeLibTelemetry::ITfThreadMgrEventSink_OnUninitDocumentMgr();
    activity.Stop();
    return E_NOTIMPL;
}

//+---------------------------------------------------------------------------
//
// ITfThreadMgrEventSink::OnSetFocus
//
// Sink called by the framework when focus changes from one document to
// another.  Either document may be NULL, meaning previously there was no
// focus document, or now no document holds the input focus.
//----------------------------------------------------------------------------

STDAPI CWindowsIME::OnSetFocus(_In_ ITfDocumentMgr *pDocMgrFocus, _In_ ITfDocumentMgr*)
{
    auto activity = WindowsImeLibTelemetry::ITfThreadMgrEventSink_OnSetFocus();

    _InitTextEditSink(pDocMgrFocus);

    _UpdateLanguageBarOnSetFocus(pDocMgrFocus);

    //
    // We have to hide/unhide candidate list depending on whether they are 
    // associated with pDocMgrFocus.
    //
    // if (m_candidateListView->IsCreated())
    {
        wil::com_ptr<ITfDocumentMgr> pCandidateListDocumentMgr;
        auto pTfContext = m_textLayoutSink._pContextDocument.get();

        if (pTfContext && SUCCEEDED_LOG(pTfContext->GetDocumentMgr(&pCandidateListDocumentMgr)))
        {
            if (pCandidateListDocumentMgr.get() != pDocMgrFocus)
            {
                // m_candidateListView->OnKillThreadFocus();
                m_singletonProcessor->CandidateListViewInternal_OnKillThreadFocus();
            }
            else
            {
                // m_candidateListView->OnSetThreadFocus();
                m_singletonProcessor->CandidateListViewInternal_OnSetThreadFocus();
            }
        }
    }

    _pDocMgrLastFocused = pDocMgrFocus;

    activity.Stop();
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfThreadMgrEventSink::OnPushContext
//
// Sink called by the framework when a context is pushed.
//----------------------------------------------------------------------------

STDAPI CWindowsIME::OnPushContext(_In_ ITfContext*)
{
    auto activity = WindowsImeLibTelemetry::ITfThreadMgrEventSink_OnPushContext();
    activity.Stop();
    return E_NOTIMPL;
}

//+---------------------------------------------------------------------------
//
// ITfThreadMgrEventSink::OnPopContext
//
// Sink called by the framework when a context is popped.
//----------------------------------------------------------------------------

STDAPI CWindowsIME::OnPopContext(_In_ ITfContext*)
{
    auto activity = WindowsImeLibTelemetry::ITfThreadMgrEventSink_OnPopContext();
    activity.Stop();
    return E_NOTIMPL;
}

//+---------------------------------------------------------------------------
//
// _InitThreadMgrEventSink
//
// Advise our sink.
//----------------------------------------------------------------------------

BOOL CWindowsIME::_InitThreadMgrEventSink()
{
    ITfSource* pSource = nullptr;
    BOOL ret = FALSE;

    if (FAILED(_pThreadMgr->QueryInterface(IID_ITfSource, (void **)&pSource)))
    {
        return ret;
    }

    if (FAILED(pSource->AdviseSink(IID_ITfThreadMgrEventSink, (ITfThreadMgrEventSink *)this, &_threadMgrEventSinkCookie)))
    {
        _threadMgrEventSinkCookie = TF_INVALID_COOKIE;
        goto Exit;
    }

    ret = TRUE;

Exit:
    pSource->Release();
    return ret;
}

//+---------------------------------------------------------------------------
//
// _UninitThreadMgrEventSink
//
// Unadvise our sink.
//----------------------------------------------------------------------------

void CWindowsIME::_UninitThreadMgrEventSink()
{
    ITfSource* pSource = nullptr;

    if (_threadMgrEventSinkCookie == TF_INVALID_COOKIE)
    {
        return; 
    }

    if (SUCCEEDED(_pThreadMgr->QueryInterface(IID_ITfSource, (void **)&pSource)))
    {
        pSource->UnadviseSink(_threadMgrEventSinkCookie);
        pSource->Release();
    }

    _threadMgrEventSinkCookie = TF_INVALID_COOKIE;
}


//+---------------------------------------------------------------------------
//
// CWindowsIME::_UpdateLanguageBarOnSetFocus
//
//----------------------------------------------------------------------------

void CWindowsIME::_UpdateLanguageBarOnSetFocus(_In_ ITfDocumentMgr *pDocMgrFocus)
{
    BOOL needDisableButtons = FALSE;

    if (!pDocMgrFocus) 
    {
        needDisableButtons = TRUE;
    } 
    else
    {
        wil::com_ptr<IEnumTfContexts> pEnumContext;
        if (FAILED(pDocMgrFocus->EnumContexts(&pEnumContext)) || !pEnumContext) 
        {
            needDisableButtons = TRUE;
        } 
        else 
        {
            ULONG fetched = 0;
            wil::com_ptr<ITfContext> pContext;

            if (FAILED(pEnumContext->Next(1, &pContext, &fetched)) || fetched != 1) 
            {
                needDisableButtons = TRUE;
            }

            if (!pContext) 
            {
                // context is not associated
                needDisableButtons = TRUE;
            }
        }
    }

    if (m_inprocClient)
    {
        m_inprocClient->SetLanguageBarStatus(TF_LBI_STATUS_DISABLED, needDisableButtons);
    }
}
