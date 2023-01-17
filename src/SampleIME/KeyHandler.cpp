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

// //+---------------------------------------------------------------------------
// //
// // _IsRangeCovered
// //
// // Returns TRUE if pRangeTest is entirely contained within pRangeCover.
// //
// //----------------------------------------------------------------------------
// 
// BOOL CKeyStateCategory::_IsRangeCovered(TfEditCookie ec, _In_ ITfRange *pRangeTest, _In_ ITfRange *pRangeCover)
// {
//     LONG lResult = 0;;
// 
//     if (FAILED(pRangeCover->CompareStart(ec, pRangeTest, TF_ANCHOR_START, &lResult)) 
//         || (lResult > 0))
//     {
//         return FALSE;
//     }
// 
//     if (FAILED(pRangeCover->CompareEnd(ec, pRangeTest, TF_ANCHOR_END, &lResult)) 
//         || (lResult < 0))
//     {
//         return FALSE;
//     }
// 
//     return TRUE;
// }
// 
// //+---------------------------------------------------------------------------
// //
// // _DeleteCandidateList
// //
// //----------------------------------------------------------------------------
// 
// VOID CKeyStateCategory::_DeleteCandidateList(BOOL isForce, _In_opt_ ITfContext *pContext)
// {
//     isForce;pContext;
// 
//     _pCompositionProcessorEngine->PurgeVirtualKey();
// 
//     if (_pCandidateListUIPresenter->IsCreated())
//     {
//         _pCandidateListUIPresenter->_EndCandidateList();
// 
//         // _candidateMode = CANDIDATE_NONE;
//         // _isCandidateWithWildcard = FALSE;
// 
//         ResetCandidateState();
//     }
// }

//+---------------------------------------------------------------------------
//
// _HandleComplete
//
//----------------------------------------------------------------------------

HRESULT CKeyStateCategory::_HandleCompleteWorker(TfEditCookie ec, _In_ ITfContext *pContext)
{
    _pTextService->_DeleteCandidateList(FALSE, pContext);

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
    return _pCompositionProcessorEngine->GetOwnerPointer()->_SubmitEditSessionTask(dto.pContext, [this, dto](TfEditCookie ec, WindowsImeLib::IWindowsIMECompositionBuffer*) -> HRESULT
    {
        return _HandleCancelWorker(ec, dto.pContext);
    }, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE);
}

