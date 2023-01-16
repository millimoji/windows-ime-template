// Copyright (c) millimoji@gmail.com
#pragma once

#include "Private.h"
#include "Globals.h"
#include "../WindowsImeLib.h"
#include "WindowsIME.h"
#include "CandidateListUIPresenter.h"

class CandidateListView :
    public WindowsImeLib::IWindowsIMECandidateListView,
    public std::enable_shared_from_this<CandidateListView>
{
public:
    CandidateListView(IInternalFrameworkService* framework) : m_framework(framework) {}
private:
    void CreateView(ATOM atom, KEYSTROKE_CATEGORY Category, _In_ std::vector<DWORD> *pIndexRange, BOOL hideWindow) override
    {
        auto textService = reinterpret_cast<CWindowsIME*>(m_framework->GetTextService());
        m_mainPresenter.reset();
        m_mainPresenter.attach(new CCandidateListUIPresenter(textService, atom, Category, pIndexRange, hideWindow));
        m_presenter = static_cast<WindowsImeLib::IWindowsIMECandidateListView*>(m_mainPresenter.get());
    }
    void DestroyView() override {
        m_mainPresenter.reset();
    }
    bool IsCreated() override {
        return !!m_mainPresenter;
    }
    ITfContext* _GetContextDocument() override {
        return m_presenter->_GetContextDocument();
    }
    HRESULT _StartCandidateList(TfClientId tfClientId, _In_ ITfDocumentMgr *pDocumentMgr, _In_ ITfContext *pContextDocument, TfEditCookie ec, _In_ ITfRange *pRangeComposition, UINT wndWidth) override
    {
        return m_presenter->_StartCandidateList(tfClientId, pDocumentMgr, pContextDocument, ec, pRangeComposition, wndWidth);
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

    DWORD_PTR _GetSelectedCandidateString(_Outptr_result_maybenull_ const WCHAR **ppwchCandidateString) override {
        return m_presenter->_GetSelectedCandidateString(ppwchCandidateString);
    }
    BOOL _SetSelectionInPage(int nPos) override {
        return m_presenter->_SetSelectionInPage(nPos);
    }

    HRESULT OnSetThreadFocus() override {
        return m_presenter->OnSetThreadFocus();
    }
    HRESULT OnKillThreadFocus() override {
        return m_presenter->OnKillThreadFocus();
    }

    void RemoveSpecificCandidateFromList(_In_ LCID Locale, _Inout_ std::vector<CCandidateListItem> &candidateList, _In_ CStringRange &srgCandidateString) override {
        return m_presenter->RemoveSpecificCandidateFromList(Locale, candidateList, srgCandidateString);
    }
    void AdviseUIChangedByArrowKey(_In_ KEYSTROKE_FUNCTION arrowKey) override {
        return m_presenter->AdviseUIChangedByArrowKey(arrowKey);
    }

private:
    IInternalFrameworkService* m_framework;
    wil::com_ptr<CCandidateListUIPresenter> m_mainPresenter;
    WindowsImeLib::IWindowsIMECandidateListView* m_presenter = nullptr;
};
