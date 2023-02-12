// Copyright (c) millimoji@gmail.com
#pragma once

#include "../WindowsImeLib.h"
#include "resource.h"

class RibbonIMECore :
    public WindowsImeLib::ICompositionProcessorEngine,
    public std::enable_shared_from_this<RibbonIMECore>
{
public:
    RibbonIMECore(
        const std::shared_ptr<WindowsImeLib::IWindowsIMECompositionBuffer>& compositionBuffer,
        const std::shared_ptr<WindowsImeLib::IWindowsIMECandidateListView>& candidateListView);

    virtual ~RibbonIMECore();

    void OnKeyEvent(WPARAM, LPARAM, BOOL*, wchar_t, UINT, bool, DWORD, bool, bool) override { }

    void GetCandidateList(std::vector<shared_wstring>& pCandidateList, BOOL isIncrementalWordSearch, BOOL isWildcardSearch) override;

    void FinalizeCandidateList() override {}
    VOID CancelCompositioon() override {}

    void UpdateCustomState(const std::string_view customStateJson) override;
    void OnSetFocus(bool /*isGotten*/, const std::wstring_view /*applicationName*/, GUID /*clientId*/) override {}

private:
    const std::shared_ptr<WindowsImeLib::IWindowsIMECompositionBuffer> m_compositionBuffer;
    const std::shared_ptr<WindowsImeLib::IWindowsIMECandidateListView> m_candidateListView;

    bool m_isImeOpen = {};
    bool m_isSecure = {};
    bool m_isStoreApp = {};
    bool m_isConsole = {};
};

