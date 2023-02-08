// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "WindowsIME.h"
#include "Globals.h"
#include "TfTextLayoutSink.h"
#include "GetTextExtentEditSession.h"


HRESULT CWindowsIME::_StartLayoutTracking(_In_ ITfContext *pContextDocument, TfEditCookie ec, _In_ ITfRange *pRangeComposition)
{
    LOG_IF_FAILED(_EndLayoutTracking());

    m_textLayoutSink._pContextDocument = pContextDocument;
    m_textLayoutSink._pRangeComposition = pRangeComposition;
    m_textLayoutSink._tfEditCookie = ec;

    wil::com_ptr<ITfSource> source;
    RETURN_IF_FAILED(pContextDocument->QueryInterface(IID_PPV_ARGS(&source)));
    RETURN_IF_FAILED(source->AdviseSink(IID_ITfTextLayoutSink, (ITfTextLayoutSink *)this, &m_textLayoutSink._dwCookieTextLayoutSink));

    // Get current rect
    wil::com_ptr<ITfContextView> pContextView;
    RETURN_IF_FAILED(pContextDocument->GetActiveView(&pContextView));

    RECT rc = {};
    BOOL isClipped = {};
    RETURN_IF_FAILED(pContextView->GetTextExt(ec, pRangeComposition, &rc, &isClipped));

    m_textLayoutSink.compositionRect = rc;
    m_textLayoutSink.isClipped = isClipped;

    HWND parentWndHandle = {};
    if (SUCCEEDED_LOG(pContextView->GetWnd(&parentWndHandle)))
    {
        m_textLayoutSink.documentWindow = parentWndHandle;
    }

    return S_OK;
}

HRESULT CWindowsIME::_EndLayoutTracking()
{
    if (m_textLayoutSink._pContextDocument && m_textLayoutSink._dwCookieTextLayoutSink != 0)
    {
        wil::com_ptr<ITfSource> source;
        RETURN_IF_FAILED(m_textLayoutSink._pContextDocument->QueryInterface(IID_PPV_ARGS(&source)));
        RETURN_IF_FAILED(source->UnadviseSink(m_textLayoutSink._dwCookieTextLayoutSink));
    }

    m_textLayoutSink._pContextDocument.reset();
    m_textLayoutSink._pRangeComposition.reset();
    m_textLayoutSink._tfEditCookie = TF_INVALID_EDIT_COOKIE;;
    m_textLayoutSink._dwCookieTextLayoutSink = 0;
    m_textLayoutSink.compositionRect = {};
    m_textLayoutSink.isClipped = {};
    m_textLayoutSink.documentWindow = {};

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfTextLayoutSink::OnLayoutChange
//
//----------------------------------------------------------------------------

IFACEMETHODIMP CWindowsIME::OnLayoutChange(_In_ ITfContext *pContext, TfLayoutCode lcode, _In_ ITfContextView *pContextView)
{
    // we're interested in only document context.
    if (pContext != m_textLayoutSink._pContextDocument.get())
    {
        return S_OK;
    }

    switch (lcode)
    {
    case TF_LC_CHANGE:
        _SubmitEditSessionTask(pContext, [&](TfEditCookie ec) -> HRESULT {
                RECT rc = {};
                BOOL isClipped = {};
                if (SUCCEEDED_LOG(pContextView->GetTextExt(ec, m_textLayoutSink._pRangeComposition.get(), &rc, &isClipped)))
                {
                    m_textLayoutSink.compositionRect = rc;
                    m_textLayoutSink.isClipped = isClipped;

                    HWND parentWndHandle = {};
                    if (SUCCEEDED_LOG(pContextView->GetWnd(&parentWndHandle)))
                    {
                        m_textLayoutSink.documentWindow = parentWndHandle;
                    }

                    m_candidateListView->_LayoutChangeNotification(parentWndHandle, &rc);
                    m_singletonProcessor->CandidateListViewInternal_LayoutChangeNotification(parentWndHandle, &rc);
                }
                return S_OK;
            }, TF_ES_SYNC | TF_ES_READ);
        break;

    case TF_LC_DESTROY:
        m_candidateListView->_LayoutDestroyNotification();
        m_singletonProcessor->CandidateListViewInternal_LayoutDestroyNotification();
        break;

    }
    return S_OK;
}

HRESULT CWindowsIME::_GetLastTextExt(_Out_ HWND* documentWindow, _Out_ RECT *lpRect)
{
    *documentWindow = m_textLayoutSink.documentWindow;
    *lpRect = m_textLayoutSink.compositionRect;
    return S_OK;

//    if (!m_textLayoutSink._pContextDocument || !m_textLayoutSink._pRangeComposition)
//    {
//        return E_FAIL;
//    }
//
//    BOOL isClipped = TRUE;
//    wil::com_ptr<ITfContextView> pContextView;
//    RETURN_IF_FAILED(m_textLayoutSink._pContextDocument->GetActiveView(&pContextView));
//    RETURN_IF_FAILED(pContextView->GetTextExt(ec, m_textLayoutSink._pRangeComposition.get(), lpRect, &isClipped));
//    return S_OK;
}

