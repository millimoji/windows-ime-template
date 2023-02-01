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
    if (_pCandidateListUIPresenter->IsCreated())
    {
        const auto candidateString = _pCandidateListUIPresenter->_GetSelectedCandidateString();

        if (candidateString->length() > 0)
        {
            RETURN_IF_FAILED(_pTextService->_AddComposingAndChar(candidateString));
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
    RETURN_HR_IF(S_OK, !_pCandidateListUIPresenter->IsCreated());

    const auto pCandidateString = _pCandidateListUIPresenter->_GetSelectedCandidateString();
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

    if (_pCandidateListUIPresenter->IsCreated())
    {
        _pCandidateListUIPresenter->_EndCandidateList();
        _pCandidateListUIPresenter->DestroyView();
        _pCompositionProcessorEngine->ResetCandidateState();
    }

    _pCandidateListUIPresenter->CreateView(_pCompositionProcessorEngine->GetCandidateListIndexRange(), FALSE);
    _pCompositionProcessorEngine->SetCandidateMode(CANDIDATE_WITH_NEXT_COMPOSITION);
    _pCompositionProcessorEngine->SetIsCandidateWithWildcard(false);

    // call _Start*Line for CCandidateListUIPresenter or CReadingLine
    _pCandidateListUIPresenter->_StartCandidateList(WindowsImeLib::g_processorFactory->GetConstantProvider()->GetCandidateWindowWidth());

    // set up candidate list if it is being shown
    _pCandidateListUIPresenter->_SetTextColor(RGB(0, 0x80, 0), GetSysColor(COLOR_WINDOW));    // Text color is green
    _pCandidateListUIPresenter->_SetFillColor((HBRUSH)(COLOR_WINDOW+1));    // Background color is window

    std::vector<shared_wstring> candidateConvertedList;
    for (const auto& candidateSrc : candidatePhraseList) {
        candidateConvertedList.emplace_back(std::make_shared<std::wstring>(candidateSrc._ItemString.Get(), candidateSrc._ItemString.GetLength()));
    }
    _pCandidateListUIPresenter->_SetText(candidateConvertedList);

    // Add composing character
    return _pTextService->_AddComposingAndChar(pCandidateString);
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
        _pCandidateListUIPresenter->AdviseUIChangedByArrowKey(candidateListFuntion);
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
    int iSelectAsNumber = FindVkInVector(*_pCompositionProcessorEngine->GetCandidateListIndexRange(), keyCode);

    if (iSelectAsNumber == -1)
    {
        return S_FALSE;
    }

    if (_pCandidateListUIPresenter->IsCreated())
    {
        if (_pCandidateListUIPresenter->_SetSelectionInPage(iSelectAsNumber))
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
    auto phraseString = _pCandidateListUIPresenter->_GetSelectedCandidateString();

    if (phraseString->length() > 0)
    {
        RETURN_IF_FAILED(_pTextService->_AddCharAndFinalize(phraseString));
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
        _pCandidateListUIPresenter->AdviseUIChangedByArrowKey(candidateListFuntion);
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
    int iSelectAsNumber = FindVkInVector(*_pCompositionProcessorEngine->GetCandidateListIndexRange(), uCode);
    if (iSelectAsNumber == -1)
    {
        return S_FALSE;
    }

    if (_pCandidateListUIPresenter->IsCreated())
    {
        if (_pCandidateListUIPresenter->_SetSelectionInPage(iSelectAsNumber))
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
