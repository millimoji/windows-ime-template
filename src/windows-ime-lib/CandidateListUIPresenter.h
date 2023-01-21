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
#include "WindowsIME.h"
#include "BaseStructure.h"

class CReadingLine;
struct IInternalFrameworkService;

//+---------------------------------------------------------------------------
//
// CCandidateListUIPresenter
//
// ITfCandidateListUIElement / ITfIntegratableCandidateListUIElement is used for 
// UILess mode support
// ITfCandidateListUIElementBehavior sends the Selection behavior message to 
// 3rd party IME.
//----------------------------------------------------------------------------

class CCandidateListUIPresenter : public CTfTextLayoutSink,
    public ITfCandidateListUIElementBehavior,
    public ITfIntegratableCandidateListUIElement
{
public:
    CCandidateListUIPresenter(_In_ IInternalFrameworkService *pTextService, ATOM atom,
        _In_ std::vector<DWORD> *pIndexRange,
        BOOL hideWindow);
    virtual ~CCandidateListUIPresenter();

public:
    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, _Outptr_ void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

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
    ITfContext* _GetContextDocument() { return CTfTextLayoutSink::_GetContextDocument(); };

    HRESULT _StartCandidateList(_In_ ITfContext *pContextDocument, TfEditCookie ec, _In_ ITfRange *pRangeComposition, UINT wndWidth);
    void _EndCandidateList();

    void _SetText(_In_ std::vector<CCandidateListItem> *pCandidateList, BOOL isAddFindKeyCode) ;
    void _ClearList();
    VOID _SetTextColor(COLORREF crColor, COLORREF crBkColor);
    VOID _SetFillColor(HBRUSH hBrush);

    DWORD_PTR _GetSelectedCandidateString(_Outptr_result_maybenull_ const WCHAR **ppwchCandidateString);

    BOOL _SetSelectionInPage(int nPos) { return _pCandidateWnd->_SetSelectionInPage(nPos); }

    BOOL _MoveSelection(_In_ int offSet);
    BOOL _SetSelection(_In_ int selectedIndex);
    BOOL _MovePage(_In_ int offSet);

    void _MoveWindowToTextExt(TfEditCookie ec);

    // CTfTextLayoutSink
    virtual VOID _LayoutChangeNotification(_In_ RECT *lpRect);
    virtual VOID _LayoutDestroyNotification();

    // Event for ITfThreadFocusSink
    virtual HRESULT OnSetThreadFocus();
    virtual HRESULT OnKillThreadFocus();

    void RemoveSpecificCandidateFromList(_In_ LCID Locale, _Inout_ std::vector<CCandidateListItem> &candidateList, _In_ CStringRange &srgCandidateString);
    void AdviseUIChangedByArrowKey(_In_ CANDIDATELIST_FUNCTION arrowKey);

private:
    virtual HRESULT CALLBACK _CandidateChangeNotification(_In_ enum CANDWND_ACTION action);

    static HRESULT _CandWndCallback(_In_ void *pv, _In_ enum CANDWND_ACTION action);

    HRESULT _UpdateUIElement();

    HRESULT ToShowCandidateWindow();
    HRESULT ToHideCandidateWindow();

    HRESULT BeginUIElement();
    HRESULT EndUIElement();

    HRESULT MakeCandidateWindow(_In_ ITfContext *pContextDocument, _In_ UINT wndWidth);
    void DisposeCandidateWindow();

    void AddCandidateToCandidateListUI(_In_ std::vector<CCandidateListItem> *pCandidateList, BOOL isAddFindKeyCode);

    void SetPageIndexWithScrollInfo(_In_ std::vector<CCandidateListItem> *pCandidateList);

protected:
    CCandidateWindow *_pCandidateWnd;
    BOOL _isShowMode;
    BOOL _hideWindow;

private:

    HWND _parentWndHandle;
    ATOM _atom;
    std::vector<DWORD>* _pIndexRange;
    DWORD _updatedFlags;
    DWORD _uiElementId;
    IInternalFrameworkService* _pTextService;
    // LONG _refCount;
};
