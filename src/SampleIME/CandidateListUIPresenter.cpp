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
        DWORD_PTR candidateLen = 0;
        const WCHAR* pCandidateString = nullptr;
        CStringRange candidateString;

        if (!_pCandidateListUIPresenter->IsCreated())
        {
            goto NoPresenter;
        }

        candidateLen = _pCandidateListUIPresenter->_GetSelectedCandidateString(&pCandidateString);

        candidateString.Set(pCandidateString, candidateLen);

        if (candidateLen)
        {
            hr = _pTextService->_AddComposingAndChar(ec, pContext, &candidateString);

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
    return _pTextService->_SubmitEditSessionTask(dto.pContext, [this, dto](TfEditCookie ec) -> HRESULT
    {
        HRESULT hrReturn = E_FAIL;
        DWORD_PTR candidateLen = 0;
        const WCHAR* pCandidateString = nullptr;
        BSTR pbstr = nullptr;
        CStringRange candidateString;
        BOOL fMakePhraseFromText = FALSE;
        std::vector<CCandidateListItem> candidatePhraseList;
    //    CANDIDATE_MODE tempCandMode = CANDIDATE_NONE;
    //    CCandidateListUIPresenter* pTempCandListUIPresenter = nullptr;
        ITfDocumentMgr* pDocumentMgr = nullptr;
    //    HRESULT hrStartCandidateList = E_FAIL;
    //    WindowsImeLib::IWindowsIMECandidateListView* candidateListInterface = nullptr;

        if (!_pCandidateListUIPresenter->IsCreated())
        {
            hrReturn = S_OK;
            goto Exit;
        }

        candidateLen = _pCandidateListUIPresenter->_GetSelectedCandidateString(&pCandidateString);
        if (0 == candidateLen)
        {
            hrReturn = S_FALSE;
            goto Exit;
        }

        candidateString.Set(pCandidateString, candidateLen);

        fMakePhraseFromText = _pCompositionProcessorEngine->IsMakePhraseFromText();
        if (fMakePhraseFromText)
        {
            _pCompositionProcessorEngine->GetCandidateStringInConverted(candidateString, &candidatePhraseList);
            LCID locale = WindowsImeLib::g_processorFactory->GetConstantProvider()->GetLocale();
            _pCandidateListUIPresenter->RemoveSpecificCandidateFromList(locale, candidatePhraseList, candidateString);
        }

        // We have a candidate list if candidatePhraseList.Cnt is not 0
        // If we are showing reverse conversion, use CCandidateListUIPresenter
        if (candidatePhraseList.size() > 0)
        {
            if (_pCandidateListUIPresenter->IsCreated())
            {
                _pCandidateListUIPresenter->_EndCandidateList();
                _pCandidateListUIPresenter->DestroyView();
                _pCompositionProcessorEngine->ResetCandidateState();
            }
    
    //        tempCandMode = CANDIDATE_WITH_NEXT_COMPOSITION;
    //
    //        pTempCandListUIPresenter = new (std::nothrow) CCandidateListUIPresenter(
    //            reinterpret_cast<CWindowsIME*>(_textService->GetTextService()),
    //            Global::AtomCandidateWindow,
    //            CATEGORY_CANDIDATE,
    //            _pCompositionProcessorEngine->GetCandidateListIndexRange(),
    //            FALSE);
    //        if (nullptr == pTempCandListUIPresenter)
    //        {
    //            hrReturn = E_OUTOFMEMORY;
    //            goto Exit;
    //        }

            _pCandidateListUIPresenter->CreateView(
                _pCompositionProcessorEngine->GetCandidateListIndexRange(),
                FALSE);

            _pCompositionProcessorEngine->SetCandidateKeyStrokeCategory(CATEGORY_CANDIDATE);
            _pCompositionProcessorEngine->SetCandidateMode(CANDIDATE_WITH_NEXT_COMPOSITION);
            _pCompositionProcessorEngine->SetIsCandidateWithWildcard(false);
            // _candidateMode = CANDIDATE_WITH_NEXT_COMPOSITION;
            // _isCandidateWithWildcard = FALSE;

            // call _Start*Line for CCandidateListUIPresenter or CReadingLine
            // we don't cache the document manager object so get it from pContext.
            if (dto.pContext->GetDocumentMgr(&pDocumentMgr) == S_OK)
            {
                ITfRange* pRange = nullptr;
                if (_pTextService->GetComposition()->GetRange(&pRange) == S_OK)
                {
    //              if (isNewWindowRequired)
                    {
    //                  hrStartCandidateList = candidateListInterface->_StartCandidateList(_tfClientId, pDocumentMgr, pContext, ec, pRange,
    //                          WindowsImeLib::g_processorFactory->GetConstantProvider()->GetCandidateWindowWidth());
                        _pCandidateListUIPresenter->_StartCandidateList(dto.pContext, ec, pRange,
                            WindowsImeLib::g_processorFactory->GetConstantProvider()->GetCandidateWindowWidth());
                    }

                    pRange->Release();
                }
                pDocumentMgr->Release();
            }

            // set up candidate list if it is being shown
            _pCandidateListUIPresenter->_SetTextColor(RGB(0, 0x80, 0), GetSysColor(COLOR_WINDOW));    // Text color is green
            _pCandidateListUIPresenter->_SetFillColor((HBRUSH)(COLOR_WINDOW+1));    // Background color is window
            _pCandidateListUIPresenter->_SetText(&candidatePhraseList, FALSE);

            // Add composing character
            hrReturn = _pTextService->_AddComposingAndChar(ec, dto.pContext, &candidateString);

    //        // close candidate list
    //        if (_pCandidateListUIPresenter)
    //        {
    //            _pCandidateListUIPresenter->_EndCandidateList();
    //            _pCandidateListUIPresenter.reset();
    //            ResetCandidateState();
    //        }
    //
    //        if (hrReturn == S_OK)
    //        {
    //            // copy temp candidate
    //            _pCandidateListUIPresenter.attach(pTempCandListUIPresenter);
    //
    //            _candidateMode = tempCandMode;
    //            _isCandidateWithWildcard = FALSE;
    //        }
        }
        else
        {
            hrReturn = _HandleCandidateFinalizeWorker(ec, dto.pContext);
        }
    
        if (pbstr)
        {
            SysFreeString(pbstr);
        }
    
Exit:
        return hrReturn;
    }, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE);
}

//+---------------------------------------------------------------------------
//
// _HandleCandidateArrowKey
//
//----------------------------------------------------------------------------

HRESULT CKeyStateCategory::_HandleCandidateArrowKey(const KeyHandlerEditSessionDTO& dto)
{
    const auto candidateListFuntion = KeyStrokeFunctionToCandidateListFunction(dto.arrowKey);
    if (candidateListFuntion != CANDIDATELIST_FUNCTION_NONE)
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
    DWORD phraseLen = 0;
    const WCHAR* pPhraseString = nullptr;

    phraseLen = (DWORD)_pCandidateListUIPresenter->_GetSelectedCandidateString(&pPhraseString);

    CStringRange phraseString;
    phraseString.Set(pPhraseString, phraseLen);

    return _pTextService->_SubmitEditSessionTask(dto.pContext, [this, dto, phraseString](TfEditCookie ec) -> HRESULT
    {
        if (phraseString.GetLength() > 0)
        {
            RETURN_IF_FAILED(_pTextService->_AddCharAndFinalize(ec, dto.pContext, const_cast<CStringRange*>(&phraseString)));
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
    if (candidateListFuntion != CANDIDATELIST_FUNCTION_NONE)
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