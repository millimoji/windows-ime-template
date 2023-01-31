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

HRESULT CKeyStateCategory::_HandleCandidateFinalize(const KeyHandlerEditSessionDTO& dto)
{
    return _pTextService->_SubmitEditSessionTask(dto.pContext, [this, dto](TfEditCookie ec) -> HRESULT
    {
        return _HandleCandidateFinalizeWorker(ec, dto.pContext);
    }, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE);
}

HRESULT CKeyStateCategory::_HandleCandidateFinalizeWorker(TfEditCookie ec, _In_ ITfContext *pContext)
{
        HRESULT hr = S_OK;
        shared_wstring candidateString;

        if (!_pCandidateListUIPresenter->IsCreated())
        {
            goto NoPresenter;
        }

        candidateString = _pCandidateListUIPresenter->_GetSelectedCandidateString();

        if (candidateString->length() > 0)
        {
            hr = _pTextService->_AddComposingAndChar(ec, pContext, candidateString);

            if (FAILED(hr))
            {
                return hr;
            }
        }

NoPresenter:
        _HandleCompleteWorker(ec, pContext);

        return hr;
}

//+---------------------------------------------------------------------------
//
// _HandleCandidateConvert
//
//----------------------------------------------------------------------------

HRESULT CKeyStateCategory::_HandleCandidateConvert(const KeyHandlerEditSessionDTO& dto)
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
        return _pTextService->_SubmitEditSessionTask(dto.pContext, [this, dto](TfEditCookie ec) -> HRESULT
        {
            return _HandleCandidateFinalizeWorker(ec, dto.pContext);
        },
        TF_ES_ASYNCDONTCARE | TF_ES_READWRITE);
    }

    if (_pCandidateListUIPresenter->IsCreated())
    {
        _pCandidateListUIPresenter->_EndCandidateList();
        _pCandidateListUIPresenter->DestroyView();
        _pCompositionProcessorEngine->ResetCandidateState();
    }

    _pCandidateListUIPresenter->CreateView(_pCompositionProcessorEngine->GetCandidateListIndexRange(), FALSE);
    _pCompositionProcessorEngine->SetCandidateKeyStrokeCategory(CATEGORY_CANDIDATE);
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
    return _pTextService->_SubmitEditSessionTask(dto.pContext, [this, dto, pCandidateString](TfEditCookie ec) -> HRESULT
    {
        return _pTextService->_AddComposingAndChar(ec, dto.pContext, pCandidateString);
    },
    TF_ES_ASYNCDONTCARE | TF_ES_READWRITE);



