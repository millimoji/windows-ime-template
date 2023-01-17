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

struct IInternalFrameworkService
{
    virtual ITfCompositionSink* GetCompositionSink() = 0;
    virtual TfClientId _GetClientId() = 0;
    virtual wil::com_ptr<ITfThreadMgr> _GetThreadMgr() = 0;
    virtual BOOL _IsStoreAppMode() = 0;
    virtual std::shared_ptr<WindowsImeLib::ICompositionProcessorEngine> GetCompositionProcessorEngine() = 0;
    virtual HRESULT _SubmitEditSessionTask(_In_ ITfContext* context, const std::function<HRESULT(TfEditCookie ec)>& editSesisonTask, DWORD tfEsFlags) = 0;
};

#include "CompositionBuffer.h"

class CLangBarItemButton;
//class CCandidateListUIPresenter;
struct SingletonProcessorBridge;

// const DWORD WM_CheckGlobalCompartment = WM_USER;
// LRESULT CALLBACK CWindowsIME_WindowProc(HWND wndHandle, UINT uMsg, WPARAM wParam, LPARAM lParam);

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
                                        ITfFnGetPreferredTouchKeyboardLayout,
                                        Microsoft::WRL::FtmBase>,
    public WindowsImeLib::IWindowsIMEInprocFramework,
    public IInternalFrameworkService
{
public:
    CWindowsIME();
    virtual ~CWindowsIME();

    // IUnknown
    // STDMETHODIMP QueryInterface(REFIID riid, _Outptr_ void **ppvObj);
    // STDMETHODIMP_(ULONG) AddRef(void);
    // STDMETHODIMP_(ULONG) Release(void);

    // ITfTextInputProcessor
    STDMETHODIMP Activate(ITfThreadMgr* pThreadMgr, TfClientId tfClientId) { return ActivateEx(pThreadMgr, tfClientId, 0); }
    // ITfTextInputProcessorEx
    STDMETHODIMP ActivateEx(ITfThreadMgr *pThreadMgr, TfClientId tfClientId, DWORD dwFlags);
    STDMETHODIMP Deactivate();

    // ITfThreadMgrEventSink
    STDMETHODIMP OnInitDocumentMgr(_In_ ITfDocumentMgr *pDocMgr);
    STDMETHODIMP OnUninitDocumentMgr(_In_ ITfDocumentMgr *pDocMgr);
    STDMETHODIMP OnSetFocus(_In_ ITfDocumentMgr *pDocMgrFocus, _In_ ITfDocumentMgr *pDocMgrPrevFocus);
    STDMETHODIMP OnPushContext(_In_ ITfContext *pContext);
    STDMETHODIMP OnPopContext(_In_ ITfContext *pContext);

    // ITfTextEditSink
    STDMETHODIMP OnEndEdit(__RPC__in_opt ITfContext *pContext, TfEditCookie ecReadOnly, __RPC__in_opt ITfEditRecord *pEditRecord);

    // ITfKeyEventSink
    STDMETHODIMP OnSetFocus(BOOL fForeground);
    STDMETHODIMP OnTestKeyDown(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pIsEaten);
    STDMETHODIMP OnKeyDown(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pIsEaten);
    STDMETHODIMP OnTestKeyUp(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pIsEaten);
    STDMETHODIMP OnKeyUp(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pIsEaten);
    STDMETHODIMP OnPreservedKey(ITfContext *pContext, REFGUID rguid, BOOL *pIsEaten);

    // ITfCompositionSink
    STDMETHODIMP OnCompositionTerminated(TfEditCookie ecWrite, _In_ ITfComposition *pComposition);

    // ITfDisplayAttributeProvider
    STDMETHODIMP EnumDisplayAttributeInfo(__RPC__deref_out_opt IEnumTfDisplayAttributeInfo **ppEnum);
    STDMETHODIMP GetDisplayAttributeInfo(__RPC__in REFGUID guidInfo, __RPC__deref_out_opt ITfDisplayAttributeInfo **ppInfo);

    // ITfActiveLanguageProfileNotifySink
    STDMETHODIMP OnActivated(_In_ REFCLSID clsid, _In_ REFGUID guidProfile, _In_ BOOL isActivated);

    // ITfThreadFocusSink
    STDMETHODIMP OnSetThreadFocus();
    STDMETHODIMP OnKillThreadFocus();

    // ITfFunctionProvider
    STDMETHODIMP GetType(__RPC__out GUID *pguid);
    STDMETHODIMP GetDescription(__RPC__deref_out_opt BSTR *pbstrDesc);
    STDMETHODIMP GetFunction(__RPC__in REFGUID rguid, __RPC__in REFIID riid, __RPC__deref_out_opt IUnknown **ppunk);

    // ITfFunction
    STDMETHODIMP GetDisplayName(_Out_ BSTR *pbstrDisplayName);

    // ITfFnGetPreferredTouchKeyboardLayout, it is the Optimized layout feature.
    STDMETHODIMP GetLayout(_Out_ TKBLayoutType *ptkblayoutType, _Out_ WORD *pwPreferredLayoutId);

    // CClassFactory factory callback
    static HRESULT CreateInstance(_In_ IUnknown *pUnkOuter, REFIID riid, _Outptr_ void **ppvObj);

    // utility function for thread manager.
    wil::com_ptr<ITfThreadMgr> _GetThreadMgr() override { return _pThreadMgr; }
    TfClientId _GetClientId() override { return _tfClientId; }

    // functions for the composition object.
//    void _SetComposition(_In_ ITfComposition *pComposition);
//    void _TerminateComposition(TfEditCookie ec, _In_ ITfContext *pContext, BOOL isCalledFromDeactivate = FALSE);
//    void _SaveCompositionContext(_In_ ITfContext *pContext);

    // key event handlers for composition/candidate/phrase common objects.
//    HRESULT _HandleComplete(TfEditCookie ec, _In_ ITfContext *pContext);
//    HRESULT _HandleCancel(TfEditCookie ec, _In_ ITfContext *pContext) override;

    // key event handlers for composition object.
//    HRESULT _HandleCompositionInput(TfEditCookie ec, _In_ ITfContext *pContext, WCHAR wch) override;
//    HRESULT _HandleCompositionFinalize(TfEditCookie ec, _In_ ITfContext *pContext, BOOL fCandidateList) override;
//    HRESULT _HandleCompositionConvert(TfEditCookie ec, _In_ ITfContext *pContext, BOOL isWildcardSearch) override;
//    HRESULT _HandleCompositionBackspace(TfEditCookie ec, _In_ ITfContext *pContext) override;
//    HRESULT _HandleCompositionArrowKey(TfEditCookie ec, _In_ ITfContext *pContext, KEYSTROKE_FUNCTION keyFunction) override;
//    HRESULT _HandleCompositionPunctuation(TfEditCookie ec, _In_ ITfContext *pContext, WCHAR wch) override;
//    HRESULT _HandleCompositionDoubleSingleByte(TfEditCookie ec, _In_ ITfContext *pContext, WCHAR wch) override;

    // key event handlers for candidate object.
//    HRESULT _HandleCandidateFinalize(TfEditCookie ec, _In_ ITfContext *pContext) override;
//    HRESULT _HandleCandidateConvert(TfEditCookie ec, _In_ ITfContext *pContext) override;
//    HRESULT _HandleCandidateArrowKey(TfEditCookie ec, _In_ ITfContext *pContext, _In_ KEYSTROKE_FUNCTION keyFunction) override;
//    HRESULT _HandleCandidateSelectByNumber(TfEditCookie ec, _In_ ITfContext *pContext, _In_ UINT uCode) override;

    // key event handlers for phrase object.
//    HRESULT _HandlePhraseFinalize(TfEditCookie ec, _In_ ITfContext *pContext) override;
//    HRESULT _HandlePhraseArrowKey(TfEditCookie ec, _In_ ITfContext *pContext, _In_ KEYSTROKE_FUNCTION keyFunction) override;
//    HRESULT _HandlePhraseSelectByNumber(TfEditCookie ec, _In_ ITfContext *pContext, _In_ UINT uCode) override;

    BOOL _IsSecureMode(void) { return (_dwActivateFlags & TF_TMAE_SECUREMODE) ? TRUE : FALSE; }
    BOOL _IsComLess(void) { return (_dwActivateFlags & TF_TMAE_COMLESS) ? TRUE : FALSE; }
    BOOL _IsStoreAppMode(void) override { return (_dwActivateFlags & TF_TMF_IMMERSIVEMODE) ? TRUE : FALSE; };

    std::shared_ptr<WindowsImeLib::ICompositionProcessorEngine> GetCompositionProcessorEngine() override { return (_pCompositionProcessorEngine); };

    // comless helpers
    static HRESULT CreateInstance(REFCLSID rclsid, REFIID riid, _Outptr_result_maybenull_ LPVOID* ppv, _Out_opt_ HINSTANCE* phInst, BOOL isComLessMode);
    static HRESULT ComLessCreateInstance(REFGUID rclsid, REFIID riid, _Outptr_result_maybenull_ void **ppv, _Out_opt_ HINSTANCE *phInst);
    static HRESULT GetComModuleName(REFGUID rclsid, _Out_writes_(cchPath)WCHAR* wchPath, DWORD cchPath);

private:
    // functions for the composition object.
//    HRESULT _HandleCompositionInputWorker(_In_ WindowsImeLib::ICompositionProcessorEngine *pCompositionProcessorEngine, TfEditCookie ec, _In_ ITfContext *pContext);
//    HRESULT _CreateAndStartCandidate(_In_ WindowsImeLib::ICompositionProcessorEngine *pCompositionProcessorEngine, TfEditCookie ec, _In_ ITfContext *pContext);
//    HRESULT _HandleCandidateWorker(TfEditCookie ec, _In_ ITfContext *pContext);

//    void _StartComposition(_In_ ITfContext *pContext) override;
//    void _EndComposition(_In_opt_ ITfContext *pContext) override;
//    BOOL _IsComposing() override;
    bool _IsKeyboardDisabled();
//    CANDIDATE_MODE _CandidateMode() override { return _candidateMode; }
//    bool IsCandidateWithWildcard() override { return _isCandidateWithWildcard; }

//    HRESULT _AddComposingAndChar(TfEditCookie ec, _In_ ITfContext *pContext, _In_ CStringRange *pstrAddString);
//    HRESULT _AddCharAndFinalize(TfEditCookie ec, _In_ ITfContext *pContext, _In_ CStringRange *pstrAddString);

//    BOOL _FindComposingRange(TfEditCookie ec, _In_ ITfContext *pContext, _In_ ITfRange *pSelection, _Outptr_result_maybenull_ ITfRange **ppRange);
//    HRESULT _SetInputString(TfEditCookie ec, _In_ ITfContext *pContext, _Out_opt_ ITfRange *pRange, _In_ CStringRange *pstrAddString, BOOL exist_composing);
//    HRESULT _InsertAtSelection(TfEditCookie ec, _In_ ITfContext *pContext, _In_ CStringRange *pstrAddString, _Outptr_ ITfRange **ppCompRange);

//    HRESULT _RemoveDummyCompositionForComposing(TfEditCookie ec, _In_ ITfComposition *pComposition);

    // Invoke key handler edit session
//    HRESULT _InvokeKeyHandler(_In_ ITfContext *pContext, UINT code, WCHAR wch, DWORD flags, _KEYSTROKE_STATE keyState) override;
    HRESULT _SubmitEditSessionTask(_In_ ITfContext* context, const std::function<HRESULT(TfEditCookie ec)>& editSesisonTask, DWORD tfEsFlags) override;
//    HRESULT KeyHandlerEditSession_DoEditSession(TfEditCookie ec, _KEYSTROKE_STATE _KeyState, _In_ ITfContext* _pContext, UINT _uCode, WCHAR _wch, void* /*pv*/)  override;

    // function for the language property
//    BOOL _SetCompositionLanguage(TfEditCookie ec, _In_ ITfContext *pContext);

    // function for the display attribute
//    void _ClearCompositionDisplayAttributes(TfEditCookie ec, _In_ ITfContext *pContext);
//    BOOL _SetCompositionDisplayAttributes(TfEditCookie ec, _In_ ITfContext *pContext, TfGuidAtom gaDisplayAttribute);
    BOOL _InitDisplayAttributeGuidAtom();

    BOOL _InitThreadMgrEventSink();
    void _UninitThreadMgrEventSink();

    BOOL _InitTextEditSink(_In_ ITfDocumentMgr *pDocMgr);

    void _UpdateLanguageBarOnSetFocus(_In_ ITfDocumentMgr *pDocMgrFocus);

    BOOL _InitKeyEventSink();
    void _UninitKeyEventSink();

    BOOL _InitActiveLanguageProfileNotifySink();
    void _UninitActiveLanguageProfileNotifySink();

//    BOOL _IsKeyEaten(_In_ ITfContext *pContext, UINT codeIn, _Out_ UINT *pCodeOut, _Out_writes_(1) WCHAR *pwch, _Out_opt_ _KEYSTROKE_STATE *pKeyState);
//    BOOL _IsRangeCovered(TfEditCookie ec, _In_ ITfRange *pRangeTest, _In_ ITfRange *pRangeCover);
//    VOID _DeleteCandidateList(BOOL fForce, _In_opt_ ITfContext *pContext) override;

    wchar_t ConvertVKey(UINT code);
    UINT VKeyFromVKPacketAndWchar(UINT vk, WCHAR wch);

    BOOL _InitThreadFocusSink();
    void _UninitThreadFocusSink();

    BOOL _InitFunctionProviderSink();
    void _UninitFunctionProviderSink();

    BOOL _AddTextProcessorEngine();

    BOOL VerifyIMECLSID(_In_ REFCLSID clsid);

    void SetDefaultCandidateTextFont();
    void UpdateCustomState() override
    {
        if (m_inprocClient)
        {
            const auto customStateJson = m_inprocClient->EncodeCustomState();
            if (_pCompositionProcessorEngine)
            {
                _pCompositionProcessorEngine->UpdateCustomState(customStateJson);
            }
            if (m_singletonProcessor)
            {
                m_singletonProcessor->UpdateCustomState(customStateJson.c_str());
            }
        }
    }
    ITfCompositionSink* GetCompositionSink() override { return this;  }
//    void* GetTextService() override { return (void*)this; }
public:
//    std::shared_ptr<WindowsImeLib::IWindowsIMECompositionBuffer> GetCompositionBuffer() override { return m_compositionBuffer; }
//    std::shared_ptr<WindowsImeLib::IWindowsIMECandidateListView> GetCandidateListView() override { return m_candidateListView; }

private:
    wil::com_ptr<ITfThreadMgr> _pThreadMgr;
    TfClientId _tfClientId = {};
    DWORD _dwActivateFlags = {};

    // The cookie of ThreadMgrEventSink
    DWORD _threadMgrEventSinkCookie = TF_INVALID_COOKIE;

    wil::com_ptr<ITfContext> _pTextEditSinkContext;
    DWORD _textEditSinkCookie = TF_INVALID_COOKIE;

    // The cookie of ActiveLanguageProfileNotifySink
    DWORD _activeLanguageProfileNotifySinkCookie = TF_INVALID_COOKIE;

    // The cookie of ThreadFocusSink
    DWORD _dwThreadFocusSinkCookie = TF_INVALID_COOKIE;

    // Composition Processor Engine object.
    std::shared_ptr<WindowsImeLib::ICompositionProcessorEngine> _pCompositionProcessorEngine;

    // guidatom for the display attibute.
    TfGuidAtom _gaDisplayAttributeInput = {};
    TfGuidAtom _gaDisplayAttributeConverted = {};

    wil::com_ptr<ITfDocumentMgr> _pDocMgrLastFocused;

    // Support the search integration
    wil::com_ptr<ITfFnSearchCandidateProvider> _pITfFnSearchCandidateProvider;

    std::shared_ptr<WindowsImeLib::IWindowsIMECompositionBuffer> m_compositionBuffer;
    std::shared_ptr<WindowsImeLib::IWindowsIMECandidateListView> m_candidateListView;

    std::shared_ptr<WindowsImeLib::IWindowsIMEInprocClient> m_inprocClient;
    wil::com_ptr<ITextInputProcessor> m_singletonProcessor;


    // Language bar item object.
    // CLangBarItemButton* _pLangBarItem = {};

    // the current composition object.
//    ITfComposition* _pComposition;

//    CANDIDATE_MODE _candidateMode;
//    CCandidateListUIPresenter *_pCandidateListUIPresenter;
//    BOOL _isCandidateWithWildcard;
//    wil::com_ptr<WindowsImeLib::IWindowsIMECandidateListVeiw> _pCandidateListUIPresenter;

//    ITfContext* _pContext;
//    ITfCompartment* _pSIPIMEOnOffCompartment;
//    DWORD _dwSIPIMEOnOffCompartmentSinkCookie;
//    HWND _msgWndHandle;
//    LONG _refCount;
};
