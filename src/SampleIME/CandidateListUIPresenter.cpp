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

HRESULT CKeyStateCategory::_HandleCandidateFinalize(TfEditCookie ec, _In_ ITfContext *pContext)
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

    _HandleComplete(ec, pContext);

    return hr;
}

//+---------------------------------------------------------------------------
//
// _HandleCandidateConvert
//
//----------------------------------------------------------------------------

HRESULT CKeyStateCategory::_HandleCandidateConvert(TfEditCookie ec, _In_ ITfContext *pContext)
{
    return _HandleCandidateWorker(ec, pContext);
}

//+---------------------------------------------------------------------------
//
// _HandleCandidateWorker
//
//----------------------------------------------------------------------------

HRESULT CKeyStateCategory::_HandleCandidateWorker(TfEditCookie ec, _In_ ITfContext *pContext)
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
            _pTextService->ResetCandidateState();
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
            WindowsImeLib::AtomCandidateWindow,
            CATEGORY_CANDIDATE,
            _pCompositionProcessorEngine->GetCandidateListIndexRange(),
            FALSE);

        _pTextService->SetCandidateMode(CANDIDATE_WITH_NEXT_COMPOSITION);
        _pTextService->SetIsCandidateWithWildcard(false);
        // _candidateMode = CANDIDATE_WITH_NEXT_COMPOSITION;
        // _isCandidateWithWildcard = FALSE;

        // call _Start*Line for CCandidateListUIPresenter or CReadingLine
        // we don't cache the document manager object so get it from pContext.
        if (pContext->GetDocumentMgr(&pDocumentMgr) == S_OK)
        {
            ITfRange* pRange = nullptr;
            if (_pTextService->GetComposition()->GetRange(&pRange) == S_OK)
            {
//              if (isNewWindowRequired)
                {
//                  hrStartCandidateList = candidateListInterface->_StartCandidateList(_tfClientId, pDocumentMgr, pContext, ec, pRange,
//                          WindowsImeLib::g_processorFactory->GetConstantProvider()->GetCandidateWindowWidth());
                    _pCandidateListUIPresenter->_StartCandidateList(
                        _pTextService->GetClientId(),
                        pDocumentMgr, pContext, ec, pRange,
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
        hrReturn = _pTextService->_AddComposingAndChar(ec, pContext, &candidateString);

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
        hrReturn = _HandleCandidateFinalize(ec, pContext);
    }

    if (pbstr)
    {
        SysFreeString(pbstr);
    }

Exit:
    return hrReturn;
}

//+---------------------------------------------------------------------------
//
// _HandleCandidateArrowKey
//
//----------------------------------------------------------------------------

HRESULT CKeyStateCategory::_HandleCandidateArrowKey(TfEditCookie ec, _In_ ITfContext *pContext, _In_ KEYSTROKE_FUNCTION keyFunction)
{
    ec;
    pContext;

    _pCandidateListUIPresenter->AdviseUIChangedByArrowKey(keyFunction);

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

HRESULT CKeyStateCategory::_HandleCandidateSelectByNumber(TfEditCookie ec, _In_ ITfContext *pContext, _In_ UINT uCode)
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
            return _HandleCandidateConvert(ec, pContext);
        }
    }

    return S_FALSE;
}

//+---------------------------------------------------------------------------
//
// _HandlePhraseFinalize
//
//----------------------------------------------------------------------------

HRESULT CKeyStateCategory::_HandlePhraseFinalize(TfEditCookie ec, _In_ ITfContext *pContext)
{
    HRESULT hr = S_OK;

    DWORD phraseLen = 0;
    const WCHAR* pPhraseString = nullptr;

    phraseLen = (DWORD)_pCandidateListUIPresenter->_GetSelectedCandidateString(&pPhraseString);

    CStringRange phraseString;
    phraseString.Set(pPhraseString, phraseLen);

    if (phraseLen)
    {
        if ((hr = _pTextService->_AddCharAndFinalize(ec, pContext, &phraseString)) != S_OK)
        {
            return hr;
        }
    }

    _HandleComplete(ec, pContext);

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _HandlePhraseArrowKey
//
//----------------------------------------------------------------------------

HRESULT CKeyStateCategory::_HandlePhraseArrowKey(TfEditCookie ec, _In_ ITfContext *pContext, _In_ KEYSTROKE_FUNCTION keyFunction)
{
    ec;
    pContext;

    _pCandidateListUIPresenter->AdviseUIChangedByArrowKey(keyFunction);

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _HandlePhraseSelectByNumber
//
//----------------------------------------------------------------------------

HRESULT CKeyStateCategory::_HandlePhraseSelectByNumber(TfEditCookie ec, _In_ ITfContext *pContext, _In_ UINT uCode)
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
            return _HandlePhraseFinalize(ec, pContext);
        }
    }

    return S_FALSE;
}

