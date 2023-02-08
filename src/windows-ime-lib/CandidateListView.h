// Copyright (c) millimoji@gmail.com
#pragma once

#include "Private.h"
#include "Globals.h"
#include "../WindowsImeLib.h"
#include "CandidateListUIPresenter.h"

struct ICandidateListViewOwner
{
    virtual HRESULT _GetLastTextExt(_Out_ HWND* documentWindow, _Out_ RECT *lpRect) = 0;
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
    // WindowsImeLib::IWindowsIMECandidateListView
//    void CreateView(_In_ std::vector<DWORD> *pIndexRange, BOOL hideWindow) override
//    {
//        m_presenter.reset();
//        THROW_IF_FAILED(Microsoft::WRL::MakeAndInitialize<CCandidateListUIPresenter>(
//            &m_presenter, m_framework, Global::AtomCandidateWindow, pIndexRange, hideWindow));
//    }
//    void DestroyView() override {
//        m_presenter.reset();
//    }
    bool IsCreated() override {
        return !!m_presenter;
    }
    HRESULT _StartCandidateList(_In_ std::vector<DWORD> *pIndexRange, UINT wndWidth) override
    {
        auto activity = WindowsImeLibTelemetry::CandidateListView_StartCandidateList::Start();

        m_presenter.reset();
        RETURN_IF_FAILED(Microsoft::WRL::MakeAndInitialize<CCandidateListUIPresenter>(
            &m_presenter, m_framework, Global::AtomCandidateWindow, pIndexRange, FALSE /*hideWindow*/));

        RETURN_IF_FAILED(m_presenter->_StartCandidateList(wndWidth));

        activity.Stop();
        return S_OK;
    }
    void _EndCandidateList() override {
        if (m_presenter) {
            auto activity = WindowsImeLibTelemetry::CandidateListView_EndCandidateList::Start();
            m_presenter->_EndCandidateList();
            m_presenter.reset();
            activity.Stop();
        }
    }
    void _ClearList() override {
        auto activity = WindowsImeLibTelemetry::CandidateListView_ClearList::Start();
        m_presenter->_ClearList();
        activity.Stop();
    }
    void _SetText(const std::vector<shared_wstring>& pCandidateList) override {
        return m_presenter->_SetText(pCandidateList);
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
    void AdviseUIChangedByArrowKey(_In_ WindowsImeLib::CANDIDATELIST_FUNCTION arrowKey) override {
        return m_presenter->AdviseUIChangedByArrowKey(arrowKey);
    }

private:
    // ICandidateListViewInternal
    std::shared_ptr<WindowsImeLib::IWindowsIMECandidateListView> GetClientInterface() override {
        return std::static_pointer_cast<WindowsImeLib::IWindowsIMECandidateListView>(shared_from_this());
    }
    VOID _LayoutChangeNotification(_In_ RECT *lpRect) override {
        if (m_presenter) {
            auto activity = WindowsImeLibTelemetry::CandidateListView_LayoutChangeNotification::Start();
            m_presenter->_LayoutChangeNotification(lpRect);
            activity.Stop();
        }
    }
    VOID _LayoutDestroyNotification() override {
        if (m_presenter) {
            auto activity = WindowsImeLibTelemetry::CandidateListView_LayoutDestroyNotification::Start();
            m_presenter->_LayoutDestroyNotification();
            activity.Stop();
        }
    }
    HRESULT OnSetThreadFocus() override {
        if (m_presenter) {
            auto activity = WindowsImeLibTelemetry::CandidateListView_OnSetThreadFocus::Start();
            const auto hr = (m_presenter ? m_presenter->OnSetThreadFocus() : S_OK);
            activity.Stop();
            return hr;
        }
        return S_OK;
    }
    HRESULT OnKillThreadFocus() override {
        if (m_presenter) {
            auto activity = WindowsImeLibTelemetry::CandidateListView_OnKillThreadFocus::Start();
            const auto hr = m_presenter->OnKillThreadFocus();
            activity.Stop();
            return hr;
        }
        return S_OK;
    }

private:
    ICandidateListViewOwner* m_framework;
    wil::com_ptr<CCandidateListUIPresenter> m_presenter;
};
