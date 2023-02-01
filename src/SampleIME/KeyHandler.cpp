// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "pch.h"
#include "../WindowsImeLib.h"
#include "KeyStateCategory.h"

// #include "Private.h"
// #include "Globals.h"
// #include "EditSession.h"
// #include "WindowsIME.h"
// #include "CandidateListUIPresenter.h"

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

BOOL CKeyStateCategory::_IsRangeCovered(TfEditCookie ec, _In_ ITfRange *pRangeTest, _In_ ITfRange *pRangeCover)
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

VOID CompositionProcessorEngine::_DeleteCandidateList(BOOL isForce, _In_opt_ ITfContext *pContext)
{
    isForce;pContext;

    PurgeVirtualKey();

    if (m_candidateListView->IsCreated())
    {
        m_candidateListView->_EndCandidateList();

        // _candidateMode = CANDIDATE_NONE;
        // _isCandidateWithWildcard = FALSE;

        ResetCandidateState();
    }
}

//+---------------------------------------------------------------------------
//
// _HandleComplete
//
//----------------------------------------------------------------------------

HRESULT CKeyStateCategory::_HandleCompleteWorker(TfEditCookie ec, _In_ ITfContext *pContext)
{
    _pCompositionProcessorEngine->_DeleteCandidateList(FALSE, pContext);

    // just terminate the composition
    _pTextService->_TerminateComposition(ec, pContext);

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _HandleCancel
//
//----------------------------------------------------------------------------

HRESULT CKeyStateCategory::_HandleCancel(const KeyHandlerEditSessionDTO& dto)
{
    return _pTextService->_SubmitEditSessionTask(dto.pContext, [this, dto](TfEditCookie ec) -> HRESULT
    {
        return _HandleCancelWorker(ec, dto.pContext);
    }, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE);
}

HRESULT CKeyStateCategory::_HandleCancelWorker(TfEditCookie ec, _In_ ITfContext *pContext)
{
    _pTextService->_RemoveDummyCompositionForComposing(ec, _pTextService->GetComposition().get());
    _pCompositionProcessorEngine->_DeleteCandidateList(FALSE, pContext);
    _pTextService->_TerminateComposition(ec, pContext);

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _HandleCompositionInput
//
// If the keystroke happens within a composition, eat the key and return S_OK.
//
//----------------------------------------------------------------------------

HRESULT CKeyStateCategory::_HandleCompositionInput(const KeyHandlerEditSessionDTO& dto, WCHAR wch)
{
    if (_pCandidateListUIPresenter->IsCreated() && (_pCompositionProcessorEngine->CandidateMode() != CANDIDATE_INCREMENTAL))
    {
        _HandleCompositionFinalize(dto, FALSE);
    }

    return _pTextService->_SubmitEditSessionTask(dto.pContext, [this, dto, wch](TfEditCookie ec) -> HRESULT
    {
        ITfRange* pRangeComposition = nullptr;
        TF_SELECTION tfSelection;
        ULONG fetched = 0;
        BOOL isCovered = TRUE;

        auto pCompositionProcessorEngine = _pCompositionProcessorEngine.get();

        // Start the new (std::nothrow) compositon if there is no composition.
        if (!_pTextService->_IsComposing())
        {
            _pTextService->_StartComposition(ec, dto.pContext);
        }

        // first, test where a keystroke would go in the document if we did an insert
        if (dto.pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &fetched) != S_OK || fetched != 1)
        {
            return S_FALSE;
        }

        // is the insertion point covered by a composition?
        if (SUCCEEDED(_pTextService->GetComposition()->GetRange(&pRangeComposition)))
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

        _HandleCompositionInputWorker(pCompositionProcessorEngine, ec, dto.pContext);

Exit:
        tfSelection.range->Release();
        return S_OK;
    }, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE);
}

//+---------------------------------------------------------------------------
//
// _HandleCompositionInputWorker
//
// If the keystroke happens within a composition, eat the key and return S_OK.
//
//----------------------------------------------------------------------------

HRESULT CKeyStateCategory::_HandleCompositionInputWorker(_In_ CompositionProcessorEngine *pCompositionProcessorEngine, TfEditCookie ec, _In_ ITfContext *pContext)
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
        auto readingSubString = readingStrings.at(index).ToSharedWstring();

        hr = _pTextService->_AddComposingAndChar(ec, pContext, readingSubString);
        if (FAILED(hr))
        {
            return hr;
        }
    }

    //
    // Get candidate string from composition processor engine
    //
    std::vector<shared_wstring> candidateList;

    pCompositionProcessorEngine->GetCandidateList(candidateList, TRUE, FALSE);

    if (candidateList.size() > 0)
    {
        hr = _CreateAndStartCandidate(pContext);
        if (SUCCEEDED(hr))
        {
            _pCandidateListUIPresenter->_ClearList();
            _pCandidateListUIPresenter->_SetText(candidateList);
        }
    }
    else if (_pCandidateListUIPresenter->IsCreated())
    {
        _pCandidateListUIPresenter->_ClearList();
    }
    else if (readingStrings.size() && isWildcardIncluded)
    {
        hr = _CreateAndStartCandidate(pContext);
        if (SUCCEEDED_LOG(hr))
        {
            _pCandidateListUIPresenter->_ClearList();
        }
    }
    return hr;
}
//+---------------------------------------------------------------------------
//
// _CreateAndStartCandidate
//
//----------------------------------------------------------------------------