HRESULT CKeyStateCategory::_HandleCancelWorker(TfEditCookie ec, _In_ ITfContext *pContext)
{
    _pTextService->_RemoveDummyCompositionForComposing(ec, _pTextService->GetComposition().get());
    _pTextService->_DeleteCandidateList(FALSE, pContext);
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
    return _pCompositionProcessorEngine->GetOwnerPointer()->_SubmitEditSessionTask(dto.pContext, [this, dto, wch](TfEditCookie ec, WindowsImeLib::IWindowsIMECompositionBuffer*) -> HRESULT
    {
        ITfRange* pRangeComposition = nullptr;
        TF_SELECTION tfSelection;
        ULONG fetched = 0;
        BOOL isCovered = TRUE;

        auto pCompositionProcessorEngine = _pCompositionProcessorEngine.get();

        if (_pCandidateListUIPresenter->IsCreated() && (_pTextService->CandidateMode() != CANDIDATE_INCREMENTAL))
        {
            _HandleCompositionFinalize(dto, FALSE);
        }

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
            isCovered = _pTextService->_IsRangeCovered(ec, tfSelection.range, pRangeComposition);

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
        hr = _pTextService->_AddComposingAndChar(ec, pContext, &readingStrings.at(index));
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
        hr = _CreateAndStartCandidate(ec, pContext);
        if (SUCCEEDED(hr))
        {
            _pCandidateListUIPresenter->_ClearList();
            _pCandidateListUIPresenter->_SetText(&candidateList, TRUE);
        }
    }
    else if (_pCandidateListUIPresenter->IsCreated())
    {
        _pCandidateListUIPresenter->_ClearList();
    }
    else if (readingStrings.size() && isWildcardIncluded)
    {
        hr = _CreateAndStartCandidate(ec, pContext);
        if (SUCCEEDED(hr))
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

HRESULT CKeyStateCategory::_CreateAndStartCandidate(TfEditCookie ec, _In_ ITfContext *pContext)
{
    HRESULT hr = S_OK;

    if (((_pTextService->CandidateMode() == CANDIDATE_PHRASE) && _pCandidateListUIPresenter->IsCreated())
        || ((_pTextService->CandidateMode() == CANDIDATE_NONE) && _pCandidateListUIPresenter->IsCreated()))
    {
        // Recreate candidate list
        _pCandidateListUIPresenter->_EndCandidateList();
        // delete _pCandidateListUIPresenter;
        // _pCandidateListUIPresenter = nullptr;
        // _pCandidateListUIPresenter.reset();
        _pCandidateListUIPresenter->DestroyView();

        _pTextService->ResetCandidateState();
        // _candidateMode = CANDIDATE_NONE;
        // _isCandidateWithWildcard = FALSE;
    }

//    if (_pCandidateListUIPresenter == nullptr)
    if (!_pCandidateListUIPresenter->IsCreated())
    {
        _pCandidateListUIPresenter->CreateView(
            WindowsImeLib::AtomCandidateWindow,
            CATEGORY_CANDIDATE,
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

        _pTextService->SetCandidateMode(CANDIDATE_INCREMENTAL);
        _pTextService->SetIsCandidateWithWildcard(false);

        // we don't cache the document manager object. So get it from pContext.
        ITfDocumentMgr* pDocumentMgr = nullptr;
        if (SUCCEEDED(pContext->GetDocumentMgr(&pDocumentMgr)))
        {
            // get the composition range.
            ITfRange* pRange = nullptr;
            if (SUCCEEDED(_pTextService->GetComposition()->GetRange(&pRange)))
            {
                hr = _pCandidateListUIPresenter->_StartCandidateList(
                        _pTextService->GetClientId(),
                        pDocumentMgr, pContext, ec, pRange,
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

HRESULT CKeyStateCategory::_HandleCompositionFinalize(const KeyHandlerEditSessionDTO& dto, BOOL isCandidateList)
{
    return _pCompositionProcessorEngine->GetOwnerPointer()->_SubmitEditSessionTask(dto.pContext, [this, dto, isCandidateList](TfEditCookie ec, WindowsImeLib::IWindowsIMECompositionBuffer*) -> HRESULT
    {
        HRESULT hr = S_OK;

        if (isCandidateList && _pCandidateListUIPresenter->IsCreated())
        {
            // Finalize selected candidate string from CCandidateListUIPresenter
            DWORD_PTR candidateLen = 0;
            const WCHAR *pCandidateString = nullptr;

            candidateLen = _pCandidateListUIPresenter->_GetSelectedCandidateString(&pCandidateString);

            CStringRange candidateString;
            candidateString.Set(pCandidateString, candidateLen);

            if (candidateLen)
            {
                // Finalize character
                hr = _pTextService->_AddCharAndFinalize(ec, dto.pContext, &candidateString);
                if (FAILED(hr))
                {
                    return hr;
                }
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
                    if (_pTextService->_IsRangeCovered(ec, tfSelection.range, pRangeComposition))
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
    return _pCompositionProcessorEngine->GetOwnerPointer()->_SubmitEditSessionTask(dto.pContext, [this, dto, isWildcardSearch](TfEditCookie ec, WindowsImeLib::IWindowsIMECompositionBuffer*) -> HRESULT
    {
        HRESULT hr = S_OK;

        std::vector<CCandidateListItem> candidateList;

        //
        // Get candidate string from composition processor engine
        //
        auto pCompositionProcessorEngine = _pCompositionProcessorEngine.get();
        pCompositionProcessorEngine->GetCandidateList(&candidateList, FALSE, isWildcardSearch);

        // If there is no candlidate listin the current reading string, we don't do anything. Just wait for
        // next char to be ready for the conversion with it.
        int nCount = static_cast<int>(candidateList.size());
        if (nCount)
        {
    //        if (_pCandidateListUIPresenter)
    //        {
    //            _pCandidateListUIPresenter->_EndCandidateList();
    //            // delete _pCandidateListUIPresenter;
    //            // _pCandidateListUIPresenter = nullptr;
    //            _pCandidateListUIPresenter.reset();
    //
    //            _candidateMode = CANDIDATE_NONE;
    //            _isCandidateWithWildcard = FALSE;
    //        }
            if (_pCandidateListUIPresenter->IsCreated())
            {
                _pCandidateListUIPresenter->_EndCandidateList();
                _pCandidateListUIPresenter->DestroyView();
                _pTextService->ResetCandidateState();
            }

            // 
            // create an instance of the candidate list class.
            // 
    //        if (_pCandidateListUIPresenter == nullptr)
    //        {
    //            _pCandidateListUIPresenter = new (std::nothrow) CCandidateListUIPresenter(
    //                reinterpret_cast<CWindowsIME*>(_textService->GetTextService()),
    //                Global::AtomCandidateWindow,
    //                CATEGORY_CANDIDATE,
    //                pCompositionProcessorEngine->GetCandidateListIndexRange(),
    //                FALSE);
    //            if (!_pCandidateListUIPresenter)
    //            {
    //                return E_OUTOFMEMORY;
    //            }
    //
    //            _candidateMode = CANDIDATE_ORIGINAL;
    //        }
            if (!_pCandidateListUIPresenter->IsCreated())
            {
                _pCandidateListUIPresenter->CreateView(
                                                WindowsImeLib::AtomCandidateWindow,
                                                CATEGORY_CANDIDATE,
                                                pCompositionProcessorEngine->GetCandidateListIndexRange(),
                                                FALSE);
                _pTextService->SetCandidateMode(CANDIDATE_ORIGINAL);
            }

            _pTextService->SetIsCandidateWithWildcard(isWildcardSearch);

            // we don't cache the document manager object. So get it from pContext.
            ITfDocumentMgr* pDocumentMgr = nullptr;
            if (SUCCEEDED(dto.pContext->GetDocumentMgr(&pDocumentMgr)))
            {
                // get the composition range.
                ITfRange* pRange = nullptr;
                if (SUCCEEDED(_pTextService->GetComposition()->GetRange(&pRange)))
                {
                    hr = _pCandidateListUIPresenter->_StartCandidateList(
                            _pTextService->GetClientId(),
                            pDocumentMgr, dto.pContext, ec, pRange,
                            WindowsImeLib::g_processorFactory->GetConstantProvider()->GetCandidateWindowWidth());
                    pRange->Release();
                }
                pDocumentMgr->Release();
            }
            if (SUCCEEDED(hr))
            {
                _pCandidateListUIPresenter->_SetText(&candidateList, FALSE);
            }
        }

        return hr;
    }, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE);
}

//+---------------------------------------------------------------------------
//
// _HandleCompositionBackspace
//
//----------------------------------------------------------------------------

HRESULT CKeyStateCategory::_HandleCompositionBackspace(const KeyHandlerEditSessionDTO& dto)
{
    return _pCompositionProcessorEngine->GetOwnerPointer()->_SubmitEditSessionTask(dto.pContext, [this, dto](TfEditCookie ec, WindowsImeLib::IWindowsIMECompositionBuffer*) -> HRESULT
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
            isCovered = _pTextService->_IsRangeCovered(ec, tfSelection.range, pRangeComposition);

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
    return _pCompositionProcessorEngine->GetOwnerPointer()->_SubmitEditSessionTask(dto.pContext, [this, dto](TfEditCookie ec, WindowsImeLib::IWindowsIMECompositionBuffer*) -> HRESULT
    {
        ITfRange* pRangeComposition = nullptr;
        TF_SELECTION tfSelection;
        ULONG fetched = 0;

        // get the selection
        if (FAILED(dto.pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &fetched))
            || fetched != 1)
        {
            // no selection, eat the keystroke
            return S_OK;
        }

        // get the composition range
        if (FAILED(_pTextService->GetComposition()->GetRange(&pRangeComposition)))
        {
            goto Exit;
        }

        // For incremental candidate list
        if (_pCandidateListUIPresenter->IsCreated())
        {
            _pCandidateListUIPresenter->AdviseUIChangedByArrowKey(dto.arrowKey);
        }

        dto.pContext->SetSelection(ec, 1, &tfSelection);

        pRangeComposition->Release();

Exit:
        tfSelection.range->Release();
        return S_OK;
    }, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE);
}

//+---------------------------------------------------------------------------
//
// _HandleCompositionPunctuation
//
//----------------------------------------------------------------------------

HRESULT CKeyStateCategory::_HandleCompositionPunctuation(const KeyHandlerEditSessionDTO& dto)
{
    return _pCompositionProcessorEngine->GetOwnerPointer()->_SubmitEditSessionTask(dto.pContext, [this, dto](TfEditCookie ec, WindowsImeLib::IWindowsIMECompositionBuffer*) -> HRESULT
    {
        HRESULT hr = S_OK;

        if (_pTextService->CandidateMode() != CANDIDATE_NONE && _pCandidateListUIPresenter->IsCreated())
        {
            DWORD_PTR candidateLen = 0;
            const WCHAR* pCandidateString = nullptr;

            candidateLen = _pCandidateListUIPresenter->_GetSelectedCandidateString(&pCandidateString);

            CStringRange candidateString;
            candidateString.Set(pCandidateString, candidateLen);

            if (candidateLen)
            {
                _pTextService->_AddComposingAndChar(ec, dto.pContext, &candidateString);
            }
        }
        //
        // Get punctuation char from composition processor engine
        //
        auto pCompositionProcessorEngine = _pCompositionProcessorEngine.get();

        WCHAR punctuation = pCompositionProcessorEngine->GetPunctuation(dto.wch);

        CStringRange punctuationString;
        punctuationString.Set(&punctuation, 1);

        // Finalize character
        hr = _pTextService->_AddCharAndFinalize(ec, dto.pContext, &punctuationString);
        if (FAILED(hr))
        {
            return hr;
        }

        _HandleCancelWorker(ec, dto.pContext);

        return S_OK;
    }, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE);
}

//+---------------------------------------------------------------------------
//
// _HandleCompositionDoubleSingleByte
//
//----------------------------------------------------------------------------

HRESULT CKeyStateCategory::_HandleCompositionDoubleSingleByte(const KeyHandlerEditSessionDTO& dto)
{
    return _pCompositionProcessorEngine->GetOwnerPointer()->_SubmitEditSessionTask(dto.pContext, [this, dto](TfEditCookie ec, WindowsImeLib::IWindowsIMECompositionBuffer*) -> HRESULT
    {
        HRESULT hr = S_OK;

        WCHAR fullWidth = Global::FullWidthCharTable[dto.wch - 0x20];

        CStringRange fullWidthString;
        fullWidthString.Set(&fullWidth, 1);

        // Finalize character
        hr = _pTextService->_AddCharAndFinalize(ec, dto.pContext, &fullWidthString);
        if (FAILED(hr))
        {
            return hr;
        }

        _HandleCancelWorker(ec, dto.pContext);

        return S_OK;
    }, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE);
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
