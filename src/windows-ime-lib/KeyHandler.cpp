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
#include "CandidateListUIPresenter.h"

//////////////////////////////////////////////////////////////////////
//
// CWindowsIME class
//
//////////////////////////////////////////////////////////////////////

//+---------------------------------------------------------------------------
//
// _IsRangeCovered
//
// Returns TRUE if pRangeTest is entirely contained within pRangeCover.
//
//----------------------------------------------------------------------------

BOOL CompositionBuffer::_IsRangeCovered(TfEditCookie ec, _In_ ITfRange *pRangeTest, _In_ ITfRange *pRangeCover)
{
    LONG lResult = 0;;

    if (FAILED(pRangeCover->CompareStart(ec, pRangeTest, TF_ANCHOR_START, &lResult)) 
        || (lResult > 0))
    {
        return FALSE;
    }

    if (FAILED(pRangeCover->CompareEnd(ec, pRangeTest, TF_ANCHOR_END, &lResult)) 
        || (lResult < 0))
    {
        return FALSE;
    }

    return TRUE;
}

//+---------------------------------------------------------------------------
//
// _DeleteCandidateList
//
//----------------------------------------------------------------------------

VOID CWindowsIME::_DeleteCandidateList(BOOL isForce, _In_opt_ ITfContext *pContext)
{
    isForce;pContext;

    auto pCompositionProcessorEngine = _pCompositionProcessorEngine.get();
    pCompositionProcessorEngine->PurgeVirtualKey();

    if (_pCandidateListUIPresenter)
    {
        _pCandidateListUIPresenter->_EndCandidateList();

        _candidateMode = CANDIDATE_NONE;
        _isCandidateWithWildcard = FALSE;
    }
}

//+---------------------------------------------------------------------------
//
// _HandleComplete
//
//----------------------------------------------------------------------------

