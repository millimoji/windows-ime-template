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
// #include "WindowsIME.h"
// #include "Globals.h"
// #include "CandidateListView.h"
// #include "BaseStructure.h"

//////////////////////////////////////////////////////////////////////
//
// CWindowsIME candidate key handler methods
//
//////////////////////////////////////////////////////////////////////

const int MOVEUP_ONE = -1;
const int MOVEDOWN_ONE = 1;
const int MOVETO_TOP = 0;
const int MOVETO_BOTTOM = -1;
//+---------------------------------------------------------------------------
//
// _HandleCandidateFinalize
//
//----------------------------------------------------------------------------

HRESULT CKeyStateCategory::_HandleCandidateFinalize()
{
    if (m_candidateListView->IsCreated())
    {
        const auto candidateString = m_candidateListView->_GetSelectedCandidateString();

        if (candidateString->length() > 0)
        {
            RETURN_IF_FAILED(m_compositionBuffer->_AddComposingAndChar(candidateString));
        }
    }

    return _HandleComplete();
}

//+---------------------------------------------------------------------------
//
// _HandleCandidateConvert
//
//----------------------------------------------------------------------------

HRESULT CKeyStateCategory::_HandleCandidateConvert()
{
    RETURN_HR_IF(S_OK, !m_candidateListView->IsCreated());

    const auto pCandidateString = m_candidateListView->_GetSelectedCandidateString();
    RETURN_HR_IF(S_FALSE, pCandidateString->length() == 0);

    const auto fMakePhraseFromText = _pCompositionProcessorEngine->IsMakePhraseFromText();

    std::vector<CCandidateListItem> candidatePhraseList;
    if (fMakePhraseFromText)
    {
        _pCompositionProcessorEngine->GetCandidateStringInConverted(pCandidateString, &candidatePhraseList);
        LCID locale = WindowsImeLib::g_processorFactory->GetConstantProvider()->GetLocale();
        RemoveSpecificCandidateFromList(locale, candidatePhraseList, pCandidateString);
    }

    if (candidatePhraseList.size() == 0)
    {
        return _HandleCandidateFinalize();
    }

    if (m_candidateListView->IsCreated())
    {
        m_candidateListView->_EndCandidateList();
        _pCompositionProcessorEngine->ResetCandidateState();
    }

    // call _Start*Line for CCandidateListUIPresenter or CReadingLine
    LOG_IF_FAILED(m_candidateListView->_StartCandidateList(CAND_WIDTH));
    // set up candidate list if it is being shown. Text color is green, Background color is window.
    m_candidateListView->_SetTextColorAndFillColor(WindowsImeLib::CANDIDATE_COLOR_STYLE::GREEN);

    _pCompositionProcessorEngine->SetCandidateMode(CANDIDATE_WITH_NEXT_COMPOSITION);
    _pCompositionProcessorEngine->SetIsCandidateWithWildcard(false);

    std::vector<shared_wstring> candidateConvertedList;
    for (const auto& candidateSrc : candidatePhraseList) {
        candidateConvertedList.emplace_back(std::make_shared<std::wstring>(candidateSrc._ItemString.Get(), candidateSrc._ItemString.GetLength()));
    }
    m_candidateListView->_SetText(candidateConvertedList);

    // Add composing character
    return m_compositionBuffer->_AddComposingAndChar(pCandidateString);
}

//+---------------------------------------------------------------------------
//
// _HandleCandidateArrowKey
//
//----------------------------------------------------------------------------

HRESULT CKeyStateCategory::_HandleCandidateArrowKey(const KeyHandlerEditSessionDTO& dto)
{
    const auto candidateListFuntion = KeyStrokeFunctionToCandidateListFunction(dto.arrowKey);
    if (candidateListFuntion != WindowsImeLib::CANDIDATELIST_FUNCTION::NONE)
    {
        m_candidateListView->AdviseUIChangedByArrowKey(candidateListFuntion);
    }
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _HandleCandidateSelectByNumber
//
//----------------------------------------------------------------------------

inline int FindVkInVector(const std::vector<DWORD>& srcVkList, UINT vk)
{
    bool isVkNumpad = (VK_NUMPAD0 <= vk) && (vk <= VK_NUMPAD9);

    for (auto it = srcVkList.begin(); it != srcVkList.end(); ++it)
    {
        if ((*it == vk) || (isVkNumpad && (*it == (vk - VK_NUMPAD0))))
        {
            return static_cast<int>(std::distance(srcVkList.begin(), it));
        }
    }
    return -1;
}

HRESULT CKeyStateCategory::_HandleCandidateSelectByNumber(UINT keyCode)
{
    int iSelectAsNumber = FindVkInVector(*m_candidateListView->GetCandidateListRange(), keyCode);

    if (iSelectAsNumber == -1)
    {
        return S_FALSE;
    }

    if (m_candidateListView->IsCreated())
    {
        if (m_candidateListView->_SetSelectionInPage(iSelectAsNumber))
        {
            return _HandleCandidateConvert();
        }
    }

    return S_FALSE;
}

//+---------------------------------------------------------------------------
//
// _HandlePhraseFinalize
//
//----------------------------------------------------------------------------

HRESULT CKeyStateCategory::_HandlePhraseFinalize()
{
    auto phraseString = m_candidateListView->_GetSelectedCandidateString();

    if (phraseString->length() > 0)
    {
        RETURN_IF_FAILED(m_compositionBuffer->_AddCharAndFinalize(phraseString));
    }

    return _HandleComplete();
}

//+---------------------------------------------------------------------------
//
// _HandlePhraseArrowKey
//
//----------------------------------------------------------------------------

HRESULT CKeyStateCategory::_HandlePhraseArrowKey(const KeyHandlerEditSessionDTO& dto)
{
    const auto candidateListFuntion = KeyStrokeFunctionToCandidateListFunction(dto.arrowKey);
    if (candidateListFuntion != WindowsImeLib::CANDIDATELIST_FUNCTION::NONE)
    {
        m_candidateListView->AdviseUIChangedByArrowKey(candidateListFuntion);
    }
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _HandlePhraseSelectByNumber
//
//----------------------------------------------------------------------------

HRESULT CKeyStateCategory::_HandlePhraseSelectByNumber(UINT uCode)
{
    int iSelectAsNumber = FindVkInVector(*m_candidateListView->GetCandidateListRange(), uCode);
    if (iSelectAsNumber == -1)
    {
        return S_FALSE;
    }

    if (m_candidateListView->IsCreated())
    {
        if (m_candidateListView->_SetSelectionInPage(iSelectAsNumber))
        {
            return _HandlePhraseFinalize();
        }
    }

    return S_FALSE;

}

void CKeyStateCategory::RemoveSpecificCandidateFromList(_In_ LCID Locale, _Inout_ std::vector<CCandidateListItem> &candidateList, const shared_wstring& candidateString)
{
    for (UINT index = 0; index < candidateList.size(); ++index)
    {
        CCandidateListItem* pLI = &candidateList.at(index);

        if (CStringRange::Compare(Locale, *candidateString, &pLI->_ItemString) == CSTR_EQUAL)
        {
            candidateList.erase(candidateList.begin() + index);
            continue;
        }

        index++;
    }
}
