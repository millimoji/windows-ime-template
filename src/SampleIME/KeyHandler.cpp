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

//+---------------------------------------------------------------------------
//
// _DeleteCandidateList
//
//----------------------------------------------------------------------------

VOID CompositionProcessorEngine::_DeleteCandidateList()
{
//    isForce;pContext;

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

HRESULT CKeyStateCategory::_HandleComplete()
{
    _pCompositionProcessorEngine->_DeleteCandidateList();
    _pTextService->_TerminateComposition();
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _HandleCancel
//
//----------------------------------------------------------------------------

HRESULT CKeyStateCategory::_HandleCancel()
{
    _pCompositionProcessorEngine->_DeleteCandidateList();
    _pTextService->_RemoveDummyCompositionForComposing();
    _pTextService->_TerminateComposition();
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _HandleCompositionInput
//
// If the keystroke happens within a composition, eat the key and return S_OK.
//
//----------------------------------------------------------------------------

HRESULT CKeyStateCategory::_HandleCompositionInput(WCHAR wch)
{
    if (_pCandidateListUIPresenter->IsCreated() && (_pCompositionProcessorEngine->CandidateMode() != CANDIDATE_INCREMENTAL))
    {
        _HandleCompositionFinalize(FALSE);
    }

    // Start the new (std::nothrow) compositon if there is no composition.
    if (!_pTextService->_IsComposing())
    {
        RETURN_IF_FAILED(_pTextService->_StartComposition());
    }

// TODO confirm range is correct
//        // first, test where a keystroke would go in the document if we did an insert
//        if (dto.pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &fetched) != S_OK || fetched != 1)
//        {
//            return S_FALSE;
//        }
//
//        // is the insertion point covered by a composition?
//        if (SUCCEEDED(_pTextService->GetComposition()->GetRange(&pRangeComposition)))
//        {
//            isCovered = _IsRangeCovered(ec, tfSelection.range, pRangeComposition);
//
//            pRangeComposition->Release();
//
//            if (!isCovered)
//            {
//                goto Exit;
//            }
//        }

    // Add virtual key to composition processor engine
    _pCompositionProcessorEngine->AddVirtualKey(wch);

    return _HandleCompositionInputWorker();
}

//+---------------------------------------------------------------------------
//
// _HandleCompositionInputWorker
//
// If the keystroke happens within a composition, eat the key and return S_OK.
//
//----------------------------------------------------------------------------

HRESULT CKeyStateCategory::_HandleCompositionInputWorker()
{
    //
    // Get reading string from composition processor engine
    //
    BOOL isWildcardIncluded;
    std::vector<CStringRange> readingStrings;
    _pCompositionProcessorEngine->GetReadingStrings(&readingStrings, &isWildcardIncluded);

    std::wstring insertionText;
    for (auto&& readingString: readingStrings)
    {
        auto readingStringView = std::wstring_view(readingString.Get(), readingString.GetLength());
        insertionText += readingStringView;
    }
    auto insertionTextShared = std::make_shared<const std::wstring>(std::move(insertionText));

    RETURN_IF_FAILED(_pTextService->_AddComposingAndChar(insertionTextShared));

    //
    // Get candidate string from composition processor engine
    //
    std::vector<shared_wstring> candidateList;
    _pCompositionProcessorEngine->GetCandidateList(candidateList, TRUE, FALSE);

    if (candidateList.size() > 0)
    {
        if (SUCCEEDED_LOG(_CreateAndStartCandidate()))
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
        if (SUCCEEDED_LOG(_CreateAndStartCandidate()))
        {
            _pCandidateListUIPresenter->_ClearList();
        }
    }
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _CreateAndStartCandidate
//
//----------------------------------------------------------------------------

HRESULT CKeyStateCategory::_CreateAndStartCandidate()
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
        _pCandidateListUIPresenter->CreateView(_pCompositionProcessorEngine->GetCandidateListIndexRange(), FALSE);

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

        _pCompositionProcessorEngine->SetCandidateMode(CANDIDATE_INCREMENTAL);
        _pCompositionProcessorEngine->SetIsCandidateWithWildcard(false);

        hr = _pCandidateListUIPresenter->_StartCandidateList(WindowsImeLib::g_processorFactory->GetConstantProvider()->GetCandidateWindowWidth());
    }

    return hr;
}

//+---------------------------------------------------------------------------
//
// _HandleCompositionFinalize
//
//----------------------------------------------------------------------------

HRESULT CKeyStateCategory::_HandleCompositionFinalize(BOOL isCandidateList)
{
    if (isCandidateList && _pCandidateListUIPresenter->IsCreated())
    {
        // Finalize selected candidate string from CCandidateListUIPresenter
        auto candidateString = _pCandidateListUIPresenter->_GetSelectedCandidateString();

        // Finalize character
        RETURN_IF_FAILED(_pTextService->_AddCharAndFinalize(candidateString));
    }
    else
    {
        // Finalize current text store strings
        if (_pTextService->_IsComposing())
        {
//TODO?? Confirm range
//                if (FAILED(dto.pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &fetched)) || fetched != 1)
//                ITfRange* pRangeComposition = nullptr;
//                if (SUCCEEDED(_pTextService->GetComposition()->GetRange(&pRangeComposition)))
//                    if (_IsRangeCovered(ec, tfSelection.range, pRangeComposition))

            _pTextService->_TerminateComposition();
        }
    }

    return _HandleCancel();
}

//+---------------------------------------------------------------------------
//
// _HandleCompositionConvert
//
//----------------------------------------------------------------------------

HRESULT CKeyStateCategory::_HandleCompositionConvert(BOOL isWildcardSearch)
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
            _pCandidateListUIPresenter->CreateView(pCompositionProcessorEngine->GetCandidateListIndexRange(), FALSE);
            _pCompositionProcessorEngine->SetCandidateMode(CANDIDATE_ORIGINAL);
        }

        _pCompositionProcessorEngine->SetIsCandidateWithWildcard(isWildcardSearch);

        hr = _pCandidateListUIPresenter->_StartCandidateList(WindowsImeLib::g_processorFactory->GetConstantProvider()->GetCandidateWindowWidth());

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

HRESULT CKeyStateCategory::_HandleCompositionBackspace()
{
	// Start the new (std::nothrow) compositon if there is no composition.
	RETURN_HR_IF(S_OK, !_pTextService->_IsComposing());

// TODO: required? range check
//        if (FAILED(dto.pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &fetched)) || fetched != 1)
//        // is the insertion point covered by a composition?
//        if (SUCCEEDED(_pTextService->GetComposition()->GetRange(&pRangeComposition)))
//            isCovered = _IsRangeCovered(ec, tfSelection.range, pRangeComposition);

    //
    // Add virtual key to composition processor engine
    //
    DWORD_PTR vKeyLen = _pCompositionProcessorEngine->GetVirtualKeyLength();

    if (vKeyLen)
    {
        _pCompositionProcessorEngine->RemoveVirtualKey(vKeyLen - 1);

        if (_pCompositionProcessorEngine->GetVirtualKeyLength() > 0)
        {
            _HandleCompositionInputWorker();
        }
        else
        {
            _HandleCancel();
        }
    }

    return S_OK;
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
            RETURN_IF_FAILED(_pTextService->_AddComposingAndChar(candidateString));
        }
    }

    //
    // Get punctuation char from composition processor engine
    //
    auto pCompositionProcessorEngine = _pCompositionProcessorEngine.get();
    WCHAR punctuation = pCompositionProcessorEngine->GetPunctuation(dto.wch);
    auto punctuationString = std::make_shared<const std::wstring>(&punctuation, 1);

    RETURN_IF_FAILED(_pTextService->_AddCharAndFinalize(punctuationString));

    return _HandleCancel();
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

    RETURN_IF_FAILED(_pTextService->_AddCharAndFinalize(sharedFullWidthString));

    return _HandleCancel();
}
