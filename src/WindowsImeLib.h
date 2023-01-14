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


//---------------------------------------------------------------------
// candidate list
//---------------------------------------------------------------------
enum CANDIDATE_MODE
{
    CANDIDATE_NONE = 0,
    CANDIDATE_ORIGINAL,
    CANDIDATE_PHRASE,
    CANDIDATE_INCREMENTAL,
    CANDIDATE_WITH_NEXT_COMPOSITION
};

//---------------------------------------------------------------------
// enum
//---------------------------------------------------------------------
enum KEYSTROKE_CATEGORY
{
    CATEGORY_NONE = 0,
    CATEGORY_COMPOSING,
    CATEGORY_CANDIDATE,
    CATEGORY_PHRASE,
    // CATEGORY_PHRASEFROMKEYSTROKE,
    CATEGORY_INVOKE_COMPOSITION_EDIT_SESSION
};

enum KEYSTROKE_FUNCTION
{
    FUNCTION_NONE = 0,
    FUNCTION_INPUT,

    FUNCTION_CANCEL,
    FUNCTION_FINALIZE_TEXTSTORE,
    FUNCTION_FINALIZE_TEXTSTORE_AND_INPUT,
    FUNCTION_FINALIZE_CANDIDATELIST,
    FUNCTION_FINALIZE_CANDIDATELIST_AND_INPUT,
    FUNCTION_CONVERT,
    FUNCTION_CONVERT_WILDCARD,
    FUNCTION_SELECT_BY_NUMBER,
    FUNCTION_BACKSPACE,
    FUNCTION_MOVE_LEFT,
    FUNCTION_MOVE_RIGHT,
    FUNCTION_MOVE_UP,
    FUNCTION_MOVE_DOWN,
    FUNCTION_MOVE_PAGE_UP,
    FUNCTION_MOVE_PAGE_DOWN,
    FUNCTION_MOVE_PAGE_TOP,
    FUNCTION_MOVE_PAGE_BOTTOM,

    // Function Double/Single byte
    FUNCTION_DOUBLE_SINGLE_BYTE,

    // Function Punctuation
    FUNCTION_PUNCTUATION
};

//---------------------------------------------------------------------
// structure
//---------------------------------------------------------------------
struct _KEYSTROKE_STATE
{
    KEYSTROKE_CATEGORY Category;
    KEYSTROKE_FUNCTION Function;
};

class CStringRange
{
public:
    CStringRange();
    ~CStringRange();

    const WCHAR *Get() const;
    const DWORD_PTR GetLength() const;
    void Clear();
    void Set(const WCHAR *pwch, DWORD_PTR dwLength);
    void Set(CStringRange &sr);
    CStringRange& operator=(const CStringRange& sr);
    void CharNext(_Inout_ CStringRange* pCharNext);
    static int Compare(LCID locale, _In_ CStringRange* pString1, _In_ CStringRange* pString2);
    static BOOL WildcardCompare(LCID locale, _In_ CStringRange* stringWithWildcard, _In_ CStringRange* targetString);

protected:
    DWORD_PTR _stringBufLen;         // Length is in character count.
    const WCHAR *_pStringBuf;    // Buffer which is not add zero terminate.
};

//---------------------------------------------------------------------
// CCandidateListItem
//  _ItemString - candidate string
//  _FindKeyCode - tailing string
//---------------------------------------------------------------------
struct CCandidateListItem
{
    CStringRange _ItemString;
    CStringRange _FindKeyCode;

    CCandidateListItem& operator =( const CCandidateListItem& rhs)
    {
        _ItemString = rhs._ItemString;
        _FindKeyCode = rhs._FindKeyCode;
        return *this;
    }
};

// struct ITfThreadMgr;
// typedef /* [uuid] */  DECLSPEC_UUID("de403c21-89fd-4f85-8b87-64584d063fbc") DWORD TfClientId;

namespace WindowsImeLib
{

struct IWindowsIMEInprocFramework
{
    virtual ~IWindowsIMEInprocFramework() {}
    virtual void UpdateCustomState() = 0;
};

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

struct ICompositionProcessorEngine;

struct IWindowsIMECandidateList : public IUnknown
{
    virtual ITfContext* _GetContextDocument() = 0;

    virtual HRESULT _StartCandidateList(TfClientId tfClientId, _In_ ITfDocumentMgr *pDocumentMgr, _In_ ITfContext *pContextDocument, TfEditCookie ec, _In_ ITfRange *pRangeComposition, UINT wndWidth) = 0;
    virtual void _EndCandidateList() = 0;
    virtual void _ClearList() = 0;
    virtual void _SetText(_In_ std::vector<CCandidateListItem> *pCandidateList, BOOL isAddFindKeyCode) = 0;
    virtual VOID _SetTextColor(COLORREF crColor, COLORREF crBkColor) = 0;
    virtual VOID _SetFillColor(HBRUSH hBrush) = 0;

