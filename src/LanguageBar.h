// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once
#include <wrl/module.h>
#include <wrl/implements.h>

class CCompartment;
class CCompartmentEventSink;

class CLangBarItemButton :
    public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>,
                                        ITfSource,
                                        ITfLangBarItem,
                                        ITfLangBarItemButton,
                                        Microsoft::WRL::FtmBase>
{
public:
    CLangBarItemButton() {}
    ~CLangBarItemButton();
    HRESULT RuntimeClassInitialize(REFGUID guidLangBar, LPCWSTR description, LPCWSTR tooltip, DWORD onIconIndex, DWORD offIconIndex, BOOL isSecureMode);

    // ITfLangBarItem
    STDMETHODIMP GetInfo(_Out_ TF_LANGBARITEMINFO *pInfo) override;
    STDMETHODIMP GetStatus(_Out_ DWORD *pdwStatus) override;
    STDMETHODIMP Show(BOOL fShow) override;
    STDMETHODIMP GetTooltipString(_Out_ BSTR *pbstrToolTip) override;

    // ITfLangBarItemButton
    STDMETHODIMP OnClick(TfLBIClick click, POINT pt, _In_ const RECT *prcArea) override;
    STDMETHODIMP InitMenu(_In_ ITfMenu *pMenu) override;
    STDMETHODIMP OnMenuSelect(UINT wID) override;
    STDMETHODIMP GetIcon(_Out_ HICON *phIcon) override;
    STDMETHODIMP GetText(_Out_ BSTR *pbstrText) override;

    // ITfSource
    STDMETHODIMP AdviseSink(__RPC__in REFIID riid, __RPC__in_opt IUnknown *punk, __RPC__out DWORD *pdwCookie) override;
    STDMETHODIMP UnadviseSink(DWORD dwCookie) override;

    // Add/Remove languagebar item
    HRESULT _AddItem(_In_ ITfThreadMgr *pThreadMgr);
    HRESULT _RemoveItem(_In_ ITfThreadMgr *pThreadMgr);

    // Register compartment for button On/Off switch
    BOOL _RegisterCompartment(_In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId, REFGUID guidCompartment);
    BOOL _UnregisterCompartment(_In_ ITfThreadMgr *pThreadMgr);

    void CleanUp();
    void SetStatus(DWORD dwStatus, BOOL fSet);

private:
    wil::com_ptr<ITfLangBarItemSink> _pLangBarItemSink;

    TF_LANGBARITEMINFO _tfLangBarItemInfo;
    std::wstring _pTooltipText;
    DWORD _onIconIndex;
    DWORD _offIconIndex;

    BOOL _isAddedToLanguageBar;
    BOOL _isSecureMode;
    DWORD _status;

    std::unique_ptr<CCompartment> _pCompartment;
    wil::com_ptr<CCompartmentEventSink> _pCompartmentEventSink;
    static HRESULT _CompartmentCallback(_In_ void *pv, REFGUID guidCompartment);

    // The cookie for the sink to CLangBarItemButton.
    static const DWORD _cookie = 0;
};
