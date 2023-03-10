// Copyright (c) millimoji@gmail.com
#pragma once

#include <memory>
#include <string>
#include <vector>

// remove windows specific header dependency
struct threadMgr;
typedef unsigned long tfClientId;

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

enum class CANDIDATE_COLOR_STYLE
{
    DEFAULT,
    GREEN,
};

// Hard to make data driven setup for compartment, langbar button and preserved key. so allow customize by code
struct IWindowsIMEInProcClient
{
    virtual ~IWindowsIMEInProcClient() {}

    virtual void Initialize(_In_ ITfThreadMgr* threadMgr, TfClientId tfClientId, BOOL isSecureMode) = 0;
    virtual void Deinitialize() = 0;
    virtual void OnPreservedKey(REFGUID rguid, _Out_ BOOL* pIsEaten, _In_ ITfThreadMgr* pThreadMgr, TfClientId tfClientId) = 0;
    virtual void SetLanguageBarStatus(DWORD status, BOOL isSet) = 0;
    virtual void ConversionModeCompartmentUpdated() = 0;
    virtual std::string EncodeCustomState() = 0;
};

struct IWindowsIMEInProcFramework
{
    virtual ~IWindowsIMEInProcFramework() {}
    virtual void UpdateCustomState() = 0;
};

struct IWindowsIMECandidateListView
{
    virtual ~IWindowsIMECandidateListView() {}

    virtual HRESULT _StartCandidateList(UINT wndWidth) = 0;
    virtual void _EndCandidateList() = 0;
    virtual void _SetText(const std::vector<shared_wstring>& pCandidateList) = 0;
    virtual VOID _SetTextColorAndFillColor(CANDIDATE_COLOR_STYLE colorStyle) = 0;

    virtual bool IsCreated() = 0;
    virtual shared_wstring _GetSelectedCandidateString() = 0;
    virtual std::shared_ptr<std::vector<DWORD>> GetCandidateListRange() = 0;
    virtual BOOL _SetSelectionInPage(int nPos) = 0;
    virtual void AdviseUIChangedByArrowKey(_In_ WindowsImeLib::CANDIDATELIST_FUNCTION arrowKey) = 0;
};

struct IWindowsIMECompositionBuffer
{
    virtual ~IWindowsIMECompositionBuffer() {}

    // functions for the composition object.
    virtual HRESULT _StartComposition() = 0;
    virtual HRESULT _TerminateComposition() = 0;
    virtual HRESULT _AddComposingAndChar(const shared_wstring& pstrAddString) = 0;
    virtual HRESULT _AddCharAndFinalize(const shared_wstring& pstrAddString) = 0;
    virtual HRESULT _RemoveDummyCompositionForComposing() = 0;

    virtual bool _IsComposing() = 0;
};

struct ICompositionProcessorEngine
{
    virtual ~ICompositionProcessorEngine() {}

    virtual void UpdateCustomState(const std::string_view customStateJson) = 0;
    virtual void OnSetFocus(bool isGotten, const std::wstring_view applicationName, GUID clientId) = 0;

    // wch: converted character from VK and keyboard state
    // vkPackSource: estimated VK from wch for VK_PACKET
    virtual void OnKeyEvent(WPARAM wParam, LPARAM lParam, BOOL *pIsEaten, wchar_t wch, UINT vkPackSource, bool isKbdDisabled,
        DWORD modifiers, DWORD uniqueModifiers, bool isTest, bool isDown) = 0;

    virtual void GetCandidateList(std::vector<shared_wstring>& pCandidateList, BOOL isIncrementalWordSearch, BOOL isWildcardSearch) = 0;

    virtual void FinalizeCandidateList() = 0; // determine selected candidate by mouse click or ITfCandidateListUIElementBehavior::Finalize
    virtual VOID CancelCompositioon() = 0; // cancel composition by external triggers
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
    virtual std::shared_ptr<IWindowsIMEInProcClient> CreateIMEInProcClient(IWindowsIMEInProcFramework* framework) = 0;
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
