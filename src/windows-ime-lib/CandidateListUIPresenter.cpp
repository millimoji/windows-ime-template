// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "Globals.h"
#include "CandidateListUIPresenter.h"
#include "CandidateListView.h"

//////////////////////////////////////////////////////////////////////
//
// CWindowsIME candidate key handler methods
//
//////////////////////////////////////////////////////////////////////

const int MOVEUP_ONE = -1;
const int MOVEDOWN_ONE = 1;
const int MOVETO_TOP = 0;
const int MOVETO_BOTTOM = -1;

// //+---------------------------------------------------------------------------
// //
// // _HandleCandidateConvert
// //
// //----------------------------------------------------------------------------
// 
// HRESULT CompositionBuffer::_HandleCandidateConvert(TfEditCookie ec, _In_ ITfContext *pContext)
// {
//     return _HandleCandidateWorker(ec, pContext);
// }
// 
// //+---------------------------------------------------------------------------
// //
// // _HandleCandidateArrowKey
// //
// //----------------------------------------------------------------------------
// 
// HRESULT CompositionBuffer::_HandleCandidateArrowKey(TfEditCookie ec, _In_ ITfContext *pContext, _In_ KEYSTROKE_FUNCTION keyFunction)
// {
//     ec;
//     pContext;
// 
//     _pCandidateListUIPresenter->AdviseUIChangedByArrowKey(keyFunction);
// 
//     return S_OK;
// }
// 
// //+---------------------------------------------------------------------------
// //
// // _HandleCandidateSelectByNumber
// //
// //----------------------------------------------------------------------------
// 
// inline int FindVkInVector(const std::vector<DWORD>& srcVkList, UINT vk)
// {
//     bool isVkNumpad = (VK_NUMPAD0 <= vk) && (vk <= VK_NUMPAD9);
// 
//     for (auto it = srcVkList.begin(); it != srcVkList.end(); ++it)
//     {
//         if ((*it == vk) || (isVkNumpad && (*it == (vk - VK_NUMPAD0))))
//         {
//             return static_cast<int>(std::distance(srcVkList.begin(), it));
//         }
//     }
//     return -1;
// }

// //+---------------------------------------------------------------------------
// //
// // _HandlePhraseArrowKey
// //
// //----------------------------------------------------------------------------
// 
// HRESULT CompositionBuffer::_HandlePhraseArrowKey(TfEditCookie ec, _In_ ITfContext *pContext, _In_ KEYSTROKE_FUNCTION keyFunction)
// {
//     ec;
//     pContext;
// 
//     _pCandidateListUIPresenter->AdviseUIChangedByArrowKey(keyFunction);
// 
//     return S_OK;
// }

//////////////////////////////////////////////////////////////////////
//
// CCandidateListUIPresenter class
//
//////////////////////////////////////////////////////////////////////

//+---------------------------------------------------------------------------
//
// ctor
//
//----------------------------------------------------------------------------

