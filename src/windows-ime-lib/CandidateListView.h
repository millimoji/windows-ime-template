// Copyright (c) millimoji@gmail.com
#pragma once

#include "Private.h"
#include "Globals.h"
#include "../WindowsImeLib.h"
#include "CandidateListUIPresenter.h"

struct ICandidateListViewOwner
{
    virtual HRESULT _StartLayout(_In_ ITfContext *pContextDocument, TfEditCookie ec, _In_ ITfRange *pRangeComposition) = 0;
    virtual void _EndLayout() = 0;
    virtual HRESULT _GetTextExt(TfEditCookie ec, _Out_ RECT *lpRect) = 0;
    virtual BOOL _IsStoreAppMode() = 0;
    virtual wil::com_ptr<ITfThreadMgr> _GetThreadMgr() = 0;
    virtual TfEditCookie GetCachedEditCookie() = 0;

    virtual std::shared_ptr<WindowsImeLib::ICompositionProcessorEngine> GetCompositionProcessorEngine() = 0;
};

struct ICandidateListViewInternal
{
    virtual std::shared_ptr<WindowsImeLib::IWindowsIMECandidateListView> GetClientInterface() = 0;
    virtual VOID _LayoutChangeNotification(_In_ RECT *lpRect) = 0;
    virtual VOID _LayoutDestroyNotification() = 0;
    virtual HRESULT OnSetThreadFocus() = 0;
    virtual HRESULT OnKillThreadFocus() = 0;
    virtual void _EndCandidateList() = 0;

    // both?
    virtual void DestroyView() = 0;
    virtual bool IsCreated() = 0;
};

class CandidateListView :
    public WindowsImeLib::IWindowsIMECandidateListView,
    public ICandidateListViewInternal,
    public std::enable_shared_from_this<CandidateListView>
{
public:
    CandidateListView(ICandidateListViewOwner* framework) : m_framework(framework) {}
private:
    void CreateView(_In_ std::vector<DWORD> *pIndexRange, BOOL hideWindow) override
    {
        m_presenter.reset();
        THROW_IF_FAILED(Microsoft::WRL::MakeAndInitialize<CCandidateListUIPresenter>(
            &m_presenter, m_framework, Global::AtomCandidateWindow, pIndexRange, hideWindow));
    }
private:
    // WindowsImeLib::IWindowsIMECandidateListView
    void DestroyView() override {
        m_presenter.reset();
    }
    bool IsCreated() override {
        return !!m_presenter;
    }
    HRESULT _StartCandidateList(_In_ ITfContext *pContextDocument, TfEditCookie ec, _In_ ITfRange *pRangeComposition, UINT wndWidth) override
    {
        return m_presenter->_StartCandidateList(pContextDocument, ec, pRangeComposition, wndWidth);
    }
    void _EndCandidateList() override {
        if (m_presenter) { m_presenter->_EndCandidateList(); }
    }
    void _ClearList() override {
        return m_presenter->_ClearList();
    }
    void _SetText(_In_ std::vector<CCandidateListItem> *pCandidateList, BOOL isAddFindKeyCode) override {
        return m_presenter->_SetText(pCandidateList, isAddFindKeyCode);
    }
    VOID _SetTextColor(COLORREF crColor, COLORREF crBkColor) override {
        return m_presenter->_SetTextColor(crColor, crBkColor);
    }
    VOID _SetFillColor(HBRUSH hBrush) override {
        return m_presenter->_SetFillColor(hBrush);
    }

    shared_wstring _GetSelectedCandidateString() override {
        return m_presenter->_GetSelectedCandidateString();
    }
    BOOL _SetSelectionInPage(int nPos) override {
        return m_presenter->_SetSelectionInPage(nPos);
    }

private:
    // ICandidateListViewInternal
    std::shared_ptr<WindowsImeLib::IWindowsIMECandidateListView> GetClientInterface() override {
		return std::static_pointer_cast<WindowsImeLib::IWindowsIMECandidateListView>(shared_from_this());
	}
    VOID _LayoutChangeNotification(_In_ RECT *lpRect) override {
        if (m_presenter) { return m_presenter->_LayoutChangeNotification(lpRect); }
    }
    VOID _LayoutDestroyNotification() override {
        if (m_presenter) { return m_presenter->_LayoutDestroyNotification(); }
    }
    HRESULT OnSetThreadFocus() override {
        if (m_presenter) { return m_presenter->OnSetThreadFocus(); } else { return S_OK; }
    }
    HRESULT OnKillThreadFocus() override {
        if (m_presenter) { return m_presenter->OnKillThreadFocus(); } else { return S_OK; }
    }

private:
    void AdviseUIChangedByArrowKey(_In_ CANDIDATELIST_FUNCTION arrowKey) override {
        return m_presenter->AdviseUIChangedByArrowKey(arrowKey);
    }

private:
    ICandidateListViewOwner* m_framework;
    wil::com_ptr<CCandidateListUIPresenter> m_presenter;
};
