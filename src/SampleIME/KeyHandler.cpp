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
// CancelCompositioon
//
//----------------------------------------------------------------------------

VOID CompositionProcessorEngine::CancelCompositioon()
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
    _pCompositionProcessorEngine->CancelCompositioon();
    LOG_IF_FAILED(m_compositionBuffer->_TerminateComposition()); // ????
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _HandleCancel
//
//----------------------------------------------------------------------------

HRESULT CKeyStateCategory::_HandleCancel()
{
    _pCompositionProcessorEngine->CancelCompositioon();
    LOG_IF_FAILED(m_compositionBuffer->_RemoveDummyCompositionForComposing());
    LOG_IF_FAILED(m_compositionBuffer->_TerminateComposition());
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
    if (m_candidateListView->IsCreated() && (_pCompositionProcessorEngine->CandidateMode() != CANDIDATE_INCREMENTAL))
    {
        _HandleCompositionFinalize(FALSE);
    }

    // Start the new (std::nothrow) compositon if there is no composition.
    if (!m_compositionBuffer->_IsComposing())
    {
        RETURN_IF_FAILED(m_compositionBuffer->_StartComposition());
    }

// TODO confirm range is correct
//        // first, test where a keystroke would go in the document if we did an insert
//        if (dto.pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &fetched) != S_OK || fetched != 1)
//        {
//            return S_FALSE;
//        }
//
//        // is the insertion point covered by a composition?
//        if (SUCCEEDED(m_compositionBuffer->GetComposition()->GetRange(&pRangeComposition)))
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

    RETURN_IF_FAILED(m_compositionBuffer->_AddComposingAndChar(insertionTextShared));

    //
    // Get candidate string from composition processor engine
    //
    std::vector<shared_wstring> candidateList;
    _pCompositionProcessorEngine->GetCandidateList(candidateList, TRUE, FALSE);

    if (candidateList.size() > 0)
    {
        if (SUCCEEDED_LOG(_CreateAndStartCandidate()))
        {
            m_candidateListView->_SetText(candidateList);
        }
    }
    else if (m_candidateListView->IsCreated())
    {
        m_candidateListView->_SetText(std::vector<shared_wstring>());
    }
    else if (readingStrings.size() && isWildcardIncluded)
    {
        if (SUCCEEDED_LOG(_CreateAndStartCandidate()))
        {
            m_candidateListView->_SetText(std::vector<shared_wstring>());
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

    if (((_pCompositionProcessorEngine->CandidateMode() == CANDIDATE_PHRASE) && m_candidateListView->IsCreated())
        || ((_pCompositionProcessorEngine->CandidateMode() == CANDIDATE_NONE) && m_candidateListView->IsCreated()))
    {
        // Recreate candidate list
        m_candidateListView->_EndCandidateList();
        _pCompositionProcessorEngine->ResetCandidateState();
        // _candidateMode = CANDIDATE_NONE;
        // _isCandidateWithWildcard = FALSE;
    }

//    if (m_candidateListView == nullptr)
    if (!m_candidateListView->IsCreated())
    {
        LOG_IF_FAILED(m_candidateListView->_StartCandidateList(
                WindowsImeLib::g_processorFactory->GetConstantProvider()->GetCandidateWindowWidth()));

        _pCompositionProcessorEngine->SetCandidateMode(CANDIDATE_INCREMENTAL);
        _pCompositionProcessorEngine->SetIsCandidateWithWildcard(false);

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
    if (isCandidateList && m_candidateListView->IsCreated())
    {
        // Finalize selected candidate string from CCandidateListUIPresenter
        auto candidateString = m_candidateListView->_GetSelectedCandidateString();

        // Finalize character
        RETURN_IF_FAILED(m_compositionBuffer->_AddCharAndFinalize(candidateString));
    }
    else
    {
        // Finalize current text store strings
        if (m_compositionBuffer->_IsComposing())
        {
//TODO?? Confirm range
//                if (FAILED(dto.pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &fetched)) || fetched != 1)
//                ITfRange* pRangeComposition = nullptr;
//                if (SUCCEEDED(m_compositionBuffer->GetComposition()->GetRange(&pRangeComposition)))
//                    if (_IsRangeCovered(ec, tfSelection.range, pRangeComposition))

            LOG_IF_FAILED(m_compositionBuffer->_TerminateComposition());
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
        if (m_candidateListView->IsCreated())
        {
            m_candidateListView->_EndCandidateList();
            _pCompositionProcessorEngine->ResetCandidateState();
        }

        // 
        // create an instance of the candidate list class.
        // 
        LOG_IF_FAILED(m_candidateListView->_StartCandidateList(
                WindowsImeLib::g_processorFactory->GetConstantProvider()->GetCandidateWindowWidth()));

        _pCompositionProcessorEngine->SetIsCandidateWithWildcard(isWildcardSearch);
        _pCompositionProcessorEngine->SetCandidateMode(CANDIDATE_ORIGINAL);

        if (SUCCEEDED(hr))
        {
            m_candidateListView->_SetText(candidateList);
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
	RETURN_HR_IF(S_OK, !m_compositionBuffer->_IsComposing());

// TODO: required? range check
//        if (FAILED(dto.pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &fetched)) || fetched != 1)
//        // is the insertion point covered by a composition?
//        if (SUCCEEDED(m_compositionBuffer->GetComposition()->GetRange(&pRangeComposition)))
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
    RETURN_HR_IF(S_OK, !m_candidateListView->IsCreated());

    const auto candidateListFuntion = KeyStrokeFunctionToCandidateListFunction(dto.arrowKey);
    RETURN_HR_IF(S_OK, candidateListFuntion != WindowsImeLib::CANDIDATELIST_FUNCTION::NONE);

    m_candidateListView->AdviseUIChangedByArrowKey(candidateListFuntion);
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _HandleCompositionPunctuation
//
//----------------------------------------------------------------------------

HRESULT CKeyStateCategory::_HandleCompositionPunctuation(const KeyHandlerEditSessionDTO& dto)
{
    if (_pCompositionProcessorEngine->CandidateMode() != CANDIDATE_NONE && m_candidateListView->IsCreated())
    {
        auto candidateString = m_candidateListView->_GetSelectedCandidateString();
        if (candidateString->length() > 0)
        {
            RETURN_IF_FAILED(m_compositionBuffer->_AddComposingAndChar(candidateString));
        }
    }

    //
    // Get punctuation char from composition processor engine
    //
    auto pCompositionProcessorEngine = _pCompositionProcessorEngine.get();
    WCHAR punctuation = pCompositionProcessorEngine->GetPunctuation(dto.wch);
    auto punctuationString = std::make_shared<const std::wstring>(&punctuation, 1);

    RETURN_IF_FAILED(m_compositionBuffer->_AddCharAndFinalize(punctuationString));

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

    RETURN_IF_FAILED(m_compositionBuffer->_AddCharAndFinalize(sharedFullWidthString));

    return _HandleCancel();
}