    virtual DWORD_PTR _GetSelectedCandidateString(_Outptr_result_maybenull_ const WCHAR **ppwchCandidateString) = 0;
    virtual BOOL _SetSelectionInPage(int nPos) = 0;

    virtual HRESULT OnSetThreadFocus() = 0;
    virtual HRESULT OnKillThreadFocus() = 0;

    virtual void RemoveSpecificCandidateFromList(_In_ LCID Locale, _Inout_ std::vector<CCandidateListItem> &candidateList, _In_ CStringRange &srgCandidateString) = 0;
    virtual void AdviseUIChangedByArrowKey(_In_ KEYSTROKE_FUNCTION arrowKey) = 0;
};

struct IWindowsIMECompositionBuffer
{
    // functions for the composition object.
    virtual void _TerminateComposition(TfEditCookie ec, _In_ ITfContext *pContext, BOOL isCalledFromDeactivate = FALSE) = 0;

    // key event handlers for composition/candidate/phrase common objects.
    virtual HRESULT _HandleComplete(TfEditCookie ec, _In_ ITfContext *pContext) = 0;
    virtual HRESULT _HandleCancel(TfEditCookie ec, _In_ ITfContext* pContext) = 0;
    // key event handlers for composition object.
    virtual HRESULT _HandleCompositionInput(TfEditCookie ec, _In_ ITfContext* pContext, WCHAR wch) = 0;
    virtual HRESULT _HandleCompositionFinalize(TfEditCookie ec, _In_ ITfContext* pContext, BOOL fCandidateList) = 0;

    virtual HRESULT _HandleCompositionConvert(TfEditCookie ec, _In_ ITfContext* pContext, BOOL isWildcardSearch) = 0;
    virtual HRESULT _HandleCompositionBackspace(TfEditCookie ec, _In_ ITfContext* pContext) = 0;
    virtual HRESULT _HandleCompositionArrowKey(TfEditCookie ec, _In_ ITfContext* pContext, KEYSTROKE_FUNCTION keyFunction) = 0;
    virtual HRESULT _HandleCompositionPunctuation(TfEditCookie ec, _In_ ITfContext* pContext, WCHAR wch) = 0;
    virtual HRESULT _HandleCompositionDoubleSingleByte(TfEditCookie ec, _In_ ITfContext* pContext, WCHAR wch) = 0;
    // key event handlers for candidate object.
    virtual HRESULT _HandleCandidateFinalize(TfEditCookie ec, _In_ ITfContext* pContext) = 0;
    virtual HRESULT _HandleCandidateConvert(TfEditCookie ec, _In_ ITfContext* pContext) = 0;
    virtual HRESULT _HandleCandidateArrowKey(TfEditCookie ec, _In_ ITfContext* pContext, _In_ KEYSTROKE_FUNCTION keyFunction) = 0;
    virtual HRESULT _HandleCandidateSelectByNumber(TfEditCookie ec, _In_ ITfContext* pContext, _In_ UINT uCode) = 0;
    // key event handlers for phrase object.
    virtual HRESULT _HandlePhraseFinalize(TfEditCookie ec, _In_ ITfContext* pContext) = 0;
    virtual HRESULT _HandlePhraseArrowKey(TfEditCookie ec, _In_ ITfContext* pContext, _In_ KEYSTROKE_FUNCTION keyFunction) = 0;
    virtual HRESULT _HandlePhraseSelectByNumber(TfEditCookie ec, _In_ ITfContext* pContext, _In_ UINT uCode) = 0;

    // functions for the composition object.
    virtual HRESULT _HandleCompositionInputWorker(_In_ WindowsImeLib::ICompositionProcessorEngine *pCompositionProcessorEngine, TfEditCookie ec, _In_ ITfContext *pContext) = 0;
    virtual HRESULT _CreateAndStartCandidate(_In_ WindowsImeLib::ICompositionProcessorEngine *pCompositionProcessorEngine, TfEditCookie ec, _In_ ITfContext *pContext) = 0;
    virtual HRESULT _HandleCandidateWorker(TfEditCookie ec, _In_ ITfContext *pContext) = 0;

    virtual HRESULT _AddComposingAndChar(TfEditCookie ec, _In_ ITfContext *pContext, _In_ CStringRange *pstrAddString) = 0;
    virtual HRESULT _AddCharAndFinalize(TfEditCookie ec, _In_ ITfContext *pContext, _In_ CStringRange *pstrAddString) = 0;

    virtual BOOL _FindComposingRange(TfEditCookie ec, _In_ ITfContext *pContext, _In_ ITfRange *pSelection, _Outptr_result_maybenull_ ITfRange **ppRange) = 0;
    virtual HRESULT _SetInputString(TfEditCookie ec, _In_ ITfContext *pContext, _Out_opt_ ITfRange *pRange, _In_ CStringRange *pstrAddString, BOOL exist_composing) = 0;
    virtual HRESULT _InsertAtSelection(TfEditCookie ec, _In_ ITfContext *pContext, _In_ CStringRange *pstrAddString, _Outptr_ ITfRange **ppCompRange) = 0;

