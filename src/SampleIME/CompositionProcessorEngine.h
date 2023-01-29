// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


#pragma once

#include "pch.h"
#include "../WindowsImeLib.h"
#include "TableDictionaryEngine.h"
#include "SampleIMEBaseStructure.h"
#include "FileMapping.h"
#include "SampleIMEDefine.h"
#include "SampleIMEGlobals.h"

class CompositionProcessorEngine :
    public WindowsImeLib::ICompositionProcessorEngine,
    public std::enable_shared_from_this<CompositionProcessorEngine>
{
public:
    CompositionProcessorEngine(
        const std::shared_ptr<WindowsImeLib::IWindowsIMECompositionBuffer>& compositionBuffer,
        const std::shared_ptr<WindowsImeLib::IWindowsIMECandidateListView>& candidateListView);
    ~CompositionProcessorEngine(void);

    BOOL Initialize() override;
    // BOOL SetupLanguageProfile() override; // LANGID langid, REFGUID guidLanguageProfile, _In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId, BOOL isSecureMode, BOOL isComLessMode) override;

	void OnKeyEvent(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pIsEaten, wchar_t wch, UINT vkPackSource, bool isKbdDisabled, DWORD modifiers, DWORD uniqueModifiers, bool isTest, bool isUp) override;

private:
	// in KeyHandlerEditSession.cpp
	HRESULT KeyHandlerEditSession_DoEditSession(_KEYSTROKE_STATE _KeyState, _In_ ITfContext* _pContext, UINT _uCode, WCHAR _wch);

public:
    BOOL AddVirtualKey(WCHAR wch);
    void RemoveVirtualKey(DWORD_PTR dwIndex);
    void PurgeVirtualKey();

    DWORD_PTR GetVirtualKeyLength() { return _keystrokeBuffer.GetLength(); }

    void GetReadingStrings(_Inout_ std::vector<CStringRange> *pReadingStrings, _Out_ BOOL *pIsWildcardIncluded);
    void GetCandidateList(_Inout_ std::vector<shared_wstring>& pCandidateList, BOOL isIncrementalWordSearch, BOOL isWildcardSearch) override;
    void GetCandidateStringInConverted(const shared_wstring& searchString, _In_ std::vector<CCandidateListItem> *pCandidateList);

//    // Preserved key handler
//    void OnPreservedKey(REFGUID rguid, _Out_ BOOL *pIsEaten, _In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId) override;

    // Punctuation
    BOOL IsPunctuation(WCHAR wch);
    WCHAR GetPunctuation(WCHAR wch);

    BOOL IsDoubleSingleByte(WCHAR wch);
    BOOL IsMakePhraseFromText() { return _hasMakePhraseFromText; }

    void EndComposition(_In_opt_ ITfContext* pContext) override;
    void FinalizeCandidateList(_In_ ITfContext *pContext) override;
    VOID _DeleteCandidateList(BOOL isForce, _In_opt_ ITfContext *pContext) override;

    // Language bar control
//    void ConversionModeCompartmentUpdated(_In_ ITfThreadMgr *pThreadMgr) override;

    inline std::vector<DWORD> *GetCandidateListIndexRange() { return &_candidateListIndexRange; }

//    HRESULT CompartmentCallback(REFGUID guidCompartment) noexcept override;
//    void ClearCompartment(ITfThreadMgr *pThreadMgr, TfClientId tfClientId) override;

    void UpdateCustomState(const std::string& stateJson) override
    {
        const auto json = nlohmann::json::parse(stateJson);
        m_compartmentIsOpen = json["isOpen"].get<bool>();
        m_compartmentIsDoubleSingleByte = json["isDouble"].get<bool>();
        m_compartmentIsPunctuation = json["isPunctuation"].get<bool>();
    }

    void SetCandidateKeyStrokeCategory(KEYSTROKE_CATEGORY keyStrokeCategory) { _keyStrokeCategory = keyStrokeCategory; }
    CANDIDATE_MODE CandidateMode() { return _candidateMode; }
    void SetCandidateMode(CANDIDATE_MODE candidateMode) { _candidateMode = candidateMode; }
    bool IsCandidateWithWildcard() { return _isCandidateWithWildcard; }
    void SetIsCandidateWithWildcard(BOOL f) { _isCandidateWithWildcard = f; }
    void ResetCandidateState()
    {
        _candidateMode  = CANDIDATE_NONE;
        _isCandidateWithWildcard = FALSE;
    }

private:
    BOOL IsVirtualKeyNeed(UINT uCode, wchar_t wch, BOOL fComposing, BOOL hasCandidateWithWildcard, _Out_opt_ _KEYSTROKE_STATE *pKeyState);
    WCHAR GetVirtualKey(DWORD_PTR dwIndex);
    BOOL IsWildcard() { return _isWildcard; }
    BOOL IsDisableWildcardAtFirst() { return _isDisableWildcardAtFirst; }
    BOOL IsWildcardChar(WCHAR wch) { return ((IsWildcardOneChar(wch) || IsWildcardAllChar(wch)) ? TRUE : FALSE); }
    BOOL IsWildcardOneChar(WCHAR wch) { return (wch == L'?' ? TRUE : FALSE); }
    BOOL IsWildcardAllChar(WCHAR wch) { return (wch == L'*' ? TRUE : FALSE); }
    BOOL IsKeystrokeSort() { return _isKeystrokeSort; }

    // Dictionary engine
    BOOL IsDictionaryAvailable() { return (_pTableDictionaryEngine ? TRUE : FALSE); }

    inline UINT GetCandidateListPhraseModifier() { return _candidateListPhraseModifier; }

private:
    void InitKeyStrokeTable();
//    BOOL InitLanguageBar(_In_ CLangBarItemButton *pLanguageBar, _In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId, REFGUID guidCompartment);

    struct _KEYSTROKE;
    BOOL IsVirtualKeyKeystrokeComposition(UINT uCode, _Out_opt_ _KEYSTROKE_STATE *pKeyState, KEYSTROKE_FUNCTION function);
    BOOL IsVirtualKeyKeystrokeCandidate(UINT uCode, _In_ _KEYSTROKE_STATE *pKeyState, _Out_ BOOL *pfRetCode, _In_ std::vector<_KEYSTROKE> *pKeystrokeMetric);
    BOOL IsKeystrokeRange(UINT uCode, _Out_ _KEYSTROKE_STATE *pKeyState);

    void SetupKeystroke();
//    void SetupPreserved(_In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId);
    void SetupConfiguration();
//    void SetupLanguageBar(_In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId, BOOL isSecureMode);
    void SetKeystrokeTable(_Inout_ std::vector<_KEYSTROKE> *pKeystroke);
    void SetupPunctuationPair();
//    void CreateLanguageBarButton(DWORD dwEnable, GUID guidLangBar, _In_z_ LPCWSTR pwszDescriptionValue, _In_z_ LPCWSTR pwszTooltipValue, DWORD dwOnIconIndex, DWORD dwOffIconIndex, _Outptr_result_maybenull_ CLangBarItemButton **ppLangBarItemButton, BOOL isSecureMode);
    void SetInitialCandidateListRange();
//    void SetDefaultCandidateTextFont();
//    void InitializeSampleIMECompartment(_In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId);

//    class XPreservedKey;
//    void SetPreservedKey(const CLSID clsid, TF_PRESERVEDKEY & tfPreservedKey, _In_z_ LPCWSTR pwszDescription, _Out_ XPreservedKey *pXPreservedKey);
//    BOOL InitPreservedKey(_In_ XPreservedKey *pXPreservedKey, _In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId);
//    BOOL CheckShiftKeyOnly(_In_ std::vector<TF_PRESERVEDKEY> *pTSFPreservedKeyTable);

//     void PrivateCompartmentsUpdated(_In_ ITfThreadMgr *pThreadMgr);
//     void KeyboardOpenCompartmentUpdated(_In_ ITfThreadMgr *pThreadMgr);
    
    BOOL SetupDictionaryFile();
    CFile* GetDictionaryFile();

    // in KeyEventSink.cpp
    BOOL _IsKeyEaten(_In_ UINT codeIn, wchar_t wch, UINT vkPackSource, bool isKbdDisabled, _Out_opt_ _KEYSTROKE_STATE *pKeyState);
    HRESULT OnTestKeyDown(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pIsEaten, wchar_t wch, UINT vkPackSource, bool isKbdDisabled);
    HRESULT OnKeyDown(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pIsEaten, wchar_t wch, UINT vkPackSource, bool isKbdDisabled);
    HRESULT OnTestKeyUp(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pIsEaten, wchar_t wch, UINT vkPackSource, bool isKbdDisabled);
    HRESULT OnKeyUp(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pIsEaten, wchar_t wch, UINT vkPackSource, bool isKbdDisabled);

private:
    const std::shared_ptr<WindowsImeLib::IWindowsIMECompositionBuffer> m_compositionBuffer;
    const std::shared_ptr<WindowsImeLib::IWindowsIMECandidateListView> m_candidateListView;

    struct _KEYSTROKE
    {
        UINT VirtualKey;
        UINT Modifiers;
        KEYSTROKE_FUNCTION Function;

        _KEYSTROKE()
        {
            VirtualKey = 0;
            Modifiers = 0;
            Function = FUNCTION_NONE;
        }
    };
    _KEYSTROKE _keystrokeTable[26];

    CTableDictionaryEngine* _pTableDictionaryEngine;
    CStringRange _keystrokeBuffer;

    BOOL _hasWildcardIncludedInKeystrokeBuffer;

//    LANGID _langid;
//    GUID _guidProfile;
//    TfClientId  _tfClientId;

    std::vector<_KEYSTROKE> _KeystrokeComposition;
    std::vector<_KEYSTROKE> _KeystrokeCandidate;
    std::vector<_KEYSTROKE> _KeystrokeCandidateWildcard;
    std::vector<_KEYSTROKE> _KeystrokeCandidateSymbol;
    std::vector<_KEYSTROKE> _KeystrokeSymbol;

    // Preserved key data
//    class XPreservedKey
//    {
//    public:
//        XPreservedKey();
//        ~XPreservedKey();
//        BOOL UninitPreservedKey(_In_ ITfThreadMgr *pThreadMgr);
//
//    public:
//        std::vector<TF_PRESERVEDKEY> TSFPreservedKeyTable;
//        GUID Guid;
//        LPCWSTR Description;
//    };
//
//    XPreservedKey _PreservedKey_IMEMode;
//    XPreservedKey _PreservedKey_DoubleSingleByte;
//    XPreservedKey _PreservedKey_Punctuation;

    // Punctuation data
    std::vector<CPunctuationPair> _PunctuationPair;
    std::vector<CPunctuationNestPair> _PunctuationNestPair;

    // Language bar data
//    CLangBarItemButton* _pLanguageBar_IMEMode;
//    CLangBarItemButton* _pLanguageBar_DoubleSingleByte;
//    CLangBarItemButton* _pLanguageBar_Punctuation;

    // Compartment
//    CCompartment* _pCompartmentConversion;
//    CCompartmentEventSink* _pCompartmentConversionEventSink;
//    CCompartmentEventSink* _pCompartmentKeyboardOpenEventSink;
//    CCompartmentEventSink* _pCompartmentDoubleSingleByteEventSink;
//    CCompartmentEventSink* _pCompartmentPunctuationEventSink;

    bool m_compartmentIsOpen = false;
    bool m_compartmentIsDoubleSingleByte = false;
    bool m_compartmentIsPunctuation = false;

    // Configuration data
    BOOL _isWildcard : 1;
    BOOL _isDisableWildcardAtFirst : 1;
    BOOL _hasMakePhraseFromText : 1;
    BOOL _isKeystrokeSort : 1;
//    BOOL _isComLessMode : 1;
    std::vector<DWORD> _candidateListIndexRange;
    UINT _candidateListPhraseModifier;
//    UINT _candidateWndWidth;

    KEYSTROKE_CATEGORY _keyStrokeCategory;
    CANDIDATE_MODE _candidateMode;
    BOOL _isCandidateWithWildcard = FALSE;

    CFileMapping* _pDictionaryFile;

    static const int OUT_OF_FILE_INDEX = -1;
};

class SampleIMEProcessor :
    public WindowsImeLib::ITextInputProcessor,
    public std::enable_shared_from_this<SampleIMEProcessor>
{
public:
    SampleIMEProcessor(WindowsImeLib::ITextInputFramework* framework);
    virtual ~SampleIMEProcessor();

    std::wstring TestMethod(const std::wstring& src) override;
    void SetFocus(bool isGotten) override;
    void UpdateCustomState(const std::string& stateJson) override;

private:
    WindowsImeLib::ITextInputFramework* m_framework;
};
