// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


#pragma once

#include "Private.h"
#include "../WindowsImeLib.h"
#include "sal.h"
//#include "TableDictionaryEngine.h"
#include "BaseStructure.h"
//#include "FileMapping.h"
#include "Compartment.h"
//#include "SampleIMEDefine.h"
#include "LanguageBar.h"

class CCompositionProcessorEngine :
    public WindowsImeLib::ICompositionProcessorEngineOwner,
    public std::enable_shared_from_this<CCompositionProcessorEngine>
{
public:
    CCompositionProcessorEngine(void);
    ~CCompositionProcessorEngine(void);

    void Initialize();
    void ClearCompartment(_In_ ITfThreadMgr* pThreadMgr, TfClientId tfClientId);

    BOOL SetupLanguageProfile(LANGID langid, REFGUID guidLanguageProfile, _In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId, BOOL isSecureMode, BOOL isComLessMode);

    // Get language profile.
    GUID GetLanguageProfile(LANGID *plangid);

    // Get locale
    LCID GetLocale();

    BOOL IsKeyEaten(_In_ ITfThreadMgr* pThreadMgr, TfClientId tfClientId, UINT code, _Inout_updates_(1) WCHAR *pwch,
        BOOL isComposing, CANDIDATE_MODE candidateMode, BOOL isCandidateWithWildcard, _Out_opt_ _KEYSTROKE_STATE *pKeyState);
    BOOL IsVirtualKeyNeed(UINT uCode, _In_reads_(1) WCHAR *pwch, BOOL fComposing, CANDIDATE_MODE candidateMode, BOOL hasCandidateWithWildcard, _Out_opt_ _KEYSTROKE_STATE *pKeyState);

    BOOL AddVirtualKey(WCHAR wch);
    void RemoveVirtualKey(DWORD_PTR dwIndex);
    void PurgeVirtualKey();

    DWORD_PTR GetVirtualKeyLength();

    void GetReadingStrings(_Inout_ CSampleImeArray<CStringRange> *pReadingStrings, _Out_ BOOL *pIsWildcardIncluded);
    void GetCandidateList(_Inout_ CSampleImeArray<CCandidateListItem> *pCandidateList, BOOL isIncrementalWordSearch, BOOL isWildcardSearch);
    void GetCandidateStringInConverted(CStringRange &searchString, _In_ CSampleImeArray<CCandidateListItem> *pCandidateList);

    // Preserved key handler
    void OnPreservedKey(REFGUID rguid, _Out_ BOOL *pIsEaten, _In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId);

    // Punctuation
    BOOL IsPunctuation(WCHAR wch);
    WCHAR GetPunctuation(WCHAR wch);

    BOOL IsDoubleSingleByte(WCHAR wch);
    BOOL IsMakePhraseFromText();

    // Language bar control
    void SetLanguageBarStatus(DWORD status, BOOL isSet);

    void ConversionModeCompartmentUpdated(_In_ ITfThreadMgr *pThreadMgr);

    void ShowAllLanguageBarIcons();
    void HideAllLanguageBarIcons();

    CCandidateRange *GetCandidateListIndexRange();
    UINT GetCandidateWindowWidth();

private:
    void SetupLanguageBar(_In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId, BOOL isSecureMode,
        _In_reads_(countButtons) const WindowsImeLib::LanguageBarButtonProperty* properties, UINT countButtons) override;
    void SetDefaultCandidateTextFont(int idsDefaultFont) override;
    BOOL GetCompartmentBool(ITfThreadMgr *pThreadMgr, TfClientId tfClientId, REFGUID guidCompartment) override;
    void SetCompartmentBool(ITfThreadMgr *pThreadMgr, TfClientId tfClientId, REFGUID guidCompartment, BOOL value) override;
    DWORD GetCompartmentDword(ITfThreadMgr *pThreadMgr, TfClientId tfClientId, REFGUID guidCompartment) override;
    void SetCompartmentDword(ITfThreadMgr *pThreadMgr, TfClientId tfClientId, REFGUID guidCompartment, DWORD value) override;
    void ClearCompartment(ITfThreadMgr *pThreadMgr, TfClientId tfClientId, REFGUID guidCompartment) override;

private:
	std::shared_ptr<WindowsImeLib::ICompositionProcessorEngine> processorEngine;

private:
//    WCHAR GetVirtualKey(DWORD_PTR dwIndex);
//    BOOL IsWildcard() { return _isWildcard; }
//    BOOL IsDisableWildcardAtFirst() { return _isDisableWildcardAtFirst; }
//    BOOL IsWildcardChar(WCHAR wch) { return ((IsWildcardOneChar(wch) || IsWildcardAllChar(wch)) ? TRUE : FALSE); }
//    BOOL IsWildcardOneChar(WCHAR wch) { return (wch == L'?' ? TRUE : FALSE); }
//    BOOL IsWildcardAllChar(WCHAR wch) { return (wch == L'*' ? TRUE : FALSE); }
//    BOOL IsKeystrokeSort() { return _isKeystrokeSort; }
//
//    // Dictionary engine
//    BOOL IsDictionaryAvailable() { return (_pTableDictionaryEngine ? TRUE : FALSE); }
//
//    inline UINT GetCandidateListPhraseModifier() { return _candidateListPhraseModifier; }

private:
//    void InitKeyStrokeTable();
    BOOL InitLanguageBar(_In_ CLangBarItemButton *pLanguageBar, _In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId, REFGUID guidCompartment);
//
//    struct _KEYSTROKE;
//    BOOL IsVirtualKeyKeystrokeComposition(UINT uCode, _Out_opt_ _KEYSTROKE_STATE *pKeyState, KEYSTROKE_FUNCTION function);
//    BOOL IsVirtualKeyKeystrokeCandidate(UINT uCode, _In_ _KEYSTROKE_STATE *pKeyState, CANDIDATE_MODE candidateMode, _Out_ BOOL *pfRetCode, _In_ CSampleImeArray<_KEYSTROKE> *pKeystrokeMetric);
//    BOOL IsKeystrokeRange(UINT uCode, _Out_ _KEYSTROKE_STATE *pKeyState, CANDIDATE_MODE candidateMode);
//
//    void SetupKeystroke();
//    void SetupPreserved(_In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId);
//    void SetupConfiguration();
//    void SetKeystrokeTable(_Inout_ CSampleImeArray<_KEYSTROKE> *pKeystroke);
//    void SetupPunctuationPair();
	void CreateLanguageBarButton(DWORD dwEnable, GUID guidLangBar, _In_z_ LPCWSTR pwszDescriptionValue, _In_z_ LPCWSTR pwszTooltipValue, DWORD dwOnIconIndex, DWORD dwOffIconIndex, _Outptr_result_maybenull_ CLangBarItemButton **ppLangBarItemButton, BOOL isSecureMode);
//    void SetInitialCandidateListRange();
//	void InitializeSampleIMECompartment(_In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId);
//
//    class XPreservedKey;
//    void SetPreservedKey(const CLSID clsid, TF_PRESERVEDKEY & tfPreservedKey, _In_z_ LPCWSTR pwszDescription, _Out_ XPreservedKey *pXPreservedKey);
//    BOOL InitPreservedKey(_In_ XPreservedKey *pXPreservedKey, _In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId);
//    BOOL CheckShiftKeyOnly(_In_ CSampleImeArray<TF_PRESERVEDKEY> *pTSFPreservedKeyTable);
//
    static HRESULT CompartmentCallback(_In_ void *pv, REFGUID guidCompartment);
//    void PrivateCompartmentsUpdated(_In_ ITfThreadMgr *pThreadMgr);
//    void KeyboardOpenCompartmentUpdated(_In_ ITfThreadMgr *pThreadMgr);
//    
//    BOOL SetupDictionaryFile();
//    CFile* GetDictionaryFile();

private:
//    struct _KEYSTROKE
//    {
//        UINT VirtualKey;
//        UINT Modifiers;
//        KEYSTROKE_FUNCTION Function;
//
//        _KEYSTROKE()
//        {
//            VirtualKey = 0;
//            Modifiers = 0;
//            Function = FUNCTION_NONE;
//        }
//    };
//    _KEYSTROKE _keystrokeTable[26];
//
//    CTableDictionaryEngine* _pTableDictionaryEngine;
//    CStringRange _keystrokeBuffer;
//
//    BOOL _hasWildcardIncludedInKeystrokeBuffer;
//
//    LANGID _langid;
//    GUID _guidProfile;
//    TfClientId  _tfClientId;
//
//    CSampleImeArray<_KEYSTROKE> _KeystrokeComposition;
//    CSampleImeArray<_KEYSTROKE> _KeystrokeCandidate;
//    CSampleImeArray<_KEYSTROKE> _KeystrokeCandidateWildcard;
//    CSampleImeArray<_KEYSTROKE> _KeystrokeCandidateSymbol;
//    CSampleImeArray<_KEYSTROKE> _KeystrokeSymbol;
//
//    // Preserved key data
//    class XPreservedKey
//    {
//    public:
//        XPreservedKey();
//        ~XPreservedKey();
//        BOOL UninitPreservedKey(_In_ ITfThreadMgr *pThreadMgr);
//
//    public:
//        CSampleImeArray<TF_PRESERVEDKEY> TSFPreservedKeyTable;
//        GUID Guid;
//        LPCWSTR Description;
//    };
//
//    XPreservedKey _PreservedKey_IMEMode;
//    XPreservedKey _PreservedKey_DoubleSingleByte;
//    XPreservedKey _PreservedKey_Punctuation;
//
//    // Punctuation data
//    CSampleImeArray<CPunctuationPair> _PunctuationPair;
//    CSampleImeArray<CPunctuationNestPair> _PunctuationNestPair;

    // Language bar data
    std::vector<wil::com_ptr<CLangBarItemButton>> m_langaugeBarButtons;

    // Compartment
	std::shared_ptr<CCompartment> _pCompartmentConversion;
	wil::com_ptr<CCompartmentEventSink> _pCompartmentConversionEventSink;

	std::vector<wil::com_ptr<CCompartmentEventSink>> m_langaugeBarButtonEventSinks;


//    // Configuration data
//    BOOL _isWildcard : 1;
//    BOOL _isDisableWildcardAtFirst : 1;
//    BOOL _hasMakePhraseFromText : 1;
//    BOOL _isKeystrokeSort : 1;
//    BOOL _isComLessMode : 1;
//    CCandidateRange _candidateListIndexRange;
//    UINT _candidateListPhraseModifier;
//    UINT _candidateWndWidth;
//
//    CFileMapping* _pDictionaryFile;
//
//    static const int OUT_OF_FILE_INDEX = -1;

};