HRESULT CCandidateListUIPresenter::RuntimeClassInitialize(_In_ ICandidateListViewOwner* pTextService, const std::shared_ptr<std::vector<DWORD>>& pIndexRange, ATOM atom, BOOL hideWindow)
    // : CTfTextLayoutSink(pTextService)
{
    _atom = atom;

    _pIndexRange = pIndexRange;

    _parentWndHandle = nullptr;
    _pCandidateWnd = nullptr;

    _updatedFlags = 0;

    _uiElementId = (DWORD)-1;
    _isShowMode = TRUE;   // store return value from BeginUIElement
    _hideWindow = hideWindow;     // Hide window flag from [Configuration] CandidateList.Phrase.HideWindow

    _pTextService = pTextService;
//    _pTextService->AddRef();

    // _refCount = 1;
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// dtor
//
//----------------------------------------------------------------------------

CCandidateListUIPresenter::~CCandidateListUIPresenter()
{
    _EndCandidateList();
//    _pTextService->Release();
}

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

//+---------------------------------------------------------------------------
//
// ITfCandidateListUIElement::ITfUIElement::GetDescription
//
//----------------------------------------------------------------------------

STDAPI CCandidateListUIPresenter::GetDescription(BSTR *pbstr)
{
    if (pbstr)
    {
        *pbstr = SysAllocString(L"Cand");
    }
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfCandidateListUIElement::ITfUIElement::GetGUID
//
//----------------------------------------------------------------------------

STDAPI CCandidateListUIPresenter::GetGUID(GUID *pguid)
{
    *pguid = WindowsImeLib::g_processorFactory->GetConstantProvider()->CandUIElement();
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfCandidateListUIElement::ITfUIElement::Show
//
//----------------------------------------------------------------------------

STDAPI CCandidateListUIPresenter::Show(BOOL showCandidateWindow)
{
    if (showCandidateWindow)
    {
        ToShowCandidateWindow();
    }
    else
    {
        ToHideCandidateWindow();
    }
    return S_OK;
}

HRESULT CCandidateListUIPresenter::ToShowCandidateWindow()
{
    if (_hideWindow)
    {
        _pCandidateWnd->_Show(FALSE);
    }
    else
    {
        _MoveWindowToTextExt();

        _pCandidateWnd->_Show(TRUE);
    }

    return S_OK;
}

HRESULT CCandidateListUIPresenter::ToHideCandidateWindow()
{
    if (_pCandidateWnd)
    {
        _pCandidateWnd->_Show(FALSE);
    }

    _updatedFlags = TF_CLUIE_SELECTION | TF_CLUIE_CURRENTPAGE;
    _UpdateUIElement();

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfCandidateListUIElement::ITfUIElement::IsShown
//
//----------------------------------------------------------------------------

STDAPI CCandidateListUIPresenter::IsShown(BOOL *pIsShow)
{
    *pIsShow = _pCandidateWnd->_IsWindowVisible();
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfCandidateListUIElement::GetUpdatedFlags
//
//----------------------------------------------------------------------------

STDAPI CCandidateListUIPresenter::GetUpdatedFlags(DWORD *pdwFlags)
{
    *pdwFlags = _updatedFlags;
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfCandidateListUIElement::GetDocumentMgr
//
//----------------------------------------------------------------------------

STDAPI CCandidateListUIPresenter::GetDocumentMgr(ITfDocumentMgr **ppdim)
{
    *ppdim = nullptr;

    return E_NOTIMPL;
}

//+---------------------------------------------------------------------------
//
// ITfCandidateListUIElement::GetCount
//
//----------------------------------------------------------------------------

STDAPI CCandidateListUIPresenter::GetCount(UINT *pCandidateCount)
{
    if (_pCandidateWnd)
    {
        *pCandidateCount = _pCandidateWnd->_GetCount();
    }
    else
    {
        *pCandidateCount = 0;
    }
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfCandidateListUIElement::GetSelection
//
//----------------------------------------------------------------------------

STDAPI CCandidateListUIPresenter::GetSelection(UINT *pSelectedCandidateIndex)
{
    if (_pCandidateWnd)
    {
        *pSelectedCandidateIndex = _pCandidateWnd->_GetSelection();
    }
    else
    {
        *pSelectedCandidateIndex = 0;
    }
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfCandidateListUIElement::GetString
//
//----------------------------------------------------------------------------

STDAPI CCandidateListUIPresenter::GetString(UINT uIndex, BSTR *pbstr)
{
    if (!_pCandidateWnd || (uIndex > _pCandidateWnd->_GetCount()))
    {
        return E_FAIL;
    }

    const auto candidateString = _pCandidateWnd->_GetCandidateString(uIndex);

    *pbstr = candidateString->empty() ? nullptr : SysAllocStringLen(candidateString->c_str(), static_cast<UINT>(candidateString->length()));

    return S_OK;
//    DWORD candidateLen = 0;
//    const WCHAR* pCandidateString = nullptr;
//
//    candidateLen = _pCandidateWnd->_GetCandidateString(uIndex, &pCandidateString);
//
//    *pbstr = (candidateLen == 0) ? nullptr : SysAllocStringLen(pCandidateString, candidateLen);
//
//    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfCandidateListUIElement::GetPageIndex
//
//----------------------------------------------------------------------------

STDAPI CCandidateListUIPresenter::GetPageIndex(UINT *pIndex, UINT uSize, UINT *puPageCnt)
{
    if (!_pCandidateWnd)
    {
        if (pIndex)
        {
            *pIndex = 0;
        }
        *puPageCnt = 0;
        return S_OK;
    }
    return _pCandidateWnd->_GetPageIndex(pIndex, uSize, puPageCnt);
}

//+---------------------------------------------------------------------------
//
// ITfCandidateListUIElement::SetPageIndex
//
//----------------------------------------------------------------------------

STDAPI CCandidateListUIPresenter::SetPageIndex(UINT *pIndex, UINT uPageCnt)
{
    if (!_pCandidateWnd)
    {
        return E_FAIL;
    }
    return _pCandidateWnd->_SetPageIndex(pIndex, uPageCnt);
}

//+---------------------------------------------------------------------------
//
// ITfCandidateListUIElement::GetCurrentPage
//
//----------------------------------------------------------------------------

STDAPI CCandidateListUIPresenter::GetCurrentPage(UINT *puPage)
{
    if (!_pCandidateWnd)
    {
        *puPage = 0;
        return S_OK;
    }
    return _pCandidateWnd->_GetCurrentPage(puPage);
}

//+---------------------------------------------------------------------------
//
// ITfCandidateListUIElementBehavior::SetSelection
// It is related of the mouse clicking behavior upon the suggestion window
//----------------------------------------------------------------------------

STDAPI CCandidateListUIPresenter::SetSelection(UINT nIndex)
{
    if (_pCandidateWnd)
    {
        _pCandidateWnd->_SetSelection(nIndex);
    }

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfCandidateListUIElementBehavior::Finalize
// It is related of the mouse clicking behavior upon the suggestion window
//----------------------------------------------------------------------------

STDAPI CCandidateListUIPresenter::Finalize(void)
{
    _CandidateChangeNotification(CAND_ITEM_SELECT);
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfCandidateListUIElementBehavior::Abort
//
//----------------------------------------------------------------------------

STDAPI CCandidateListUIPresenter::Abort(void)
{
    return E_NOTIMPL;
}

//+---------------------------------------------------------------------------
//
// ITfIntegratableCandidateListUIElement::SetIntegrationStyle
// To show candidateNumbers on the suggestion window
//----------------------------------------------------------------------------

STDAPI CCandidateListUIPresenter::SetIntegrationStyle(GUID guidIntegrationStyle)
{
    return (guidIntegrationStyle == GUID_INTEGRATIONSTYLE_SEARCHBOX) ? S_OK : E_NOTIMPL;
}

//+---------------------------------------------------------------------------
//
// ITfIntegratableCandidateListUIElement::GetSelectionStyle
//
//----------------------------------------------------------------------------

STDAPI CCandidateListUIPresenter::GetSelectionStyle(_Out_ TfIntegratableCandidateListSelectionStyle *ptfSelectionStyle)
{
    *ptfSelectionStyle = STYLE_ACTIVE_SELECTION;
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfIntegratableCandidateListUIElement::OnKeyDown
//
//----------------------------------------------------------------------------

STDAPI CCandidateListUIPresenter::OnKeyDown(_In_ WPARAM wParam, _In_ LPARAM lParam, _Out_ BOOL *pIsEaten)
{
    wParam;
    lParam;

    *pIsEaten = TRUE;
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfIntegratableCandidateListUIElement::ShowCandidateNumbers
//
//----------------------------------------------------------------------------

STDAPI CCandidateListUIPresenter::ShowCandidateNumbers(_Out_ BOOL *pIsShow)
{
    *pIsShow = TRUE;
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfIntegratableCandidateListUIElement::FinalizeExactCompositionString
//
//----------------------------------------------------------------------------

STDAPI CCandidateListUIPresenter::FinalizeExactCompositionString()
{
    return E_NOTIMPL;
}


//+---------------------------------------------------------------------------
//
// _StartCandidateList
//
//----------------------------------------------------------------------------

HRESULT CCandidateListUIPresenter::_StartCandidateList(UINT wndWidth)
{
    HWND hwnd;
    RECT rcTextExt;
    RETURN_IF_FAILED(_pTextService->_GetLastTextExt(&hwnd, &rcTextExt));

    RETURN_IF_FAILED(BeginUIElement());

    RETURN_IF_FAILED(MakeCandidateWindow(wndWidth, hwnd));

    Show(_isShowMode);

    _LayoutChangeNotification(&rcTextExt);

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _EndCandidateList
//
//----------------------------------------------------------------------------

void CCandidateListUIPresenter::_EndCandidateList()
{
    EndUIElement();

    DisposeCandidateWindow();

//    _pTextService->_EndLayoutTracking();
}

//+---------------------------------------------------------------------------
//
// _SetText
//
//----------------------------------------------------------------------------

void CCandidateListUIPresenter::_SetText(const std::vector<shared_wstring>& pCandidateList)
{
    AddCandidateToCandidateListUI(pCandidateList);

    SetPageIndexWithScrollInfo(pCandidateList);

    if (_isShowMode)
    {
        _pCandidateWnd->_InvalidateRect();
    }
    else
    {
        _updatedFlags = TF_CLUIE_COUNT       |
            TF_CLUIE_SELECTION   |
            TF_CLUIE_STRING      |
            TF_CLUIE_PAGEINDEX   |
            TF_CLUIE_CURRENTPAGE;
        _UpdateUIElement();
    }
}

void CCandidateListUIPresenter::AddCandidateToCandidateListUI(const std::vector<shared_wstring>& pCandidateList)
{
    for (UINT index = 0; index < pCandidateList.size(); index++)
    {
        _pCandidateWnd->_AddString(pCandidateList.at(index));
    }
}

void CCandidateListUIPresenter::SetPageIndexWithScrollInfo(const std::vector<shared_wstring>& pCandidateList)
{
    UINT candCntInPage = static_cast<UINT>(_pIndexRange->size());
    UINT bufferSize = static_cast<UINT>(pCandidateList.size() / candCntInPage + 1);
    UINT* puPageIndex = new (std::nothrow) UINT[ bufferSize ];
    if (puPageIndex != nullptr)
    {
        for (UINT i = 0; i < bufferSize; i++)
        {
            puPageIndex[i] = i * candCntInPage;
        }

        _pCandidateWnd->_SetPageIndex(puPageIndex, bufferSize);
        delete [] puPageIndex;
    }
    _pCandidateWnd->_SetScrollInfo(static_cast<int>(pCandidateList.size()), candCntInPage);  // nMax:range of max, nPage:number of items in page

}
//+---------------------------------------------------------------------------
//
// _ClearList
//
//----------------------------------------------------------------------------

void CCandidateListUIPresenter::_ClearList()
{
    _pCandidateWnd->_ClearList();
    _pCandidateWnd->_InvalidateRect();
}

//+---------------------------------------------------------------------------
//
// _SetTextColor
// _SetFillColor
//
//----------------------------------------------------------------------------

void CCandidateListUIPresenter::_SetTextColor(COLORREF crColor, COLORREF crBkColor)
{
    _pCandidateWnd->_SetTextColor(crColor, crBkColor);
}

void CCandidateListUIPresenter::_SetFillColor(HBRUSH hBrush)
{
    _pCandidateWnd->_SetFillColor(hBrush);
}

//+---------------------------------------------------------------------------
//
// _GetSelectedCandidateString
//
//----------------------------------------------------------------------------

shared_wstring CCandidateListUIPresenter::_GetSelectedCandidateString()
{
    return _pCandidateWnd->_GetSelectedCandidateString();
}

//+---------------------------------------------------------------------------
//
// _MoveSelection
//
//----------------------------------------------------------------------------

BOOL CCandidateListUIPresenter::_MoveSelection(_In_ int offSet)
{
    BOOL ret = _pCandidateWnd->_MoveSelection(offSet, TRUE);
    if (ret)
    {
        if (_isShowMode)
        {
            _pCandidateWnd->_InvalidateRect();
        }
        else
        {
            _updatedFlags = TF_CLUIE_SELECTION;
            _UpdateUIElement();
        }
    }
    return ret;
}

//+---------------------------------------------------------------------------
//
// _SetSelection
//
//----------------------------------------------------------------------------

BOOL CCandidateListUIPresenter::_SetSelection(_In_ int selectedIndex)
{
    BOOL ret = _pCandidateWnd->_SetSelection(selectedIndex, TRUE);
    if (ret)
    {
        if (_isShowMode)
        {
            _pCandidateWnd->_InvalidateRect();
        }
        else
        {
            _updatedFlags = TF_CLUIE_SELECTION |
                TF_CLUIE_CURRENTPAGE;
            _UpdateUIElement();
        }
    }
    return ret;
}

//+---------------------------------------------------------------------------
//
// _MovePage
//
//----------------------------------------------------------------------------

BOOL CCandidateListUIPresenter::_MovePage(_In_ int offSet)
{
    BOOL ret = _pCandidateWnd->_MovePage(offSet, TRUE);
    if (ret)
    {
        if (_isShowMode)
        {
            _pCandidateWnd->_InvalidateRect();
        }
        else
        {
            _updatedFlags = TF_CLUIE_SELECTION |
                TF_CLUIE_CURRENTPAGE;
            _UpdateUIElement();
        }
    }
    return ret;
}

//+---------------------------------------------------------------------------
//
// _MoveWindowToTextExt
//
//----------------------------------------------------------------------------

void CCandidateListUIPresenter::_MoveWindowToTextExt()
{
    HWND hwnd;
    RECT rc;

    if (FAILED(_pTextService->_GetLastTextExt(&hwnd, &rc)))
    {
        return;
    }

    _pCandidateWnd->_Move(rc.left, rc.bottom);
}
//+---------------------------------------------------------------------------
//
// _LayoutChangeNotification
//
//----------------------------------------------------------------------------

VOID CCandidateListUIPresenter::_LayoutChangeNotification(_In_ RECT *lpRect)
{
    RECT rectCandidate = {0, 0, 0, 0};
    POINT ptCandidate = {0, 0};

    _pCandidateWnd->_GetClientRect(&rectCandidate);
    _pCandidateWnd->_GetWindowExtent(lpRect, &rectCandidate, &ptCandidate);
    _pCandidateWnd->_Move(ptCandidate.x, ptCandidate.y);
}

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

//+---------------------------------------------------------------------------
//
// _CandidateChangeNotifiction
//
//----------------------------------------------------------------------------

HRESULT CCandidateListUIPresenter::_CandidateChangeNotification(_In_ enum CANDWND_ACTION action)
{
    RETURN_HR_IF(S_OK, CAND_ITEM_SELECT != action);

    _pTextService->NotifyFinalizeCandidateList();

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _CandWndCallback
//
//----------------------------------------------------------------------------

// static
HRESULT CCandidateListUIPresenter::_CandWndCallback(_In_ void *pv, _In_ enum CANDWND_ACTION action)
{
    CCandidateListUIPresenter* fakeThis = (CCandidateListUIPresenter*)pv;

    return fakeThis->_CandidateChangeNotification(action);
}

//+---------------------------------------------------------------------------
//
// _UpdateUIElement
//
//----------------------------------------------------------------------------

HRESULT CCandidateListUIPresenter::_UpdateUIElement()
{
    HRESULT hr = S_OK;

    auto pThreadMgr = _pTextService->_GetThreadMgr();
    if (!pThreadMgr)
    {
        return S_OK;
    }

    ITfUIElementMgr* pUIElementMgr = nullptr;

    hr = pThreadMgr->QueryInterface(IID_ITfUIElementMgr, (void **)&pUIElementMgr);
    if (hr == S_OK)
    {
        pUIElementMgr->UpdateUIElement(_uiElementId);
        pUIElementMgr->Release();
    }

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// OnSetThreadFocus
//
//----------------------------------------------------------------------------

HRESULT CCandidateListUIPresenter::OnSetThreadFocus()
{
    if (_isShowMode)
    {
        Show(TRUE);
    }
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// OnKillThreadFocus
//
//----------------------------------------------------------------------------

HRESULT CCandidateListUIPresenter::OnKillThreadFocus()
{
    if (_isShowMode)
    {
        Show(FALSE);
    }
    return S_OK;
}

void CCandidateListUIPresenter::AdviseUIChangedByArrowKey(_In_ WindowsImeLib::CANDIDATELIST_FUNCTION arrowKey)
{
    switch (arrowKey)
    {
    case WindowsImeLib::CANDIDATELIST_FUNCTION::MOVE_UP:
        {
            _MoveSelection(MOVEUP_ONE);
            break;
        }
    case WindowsImeLib::CANDIDATELIST_FUNCTION::MOVE_DOWN:
        {
            _MoveSelection(MOVEDOWN_ONE);
            break;
        }
    case WindowsImeLib::CANDIDATELIST_FUNCTION::MOVE_PAGE_UP:
        {
            _MovePage(MOVEUP_ONE);
            break;
        }
    case WindowsImeLib::CANDIDATELIST_FUNCTION::MOVE_PAGE_DOWN:
        {
            _MovePage(MOVEDOWN_ONE);
            break;
        }
    case WindowsImeLib::CANDIDATELIST_FUNCTION::MOVE_PAGE_TOP:
        {
            _SetSelection(MOVETO_TOP);
            break;
        }
    case WindowsImeLib::CANDIDATELIST_FUNCTION::MOVE_PAGE_BOTTOM:
        {
            _SetSelection(MOVETO_BOTTOM);
            break;
        }
    default:
        break;
    }
}

HRESULT CCandidateListUIPresenter::BeginUIElement()
{
    HRESULT hr = S_OK;

    auto pThreadMgr = _pTextService->_GetThreadMgr();
    if (!pThreadMgr)
    {
        hr = S_OK; // E_FAIL;
        goto Exit;
    }

    {
        ITfUIElementMgr* pUIElementMgr = nullptr;
        hr = pThreadMgr->QueryInterface(IID_ITfUIElementMgr, (void**)&pUIElementMgr);
        if (hr == S_OK)
        {
            wil::com_ptr<ITfUIElement> uiElement;
            RETURN_IF_FAILED(QueryInterface(IID_PPV_ARGS(&uiElement)));
            pUIElementMgr->BeginUIElement(uiElement.get(), &_isShowMode, &_uiElementId);
            pUIElementMgr->Release();
        }
    }

Exit:
    return hr;
}

HRESULT CCandidateListUIPresenter::EndUIElement()
{
    HRESULT hr = S_OK;

    auto pThreadMgr = _pTextService->_GetThreadMgr();
    if (!pThreadMgr || (-1 == _uiElementId))
    {
        hr = S_OK; // E_FAIL;
        goto Exit;
    }

    {
        ITfUIElementMgr* pUIElementMgr = nullptr;
        hr = pThreadMgr->QueryInterface(IID_ITfUIElementMgr, (void**)&pUIElementMgr);
        if (hr == S_OK)
        {
            pUIElementMgr->EndUIElement(_uiElementId);
            pUIElementMgr->Release();
        }
    }

Exit:
    return hr;
}

HRESULT CCandidateListUIPresenter::MakeCandidateWindow(UINT wndWidth, HWND parentWndHandle)
{
    HRESULT hr = S_OK;

    if (nullptr != _pCandidateWnd)
    {
        return hr;
    }

    _pCandidateWnd = new (std::nothrow) CCandidateWindow(_CandWndCallback, this, _pIndexRange, _pTextService->_IsStoreAppMode());
    if (_pCandidateWnd == nullptr)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    {
//        HWND parentWndHandle = nullptr;
//        ITfContextView* pView = nullptr;
//        if (SUCCEEDED(pContextDocument->GetActiveView(&pView)))
//        {
//            pView->GetWnd(&parentWndHandle);
//        }

        if (!_pCandidateWnd->_Create(_atom, wndWidth, parentWndHandle))
        {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }
    }

Exit:
    return hr;
}

void CCandidateListUIPresenter::DisposeCandidateWindow()
{
    if (nullptr == _pCandidateWnd)
    {
        return;
    }

    _pCandidateWnd->_Destroy();

    delete _pCandidateWnd;
    _pCandidateWnd = nullptr;
}