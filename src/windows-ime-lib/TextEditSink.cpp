// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "Globals.h"
#include "WindowsIME.h"

BOOL _IsRangeCovered(TfEditCookie ec, _In_ ITfRange *pRangeTest, _In_ ITfRange *pRangeCover)
{
    LONG lResult = 0;
    if (FAILED(pRangeCover->CompareStart(ec, pRangeTest, TF_ANCHOR_START, &lResult)) || (lResult > 0)) {
        return FALSE;
    }
    if (FAILED(pRangeCover->CompareEnd(ec, pRangeTest, TF_ANCHOR_END, &lResult)) || (lResult < 0)) {
        return FALSE;
    }
    return TRUE;
}

//+---------------------------------------------------------------------------
//
// ITfTextEditSink::OnEndEdit
//
// Called by the system whenever anyone releases a write-access document lock.
//----------------------------------------------------------------------------

STDAPI CWindowsIME::OnEndEdit(__RPC__in_opt ITfContext *pContext, TfEditCookie ecReadOnly, __RPC__in_opt ITfEditRecord *pEditRecord)
{
    auto activity = WindowsImeLibTelemetry::ITfTextEditSink_OnEndEdit();

    BOOL isSelectionChanged;

    //
    // did the selection change?
    // The selection change includes the movement of caret as well. 
    // The caret position is represent as the empty selection range when
    // there is no selection.
    //
    if (pEditRecord == nullptr)
    {
        return E_INVALIDARG;
    }
    if (SUCCEEDED(pEditRecord->GetSelectionStatus(&isSelectionChanged)) &&
        isSelectionChanged)
    {
        // If the selection is moved to out side of the current composition,
        // we terminate the composition. This TextService supports only one
        // composition in one context object.
        if (m_compositionBuffer->_IsComposing())
        {
            TF_SELECTION tfSelection;
            ULONG fetched = 0;

            if (pContext == nullptr)
            {
                return E_INVALIDARG;
            }
            if (FAILED(pContext->GetSelection(ecReadOnly, TF_DEFAULT_SELECTION, 1, &tfSelection, &fetched)) || fetched != 1)
            {
                return S_FALSE;
            }

            auto _pComposition = m_compositionBuffer->GetComposition();
            ITfRange* pRangeComposition = nullptr;
            if (SUCCEEDED(_pComposition->GetRange(&pRangeComposition)))
            {
                if (!_IsRangeCovered(ecReadOnly, tfSelection.range, pRangeComposition))
                {
                    LOG_IF_FAILED(m_compositionBuffer->_TerminateCompositionInternal());
                }

                pRangeComposition->Release();
            }

            tfSelection.range->Release();
        }
    }

    activity.Stop();
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _InitTextEditSink
//
// Init a text edit sink on the topmost context of the document.
// Always release any previous sink.
//----------------------------------------------------------------------------

BOOL CWindowsIME::_InitTextEditSink(_In_ ITfDocumentMgr *pDocMgr)
{
    ITfSource* pSource = nullptr;
    BOOL ret = TRUE;

    // clear out any previous sink first
    if (_textEditSinkCookie != TF_INVALID_COOKIE)
    {
        if (SUCCEEDED(_pTextEditSinkContext->QueryInterface(IID_ITfSource, (void **)&pSource)))
        {
            pSource->UnadviseSink(_textEditSinkCookie);
            pSource->Release();
        }

//        _pTextEditSinkContext->Release();
//        _pTextEditSinkContext = nullptr;
        _pTextEditSinkContext.reset();
        _textEditSinkCookie = TF_INVALID_COOKIE;
    }

    if (pDocMgr == nullptr)
    {
        return TRUE; // caller just wanted to clear the previous sink
    }

    if (FAILED(pDocMgr->GetTop(&_pTextEditSinkContext)))
    {
        return FALSE;
    }

    if (!_pTextEditSinkContext)
    {
        return TRUE; // empty document, no sink possible
    }

    ret = FALSE;
    if (SUCCEEDED(_pTextEditSinkContext->QueryInterface(IID_ITfSource, (void **)&pSource)))
    {
        if (SUCCEEDED(pSource->AdviseSink(IID_ITfTextEditSink, (ITfTextEditSink *)this, &_textEditSinkCookie)))
        {
            ret = TRUE;
        }
        else
        {
            _textEditSinkCookie = TF_INVALID_COOKIE;
        }
        pSource->Release();
    }

    if (ret == FALSE)
    {
//        _pTextEditSinkContext->Release();
//        _pTextEditSinkContext = nullptr;
        _pTextEditSinkContext.reset();
    }

    return ret;
}
