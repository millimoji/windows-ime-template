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
#include "../Compartment.h"
//#include "SampleIMEDefine.h"
#include "../LanguageBar.h"

class CCompositionProcessorEngine : public WindowsImeLib::ICompositionProcessorEngine
{
public:
    CCompositionProcessorEngine(void);
    ~CCompositionProcessorEngine(void);

    BOOL SetupLanguageProfile(LANGID langid, REFGUID guidLanguageProfile, _In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId, BOOL isSecureMode, BOOL isComLessMode) override;

    // Get language profile.
    GUID GetLanguageProfile(LANGID *plangid) override;

    // Get locale
    LCID GetLocale() override;

    BOOL IsVirtualKeyNeed(UINT uCode, _In_reads_(1) WCHAR *pwch, BOOL fComposing, CANDIDATE_MODE candidateMode, BOOL hasCandidateWithWildcard, _Out_opt_ _KEYSTROKE_STATE *pKeyState) override;

    BOOL AddVirtualKey(WCHAR wch) override;
    void RemoveVirtualKey(DWORD_PTR dwIndex) override;
    void PurgeVirtualKey() override;

    DWORD_PTR GetVirtualKeyLength() override;

    void GetReadingStrings(_Inout_ CSampleImeArray<CStringRange> *pReadingStrings, _Out_ BOOL *pIsWildcardIncluded) override;
    void GetCandidateList(_Inout_ CSampleImeArray<CCandidateListItem> *pCandidateList, BOOL isIncrementalWordSearch, BOOL isWildcardSearch) override;
    void GetCandidateStringInConverted(CStringRange &searchString, _In_ CSampleImeArray<CCandidateListItem> *pCandidateList) override;

    // Preserved key handler
    void OnPreservedKey(REFGUID rguid, _Out_ BOOL *pIsEaten, _In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId) override;

    // Punctuation
    BOOL IsPunctuation(WCHAR wch) override;
    WCHAR GetPunctuation(WCHAR wch) override;

    BOOL IsDoubleSingleByte(WCHAR wch) override;
    BOOL IsMakePhraseFromText() override;

    // Language bar control
    void SetLanguageBarStatus(DWORD status, BOOL isSet) override;

    void ConversionModeCompartmentUpdated(_In_ ITfThreadMgr *pThreadMgr) override;

    void ShowAllLanguageBarIcons() override;
    void HideAllLanguageBarIcons() override;

    CCandidateRange *GetCandidateListIndexRange() override;
    UINT GetCandidateWindowWidth() override;

private:
	std::shared_ptr<WindowsImeLib::ICompositionProcessorEngine> processorEngine;
};

