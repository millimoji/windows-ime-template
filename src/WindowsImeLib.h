// Copyright (c) millimoji@gmail.com
#pragma once

#include <windows.h>
#include <ctffunc.h>
#include <cassert>
#include <memory>
#include <string>
#include <vector>

// TODO: will remove
#include "Compartment.h"
#include "LanguageBar.h"

typedef std::shared_ptr<const std::wstring> shared_wstring;

namespace WindowsImeLib
{

//---------------------------------------------------------------------
// enum
//---------------------------------------------------------------------
enum class CANDIDATELIST_FUNCTION
{
    NONE = 0,
    MOVE_UP,
    MOVE_DOWN,
    MOVE_PAGE_UP,
    MOVE_PAGE_DOWN,
    MOVE_PAGE_TOP,
    MOVE_PAGE_BOTTOM,
};

// Hard to make data driven setup for compartment, langbar button and preserved key. so allow customize by code
struct IWindowsIMEInprocClient
{
    virtual ~IWindowsIMEInprocClient() {}

    virtual void Initialize(_In_ ITfThreadMgr* threadMgr, TfClientId tfClientId, BOOL isSecureMode) = 0;
    virtual void Deinitialize() = 0;
    virtual void OnPreservedKey(REFGUID rguid, _Out_ BOOL* pIsEaten, _In_ ITfThreadMgr* pThreadMgr, TfClientId tfClientId) = 0;
    virtual void SetLanguageBarStatus(DWORD status, BOOL isSet) = 0;
    virtual void ConversionModeCompartmentUpdated() = 0;
    virtual std::string EncodeCustomState() = 0;
};

struct IWindowsIMEInprocFramework
{
    virtual ~IWindowsIMEInprocFramework() {}
    virtual void UpdateCustomState() = 0;
};

struct IWindowsIMECandidateListView
{
    virtual ~IWindowsIMECandidateListView() {}

    virtual void CreateView(_In_ std::vector<DWORD> *pIndexRange, BOOL hideWindow) = 0;
    virtual void DestroyView() = 0;
    virtual bool IsCreated() = 0;

    virtual HRESULT _StartCandidateList(_In_ ITfContext *pContextDocument, TfEditCookie ec, _In_ ITfRange *pRangeComposition, UINT wndWidth) = 0;
    virtual void _EndCandidateList() = 0;
    virtual void _ClearList() = 0;
    virtual void _SetText(const std::vector<shared_wstring>& pCandidateList) = 0;
    virtual VOID _SetTextColor(COLORREF crColor, COLORREF crBkColor) = 0;
    virtual VOID _SetFillColor(HBRUSH hBrush) = 0;

    virtual shared_wstring _GetSelectedCandidateString() = 0;
    virtual BOOL _SetSelectionInPage(int nPos) = 0;
    virtual void AdviseUIChangedByArrowKey(_In_ WindowsImeLib::CANDIDATELIST_FUNCTION arrowKey) = 0;
};

struct IWindowsIMECompositionBuffer
{
    virtual ~IWindowsIMECompositionBuffer() {}

    // functions for the composition object.
    virtual HRESULT _StartComposition(TfEditCookie ec, _In_ ITfContext *_pContext) = 0;
    virtual void _TerminateComposition(TfEditCookie ec, _In_ ITfContext *pContext, BOOL isCalledFromDeactivate = FALSE) = 0;
    virtual HRESULT _AddComposingAndChar(TfEditCookie ec, _In_ ITfContext *pContext, const shared_wstring& pstrAddString) = 0;
    virtual HRESULT _AddCharAndFinalize(TfEditCookie ec, _In_ ITfContext *pContext, const shared_wstring& pstrAddString) = 0;
    virtual HRESULT _RemoveDummyCompositionForComposing(TfEditCookie ec, _In_ ITfComposition *pComposition) = 0;

    // function for the display attribute
    virtual bool _IsComposing() = 0;

