// Copyright (c) millimoji@gmail.com
#pragma once

#include "Private.h"
#include "Globals.h"
#include "../WindowsImeLib.h"
#include "CandidateListUIPresenter.h"
#include "ThreadTaskRunner.h"

struct ICandidateListViewOwner
{
    virtual HRESULT _GetLastTextExt(_Out_ HWND* documentWindow, _Out_ RECT *lpRect) = 0;
    virtual BOOL _IsStoreAppMode() = 0;
    virtual void NotifyFinalizeCandidateList() = 0;

    virtual wil::com_ptr<ITfThreadMgr> _GetThreadMgr() = 0; // for ITfCandidateListUIElement
};

struct ICandidateListViewInternal
{
    virtual std::shared_ptr<WindowsImeLib::IWindowsIMECandidateListView> GetClientInterface() = 0;
    virtual VOID _LayoutChangeNotification(HWND hwnd, _In_ RECT *lpRect) = 0;
    virtual void _EndCandidateList() = 0;
    virtual HRESULT OnSetThreadFocus() = 0;
    virtual HRESULT OnKillThreadFocus() = 0;

    // both?
    virtual bool IsCreated() = 0;
};

class CandidateListView :
    public WindowsImeLib::IWindowsIMECandidateListView,
    public ICandidateListViewInternal,
    public std::enable_shared_from_this<CandidateListView>
{
public:
    CandidateListView(ICandidateListViewOwner* framework) :
        m_framework(framework)
    {
        m_indexRange = std::make_shared<std::vector<DWORD>>();
    }
private:
    // WindowsImeLib::IWindowsIMECandidateListView
    bool IsCreated() override {
        return !!m_presenter;
    }
    HRESULT _StartCandidateList(UINT wndWidth) override
    {
        auto activity = WindowsImeLibTelemetry::CandidateListView_StartCandidateList::Start();

        m_indexRange->clear();
        for (DWORD i = 1; i <= 10; ++i) { m_indexRange->emplace_back(i % 10); };

        m_taskRunner.RunOnThread([&]()
        {
            m_presenter.reset();
            LOG_IF_FAILED(Microsoft::WRL::MakeAndInitialize<CCandidateListUIPresenter>(
                &m_presenter, m_framework, m_indexRange, Global::AtomCandidateWindow, FALSE /*hideWindow*/));

            LOG_IF_FAILED(m_presenter->_StartCandidateList(wndWidth));
        });
        activity.Stop();
        return S_OK;
    }
    void _EndCandidateList() override {
        m_taskRunner.RunOnThread([&]()
        {
            if (m_presenter) {
                auto activity = WindowsImeLibTelemetry::CandidateListView_EndCandidateList::Start();
                m_presenter->_EndCandidateList();
                m_presenter.reset();
                activity.Stop();
            }
        });
    }
    void _SetText(const std::vector<shared_wstring>& pCandidateList) override {
        m_taskRunner.RunOnThread([&]()
        {
            m_presenter->_ClearList();
            if (!pCandidateList.empty()) {
                m_presenter->_SetText(pCandidateList);
            }
        });
    }
    void _SetTextColorAndFillColor(WindowsImeLib::CANDIDATE_COLOR_STYLE colorStyle) override {
        m_taskRunner.RunOnThread([&]()
        {
            switch (colorStyle) {
            case WindowsImeLib::CANDIDATE_COLOR_STYLE::DEFAULT:
                m_presenter->_SetTextColor(CANDWND_ITEM_COLOR, GetSysColor(COLOR_WINDOW));
                m_presenter->_SetFillColor((HBRUSH)(COLOR_WINDOW+1));
                break;
            case WindowsImeLib::CANDIDATE_COLOR_STYLE::GREEN:
                m_presenter->_SetTextColor(RGB(0, 0x80, 0), GetSysColor(COLOR_WINDOW));
                m_presenter->_SetFillColor((HBRUSH)(COLOR_WINDOW+1));
                break;
            }
        });
    }
    shared_wstring _GetSelectedCandidateString() override {
        return m_presenter->_GetSelectedCandidateString();
    }
    std::shared_ptr<std::vector<DWORD>> GetCandidateListRange() override {
        return m_indexRange;
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
    VOID _LayoutChangeNotification(HWND /*hwnd*/, _In_ RECT *lpRect) override {
        m_taskRunner.RunOnThread([&]()
        {
            if (m_presenter) {
                auto activity = WindowsImeLibTelemetry::CandidateListView_LayoutChangeNotification::Start();
                m_presenter->_LayoutChangeNotification(lpRect);
                activity.Stop();
            }
        });
    }
    HRESULT OnSetThreadFocus() override {
        m_taskRunner.RunOnThread([&]()
        {
            if (m_presenter) {
                auto activity = WindowsImeLibTelemetry::CandidateListView_OnSetThreadFocus::Start();
                //const auto hr = (m_presenter ? m_presenter->OnSetThreadFocus() : S_OK);
                (void)m_presenter->OnSetThreadFocus();
                activity.Stop();
                // return hr;
            }
            // return S_OK;
        });
        return S_OK;
    }
    HRESULT OnKillThreadFocus() override {
        m_taskRunner.RunOnThread([&]()
        {
            if (m_presenter) {
                auto activity = WindowsImeLibTelemetry::CandidateListView_OnKillThreadFocus::Start();
                // const auto hr = m_presenter->OnKillThreadFocus();
                (void)m_presenter->OnKillThreadFocus();
                activity.Stop();
                // return hr;
            }
            // return S_OK;
        });
        return S_OK;
    }

private:
    std::shared_ptr<std::vector<DWORD>> m_indexRange;
    ICandidateListViewOwner* m_framework;
    wil::com_ptr<CCandidateListUIPresenter> m_presenter;
    UIThreadTaskRunner m_taskRunner;
};