HRESULT CKeyStateCategory::_CreateAndStartCandidate(_In_ ITfContext *pContext)
{
    HRESULT hr = S_OK;

    if (((_pCompositionProcessorEngine->CandidateMode() == CANDIDATE_PHRASE) && _pCandidateListUIPresenter->IsCreated())
        || ((_pCompositionProcessorEngine->CandidateMode() == CANDIDATE_NONE) && _pCandidateListUIPresenter->IsCreated()))
    {
        // Recreate candidate list
        _pCandidateListUIPresenter->_EndCandidateList();
        // delete _pCandidateListUIPresenter;
        // _pCandidateListUIPresenter = nullptr;
        // _pCandidateListUIPresenter.reset();
        _pCandidateListUIPresenter->DestroyView();

        _pCompositionProcessorEngine->ResetCandidateState();
        // _candidateMode = CANDIDATE_NONE;
        // _isCandidateWithWildcard = FALSE;
    }

//    if (_pCandidateListUIPresenter == nullptr)
    if (!_pCandidateListUIPresenter->IsCreated())
    {
        _pCandidateListUIPresenter->CreateView(
            _pCompositionProcessorEngine->GetCandidateListIndexRange(),
            FALSE);

//        _pCandidateListUIPresenter = new (std::nothrow) CCandidateListUIPresenter(
//            reinterpret_cast<CWindowsIME*>(_textService->GetTextService()),
//            Global::AtomCandidateWindow,
//            CATEGORY_CANDIDATE,
//            pCompositionProcessorEngine->GetCandidateListIndexRange(),
//            FALSE);
//        if (!_pCandidateListUIPresenter)
//        {
//            return E_OUTOFMEMORY;
//        }

        _pCompositionProcessorEngine->SetCandidateKeyStrokeCategory(CATEGORY_CANDIDATE);
        _pCompositionProcessorEngine->SetCandidateMode(CANDIDATE_INCREMENTAL);
        _pCompositionProcessorEngine->SetIsCandidateWithWildcard(false);

        // we don't cache the document manager object. So get it from pContext.
        ITfDocumentMgr* pDocumentMgr = nullptr;
        if (SUCCEEDED(pContext->GetDocumentMgr(&pDocumentMgr)))
        {
            // get the composition range.
            ITfRange* pRange = nullptr;
            if (SUCCEEDED(_pTextService->GetComposition()->GetRange(&pRange)))
            {
                hr = _pCandidateListUIPresenter->_StartCandidateList(WindowsImeLib::g_processorFactory->GetConstantProvider()->GetCandidateWindowWidth());
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

HRESULT CKeyStateCategory::_HandleCompositionFinalize(const KeyHandlerEditSessionDTO& dto, BOOL isCandidateList)
{
    return _pTextService->_SubmitEditSessionTask(dto.pContext, [this, dto, isCandidateList](TfEditCookie ec) -> HRESULT
    {
        if (isCandidateList && _pCandidateListUIPresenter->IsCreated())
        {
            // Finalize selected candidate string from CCandidateListUIPresenter
            auto candidateString = _pCandidateListUIPresenter->_GetSelectedCandidateString();

            if (candidateString->length() > 0)
            {
                // Finalize character
                RETURN_IF_FAILED(_pTextService->_AddCharAndFinalize(ec, dto.pContext, candidateString));
            }
        }
        else
        {
            // Finalize current text store strings
            if (_pTextService->_IsComposing())
            {
                ULONG fetched = 0;
                TF_SELECTION tfSelection;

                if (FAILED(dto.pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &fetched)) || fetched != 1)
                {
                    return S_FALSE;
                }

                ITfRange* pRangeComposition = nullptr;
                if (SUCCEEDED(_pTextService->GetComposition()->GetRange(&pRangeComposition)))
                {
                    if (_IsRangeCovered(ec, tfSelection.range, pRangeComposition))
                    {
                        _pTextService->_TerminateComposition(ec, dto.pContext, FALSE);
                        // _textService->_EndComposition(pContext);
                    }

                    pRangeComposition->Release();
                }

                tfSelection.range->Release();
            }
        }

        _HandleCancelWorker(ec, dto.pContext);

        return S_OK;
    }, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE);
}

//+---------------------------------------------------------------------------
//
// _HandleCompositionConvert
//
//----------------------------------------------------------------------------

HRESULT CKeyStateCategory::_HandleCompositionConvert(const KeyHandlerEditSessionDTO& dto, BOOL isWildcardSearch)
{
    HRESULT hr = S_OK;

    std::vector<shared_wstring> candidateList;

    //
    // Get candidate string from composition processor engine
    //
    auto pCompositionProcessorEngine = _pCompositionProcessorEngine.get();
    pCompositionProcessorEngine->GetCandidateList(candidateList, FALSE, isWildcardSearch);

    // If there is no candlidate listin the current reading string, we don't do anything. Just wait for
    // next char to be ready for the conversion with it.
    int nCount = static_cast<int>(candidateList.size());
    if (nCount)
    {
        if (_pCandidateListUIPresenter->IsCreated())
        {
            _pCandidateListUIPresenter->_EndCandidateList();
            _pCandidateListUIPresenter->DestroyView();
            _pCompositionProcessorEngine->ResetCandidateState();
        }

        // 
        // create an instance of the candidate list class.
        // 
        if (!_pCandidateListUIPresenter->IsCreated())
        {
            _pCandidateListUIPresenter->CreateView(
                                            pCompositionProcessorEngine->GetCandidateListIndexRange(),
                                            FALSE);
            _pCompositionProcessorEngine->SetCandidateKeyStrokeCategory(CATEGORY_CANDIDATE);
            _pCompositionProcessorEngine->SetCandidateMode(CANDIDATE_ORIGINAL);
        }

        _pCompositionProcessorEngine->SetIsCandidateWithWildcard(isWildcardSearch);

        // we don't cache the document manager object. So get it from pContext.
        ITfDocumentMgr* pDocumentMgr = nullptr;
        if (SUCCEEDED(dto.pContext->GetDocumentMgr(&pDocumentMgr)))
        {
            // get the composition range.
            ITfRange* pRange = nullptr;
            if (SUCCEEDED(_pTextService->GetComposition()->GetRange(&pRange)))
            {
                hr = _pCandidateListUIPresenter->_StartCandidateList(WindowsImeLib::g_processorFactory->GetConstantProvider()->GetCandidateWindowWidth());
                pRange->Release();
            }
            pDocumentMgr->Release();
        }
        if (SUCCEEDED(hr))
        {
            _pCandidateListUIPresenter->_SetText(candidateList);
        }
    }
    return hr;
}

//+---------------------------------------------------------------------------
//
// _HandleCompositionBackspace
//
//----------------------------------------------------------------------------

HRESULT CKeyStateCategory::_HandleCompositionBackspace(const KeyHandlerEditSessionDTO& dto)
{
    return _pTextService->_SubmitEditSessionTask(dto.pContext, [this, dto](TfEditCookie ec) -> HRESULT
    {
        ITfRange* pRangeComposition = nullptr;
        TF_SELECTION tfSelection;
        ULONG fetched = 0;
        BOOL isCovered = TRUE;

        // Start the new (std::nothrow) compositon if there is no composition.
        if (!_pTextService->_IsComposing())
        {
            return S_OK;
        }

        // first, test where a keystroke would go in the document if we did an insert
        if (FAILED(dto.pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &fetched)) || fetched != 1)
        {
            return S_FALSE;
        }

        // is the insertion point covered by a composition?
        if (SUCCEEDED(_pTextService->GetComposition()->GetRange(&pRangeComposition)))
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
            auto pCompositionProcessorEngine = _pCompositionProcessorEngine.get();

            DWORD_PTR vKeyLen = pCompositionProcessorEngine->GetVirtualKeyLength();

            if (vKeyLen)
            {
                pCompositionProcessorEngine->RemoveVirtualKey(vKeyLen - 1);

                if (pCompositionProcessorEngine->GetVirtualKeyLength() > 0)
                {
                    _HandleCompositionInputWorker(pCompositionProcessorEngine, ec, dto.pContext);
                }
                else
                {
                    _HandleCancelWorker(ec, dto.pContext);
                }
            }
        }

Exit:
        tfSelection.range->Release();
        return S_OK;
    }, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE);
}

//+---------------------------------------------------------------------------
//
// _HandleCompositionArrowKey
//
// Update the selection within a composition.
//
//----------------------------------------------------------------------------

HRESULT CKeyStateCategory::_HandleCompositionArrowKey(const KeyHandlerEditSessionDTO& dto)
{
    // For incremental candidate list
    RETURN_HR_IF(S_OK, !_pCandidateListUIPresenter->IsCreated());

    const auto candidateListFuntion = KeyStrokeFunctionToCandidateListFunction(dto.arrowKey);
    RETURN_HR_IF(S_OK, candidateListFuntion != WindowsImeLib::CANDIDATELIST_FUNCTION::NONE);

    _pCandidateListUIPresenter->AdviseUIChangedByArrowKey(candidateListFuntion);
    return S_OK;
}

//    return _pTextService->_SubmitEditSessionTask(dto.pContext, [this, dto](TfEditCookie ec) -> HRESULT
//    {
//        ITfRange* pRangeComposition = nullptr;
//        TF_SELECTION tfSelection;
//        ULONG fetched = 0;
//
//        // get the selection
//        if (FAILED(dto.pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &fetched))
//            || fetched != 1)
//        {
//            // no selection, eat the keystroke
//            return S_OK;
//        }
//
//        // get the composition range
//        if (FAILED(_pTextService->GetComposition()->GetRange(&pRangeComposition)))
//        {
//            goto Exit;
//        }
//
//        // For incremental candidate list
//        if (_pCandidateListUIPresenter->IsCreated())
//        {
//            const auto candidateListFuntion = KeyStrokeFunctionToCandidateListFunction(dto.arrowKey);
//            if (candidateListFuntion != CANDIDATELIST_FUNCTION_NONE)
//            {
//                _pCandidateListUIPresenter->AdviseUIChangedByArrowKey(candidateListFuntion);
//            }
//        }
//
//        dto.pContext->SetSelection(ec, 1, &tfSelection);
//
//        pRangeComposition->Release();
//
//Exit:
//        tfSelection.range->Release();
//        return S_OK;
//    }, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE);
//}

//+---------------------------------------------------------------------------
//
// _HandleCompositionPunctuation
//
//----------------------------------------------------------------------------

HRESULT CKeyStateCategory::_HandleCompositionPunctuation(const KeyHandlerEditSessionDTO& dto)
{
    if (_pCompositionProcessorEngine->CandidateMode() != CANDIDATE_NONE && _pCandidateListUIPresenter->IsCreated())
    {
        auto candidateString = _pCandidateListUIPresenter->_GetSelectedCandidateString();
        if (candidateString->length() > 0)
        {
            _pTextService->_SubmitEditSessionTask(dto.pContext, [this, dto, candidateString](TfEditCookie ec) -> HRESULT
            {
                return _pTextService->_AddComposingAndChar(ec, dto.pContext, candidateString);
            },
            TF_ES_ASYNCDONTCARE | TF_ES_READWRITE);
        }
    }

    //
    // Get punctuation char from composition processor engine
    //
    auto pCompositionProcessorEngine = _pCompositionProcessorEngine.get();
    WCHAR punctuation = pCompositionProcessorEngine->GetPunctuation(dto.wch);
    auto punctuationString = std::make_shared<const std::wstring>(&punctuation, 1);

    _pTextService->_SubmitEditSessionTask(dto.pContext, [this, dto, punctuationString](TfEditCookie ec) -> HRESULT
    {
        return _pTextService->_AddCharAndFinalize(ec, dto.pContext, punctuationString);
    },
    TF_ES_ASYNCDONTCARE | TF_ES_READWRITE);

    return _HandleCancel(dto);

//    return _pTextService->_SubmitEditSessionTask(dto.pContext, [this, dto](TfEditCookie ec) -> HRESULT
//    {
//        if (_pCompositionProcessorEngine->CandidateMode() != CANDIDATE_NONE && _pCandidateListUIPresenter->IsCreated())
//        {
//            auto candidateString = _pCandidateListUIPresenter->_GetSelectedCandidateString();
//
//            if (candidateString->length() > 0)
//            {
//                _pTextService->_AddComposingAndChar(ec, dto.pContext, candidateString);
//            }
//        }
//        //
//        // Get punctuation char from composition processor engine
//        //
//        auto pCompositionProcessorEngine = _pCompositionProcessorEngine.get();
//
//        WCHAR punctuation = pCompositionProcessorEngine->GetPunctuation(dto.wch);
//
//        auto punctuationString = std::make_shared<const std::wstring>(&punctuation, 1);
//
//        // Finalize character
//        RETURN_IF_FAILED(_pTextService->_AddCharAndFinalize(ec, dto.pContext, punctuationString));
//
//        _HandleCancelWorker(ec, dto.pContext);
//
//        return S_OK;
//    }, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE);
}

//+---------------------------------------------------------------------------
//
// _HandleCompositionDoubleSingleByte
//
//----------------------------------------------------------------------------

HRESULT CKeyStateCategory::_HandleCompositionDoubleSingleByte(const KeyHandlerEditSessionDTO& dto)
{
    WCHAR fullWidth = Global::FullWidthCharTable[dto.wch - 0x20];
    CStringRange fullWidthString;
    fullWidthString.Set(&fullWidth, 1);
    const auto sharedFullWidthString = fullWidthString.ToSharedWstring();

    return _pTextService->_SubmitEditSessionTask(dto.pContext, [this, dto, sharedFullWidthString](TfEditCookie ec) -> HRESULT
    {
        // Finalize character
        return _pTextService->_AddCharAndFinalize(ec, dto.pContext, sharedFullWidthString);
    },
    TF_ES_ASYNCDONTCARE | TF_ES_READWRITE);

    return _HandleCancel(dto);
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