// //////////////////////////////////////////////////////////////////////
// //
// // CCandidateListUIPresenter class
// //
// //////////////////////////////////////////////////////////////////////
// 
// //+---------------------------------------------------------------------------
// //
// // ctor
// //
// //----------------------------------------------------------------------------
// 
// CCandidateListUIPresenter::CCandidateListUIPresenter(_In_ IInternalFrameworkService* pTextService, ATOM atom, KEYSTROKE_CATEGORY Category, _In_ std::vector<DWORD> *pIndexRange, BOOL hideWindow) : CTfTextLayoutSink(pTextService)
// {
//     _atom = atom;
// 
//     _pIndexRange = pIndexRange;
// 
//     _parentWndHandle = nullptr;
//     _pCandidateWnd = nullptr;
// 
//     _Category = Category;
// 
//     _updatedFlags = 0;
// 
//     _uiElementId = (DWORD)-1;
//     _isShowMode = TRUE;   // store return value from BeginUIElement
//     _hideWindow = hideWindow;     // Hide window flag from [Configuration] CandidateList.Phrase.HideWindow
// 
//     _pTextService = pTextService;
// //    _pTextService->AddRef();
// 
//     // _refCount = 1;
// }
// 
// //+---------------------------------------------------------------------------
// //
// // dtor
// //
// //----------------------------------------------------------------------------
// 
// CCandidateListUIPresenter::~CCandidateListUIPresenter()
// {
//     _EndCandidateList();
// //    _pTextService->Release();
// }
// 
// //+---------------------------------------------------------------------------
// //
// // ITfCandidateListUIElement::IUnknown::QueryInterface
// //
// //----------------------------------------------------------------------------
// 
// STDAPI CCandidateListUIPresenter::QueryInterface(REFIID riid, _Outptr_ void **ppvObj)
// {
//     if (CTfTextLayoutSink::QueryInterface(riid, ppvObj) == S_OK)
//     {
//         return S_OK;
//     }
// 
//     if (ppvObj == nullptr)
//     {
//         return E_INVALIDARG;
//     }
// 
//     *ppvObj = nullptr;
// 
//     if (IsEqualIID(riid, IID_ITfUIElement) ||
//         IsEqualIID(riid, IID_ITfCandidateListUIElement))
//     {
//         *ppvObj = (ITfCandidateListUIElement*)this;
//     }
//     else if (IsEqualIID(riid, IID_IUnknown) || 
//         IsEqualIID(riid, IID_ITfCandidateListUIElementBehavior)) 
//     {
//         *ppvObj = (ITfCandidateListUIElementBehavior*)this;
//     }
//     else if (IsEqualIID(riid, __uuidof(ITfIntegratableCandidateListUIElement))) 
//     {
//         *ppvObj = (ITfIntegratableCandidateListUIElement*)this;
//     }
// 
//     if (*ppvObj)
//     {
//         AddRef();
//         return S_OK;
//     }
// 
//     return E_NOINTERFACE;
// }
// 
// //+---------------------------------------------------------------------------
// //
// // ITfCandidateListUIElement::IUnknown::AddRef
// //
// //----------------------------------------------------------------------------
// 
// STDAPI_(ULONG) CCandidateListUIPresenter::AddRef()
// {
//     return CTfTextLayoutSink::AddRef();
//     // return ++_refCount;
// }
// 
// //+---------------------------------------------------------------------------
// //
// // ITfCandidateListUIElement::IUnknown::Release
// //
// //----------------------------------------------------------------------------
// 
// STDAPI_(ULONG) CCandidateListUIPresenter::Release()
// {
//     return CTfTextLayoutSink::Release();
// 
// // Bug?? double free
// //    LONG cr = --_refCount;
// //
// //    assert(_refCount >= 0);
// //
// //    if (_refCount == 0)
// //    {
// //        delete this;
// //    }
// //
// //    return cr;
// }
// 
// //+---------------------------------------------------------------------------
// //
// // ITfCandidateListUIElement::ITfUIElement::GetDescription
// //
// //----------------------------------------------------------------------------
// 
// STDAPI CCandidateListUIPresenter::GetDescription(BSTR *pbstr)
// {
//     if (pbstr)
//     {
//         *pbstr = SysAllocString(L"Cand");
//     }
//     return S_OK;
// }
// 
// //+---------------------------------------------------------------------------
// //
// // ITfCandidateListUIElement::ITfUIElement::GetGUID
// //
// //----------------------------------------------------------------------------
// 
// STDAPI CCandidateListUIPresenter::GetGUID(GUID *pguid)
// {
//     *pguid = WindowsImeLib::g_processorFactory->GetConstantProvider()->CandUIElement();
//     return S_OK;
// }
// 
// //+---------------------------------------------------------------------------
// //
// // ITfCandidateListUIElement::ITfUIElement::Show
// //
// //----------------------------------------------------------------------------
// 
// STDAPI CCandidateListUIPresenter::Show(BOOL showCandidateWindow)
// {
//     if (showCandidateWindow)
//     {
//         ToShowCandidateWindow();
//     }
//     else
//     {
//         ToHideCandidateWindow();
//     }
//     return S_OK;
// }
// 
// HRESULT CCandidateListUIPresenter::ToShowCandidateWindow()
// {
//     if (_hideWindow)
//     {
//         _pCandidateWnd->_Show(FALSE);
//     }
//     else
//     {
//         _MoveWindowToTextExt();
// 
//         _pCandidateWnd->_Show(TRUE);
//     }
// 
//     return S_OK;
// }
// 
// HRESULT CCandidateListUIPresenter::ToHideCandidateWindow()
// {
//     if (_pCandidateWnd)
//     {
//         _pCandidateWnd->_Show(FALSE);
//     }
// 
//     _updatedFlags = TF_CLUIE_SELECTION | TF_CLUIE_CURRENTPAGE;
//     _UpdateUIElement();
// 
//     return S_OK;
// }
// 
// //+---------------------------------------------------------------------------
// //
// // ITfCandidateListUIElement::ITfUIElement::IsShown
// //
// //----------------------------------------------------------------------------
// 
// STDAPI CCandidateListUIPresenter::IsShown(BOOL *pIsShow)
// {
//     *pIsShow = _pCandidateWnd->_IsWindowVisible();
//     return S_OK;
// }
// 
// //+---------------------------------------------------------------------------
// //
// // ITfCandidateListUIElement::GetUpdatedFlags
// //
// //----------------------------------------------------------------------------
// 
// STDAPI CCandidateListUIPresenter::GetUpdatedFlags(DWORD *pdwFlags)
// {
//     *pdwFlags = _updatedFlags;
//     return S_OK;
// }
// 
// //+---------------------------------------------------------------------------
// //
// // ITfCandidateListUIElement::GetDocumentMgr
// //
// //----------------------------------------------------------------------------
// 
// STDAPI CCandidateListUIPresenter::GetDocumentMgr(ITfDocumentMgr **ppdim)
// {
//     *ppdim = nullptr;
// 
//     return E_NOTIMPL;
// }
// 
// //+---------------------------------------------------------------------------
// //
// // ITfCandidateListUIElement::GetCount
// //
// //----------------------------------------------------------------------------
// 
// STDAPI CCandidateListUIPresenter::GetCount(UINT *pCandidateCount)
// {
//     if (_pCandidateWnd)
//     {
//         *pCandidateCount = _pCandidateWnd->_GetCount();
//     }
//     else
//     {
//         *pCandidateCount = 0;
//     }
//     return S_OK;
// }
// 
// //+---------------------------------------------------------------------------
// //
// // ITfCandidateListUIElement::GetSelection
// //
// //----------------------------------------------------------------------------
// 
// STDAPI CCandidateListUIPresenter::GetSelection(UINT *pSelectedCandidateIndex)
// {
//     if (_pCandidateWnd)
//     {
//         *pSelectedCandidateIndex = _pCandidateWnd->_GetSelection();
//     }
//     else
//     {
//         *pSelectedCandidateIndex = 0;
//     }
//     return S_OK;
// }
// 
// //+---------------------------------------------------------------------------
// //
// // ITfCandidateListUIElement::GetString
// //
// //----------------------------------------------------------------------------
// 
// STDAPI CCandidateListUIPresenter::GetString(UINT uIndex, BSTR *pbstr)
// {
//     if (!_pCandidateWnd || (uIndex > _pCandidateWnd->_GetCount()))
//     {
//         return E_FAIL;
//     }
// 
//     DWORD candidateLen = 0;
//     const WCHAR* pCandidateString = nullptr;
// 
//     candidateLen = _pCandidateWnd->_GetCandidateString(uIndex, &pCandidateString);
// 
//     *pbstr = (candidateLen == 0) ? nullptr : SysAllocStringLen(pCandidateString, candidateLen);
// 
//     return S_OK;
// }
// 
// //+---------------------------------------------------------------------------
// //
// // ITfCandidateListUIElement::GetPageIndex
// //
// //----------------------------------------------------------------------------
// 
// STDAPI CCandidateListUIPresenter::GetPageIndex(UINT *pIndex, UINT uSize, UINT *puPageCnt)
// {
//     if (!_pCandidateWnd)
//     {
//         if (pIndex)
//         {
//             *pIndex = 0;
//         }
//         *puPageCnt = 0;
//         return S_OK;
//     }
//     return _pCandidateWnd->_GetPageIndex(pIndex, uSize, puPageCnt);
// }
// 
// //+---------------------------------------------------------------------------
// //
// // ITfCandidateListUIElement::SetPageIndex
// //
// //----------------------------------------------------------------------------
// 
// STDAPI CCandidateListUIPresenter::SetPageIndex(UINT *pIndex, UINT uPageCnt)
// {
//     if (!_pCandidateWnd)
//     {
//         return E_FAIL;
//     }
//     return _pCandidateWnd->_SetPageIndex(pIndex, uPageCnt);
// }
// 
// //+---------------------------------------------------------------------------
// //
// // ITfCandidateListUIElement::GetCurrentPage
// //
// //----------------------------------------------------------------------------
// 
// STDAPI CCandidateListUIPresenter::GetCurrentPage(UINT *puPage)
// {
//     if (!_pCandidateWnd)
//     {
//         *puPage = 0;
//         return S_OK;
//     }
//     return _pCandidateWnd->_GetCurrentPage(puPage);
// }
// 
// //+---------------------------------------------------------------------------
// //
// // ITfCandidateListUIElementBehavior::SetSelection
// // It is related of the mouse clicking behavior upon the suggestion window
// //----------------------------------------------------------------------------
// 
// STDAPI CCandidateListUIPresenter::SetSelection(UINT nIndex)
// {
//     if (_pCandidateWnd)
//     {
//         _pCandidateWnd->_SetSelection(nIndex);
//     }
// 
//     return S_OK;
// }
// 
// //+---------------------------------------------------------------------------
// //
// // ITfCandidateListUIElementBehavior::Finalize
// // It is related of the mouse clicking behavior upon the suggestion window
// //----------------------------------------------------------------------------
// 
// STDAPI CCandidateListUIPresenter::Finalize(void)
// {
//     _CandidateChangeNotification(CAND_ITEM_SELECT);
//     return S_OK;
// }
// 
// //+---------------------------------------------------------------------------
// //
// // ITfCandidateListUIElementBehavior::Abort
// //
// //----------------------------------------------------------------------------
// 
// STDAPI CCandidateListUIPresenter::Abort(void)
// {
//     return E_NOTIMPL;
// }
// 
// //+---------------------------------------------------------------------------
// //
// // ITfIntegratableCandidateListUIElement::SetIntegrationStyle
// // To show candidateNumbers on the suggestion window
// //----------------------------------------------------------------------------
// 
// STDAPI CCandidateListUIPresenter::SetIntegrationStyle(GUID guidIntegrationStyle)
// {
//     return (guidIntegrationStyle == GUID_INTEGRATIONSTYLE_SEARCHBOX) ? S_OK : E_NOTIMPL;
// }
// 
// //+---------------------------------------------------------------------------
// //
// // ITfIntegratableCandidateListUIElement::GetSelectionStyle
// //
// //----------------------------------------------------------------------------
// 
// STDAPI CCandidateListUIPresenter::GetSelectionStyle(_Out_ TfIntegratableCandidateListSelectionStyle *ptfSelectionStyle)
// {
//     *ptfSelectionStyle = STYLE_ACTIVE_SELECTION;
//     return S_OK;
// }
// 
// //+---------------------------------------------------------------------------
// //
// // ITfIntegratableCandidateListUIElement::OnKeyDown
// //
// //----------------------------------------------------------------------------
// 
// STDAPI CCandidateListUIPresenter::OnKeyDown(_In_ WPARAM wParam, _In_ LPARAM lParam, _Out_ BOOL *pIsEaten)
// {
//     wParam;
//     lParam;
// 
//     *pIsEaten = TRUE;
//     return S_OK;
// }
// 
// //+---------------------------------------------------------------------------
// //
// // ITfIntegratableCandidateListUIElement::ShowCandidateNumbers
// //
// //----------------------------------------------------------------------------
// 
// STDAPI CCandidateListUIPresenter::ShowCandidateNumbers(_Out_ BOOL *pIsShow)
// {
//     *pIsShow = TRUE;
//     return S_OK;
// }
// 
// //+---------------------------------------------------------------------------
// //
// // ITfIntegratableCandidateListUIElement::FinalizeExactCompositionString
// //
// //----------------------------------------------------------------------------
// 
// STDAPI CCandidateListUIPresenter::FinalizeExactCompositionString()
// {
//     return E_NOTIMPL;
// }
// 
// 
// //+---------------------------------------------------------------------------
// //
// // _StartCandidateList
// //
// //----------------------------------------------------------------------------
// 
// HRESULT CCandidateListUIPresenter::_StartCandidateList(TfClientId tfClientId, _In_ ITfDocumentMgr *pDocumentMgr, _In_ ITfContext *pContextDocument, TfEditCookie ec, _In_ ITfRange *pRangeComposition, UINT wndWidth)
// {
//     pDocumentMgr;tfClientId;
// 
//     HRESULT hr = E_FAIL;
// 
//     if (FAILED(_StartLayout(pContextDocument, ec, pRangeComposition)))
//     {
//         goto Exit;
//     }
// 
//     BeginUIElement();
// 
//     hr = MakeCandidateWindow(pContextDocument, wndWidth);
//     if (FAILED(hr))
//     {
//         goto Exit;
//     }
// 
//     Show(_isShowMode);
// 
//     RECT rcTextExt;
//     if (SUCCEEDED(_GetTextExt(&rcTextExt)))
//     {
//         _LayoutChangeNotification(&rcTextExt);
//     }
// 
// Exit:
//     if (FAILED(hr))
//     {
//         _EndCandidateList();
//     }
//     return hr;
// }
// 
// //+---------------------------------------------------------------------------
// //
// // _EndCandidateList
// //
// //----------------------------------------------------------------------------
// 
// void CCandidateListUIPresenter::_EndCandidateList()
// {
//     EndUIElement();
// 
//     DisposeCandidateWindow();
// 
//     _EndLayout();
// }
// 
// //+---------------------------------------------------------------------------
// //
// // _SetText
// //
// //----------------------------------------------------------------------------
// 
// void CCandidateListUIPresenter::_SetText(_In_ std::vector<CCandidateListItem>* pCandidateList, BOOL isAddFindKeyCode)
// {
//     AddCandidateToCandidateListUI(pCandidateList, isAddFindKeyCode);
// 
//     SetPageIndexWithScrollInfo(pCandidateList);
// 
//     if (_isShowMode)
//     {
//         _pCandidateWnd->_InvalidateRect();
//     }
//     else
//     {
//         _updatedFlags = TF_CLUIE_COUNT       |
//             TF_CLUIE_SELECTION   |
//             TF_CLUIE_STRING      |
//             TF_CLUIE_PAGEINDEX   |
//             TF_CLUIE_CURRENTPAGE;
//         _UpdateUIElement();
//     }
// }
// 
// void CCandidateListUIPresenter::AddCandidateToCandidateListUI(_In_ std::vector<CCandidateListItem>* pCandidateList, BOOL isAddFindKeyCode)
// {
//     for (UINT index = 0; index < pCandidateList->size(); index++)
//     {
//         _pCandidateWnd->_AddString(&pCandidateList->at(index), isAddFindKeyCode);
//     }
// }
// 
// void CCandidateListUIPresenter::SetPageIndexWithScrollInfo(_In_ std::vector<CCandidateListItem>* pCandidateList)
// {
//     UINT candCntInPage = static_cast<UINT>(_pIndexRange->size());
//     UINT bufferSize = static_cast<UINT>(pCandidateList->size() / candCntInPage + 1);
//     UINT* puPageIndex = new (std::nothrow) UINT[ bufferSize ];
//     if (puPageIndex != nullptr)
//     {
//         for (UINT i = 0; i < bufferSize; i++)
//         {
//             puPageIndex[i] = i * candCntInPage;
//         }
// 
//         _pCandidateWnd->_SetPageIndex(puPageIndex, bufferSize);
//         delete [] puPageIndex;
//     }
//     _pCandidateWnd->_SetScrollInfo(static_cast<int>(pCandidateList->size()), candCntInPage);  // nMax:range of max, nPage:number of items in page
// 
// }
// //+---------------------------------------------------------------------------
// //
// // _ClearList
// //
// //----------------------------------------------------------------------------
// 
// void CCandidateListUIPresenter::_ClearList()
// {
//     _pCandidateWnd->_ClearList();
//     _pCandidateWnd->_InvalidateRect();
// }
// 
// //+---------------------------------------------------------------------------
// //
// // _SetTextColor
// // _SetFillColor
// //
// //----------------------------------------------------------------------------
// 
// void CCandidateListUIPresenter::_SetTextColor(COLORREF crColor, COLORREF crBkColor)
// {
//     _pCandidateWnd->_SetTextColor(crColor, crBkColor);
// }
// 
// void CCandidateListUIPresenter::_SetFillColor(HBRUSH hBrush)
// {
//     _pCandidateWnd->_SetFillColor(hBrush);
// }
// 
// //+---------------------------------------------------------------------------
// //
// // _GetSelectedCandidateString
// //
// //----------------------------------------------------------------------------
// 
// DWORD_PTR CCandidateListUIPresenter::_GetSelectedCandidateString(_Outptr_result_maybenull_ const WCHAR **ppwchCandidateString)
// {
//     return _pCandidateWnd->_GetSelectedCandidateString(ppwchCandidateString);
// }
// 
// //+---------------------------------------------------------------------------
// //
// // _MoveSelection
// //
// //----------------------------------------------------------------------------
// 
// BOOL CCandidateListUIPresenter::_MoveSelection(_In_ int offSet)
// {
//     BOOL ret = _pCandidateWnd->_MoveSelection(offSet, TRUE);
//     if (ret)
//     {
//         if (_isShowMode)
//         {
//             _pCandidateWnd->_InvalidateRect();
//         }
//         else
//         {
//             _updatedFlags = TF_CLUIE_SELECTION;
//             _UpdateUIElement();
//         }
//     }
//     return ret;
// }
// 
// //+---------------------------------------------------------------------------
// //
// // _SetSelection
// //
// //----------------------------------------------------------------------------
// 
// BOOL CCandidateListUIPresenter::_SetSelection(_In_ int selectedIndex)
// {
//     BOOL ret = _pCandidateWnd->_SetSelection(selectedIndex, TRUE);
//     if (ret)
//     {
//         if (_isShowMode)
//         {
//             _pCandidateWnd->_InvalidateRect();
//         }
//         else
//         {
//             _updatedFlags = TF_CLUIE_SELECTION |
//                 TF_CLUIE_CURRENTPAGE;
//             _UpdateUIElement();
//         }
//     }
//     return ret;
// }
// 
// //+---------------------------------------------------------------------------
// //
// // _MovePage
// //
// //----------------------------------------------------------------------------
// 
// BOOL CCandidateListUIPresenter::_MovePage(_In_ int offSet)
// {
//     BOOL ret = _pCandidateWnd->_MovePage(offSet, TRUE);
//     if (ret)
//     {
//         if (_isShowMode)
//         {
//             _pCandidateWnd->_InvalidateRect();
//         }
//         else
//         {
//             _updatedFlags = TF_CLUIE_SELECTION |
//                 TF_CLUIE_CURRENTPAGE;
//             _UpdateUIElement();
//         }
//     }
//     return ret;
// }
// 
// //+---------------------------------------------------------------------------
// //
// // _MoveWindowToTextExt
// //
// //----------------------------------------------------------------------------
// 
// void CCandidateListUIPresenter::_MoveWindowToTextExt()
// {
//     RECT rc;
// 
//     if (FAILED(_GetTextExt(&rc)))
//     {
//         return;
//     }
// 
//     _pCandidateWnd->_Move(rc.left, rc.bottom);
// }
// //+---------------------------------------------------------------------------
// //
// // _LayoutChangeNotification
// //
// //----------------------------------------------------------------------------
// 
// VOID CCandidateListUIPresenter::_LayoutChangeNotification(_In_ RECT *lpRect)
// {
//     RECT rectCandidate = {0, 0, 0, 0};
//     POINT ptCandidate = {0, 0};
// 
//     _pCandidateWnd->_GetClientRect(&rectCandidate);
//     _pCandidateWnd->_GetWindowExtent(lpRect, &rectCandidate, &ptCandidate);
//     _pCandidateWnd->_Move(ptCandidate.x, ptCandidate.y);
// }
// 
// //+---------------------------------------------------------------------------
// //
// // _LayoutDestroyNotification
// //
// //----------------------------------------------------------------------------
// 
// VOID CCandidateListUIPresenter::_LayoutDestroyNotification()
// {
//     _EndCandidateList();
// }
// 
// //+---------------------------------------------------------------------------
// //
// // _CandidateChangeNotifiction
// //
// //----------------------------------------------------------------------------
// 
// HRESULT CCandidateListUIPresenter::_CandidateChangeNotification(_In_ enum CANDWND_ACTION action)
// {
//     HRESULT hr = S_OK;
// 
// //    TfClientId tfClientId = _pTextService->_GetClientId();
// //    wil::com_ptr<ITfThreadMgr> pThreadMgr;
// //    ITfDocumentMgr* pDocumentMgr = nullptr;
// //    ITfContext* pContext = nullptr;
// 
// //    _KEYSTROKE_STATE KeyState = {};
// //    KeyState.Category = _Category;
// //    KeyState.Function = FUNCTION_FINALIZE_CANDIDATELIST;
// 
//     if (CAND_ITEM_SELECT != action)
//     {
//         goto Exit;
//     }
// 
//     {
//         wil::com_ptr<ITfDocumentMgr> pDocumentMgr;
//         RETURN_IF_FAILED(_pTextService->_GetThreadMgr()->GetFocus(&pDocumentMgr));
// 
//         wil::com_ptr<ITfContext> pContext;
//         RETURN_IF_FAILED(pDocumentMgr->GetTop(&pContext));
// 
//         auto compositionBuffer = _pTextService->GetCompositionProcessorEngine();
//         compositionBuffer->FinalizeCandidateList(pContext.get(), _Category);
//     }
// 
// //    pThreadMgr = _pTextService->_GetThreadMgr();
// //    if (nullptr == pThreadMgr)
// //    {
// //        goto Exit;
// //    }
// //
// //    hr = pThreadMgr->GetFocus(&pDocumentMgr);
// //    if (FAILED(hr))
// //    {
// //        goto Exit;
// //    }
// //
// //    hr = pDocumentMgr->GetTop(&pContext);
// //    if (FAILED(hr))
// //    {
// //        pDocumentMgr->Release();
// //        goto Exit;
// //    }
// //
// ////    {
// ////        CKeyHandlerEditSession* pEditSession = new (std::nothrow) CKeyHandlerEditSession(_pTextService, pContext, 0, 0, KeyState);
// ////        if (nullptr != pEditSession)
// ////        {
// ////            HRESULT hrSession = S_OK;
// ////            hr = pContext->RequestEditSession(tfClientId, pEditSession, TF_ES_SYNC | TF_ES_READWRITE, &hrSession);
// ////            if (hrSession == TF_E_SYNCHRONOUS || hrSession == TS_E_READONLY)
// ////            {
// ////                hr = pContext->RequestEditSession(tfClientId, pEditSession, TF_ES_ASYNC | TF_ES_READWRITE, &hrSession);
// ////            }
// ////            pEditSession->Release();
// ////        }
// ////    }
// //
// //    {
// //        auto compositionBuffer = _pTextService->GetCompositionBuffer();
// //        auto pTextService = static_cast<WindowsImeLib::ICompositionProcessorEngineOwner*>(_pTextService);
// //        auto compositionEngiene = _pTextService->GetCompositionProcessorEngine();
// //        RETURN_IF_FAILED(pTextService->_SubmitEditSessionTask(pContext, [pTextService, compositionBuffer, compositionEngiene, KeyState, pContext]
// //            (TfEditCookie ec, WindowsImeLib::IWindowsIMECompositionBuffer* /*pv*/) ->  HRESULT
// //            {
// //                return compositionEngiene->KeyHandlerEditSession_DoEditSession(ec, KeyState, pContext, 0, 0, compositionBuffer.get());
// //            }, TF_ES_ASYNC | TF_ES_READWRITE));
// //    }
// //
// //    pContext->Release();
// //    pDocumentMgr->Release();
// 
// Exit:
//     return hr;
// }
// 
// //+---------------------------------------------------------------------------
// //
// // _CandWndCallback
// //
// //----------------------------------------------------------------------------
// 
// // static
// HRESULT CCandidateListUIPresenter::_CandWndCallback(_In_ void *pv, _In_ enum CANDWND_ACTION action)
// {
//     CCandidateListUIPresenter* fakeThis = (CCandidateListUIPresenter*)pv;
// 
//     return fakeThis->_CandidateChangeNotification(action);
// }
// 
// //+---------------------------------------------------------------------------
// //
// // _UpdateUIElement
// //
// //----------------------------------------------------------------------------
// 
// HRESULT CCandidateListUIPresenter::_UpdateUIElement()
// {
//     HRESULT hr = S_OK;
// 
//     auto pThreadMgr = _pTextService->_GetThreadMgr();
//     if (!pThreadMgr)
//     {
//         return S_OK;
//     }
// 
//     ITfUIElementMgr* pUIElementMgr = nullptr;
// 
//     hr = pThreadMgr->QueryInterface(IID_ITfUIElementMgr, (void **)&pUIElementMgr);
//     if (hr == S_OK)
//     {
//         pUIElementMgr->UpdateUIElement(_uiElementId);
//         pUIElementMgr->Release();
//     }
// 
//     return S_OK;
// }
// 
// //+---------------------------------------------------------------------------
// //
// // OnSetThreadFocus
// //
// //----------------------------------------------------------------------------
// 
// HRESULT CCandidateListUIPresenter::OnSetThreadFocus()
// {
//     if (_isShowMode)
//     {
//         Show(TRUE);
//     }
//     return S_OK;
// }
// 
// //+---------------------------------------------------------------------------
// //
// // OnKillThreadFocus
// //
// //----------------------------------------------------------------------------
// 
// HRESULT CCandidateListUIPresenter::OnKillThreadFocus()
// {
//     if (_isShowMode)
//     {
//         Show(FALSE);
//     }
//     return S_OK;
// }
// 
// void CCandidateListUIPresenter::RemoveSpecificCandidateFromList(_In_ LCID Locale, _Inout_ std::vector<CCandidateListItem> &candidateList, _In_ CStringRange &candidateString)
// {
//     for (UINT index = 0; index < candidateList.size();)
//     {
//         CCandidateListItem* pLI = &candidateList.at(index);
// 
//         if (CStringRange::Compare(Locale, &candidateString, &pLI->_ItemString) == CSTR_EQUAL)
//         {
//             candidateList.erase(candidateList.begin() + index);
//             continue;
//         }
// 
//         index++;
//     }
// }
// 
// void CCandidateListUIPresenter::AdviseUIChangedByArrowKey(_In_ KEYSTROKE_FUNCTION arrowKey)
// {
//     switch (arrowKey)
//     {
//     case FUNCTION_MOVE_UP:
//         {
//             _MoveSelection(MOVEUP_ONE);
//             break;
//         }
//     case FUNCTION_MOVE_DOWN:
//         {
//             _MoveSelection(MOVEDOWN_ONE);
//             break;
//         }
//     case FUNCTION_MOVE_PAGE_UP:
//         {
//             _MovePage(MOVEUP_ONE);
//             break;
//         }
//     case FUNCTION_MOVE_PAGE_DOWN:
//         {
//             _MovePage(MOVEDOWN_ONE);
//             break;
//         }
//     case FUNCTION_MOVE_PAGE_TOP:
//         {
//             _SetSelection(MOVETO_TOP);
//             break;
//         }
//     case FUNCTION_MOVE_PAGE_BOTTOM:
//         {
//             _SetSelection(MOVETO_BOTTOM);
//             break;
//         }
//     default:
//         break;
//     }
// }
// 
// HRESULT CCandidateListUIPresenter::BeginUIElement()
// {
//     HRESULT hr = S_OK;
// 
//     auto pThreadMgr = _pTextService->_GetThreadMgr();
//     if (!pThreadMgr)
//     {
//         hr = E_FAIL;
//         goto Exit;
//     }
// 
//     {
//         ITfUIElementMgr* pUIElementMgr = nullptr;
//         hr = pThreadMgr->QueryInterface(IID_ITfUIElementMgr, (void**)&pUIElementMgr);
//         if (hr == S_OK)
//         {
//             pUIElementMgr->BeginUIElement(this, &_isShowMode, &_uiElementId);
//             pUIElementMgr->Release();
//         }
//     }
// 
// Exit:
//     return hr;
// }
// 
// HRESULT CCandidateListUIPresenter::EndUIElement()
// {
//     HRESULT hr = S_OK;
// 
//     auto pThreadMgr = _pTextService->_GetThreadMgr();
//     if (!pThreadMgr || (-1 == _uiElementId))
//     {
//         hr = E_FAIL;
//         goto Exit;
//     }
// 
//     {
//         ITfUIElementMgr* pUIElementMgr = nullptr;
//         hr = pThreadMgr->QueryInterface(IID_ITfUIElementMgr, (void**)&pUIElementMgr);
//         if (hr == S_OK)
//         {
//             pUIElementMgr->EndUIElement(_uiElementId);
//             pUIElementMgr->Release();
//         }
//     }
// 
// Exit:
//     return hr;
// }
// 
// HRESULT CCandidateListUIPresenter::MakeCandidateWindow(_In_ ITfContext *pContextDocument, _In_ UINT wndWidth)
// {
//     HRESULT hr = S_OK;
// 
//     if (nullptr != _pCandidateWnd)
//     {
//         return hr;
//     }
// 
//     _pCandidateWnd = new (std::nothrow) CCandidateWindow(_CandWndCallback, this, _pIndexRange, _pTextService->_IsStoreAppMode());
//     if (_pCandidateWnd == nullptr)
//     {
//         hr = E_OUTOFMEMORY;
//         goto Exit;
//     }
// 
//     {
//         HWND parentWndHandle = nullptr;
//         ITfContextView* pView = nullptr;
//         if (SUCCEEDED(pContextDocument->GetActiveView(&pView)))
//         {
//             pView->GetWnd(&parentWndHandle);
//         }
// 
//         if (!_pCandidateWnd->_Create(_atom, wndWidth, parentWndHandle))
//         {
//             hr = E_OUTOFMEMORY;
//             goto Exit;
//         }
//     }
// 
// Exit:
//     return hr;
// }
// 
// void CCandidateListUIPresenter::DisposeCandidateWindow()
// {
//     if (nullptr == _pCandidateWnd)
//     {
//         return;
//     }
// 
//     _pCandidateWnd->_Destroy();
// 
//     delete _pCandidateWnd;
//     _pCandidateWnd = nullptr;
// }
