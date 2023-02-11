// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include "../WindowsImeLib.h"
#include "SingletonProcessor.h"
#include "BaseStructure.h"

#include "CompositionBuffer.h"
#include "CandidateListView.h"
#include "SearchCandidateProvider.h"

class CLangBarItemButton;
struct SingletonProcessorBridge;

class CWindowsIME :
    public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>,
                                        ITfTextInputProcessor,
                                        ITfTextInputProcessorEx,
                                        ITfThreadMgrEventSink,
                                        ITfTextEditSink,
                                        ITfKeyEventSink,
                                        ITfCompositionSink,
                                        ITfDisplayAttributeProvider,
                                        ITfActiveLanguageProfileNotifySink,
                                        ITfThreadFocusSink,
                                        ITfFunctionProvider,
                                        ITfTextLayoutSink,
                                        ITfFnGetPreferredTouchKeyboardLayout,
                                        Microsoft::WRL::FtmBase>,
    public WindowsImeLib::IWindowsIMEInProcFramework,
    public ICandidateListViewOwner,
    public ICompositionBufferOwner,
    public ISearchCandidateProviderOwner
{
public:
    CWindowsIME();
    virtual ~CWindowsIME();

    // CClassFactory factory callback
    static HRESULT CreateInstance(_In_ IUnknown* pUnkOuter, REFIID riid, _Outptr_ void** ppvObj);

private:
    // ITfTextInputProcessor
    STDMETHODIMP Activate(ITfThreadMgr* pThreadMgr, TfClientId tfClientId)  override { return ActivateEx(pThreadMgr, tfClientId, 0); }
    // ITfTextInputProcessorEx
    STDMETHODIMP ActivateEx(ITfThreadMgr *pThreadMgr, TfClientId tfClientId, DWORD dwFlags) override;
    STDMETHODIMP Deactivate() override;

    // ITfThreadMgrEventSink
    STDMETHODIMP OnInitDocumentMgr(_In_ ITfDocumentMgr *pDocMgr) override;
    STDMETHODIMP OnUninitDocumentMgr(_In_ ITfDocumentMgr *pDocMgr) override;
    STDMETHODIMP OnSetFocus(_In_ ITfDocumentMgr *pDocMgrFocus, _In_ ITfDocumentMgr *pDocMgrPrevFocus) override;
    STDMETHODIMP OnPushContext(_In_ ITfContext *pContext) override;
    STDMETHODIMP OnPopContext(_In_ ITfContext *pContext) override;

    // ITfTextEditSink
    STDMETHODIMP OnEndEdit(__RPC__in_opt ITfContext *pContext, TfEditCookie ecReadOnly, __RPC__in_opt ITfEditRecord *pEditRecord) override;

    // ITfKeyEventSink
    STDMETHODIMP OnSetFocus(BOOL fForeground) override;
    STDMETHODIMP OnTestKeyDown(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pIsEaten) override;
    STDMETHODIMP OnKeyDown(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pIsEaten) override;
    STDMETHODIMP OnTestKeyUp(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pIsEaten) override;
    STDMETHODIMP OnKeyUp(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pIsEaten) override;
    STDMETHODIMP OnPreservedKey(ITfContext *pContext, REFGUID rguid, BOOL *pIsEaten) override;

    // ITfCompositionSink
    STDMETHODIMP OnCompositionTerminated(TfEditCookie ecWrite, _In_ ITfComposition *pComposition) override;

    // ITfDisplayAttributeProvider
    STDMETHODIMP EnumDisplayAttributeInfo(__RPC__deref_out_opt IEnumTfDisplayAttributeInfo **ppEnum) override;
    STDMETHODIMP GetDisplayAttributeInfo(__RPC__in REFGUID guidInfo, __RPC__deref_out_opt ITfDisplayAttributeInfo **ppInfo) override;

    // ITfActiveLanguageProfileNotifySink
    STDMETHODIMP OnActivated(_In_ REFCLSID clsid, _In_ REFGUID guidProfile, _In_ BOOL isActivated) override;

    // ITfThreadFocusSink
    STDMETHODIMP OnSetThreadFocus() override;
    STDMETHODIMP OnKillThreadFocus() override;

    // ITfFunctionProvider
    STDMETHODIMP GetType(__RPC__out GUID *pguid) override;
    STDMETHODIMP GetDescription(__RPC__deref_out_opt BSTR *pbstrDesc)  override;
    STDMETHODIMP GetFunction(__RPC__in REFGUID rguid, __RPC__in REFIID riid, __RPC__deref_out_opt IUnknown **ppunk) override;

    // ITfFunction
    STDMETHODIMP GetDisplayName(_Out_ BSTR *pbstrDisplayName) override;

    // ITfFnGetPreferredTouchKeyboardLayout, it is the Optimized layout feature.
    STDMETHODIMP GetLayout(_Out_ TKBLayoutType *ptkblayoutType, _Out_ WORD *pwPreferredLayoutId) override;

    // ITfTextLayoutSink
    IFACEMETHODIMP OnLayoutChange(_In_ ITfContext *pic, TfLayoutCode lcode, _In_ ITfContextView *pView) override;

    BOOL _IsSecureMode(void) { return (_dwActivateFlags & TF_TMAE_SECUREMODE) ? TRUE : FALSE; }
    BOOL _IsComLess(void) { return (_dwActivateFlags & TF_TMAE_COMLESS) ? TRUE : FALSE; }

    // comless helpers
    static HRESULT CreateInstance(REFCLSID rclsid, REFIID riid, _Outptr_result_maybenull_ LPVOID* ppv, _Out_opt_ HINSTANCE* phInst, BOOL isComLessMode);
    static HRESULT ComLessCreateInstance(REFGUID rclsid, REFIID riid, _Outptr_result_maybenull_ void **ppv, _Out_opt_ HINSTANCE *phInst);
    static HRESULT GetComModuleName(REFGUID rclsid, _Out_writes_(cchPath)WCHAR* wchPath, DWORD cchPath);

private:
    bool _IsKeyboardDisabled();

    // Invoke key handler edit session
    HRESULT _SubmitEditSessionTask(_In_ ITfContext* context, const std::function<HRESULT(TfEditCookie ec)>& editSesisonTask, DWORD tfEsFlags) override;

    // function for the display attribute
    BOOL _InitDisplayAttributeGuidAtom();

    BOOL _InitThreadMgrEventSink();
    void _UninitThreadMgrEventSink();

    BOOL _InitTextEditSink(_In_ ITfDocumentMgr *pDocMgr);

    void _UpdateLanguageBarOnSetFocus(_In_ ITfDocumentMgr *pDocMgrFocus);

    BOOL _InitKeyEventSink();
    void _UninitKeyEventSink();

    BOOL _InitActiveLanguageProfileNotifySink();
    void _UninitActiveLanguageProfileNotifySink();

    wchar_t ConvertVKey(UINT code);
    UINT VKeyFromVKPacketAndWchar(UINT vk, WCHAR wch);

    BOOL _InitThreadFocusSink();
    void _UninitThreadFocusSink();

    BOOL _InitFunctionProviderSink();
    void _UninitFunctionProviderSink();

    BOOL _AddTextProcessorEngine();

    BOOL VerifyIMECLSID(_In_ REFCLSID clsid);

    void UpdateCustomState() override
    {
        if (m_inprocClient)
        {
            const auto customStateJson = m_inprocClient->EncodeCustomState();
            if (m_singletonProcessor)
            {
                m_singletonProcessor->UpdateCustomState(customStateJson.c_str());
            }
        }
    }
    ITfCompositionSink* GetCompositionSink() override { return this;  }

    // ICandidateListViewOwner
    HRESULT _GetLastTextExt(_Out_ HWND* documentWindow, _Out_ RECT *lpRect) override;
    BOOL _IsStoreAppMode(void) override { return (_dwActivateFlags & TF_TMF_IMMERSIVEMODE) ? TRUE : FALSE; };
    void NotifyFinalizeCandidateList() override { m_singletonProcessor->FinalizeCandidateList(); }

// FUTURE: cross proc candidate window
    wil::com_ptr<ITfThreadMgr> _GetThreadMgr() override { return wil::com_ptr<ITfThreadMgr>(); }

    // ICompositionBufferOwner
    wil::com_ptr<ITextInputProcessor> GetTextInputProcessor() override { return m_singletonProcessor; }

    HRESULT _StartLayoutTracking(_In_ ITfContext *pContextDocument, TfEditCookie ec, _In_ ITfRange *pRangeComposition) override;
    HRESULT _EndLayoutTracking() override;

private:
    wil::com_ptr<ITfThreadMgr> _pThreadMgr;
    TfClientId _tfClientId = {};
    DWORD _dwActivateFlags = {};
    DWORD m_dwActiveFlags = {}; // via ITfThreadMgrEx::GetActiveFlags, TF_TMF_*

    std::wstring m_processName;
    wil::unique_bstr m_processNameBstr;
    GUID m_clientGuid;

    // The cookie of ThreadMgrEventSink
    DWORD _threadMgrEventSinkCookie = TF_INVALID_COOKIE;

    wil::com_ptr<ITfContext> _pTextEditSinkContext;
    DWORD _textEditSinkCookie = TF_INVALID_COOKIE;

    // The cookie of ActiveLanguageProfileNotifySink
    DWORD _activeLanguageProfileNotifySinkCookie = TF_INVALID_COOKIE;

    // The cookie of ThreadFocusSink
    DWORD _dwThreadFocusSinkCookie = TF_INVALID_COOKIE;

    // Composition Processor Engine object.
    wil::com_ptr<ITextInputProcessor> m_singletonProcessor;

    // guidatom for the display attibute.
    TfGuidAtom _gaDisplayAttributeInput = {};
    TfGuidAtom _gaDisplayAttributeConverted = {};

    wil::com_ptr<ITfDocumentMgr> _pDocMgrLastFocused;

    // Support the search integration
    wil::com_ptr<ITfFnSearchCandidateProvider> _pITfFnSearchCandidateProvider;

    std::shared_ptr<ICompositionBufferInternal> m_compositionBuffer;
//    std::shared_ptr<ICandidateListViewInternal> m_candidateListView;

    std::shared_ptr<WindowsImeLib::IWindowsIMEInProcClient> m_inprocClient;

    struct TextLayoutSinkState
    {
        TfEditCookie _tfEditCookie = TF_INVALID_EDIT_COOKIE;
        wil::com_ptr<ITfContext> _pContextDocument;
        wil::com_ptr<ITfRange> _pRangeComposition;
        DWORD _dwCookieTextLayoutSink = 0;
        RECT compositionRect = {};
        BOOL isClipped = {};
        HWND documentWindow = {};
    } m_textLayoutSink;
};