HRESULT CompositionBuffer::_HandleComplete(TfEditCookie ec, _In_ ITfContext *pContext)
{
    m_textService->_DeleteCandidateList(FALSE, pContext);

    // just terminate the composition
    _TerminateComposition(ec, pContext);

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _HandleCancel
//
//----------------------------------------------------------------------------

HRESULT CompositionBuffer::_HandleCancel(TfEditCookie ec, _In_ ITfContext *pContext)
{
    _RemoveDummyCompositionForComposing(ec, m_pComposition);

    m_textService->_DeleteCandidateList(FALSE, pContext);

    _TerminateComposition(ec, pContext);

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _HandleCompositionInput
//
// If the keystroke happens within a composition, eat the key and return S_OK.
//
//----------------------------------------------------------------------------

HRESULT CompositionBuffer::_HandleCompositionInput(TfEditCookie ec, _In_ ITfContext *pContext, WCHAR wch)
{
    ITfRange* pRangeComposition = nullptr;
    TF_SELECTION tfSelection;
    ULONG fetched = 0;
    BOOL isCovered = TRUE;

    auto pCompositionProcessorEngine = m_pCompositionProcessorEngine.get();

    if ((m_pCandidateListUIPresenter != nullptr) && (m_candidateMode != CANDIDATE_INCREMENTAL))
    {
        _HandleCompositionFinalize(ec, pContext, FALSE);
    }

    // Start the new (std::nothrow) compositon if there is no composition.
    if (!m_textService->_IsComposing())
    {
        m_textService->_StartComposition(pContext);
    }

    // first, test where a keystroke would go in the document if we did an insert
    if (pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &fetched) != S_OK || fetched != 1)
    {
        return S_FALSE;
    }

    // is the insertion point covered by a composition?
    if (SUCCEEDED(m_pComposition->GetRange(&pRangeComposition)))
    {
        isCovered = _IsRangeCovered(ec, tfSelection.range, pRangeComposition);

        pRangeComposition->Release();

        if (!isCovered)
        {
            goto Exit;
        }
    }

    // Add virtual key to composition processor engine
    pCompositionProcessorEngine->AddVirtualKey(wch);

    _HandleCompositionInputWorker(pCompositionProcessorEngine, ec, pContext);

Exit:
    tfSelection.range->Release();
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _HandleCompositionInputWorker
//
// If the keystroke happens within a composition, eat the key and return S_OK.
//
//----------------------------------------------------------------------------

HRESULT CompositionBuffer::_HandleCompositionInputWorker(_In_ WindowsImeLib::ICompositionProcessorEngine *pCompositionProcessorEngine, TfEditCookie ec, _In_ ITfContext *pContext)
{
    HRESULT hr = S_OK;
    std::vector<CStringRange> readingStrings;
    BOOL isWildcardIncluded = TRUE;

    //
    // Get reading string from composition processor engine
    //
    pCompositionProcessorEngine->GetReadingStrings(&readingStrings, &isWildcardIncluded);

    for (UINT index = 0; index < readingStrings.size(); index++)
    {
        hr = _AddComposingAndChar(ec, pContext, &readingStrings.at(index));
        if (FAILED(hr))
        {
            return hr;
        }
    }

    //
    // Get candidate string from composition processor engine
    //
    std::vector<CCandidateListItem> candidateList;

    pCompositionProcessorEngine->GetCandidateList(&candidateList, TRUE, FALSE);

    if ((candidateList.size()))
    {
        hr = _CreateAndStartCandidate(pCompositionProcessorEngine, ec, pContext);
        if (SUCCEEDED(hr))
        {
            m_pCandidateListUIPresenter->_ClearList();
            m_pCandidateListUIPresenter->_SetText(&candidateList, TRUE);
        }
    }
    else if (m_pCandidateListUIPresenter)
    {
        m_pCandidateListUIPresenter->_ClearList();
    }
    else if (readingStrings.size() && isWildcardIncluded)
    {
        hr = _CreateAndStartCandidate(pCompositionProcessorEngine, ec, pContext);
        if (SUCCEEDED(hr))
        {
            m_pCandidateListUIPresenter->_ClearList();
        }
    }
    return hr;
}
//+---------------------------------------------------------------------------
//
// _CreateAndStartCandidate
//
//----------------------------------------------------------------------------

HRESULT CompositionBuffer::_CreateAndStartCandidate(_In_ WindowsImeLib::ICompositionProcessorEngine *pCompositionProcessorEngine, TfEditCookie ec, _In_ ITfContext *pContext)
{
    HRESULT hr = S_OK;

    if (((m_candidateMode == CANDIDATE_PHRASE) && (m_pCandidateListUIPresenter))
        || ((m_candidateMode == CANDIDATE_NONE) && (m_pCandidateListUIPresenter)))
    {
        // Recreate candidate list
        m_pCandidateListUIPresenter->_EndCandidateList();
        delete m_pCandidateListUIPresenter;
        m_pCandidateListUIPresenter = nullptr;

        m_candidateMode = CANDIDATE_NONE;
        m_isCandidateWithWildcard = FALSE;
    }

    if (m_pCandidateListUIPresenter == nullptr)
    {
        m_pCandidateListUIPresenter = new (std::nothrow) CCandidateListUIPresenter(
            reinterpret_cast<CWindowsIME*>(m_textService->GetTextService()),
            Global::AtomCandidateWindow,
            CATEGORY_CANDIDATE,
            pCompositionProcessorEngine->GetCandidateListIndexRange(),
            FALSE);
        if (!m_pCandidateListUIPresenter)
        {
            return E_OUTOFMEMORY;
        }

        m_candidateMode = CANDIDATE_INCREMENTAL;
        m_isCandidateWithWildcard = FALSE;

        // we don't cache the document manager object. So get it from pContext.
        ITfDocumentMgr* pDocumentMgr = nullptr;
        if (SUCCEEDED(pContext->GetDocumentMgr(&pDocumentMgr)))
        {
            // get the composition range.
            ITfRange* pRange = nullptr;
            if (SUCCEEDED(m_pComposition->GetRange(&pRange)))
            {
                hr = m_pCandidateListUIPresenter->_StartCandidateList(m_tfClientId, pDocumentMgr, pContext, ec, pRange,
                        WindowsImeLib::g_processorFactory->GetConstantProvider()->GetCandidateWindowWidth());
                pRange->Release();
            }
            pDocumentMgr->Release();
        }
    }

    return hr;
}

//+---------------------------------------------------------------------------
//
// _HandleCompositionFinalize
//
//----------------------------------------------------------------------------

HRESULT CompositionBuffer::_HandleCompositionFinalize(TfEditCookie ec, _In_ ITfContext *pContext, BOOL isCandidateList)
{
    HRESULT hr = S_OK;

    if (isCandidateList && m_pCandidateListUIPresenter)
    {
        // Finalize selected candidate string from CCandidateListUIPresenter
        DWORD_PTR candidateLen = 0;
        const WCHAR *pCandidateString = nullptr;

        candidateLen = m_pCandidateListUIPresenter->_GetSelectedCandidateString(&pCandidateString);

        CStringRange candidateString;
        candidateString.Set(pCandidateString, candidateLen);

        if (candidateLen)
        {
            // Finalize character
            hr = _AddCharAndFinalize(ec, pContext, &candidateString);
            if (FAILED(hr))
            {
                return hr;
            }
        }
    }
    else
    {
        // Finalize current text store strings
        if (m_textService->_IsComposing())
        {
            ULONG fetched = 0;
            TF_SELECTION tfSelection;

            if (FAILED(pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &fetched)) || fetched != 1)
            {
                return S_FALSE;
            }

            ITfRange* pRangeComposition = nullptr;
            if (SUCCEEDED(m_pComposition->GetRange(&pRangeComposition)))
            {
                if (_IsRangeCovered(ec, tfSelection.range, pRangeComposition))
                {
                    m_textService->_EndComposition(pContext);
                }

                pRangeComposition->Release();
            }

            tfSelection.range->Release();
        }
    }

    _HandleCancel(ec, pContext);

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _HandleCompositionConvert
//
//----------------------------------------------------------------------------

HRESULT CompositionBuffer::_HandleCompositionConvert(TfEditCookie ec, _In_ ITfContext *pContext, BOOL isWildcardSearch)
{
    HRESULT hr = S_OK;

    std::vector<CCandidateListItem> candidateList;

    //
    // Get candidate string from composition processor engine
    //
    auto pCompositionProcessorEngine = m_pCompositionProcessorEngine.get();
    pCompositionProcessorEngine->GetCandidateList(&candidateList, FALSE, isWildcardSearch);

    // If there is no candlidate listin the current reading string, we don't do anything. Just wait for
    // next char to be ready for the conversion with it.
    int nCount = static_cast<int>(candidateList.size());
    if (nCount)
    {
        if (m_pCandidateListUIPresenter)
        {
            m_pCandidateListUIPresenter->_EndCandidateList();
            delete m_pCandidateListUIPresenter;
            m_pCandidateListUIPresenter = nullptr;

            m_candidateMode = CANDIDATE_NONE;
            m_isCandidateWithWildcard = FALSE;
        }

        // 
        // create an instance of the candidate list class.
        // 
        if (m_pCandidateListUIPresenter == nullptr)
        {
            m_pCandidateListUIPresenter = new (std::nothrow) CCandidateListUIPresenter(
                reinterpret_cast<CWindowsIME*>(m_textService->GetTextService()),
                Global::AtomCandidateWindow,
                CATEGORY_CANDIDATE,
                pCompositionProcessorEngine->GetCandidateListIndexRange(),
                FALSE);
            if (!m_pCandidateListUIPresenter)
            {
                return E_OUTOFMEMORY;
            }

            m_candidateMode = CANDIDATE_ORIGINAL;
        }

        m_isCandidateWithWildcard = isWildcardSearch;

        // we don't cache the document manager object. So get it from pContext.
        ITfDocumentMgr* pDocumentMgr = nullptr;
        if (SUCCEEDED(pContext->GetDocumentMgr(&pDocumentMgr)))
        {
            // get the composition range.
            ITfRange* pRange = nullptr;
            if (SUCCEEDED(m_pComposition->GetRange(&pRange)))
            {
                hr = m_pCandidateListUIPresenter->_StartCandidateList(m_tfClientId, pDocumentMgr, pContext, ec, pRange,
                        WindowsImeLib::g_processorFactory->GetConstantProvider()->GetCandidateWindowWidth());
                pRange->Release();
            }
            pDocumentMgr->Release();
        }
        if (SUCCEEDED(hr))
        {
            m_pCandidateListUIPresenter->_SetText(&candidateList, FALSE);
        }
    }

    return hr;
}

//+---------------------------------------------------------------------------
//
// _HandleCompositionBackspace
//
//----------------------------------------------------------------------------

HRESULT CompositionBuffer::_HandleCompositionBackspace(TfEditCookie ec, _In_ ITfContext *pContext)
{
    ITfRange* pRangeComposition = nullptr;
    TF_SELECTION tfSelection;
    ULONG fetched = 0;
    BOOL isCovered = TRUE;

    // Start the new (std::nothrow) compositon if there is no composition.
    if (!m_textService->_IsComposing())
    {
        return S_OK;
    }

    // first, test where a keystroke would go in the document if we did an insert
    if (FAILED(pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &fetched)) || fetched != 1)
    {
        return S_FALSE;
    }

    // is the insertion point covered by a composition?
    if (SUCCEEDED(m_pComposition->GetRange(&pRangeComposition)))
    {
        isCovered = _IsRangeCovered(ec, tfSelection.range, pRangeComposition);

        pRangeComposition->Release();

        if (!isCovered)
        {
            goto Exit;
        }
    }

    //
    // Add virtual key to composition processor engine
    //
    {
        auto pCompositionProcessorEngine = m_pCompositionProcessorEngine.get();

        DWORD_PTR vKeyLen = pCompositionProcessorEngine->GetVirtualKeyLength();

        if (vKeyLen)
        {
            pCompositionProcessorEngine->RemoveVirtualKey(vKeyLen - 1);

            if (pCompositionProcessorEngine->GetVirtualKeyLength())
            {
                _HandleCompositionInputWorker(pCompositionProcessorEngine, ec, pContext);
            }
            else
            {
                _HandleCancel(ec, pContext);
            }
        }
    }

Exit:
    tfSelection.range->Release();
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _HandleCompositionArrowKey
//
// Update the selection within a composition.
//
//----------------------------------------------------------------------------

HRESULT CompositionBuffer::_HandleCompositionArrowKey(TfEditCookie ec, _In_ ITfContext *pContext, KEYSTROKE_FUNCTION keyFunction)
{
    ITfRange* pRangeComposition = nullptr;
    TF_SELECTION tfSelection;
    ULONG fetched = 0;

    // get the selection
    if (FAILED(pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &fetched))
        || fetched != 1)
    {
        // no selection, eat the keystroke
        return S_OK;
    }

    // get the composition range
    if (FAILED(m_pComposition->GetRange(&pRangeComposition)))
    {
        goto Exit;
    }

    // For incremental candidate list
    if (m_pCandidateListUIPresenter)
    {
        m_pCandidateListUIPresenter->AdviseUIChangedByArrowKey(keyFunction);
    }

    pContext->SetSelection(ec, 1, &tfSelection);

    pRangeComposition->Release();

Exit:
    tfSelection.range->Release();
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _HandleCompositionPunctuation
//
//----------------------------------------------------------------------------

HRESULT CompositionBuffer::_HandleCompositionPunctuation(TfEditCookie ec, _In_ ITfContext *pContext, WCHAR wch)
{
    HRESULT hr = S_OK;

    if (m_candidateMode != CANDIDATE_NONE && m_pCandidateListUIPresenter)
    {
        DWORD_PTR candidateLen = 0;
        const WCHAR* pCandidateString = nullptr;

        candidateLen = m_pCandidateListUIPresenter->_GetSelectedCandidateString(&pCandidateString);

        CStringRange candidateString;
        candidateString.Set(pCandidateString, candidateLen);

        if (candidateLen)
        {
            _AddComposingAndChar(ec, pContext, &candidateString);
        }
    }
    //
    // Get punctuation char from composition processor engine
    //
    auto pCompositionProcessorEngine = m_pCompositionProcessorEngine.get();

    WCHAR punctuation = pCompositionProcessorEngine->GetPunctuation(wch);

    CStringRange punctuationString;
    punctuationString.Set(&punctuation, 1);

    // Finalize character
    hr = _AddCharAndFinalize(ec, pContext, &punctuationString);
    if (FAILED(hr))
    {
        return hr;
    }

    _HandleCancel(ec, pContext);

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _HandleCompositionDoubleSingleByte
//
//----------------------------------------------------------------------------

HRESULT CompositionBuffer::_HandleCompositionDoubleSingleByte(TfEditCookie ec, _In_ ITfContext *pContext, WCHAR wch)
{
    HRESULT hr = S_OK;

    WCHAR fullWidth = Global::FullWidthCharTable[wch - 0x20];

    CStringRange fullWidthString;
    fullWidthString.Set(&fullWidth, 1);

    // Finalize character
    hr = _AddCharAndFinalize(ec, pContext, &fullWidthString);
    if (FAILED(hr))
    {
        return hr;
    }

    _HandleCancel(ec, pContext);

    return S_OK;
}

// //+---------------------------------------------------------------------------
// //
// // _InvokeKeyHandler
// //
// // This text service is interested in handling keystrokes to demonstrate the
// // use the compositions. Some apps will cancel compositions if they receive
// // keystrokes while a compositions is ongoing.
// //
// // param
// //    [in] uCode - virtual key code of WM_KEYDOWN wParam
// //    [in] dwFlags - WM_KEYDOWN lParam
// //    [in] dwKeyFunction - Function regarding virtual key
// //----------------------------------------------------------------------------
// 
// HRESULT CWindowsIME::_InvokeKeyHandler(_In_ ITfContext *pContext, UINT code, WCHAR wch, DWORD flags, _KEYSTROKE_STATE keyState)
// {
//     flags;
// 
//     CKeyHandlerEditSession* pEditSession = nullptr;
//     HRESULT hr = E_FAIL;
// 
//     // we'll insert a char ourselves in place of this keystroke
//     pEditSession = new (std::nothrow) CKeyHandlerEditSession(this, pContext, code, wch, keyState);
//     if (pEditSession == nullptr)
//     {
//         goto Exit;
//     }
// 
//     //
//     // Call CKeyHandlerEditSession::DoEditSession().
//     //
//     // Do not specify TF_ES_SYNC so edit session is not invoked on WinWord
//     //
//     hr = pContext->RequestEditSession(_tfClientId, pEditSession, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, &hr);
// 
//     pEditSession->Release();
// 
// Exit:
//     return hr;
// }
