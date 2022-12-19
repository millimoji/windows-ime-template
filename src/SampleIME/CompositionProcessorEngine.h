// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


#pragma once

#include "pch.h"
#include "../WindowsImeLib.h"
#include "sal.h"
#include "TableDictionaryEngine.h"
// #include "KeyHandlerEditSession.h"
#include "../SampleIMEBaseStructure.h"
#include "FileMapping.h"
#include "../Compartment.h"
#include "../define.h"
#include "../LanguageBar.h"

class CCompositionProcessorEngine : public std::enable_shared_from_this<CCompositionProcessorEngine>, public WindowsImeLib::ICompositionProcessorEngine
{
public:
    CCompositionProcessorEngine(void);
    ~CCompositionProcessorEngine(void);

    BOOL SetupLanguageProfile(LANGID langid, REFGUID guidLanguageProfile, _In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId, BOOL isSecureMode, BOOL isComLessMode) override;

    // Get language profile.
    GUID GetLanguageProfile(LANGID *plangid) override
    {
        *plangid = _langid;
        return _guidProfile;
    }
    // Get locale
    LCID GetLocale() override
    {
        return MAKELCID(_langid, SORT_DEFAULT);
    }

    BOOL IsVirtualKeyNeed(UINT uCode, _In_reads_(1) WCHAR *pwch, BOOL fComposing, CANDIDATE_MODE candidateMode, BOOL hasCandidateWithWildcard, _Out_opt_ _KEYSTROKE_STATE *pKeyState) override;

    BOOL AddVirtualKey(WCHAR wch) override;
    void RemoveVirtualKey(DWORD_PTR dwIndex) override;
    void PurgeVirtualKey() override;

    DWORD_PTR GetVirtualKeyLength()  override { return _keystrokeBuffer.GetLength(); }

    void GetReadingStrings(_Inout_ CSampleImeArray<CStringRange> *pReadingStrings, _Out_ BOOL *pIsWildcardIncluded) override;
    void GetCandidateList(_Inout_ CSampleImeArray<CCandidateListItem> *pCandidateList, BOOL isIncrementalWordSearch, BOOL isWildcardSearch) override;
    void GetCandidateStringInConverted(CStringRange &searchString, _In_ CSampleImeArray<CCandidateListItem> *pCandidateList) override;

    // Preserved key handler
    void OnPreservedKey(REFGUID rguid, _Out_ BOOL *pIsEaten, _In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId) override;

    // Punctuation
    BOOL IsPunctuation(WCHAR wch) override;
    WCHAR GetPunctuation(WCHAR wch) override;

    BOOL IsDoubleSingleByte(WCHAR wch) override;
    BOOL IsMakePhraseFromText()  override { return _hasMakePhraseFromText; }

    // Language bar control
    void SetLanguageBarStatus(DWORD status, BOOL isSet) override;

    void ConversionModeCompartmentUpdated(_In_ ITfThreadMgr *pThreadMgr) override;

    void ShowAllLanguageBarIcons() override;
    void HideAllLanguageBarIcons() override;

    inline CCandidateRange *GetCandidateListIndexRange()  override { return &_candidateListIndexRange; }
    inline UINT GetCandidateWindowWidth()  override { return _candidateWndWidth; }

private:
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
    BOOL InitLanguageBar(_In_ CLangBarItemButton *pLanguageBar, _In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId, REFGUID guidCompartment);

    struct _KEYSTROKE;
    BOOL IsVirtualKeyKeystrokeComposition(UINT uCode, _Out_opt_ _KEYSTROKE_STATE *pKeyState, KEYSTROKE_FUNCTION function);
    BOOL IsVirtualKeyKeystrokeCandidate(UINT uCode, _In_ _KEYSTROKE_STATE *pKeyState, CANDIDATE_MODE candidateMode, _Out_ BOOL *pfRetCode, _In_ CSampleImeArray<_KEYSTROKE> *pKeystrokeMetric);
    BOOL IsKeystrokeRange(UINT uCode, _Out_ _KEYSTROKE_STATE *pKeyState, CANDIDATE_MODE candidateMode);