    //
    virtual wil::com_ptr<ITfComposition> GetComposition() = 0;
    virtual HRESULT _SubmitEditSessionTask(_In_ ITfContext* context, const std::function<HRESULT(TfEditCookie ec)>& editSesisonTask, DWORD tfEsFlags) = 0;
};

struct ICompositionProcessorEngine
{
    virtual ~ICompositionProcessorEngine() {}

    virtual BOOL Initialize() = 0;
    virtual void UpdateCustomState(const std::string& customStateJson) = 0;

    // wch: converted character from VK and keyboard state
    // vkPackSource: estimated VK from wch for VK_PACKET
    virtual void OnKeyEvent(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pIsEaten, wchar_t wch, UINT vkPackSource, bool isKbdDisabled,
        DWORD modifiers, DWORD uniqueModifiers, bool isTest, bool isDown) = 0;

    virtual void GetCandidateList(std::vector<shared_wstring>& pCandidateList, BOOL isIncrementalWordSearch, BOOL isWildcardSearch) = 0;

    virtual void PurgeVirtualKey() = 0;
    virtual void EndComposition(_In_opt_ ITfContext* pContext) = 0;
    virtual void FinalizeCandidateList(_In_ ITfContext* pContext) = 0;
    virtual VOID _DeleteCandidateList(BOOL fForce, _In_opt_ ITfContext *pContext) = 0;
};

struct ITextInputFramework
{
    virtual ~ITextInputFramework() {}

    virtual void Test() = 0;
};

struct ITextInputProcessor
{
    virtual ~ITextInputProcessor() {}

    virtual std::wstring TestMethod(const std::wstring& src) = 0;
    virtual void UpdateCustomState(const std::string& stateJson) = 0;
    virtual void SetFocus(bool isGotten) = 0;
};

struct IConstantProvider
{
    virtual const CLSID& IMECLSID() noexcept = 0;
    virtual const GUID& IMEProfileGuid() noexcept = 0;
    virtual const GUID& DisplayAttributeInput() noexcept = 0;
    virtual const GUID& DisplayAttributeConverted() noexcept = 0;
    virtual const GUID& CandUIElement() noexcept = 0;
    virtual const LANGID GetLangID() noexcept = 0;
    virtual const LCID GetLocale() noexcept = 0;
    virtual const GUID& ServerCLSID() noexcept = 0;
    virtual const GUID& ServerAppID() noexcept = 0;
    virtual const wchar_t* ServerName() noexcept = 0;
    virtual UINT GetCandidateWindowWidth() noexcept = 0;
    virtual const int GetDefaultCandidateTextFontResourceID() = 0;
    virtual void GetPreferredTouchKeyboardLayout(_Out_ TKBLayoutType* layoutType, _Out_ WORD* preferredLayoutId) = 0;
};

struct IProcessorFactory
{
    virtual ~IProcessorFactory() {}

    virtual std::shared_ptr<ICompositionProcessorEngine> CreateCompositionProcessorEngine(
        const std::shared_ptr<IWindowsIMECompositionBuffer>& compositionBuffer,
        const std::shared_ptr<IWindowsIMECandidateListView>& candidateListView) = 0;
    virtual std::shared_ptr<IConstantProvider> GetConstantProvider() = 0;
    virtual std::shared_ptr<ITextInputProcessor> CreateTextInputProcessor(ITextInputFramework* framework) = 0;
    virtual std::shared_ptr<IWindowsIMEInprocClient> CreateIMEInprocClient(IWindowsIMEInprocFramework* framework) = 0;
};

// TODO: re-design how to inject factory
extern std::shared_ptr<IProcessorFactory> g_processorFactory;

// legacy code, hard to make all be in specific namspace.
extern BOOL DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID pvReserved);
extern HRESULT DllGetClassObject(_In_ REFCLSID rclsid, _In_ REFIID riid, _Outptr_ void** ppv);
extern HRESULT DllCanUnloadNow(void);
extern HRESULT DllUnregisterServer(void);
extern HRESULT DllRegisterServer(int textServiceIconIndex);
extern void TraceLog(const char* format, ...);
extern void TraceLog(const wchar_t* format, ...);
}