    virtual HRESULT _RemoveDummyCompositionForComposing(TfEditCookie ec, _In_ ITfComposition *pComposition) = 0;

    // function for the language property
    virtual BOOL _SetCompositionLanguage(TfEditCookie ec, _In_ ITfContext *pContext) = 0;

    // function for the display attribute
    virtual void _ClearCompositionDisplayAttributes(TfEditCookie ec, _In_ ITfContext *pContext) = 0;
    virtual BOOL _SetCompositionDisplayAttributes(TfEditCookie ec, _In_ ITfContext *pContext, TfGuidAtom gaDisplayAttribute) = 0;

    virtual BOOL _IsRangeCovered(TfEditCookie ec, _In_ ITfRange *pRangeTest, _In_ ITfRange *pRangeCover) = 0;

    //
    virtual wil::com_ptr<IWindowsIMECandidateList> GetCandidateList() = 0;
    virtual wil::com_ptr<ITfContext> GetContext() = 0;
    virtual CANDIDATE_MODE CandidateMode() = 0;
    virtual bool IsCandidateWithWildcard() = 0;
    virtual void ResetCandidateState() = 0;
};

struct ICompositionProcessorEngineOwner
{
    virtual ~ICompositionProcessorEngineOwner() {}

    virtual wchar_t ConvertVKey(UINT code) = 0;
    virtual UINT VKeyFromVKPacketAndWchar(UINT vk, WCHAR wch) = 0;
    virtual bool _IsKeyboardDisabled() = 0;
    virtual BOOL _IsComposing() = 0;
    virtual std::shared_ptr<IWindowsIMECompositionBuffer> GetCompositionBuffer() = 0;

    virtual HRESULT _SubmitEditSessionTask(_In_ ITfContext* context, const std::function<HRESULT (TfEditCookie ec, IWindowsIMECompositionBuffer* pv)>& editSesisonTask, DWORD tfEsFlags) = 0;

//    virtual void _StartComposition(_In_ ITfContext *pContext) = 0;
//    virtual void _EndComposition(_In_opt_ ITfContext *pContext) = 0;
    virtual VOID _DeleteCandidateList(BOOL fForce, _In_opt_ ITfContext *pContext) = 0;
    virtual void* GetTextService() = 0;
};

struct ICompositionProcessorEngine
{
    virtual ~ICompositionProcessorEngine() {}

    virtual BOOL Initialize() = 0;

    virtual void OnKeyEvent(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pIsEaten, DWORD modifiers, DWORD uniqueModifiers, bool isTest, bool isDown) = 0;
    virtual HRESULT KeyHandlerEditSession_DoEditSession(TfEditCookie ec, _KEYSTROKE_STATE _KeyState, _In_ ITfContext* _pContext, UINT _uCode, WCHAR _wch,
        _In_ WindowsImeLib::IWindowsIMECompositionBuffer* textService) = 0;

    virtual BOOL AddVirtualKey(WCHAR wch) = 0;
    virtual void RemoveVirtualKey(DWORD_PTR dwIndex) = 0;
    virtual void PurgeVirtualKey() = 0;

    virtual DWORD_PTR GetVirtualKeyLength() = 0;

    virtual void GetReadingStrings(_Inout_ std::vector<CStringRange> *pReadingStrings, _Out_ BOOL *pIsWildcardIncluded) = 0;
    virtual void GetCandidateList(_Inout_ std::vector<CCandidateListItem> *pCandidateList, BOOL isIncrementalWordSearch, BOOL isWildcardSearch) = 0;
    virtual void GetCandidateStringInConverted(CStringRange &searchString, _In_ std::vector<CCandidateListItem> *pCandidateList) = 0;

//     // Preserved key handler
//     virtual void OnPreservedKey(REFGUID rguid, _Out_ BOOL *pIsEaten, _In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId) = 0;

    // Punctuation
    // virtual BOOL IsPunctuation(WCHAR wch) = 0;
    virtual WCHAR GetPunctuation(WCHAR wch) = 0;

    virtual BOOL IsDoubleSingleByte(WCHAR wch) = 0;
    virtual BOOL IsMakePhraseFromText() = 0;

    virtual void EndComposition(_In_opt_ ITfContext* pContext) = 0;

    // Language bar control
//     virtual void ConversionModeCompartmentUpdated(_In_ ITfThreadMgr *pThreadMgr) = 0;

    virtual std::vector<DWORD>* GetCandidateListIndexRange() = 0;

//    // Compartment
//    virtual HRESULT CompartmentCallback(REFGUID guidCompartment) noexcept = 0;
//    virtual void ClearCompartment(ITfThreadMgr *pThreadMgr, TfClientId tfClientId) = 0;

	virtual void UpdateCustomState(const std::string& customStateJson) = 0;
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
    // GUIDs
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

    virtual std::shared_ptr<ICompositionProcessorEngine> CreateCompositionProcessorEngine(ICompositionProcessorEngineOwner* owner) = 0;
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