    void SetupKeystroke();
    void SetupPreserved(_In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId);
    void SetupConfiguration();
    void SetupLanguageBar(_In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId, BOOL isSecureMode);
    void SetKeystrokeTable(_Inout_ CSampleImeArray<_KEYSTROKE> *pKeystroke);
    void SetupPunctuationPair();
    void CreateLanguageBarButton(DWORD dwEnable, GUID guidLangBar, _In_z_ LPCWSTR pwszDescriptionValue, _In_z_ LPCWSTR pwszTooltipValue, DWORD dwOnIconIndex, DWORD dwOffIconIndex, _Outptr_result_maybenull_ CLangBarItemButton **ppLangBarItemButton, BOOL isSecureMode);
    void SetInitialCandidateListRange();
    void SetDefaultCandidateTextFont();
	void InitializeSampleIMECompartment(_In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId);

    class XPreservedKey;
    void SetPreservedKey(const CLSID clsid, TF_PRESERVEDKEY & tfPreservedKey, _In_z_ LPCWSTR pwszDescription, _Out_ XPreservedKey *pXPreservedKey);
    BOOL InitPreservedKey(_In_ XPreservedKey *pXPreservedKey, _In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId);
    BOOL CheckShiftKeyOnly(_In_ CSampleImeArray<TF_PRESERVEDKEY> *pTSFPreservedKeyTable);

    static HRESULT CompartmentCallback(_In_ void *pv, REFGUID guidCompartment);
    void PrivateCompartmentsUpdated(_In_ ITfThreadMgr *pThreadMgr);
    void KeyboardOpenCompartmentUpdated(_In_ ITfThreadMgr *pThreadMgr);
    
    BOOL SetupDictionaryFile();
    CFile* GetDictionaryFile();

private:
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

    LANGID _langid;
    GUID _guidProfile;
    TfClientId  _tfClientId;

    CSampleImeArray<_KEYSTROKE> _KeystrokeComposition;
    CSampleImeArray<_KEYSTROKE> _KeystrokeCandidate;
    CSampleImeArray<_KEYSTROKE> _KeystrokeCandidateWildcard;
    CSampleImeArray<_KEYSTROKE> _KeystrokeCandidateSymbol;
    CSampleImeArray<_KEYSTROKE> _KeystrokeSymbol;

    // Preserved key data
    class XPreservedKey
    {
    public:
        XPreservedKey();
        ~XPreservedKey();
        BOOL UninitPreservedKey(_In_ ITfThreadMgr *pThreadMgr);

    public:
        CSampleImeArray<TF_PRESERVEDKEY> TSFPreservedKeyTable;
        GUID Guid;
        LPCWSTR Description;
    };

    XPreservedKey _PreservedKey_IMEMode;
    XPreservedKey _PreservedKey_DoubleSingleByte;
    XPreservedKey _PreservedKey_Punctuation;

    // Punctuation data
    CSampleImeArray<CPunctuationPair> _PunctuationPair;
    CSampleImeArray<CPunctuationNestPair> _PunctuationNestPair;

    // Language bar data
    CLangBarItemButton* _pLanguageBar_IMEMode;
    CLangBarItemButton* _pLanguageBar_DoubleSingleByte;
    CLangBarItemButton* _pLanguageBar_Punctuation;

    // Compartment
    CCompartment* _pCompartmentConversion;
    CCompartmentEventSink* _pCompartmentConversionEventSink;
    CCompartmentEventSink* _pCompartmentKeyboardOpenEventSink;
    CCompartmentEventSink* _pCompartmentDoubleSingleByteEventSink;
    CCompartmentEventSink* _pCompartmentPunctuationEventSink;

    // Configuration data
    BOOL _isWildcard : 1;
    BOOL _isDisableWildcardAtFirst : 1;
    BOOL _hasMakePhraseFromText : 1;
    BOOL _isKeystrokeSort : 1;
    BOOL _isComLessMode : 1;
    CCandidateRange _candidateListIndexRange;
    UINT _candidateListPhraseModifier;
    UINT _candidateWndWidth;

    CFileMapping* _pDictionaryFile;

    static const int OUT_OF_FILE_INDEX = -1;
};

