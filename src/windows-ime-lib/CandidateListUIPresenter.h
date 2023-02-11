// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


#pragma once

// #include "KeyHandlerEditSession.h"
#include "CandidateWindow.h"
#include "TfTextLayoutSink.h"
#include "BaseStructure.h"

class CReadingLine;
struct ICandidateListViewOwner;


//+---------------------------------------------------------------------------
//
// CCandidateListUIPresenter
//
// ITfCandidateListUIElement / ITfIntegratableCandidateListUIElement is used for 
// UILess mode support
// ITfCandidateListUIElementBehavior sends the Selection behavior message to 
// 3rd party IME.
//----------------------------------------------------------------------------

class CCandidateListUIPresenter :
    public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>,
                                        ITfUIElement,
                                        ITfCandidateListUIElement,
                                        ITfCandidateListUIElementBehavior,
                                        ITfIntegratableCandidateListUIElement,
                                        Microsoft::WRL::FtmBase>
{
public:
    CCandidateListUIPresenter() {}
    virtual ~CCandidateListUIPresenter();
    HRESULT RuntimeClassInitialize(_In_ ICandidateListViewOwner* pTextService, const std::shared_ptr<std::vector<DWORD>>& _pIndexRange, ATOM atom, BOOL hideWindow);

private:
    // ITfUIElement
    STDMETHODIMP GetDescription(BSTR *pbstr);
    STDMETHODIMP GetGUID(GUID *pguid);
    STDMETHODIMP Show(BOOL showCandidateWindow);
    STDMETHODIMP IsShown(BOOL *pIsShow);

    // ITfCandidateListUIElement
    STDMETHODIMP GetUpdatedFlags(DWORD *pdwFlags);
    STDMETHODIMP GetDocumentMgr(ITfDocumentMgr **ppdim);
    STDMETHODIMP GetCount(UINT *pCandidateCount);
    STDMETHODIMP GetSelection(UINT *pSelectedCandidateIndex);
    STDMETHODIMP GetString(UINT uIndex, BSTR *pbstr);
    STDMETHODIMP GetPageIndex(UINT *pIndex, UINT uSize, UINT *puPageCnt);
    STDMETHODIMP SetPageIndex(UINT *pIndex, UINT uPageCnt);
    STDMETHODIMP GetCurrentPage(UINT *puPage);

    // ITfCandidateListUIElementBehavior methods
    STDMETHODIMP SetSelection(UINT nIndex);
    STDMETHODIMP Finalize(void);
    STDMETHODIMP Abort(void);

    // ITfIntegratableCandidateListUIElement
    STDMETHODIMP SetIntegrationStyle(GUID guidIntegrationStyle);
    STDMETHODIMP GetSelectionStyle(_Out_ TfIntegratableCandidateListSelectionStyle *ptfSelectionStyle);
    STDMETHODIMP OnKeyDown(_In_ WPARAM wParam, _In_ LPARAM lParam, _Out_ BOOL *pIsEaten);
    STDMETHODIMP ShowCandidateNumbers(_Out_ BOOL *pIsShow); 
    STDMETHODIMP FinalizeExactCompositionString();

public: // WindowsImeLib::IWindowsIMECandidateListView
    // transfer from parent class
    HRESULT _StartCandidateList(UINT wndWidth);
    void _EndCandidateList();

    void _SetText(const std::vector<shared_wstring>& pCandidateList);
    void _ClearList();
    VOID _SetTextColor(COLORREF crColor, COLORREF crBkColor);
    VOID _SetFillColor(HBRUSH hBrush);

    shared_wstring _GetSelectedCandidateString();

    BOOL _SetSelectionInPage(int nPos) { return _pCandidateWnd->_SetSelectionInPage(nPos); }

    BOOL _MoveSelection(_In_ int offSet);
    BOOL _SetSelection(_In_ int selectedIndex);
    BOOL _MovePage(_In_ int offSet);

    void _MoveWindowToTextExt();

    // CTfTextLayoutSink
    virtual VOID _LayoutChangeNotification(_In_ RECT *lpRect);

    // Event for ITfThreadFocusSink
    virtual HRESULT OnSetThreadFocus();
    virtual HRESULT OnKillThreadFocus();

    void AdviseUIChangedByArrowKey(_In_ WindowsImeLib::CANDIDATELIST_FUNCTION arrowKey);

private:
    virtual HRESULT CALLBACK _CandidateChangeNotification(_In_ enum CANDWND_ACTION action);

    static HRESULT _CandWndCallback(_In_ void *pv, _In_ enum CANDWND_ACTION action);

    HRESULT _UpdateUIElement();

    HRESULT ToShowCandidateWindow();
    HRESULT ToHideCandidateWindow();

    HRESULT BeginUIElement();
    HRESULT EndUIElement();

    HRESULT MakeCandidateWindow(UINT wndWidth, HWND parentWndHandle);
    void DisposeCandidateWindow();
    void AddCandidateToCandidateListUI(const std::vector<shared_wstring>& pCandidateList);
    void SetPageIndexWithScrollInfo(const std::vector<shared_wstring>& pCandidateList);

protected:
    CCandidateWindow* _pCandidateWnd = {};
    BOOL _isShowMode = {};
    BOOL _hideWindow = {};

private:
    HWND _parentWndHandle = {};
    ATOM _atom = {};
    std::shared_ptr<std::vector<DWORD>> _pIndexRange;
    DWORD _updatedFlags = {};
    DWORD _uiElementId = {};
    ICandidateListViewOwner* _pTextService = {};
};