//    return _pTextService->_SubmitEditSessionTask(dto.pContext, [this, dto](TfEditCookie ec) -> HRESULT
//    {
//        HRESULT hrReturn = E_FAIL;
//        shared_wstring pCandidateString;
//        BSTR pbstr = nullptr;
//        BOOL fMakePhraseFromText = FALSE;
//        std::vector<CCandidateListItem> candidatePhraseList;
//        ITfDocumentMgr* pDocumentMgr = nullptr;
//
//        if (!_pCandidateListUIPresenter->IsCreated())
//        {
//            hrReturn = S_OK;
//            goto Exit;
//        }
//
//        pCandidateString = _pCandidateListUIPresenter->_GetSelectedCandidateString();
//        if (pCandidateString->length() == 0)
//        {
//            hrReturn = S_FALSE;
//            goto Exit;
//        }
//
//        fMakePhraseFromText = _pCompositionProcessorEngine->IsMakePhraseFromText();
//        if (fMakePhraseFromText)
//        {
//
//            _pCompositionProcessorEngine->GetCandidateStringInConverted(pCandidateString, &candidatePhraseList);
//            LCID locale = WindowsImeLib::g_processorFactory->GetConstantProvider()->GetLocale();
//            RemoveSpecificCandidateFromList(locale, candidatePhraseList, pCandidateString);
//        }
//
//        // We have a candidate list if candidatePhraseList.Cnt is not 0
//        // If we are showing reverse conversion, use CCandidateListUIPresenter
//        if (candidatePhraseList.size() > 0)
//        {
//            if (_pCandidateListUIPresenter->IsCreated())
//            {
//                _pCandidateListUIPresenter->_EndCandidateList();
//                _pCandidateListUIPresenter->DestroyView();
//                _pCompositionProcessorEngine->ResetCandidateState();
//            }
//    
//    //        tempCandMode = CANDIDATE_WITH_NEXT_COMPOSITION;
//    //
//    //        pTempCandListUIPresenter = new (std::nothrow) CCandidateListUIPresenter(
//    //            reinterpret_cast<CWindowsIME*>(_textService->GetTextService()),
//    //            Global::AtomCandidateWindow,
//    //            CATEGORY_CANDIDATE,
//    //            _pCompositionProcessorEngine->GetCandidateListIndexRange(),
//    //            FALSE);
//    //        if (nullptr == pTempCandListUIPresenter)
//    //        {
//    //            hrReturn = E_OUTOFMEMORY;
//    //            goto Exit;
//    //        }
//
//            _pCandidateListUIPresenter->CreateView(
//                _pCompositionProcessorEngine->GetCandidateListIndexRange(),
//                FALSE);
//
//            _pCompositionProcessorEngine->SetCandidateKeyStrokeCategory(CATEGORY_CANDIDATE);
//            _pCompositionProcessorEngine->SetCandidateMode(CANDIDATE_WITH_NEXT_COMPOSITION);
//            _pCompositionProcessorEngine->SetIsCandidateWithWildcard(false);
//            // _candidateMode = CANDIDATE_WITH_NEXT_COMPOSITION;
//            // _isCandidateWithWildcard = FALSE;
//
//            // call _Start*Line for CCandidateListUIPresenter or CReadingLine
//            // we don't cache the document manager object so get it from pContext.
//            if (dto.pContext->GetDocumentMgr(&pDocumentMgr) == S_OK)
//            {
//                ITfRange* pRange = nullptr;
//                if (_pTextService->GetComposition()->GetRange(&pRange) == S_OK)
//                {
//    //              if (isNewWindowRequired)
//                    {
//    //                  hrStartCandidateList = candidateListInterface->_StartCandidateList(_tfClientId, pDocumentMgr, pContext, pRange,
//    //                          WindowsImeLib::g_processorFactory->GetConstantProvider()->GetCandidateWindowWidth());
//                        _pCandidateListUIPresenter->_StartCandidateList(WindowsImeLib::g_processorFactory->GetConstantProvider()->GetCandidateWindowWidth());
//                    }
//
//                    pRange->Release();
//                }
//                pDocumentMgr->Release();
//            }
//
//            // set up candidate list if it is being shown
//            _pCandidateListUIPresenter->_SetTextColor(RGB(0, 0x80, 0), GetSysColor(COLOR_WINDOW));    // Text color is green
//            _pCandidateListUIPresenter->_SetFillColor((HBRUSH)(COLOR_WINDOW+1));    // Background color is window
//
//            std::vector<shared_wstring> candidateConvertedList;
//            for (const auto& candidateSrc : candidatePhraseList) {
//                candidateConvertedList.emplace_back(std::make_shared<std::wstring>(
//                    candidateSrc._ItemString.Get(), candidateSrc._ItemString.GetLength()));
//            }
//            _pCandidateListUIPresenter->_SetText(candidateConvertedList);
//
//            // Add composing character
//            hrReturn = _pTextService->_AddComposingAndChar(ec, dto.pContext, pCandidateString);
//
//    //        // close candidate list
//    //        if (_pCandidateListUIPresenter)
//    //        {
//    //            _pCandidateListUIPresenter->_EndCandidateList();
//    //            _pCandidateListUIPresenter.reset();
//    //            ResetCandidateState();
//    //        }
//    //
//    //        if (hrReturn == S_OK)
//    //        {
//    //            // copy temp candidate
//    //            _pCandidateListUIPresenter.attach(pTempCandListUIPresenter);
//    //
//    //            _candidateMode = tempCandMode;
//    //            _isCandidateWithWildcard = FALSE;
//    //        }
//        }
//        else
//        {
//            hrReturn = _HandleCandidateFinalizeWorker(ec, dto.pContext);
//        }
//    
//        if (pbstr)
//        {
//            SysFreeString(pbstr);
//        }
//    
//Exit:
//        return hrReturn;
//    }, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE);
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

HRESULT CKeyStateCategory::_HandleCandidateSelectByNumber(const KeyHandlerEditSessionDTO& dto)
{
    int iSelectAsNumber = FindVkInVector(*_pCompositionProcessorEngine->GetCandidateListIndexRange(), dto.code);

    if (iSelectAsNumber == -1)
    {
        return S_FALSE;
    }

    if (_pCandidateListUIPresenter->IsCreated())
    {
        if (_pCandidateListUIPresenter->_SetSelectionInPage(iSelectAsNumber))
        {
            return _HandleCandidateConvert(dto);
        }
    }

    return S_FALSE;
}

//+---------------------------------------------------------------------------
//
// _HandlePhraseFinalize
//
//----------------------------------------------------------------------------

HRESULT CKeyStateCategory::_HandlePhraseFinalize(const KeyHandlerEditSessionDTO& dto)
{
    auto phraseString = _pCandidateListUIPresenter->_GetSelectedCandidateString();

    return _pTextService->_SubmitEditSessionTask(dto.pContext, [this, dto, phraseString](TfEditCookie ec) -> HRESULT
    {
        if (phraseString->length() > 0)
        {
            RETURN_IF_FAILED(_pTextService->_AddCharAndFinalize(ec, dto.pContext, phraseString));
        }
        return _HandleCompleteWorker(ec, dto.pContext);
    }, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE);
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

HRESULT CKeyStateCategory::_HandlePhraseSelectByNumber(const KeyHandlerEditSessionDTO& dto, UINT uCode)
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
            return _HandlePhraseFinalize(dto);
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
