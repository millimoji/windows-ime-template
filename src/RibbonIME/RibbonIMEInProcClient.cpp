// Copyright (c) millimoji@gmail.com

#include "pch.h"
#include "resource.h"
#include "../WindowsImeLib.h"
#include "RibbonIMEInProcClient.h"



class RibbonIMEInProcClient :
    public WindowsImeLib::IWindowsIMEInProcClient,
    public std::enable_shared_from_this<RibbonIMEInProcClient>
{
public:
    RibbonIMEInProcClient(WindowsImeLib::IWindowsIMEInProcFramework* framework) : m_framework(framework) {}
    ~RibbonIMEInProcClient() {}

private:
    void Initialize(_In_ ITfThreadMgr* threadMgr, TfClientId tfClientId, BOOL isSecureMode) override
    {
        (void)threadMgr; (void)tfClientId; (void)isSecureMode;
    }
    void Deinitialize() override
    {
    }
    std::string EncodeCustomState() override
    {
        return "";
    }
    void OnPreservedKey(REFGUID rguid, _Out_ BOOL* pIsEaten, _In_ ITfThreadMgr* pThreadMgr, TfClientId tfClientId) override
    {
        *pIsEaten = FALSE;
        (void)rguid; (void)pThreadMgr; (void)tfClientId;
    }
    void SetLanguageBarStatus(DWORD status, BOOL isSet) override
    {
        (void)status; (void)isSet;
    }
    void ConversionModeCompartmentUpdated() override
    {
    }

private:
    WindowsImeLib::IWindowsIMEInProcFramework* m_framework;
};

std::shared_ptr<WindowsImeLib::IWindowsIMEInProcClient> RibbonIMEInProcClient_CreateInstance(WindowsImeLib::IWindowsIMEInProcFramework* framework)
{
    return std::make_shared<RibbonIMEInProcClient>(framework);
}
