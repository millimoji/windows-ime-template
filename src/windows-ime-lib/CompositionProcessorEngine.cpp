﻿// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "../WindowsImeLib.h"
#include "Globals.h"
#include "CompositionProcessorEngine.h"
#include "Compartment.h"
#include "LanguageBar.h"

//////////////////////////////////////////////////////////////////////
//
// CompositionprocessorEngine implementation.
//
//////////////////////////////////////////////////////////////////////

//+---------------------------------------------------------------------------
//
// ctor
//
//----------------------------------------------------------------------------

CCompositionProcessorEngine::CCompositionProcessorEngine()
{
//    _pTableDictionaryEngine = nullptr;
//    _pDictionaryFile = nullptr;
//
//    _langid = 0xffff;
//    _guidProfile = GUID_NULL;
//    _tfClientId = TF_CLIENTID_NULL;
//
//    _pLanguageBar_IMEMode = nullptr;
//    _pLanguageBar_DoubleSingleByte = nullptr;
//    _pLanguageBar_Punctuation = nullptr;
//
//    _pCompartmentConversion = nullptr;
//    _pCompartmentKeyboardOpenEventSink = nullptr;
//    _pCompartmentConversionEventSink = nullptr;
//    _pCompartmentDoubleSingleByteEventSink = nullptr;
//    _pCompartmentPunctuationEventSink = nullptr;
//
//    _hasWildcardIncludedInKeystrokeBuffer = FALSE;
//
//    _isWildcard = FALSE;
//    _isDisableWildcardAtFirst = FALSE;
//    _hasMakePhraseFromText = FALSE;
//    _isKeystrokeSort = FALSE;
//
//    _candidateListPhraseModifier = 0;
//
//    _candidateWndWidth = CAND_WIDTH;
//
//    InitKeyStrokeTable();
}

void CCompositionProcessorEngine::Initialize()
{
    const auto thisAsOwner = std::static_pointer_cast<WindowsImeLib::ICompositionProcessorEngineOwner>(shared_from_this());
    processorEngine = WindowsImeLib::g_processorFactory->CreateCompositionProcessorEngine(std::weak_ptr(thisAsOwner));
}

//+---------------------------------------------------------------------------
//
// dtor
//
//----------------------------------------------------------------------------

void CCompositionProcessorEngine::ClearCompartment(ITfThreadMgr* pThreadMgr, TfClientId tfClientId)
{
    processorEngine->ClearCompartment(pThreadMgr, tfClientId);
}

CCompositionProcessorEngine::~CCompositionProcessorEngine()
{
    if (_pCompartmentConversion)
    {
        _pCompartmentConversion.reset();
    }
    if (_pCompartmentConversionEventSink)
    {
        _pCompartmentConversionEventSink->_Unadvise();
        _pCompartmentConversionEventSink.reset();
    }

    for (auto&& langBarButton: m_langaugeBarButtons)
    {
        if (langBarButton)
        {
            langBarButton->CleanUp();
            langBarButton.reset();
        }
    }

    for (auto&& eventSink: m_langaugeBarButtonEventSinks)
    {
        if (eventSink)
        {
            eventSink->_Unadvise();
            eventSink.reset();
        }
    }

//
//    if (_pDictionaryFile)
//    {
//        delete _pDictionaryFile;
//        _pDictionaryFile = nullptr;
//    }
}

//+---------------------------------------------------------------------------
//
// SetupLanguageProfile
//
// Setup language profile for Composition Processor Engine.
// param
//     [in] LANGID langid = Specify language ID
//     [in] GUID guidLanguageProfile - Specify GUID language profile which GUID is as same as Text Service Framework language profile.
//     [in] ITfThreadMgr - pointer ITfThreadMgr.
//     [in] tfClientId - TfClientId value.
//     [in] isSecureMode - secure mode
// returns
//     If setup succeeded, returns true. Otherwise returns false.
// N.B. For reverse conversion, ITfThreadMgr is NULL, TfClientId is 0 and isSecureMode is ignored.
//+---------------------------------------------------------------------------

BOOL CCompositionProcessorEngine::SetupLanguageProfile(LANGID langid, REFGUID guidLanguageProfile, _In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId, BOOL isSecureMode, BOOL isComLessMode)
{
    return processorEngine->SetupLanguageProfile(langid, guidLanguageProfile, pThreadMgr, tfClientId, isSecureMode, isComLessMode);
//    BOOL ret = TRUE;
//    if ((tfClientId == 0) && (pThreadMgr == nullptr))
//    {
//        ret = FALSE;
//        goto Exit;
//    }
//
//    _isComLessMode = isComLessMode;
//    _langid = langid;
//    _guidProfile = guidLanguageProfile;
//    _tfClientId = tfClientId;
//
//    SetupPreserved(pThreadMgr, tfClientId); 
//    InitializeSampleIMECompartment(pThreadMgr, tfClientId);
//    SetupPunctuationPair();
//    SetupLanguageBar(pThreadMgr, tfClientId, isSecureMode);
//    SetupKeystroke();
//    SetupConfiguration();
//    SetupDictionaryFile();
//
//Exit:
//    return ret;
}

// Get language profile.
GUID CCompositionProcessorEngine::GetLanguageProfile(LANGID *plangid)
{
    return processorEngine->GetLanguageProfile(plangid);
//  *plangid = _langid;
//  return _guidProfile;
}

// Get locale
LCID CCompositionProcessorEngine::GetLocale()
{
    return processorEngine->GetLocale();
    // return MAKELCID(_langid, SORT_DEFAULT);
}

BOOL CCompositionProcessorEngine::IsKeyEaten(
    _In_ ITfThreadMgr* pThreadMgr, TfClientId tfClientId, UINT code, _Inout_updates_(1) WCHAR *pwch,
    BOOL isComposing, CANDIDATE_MODE candidateMode, BOOL isCandidateWithWildcard, _Out_opt_ _KEYSTROKE_STATE *pKeyState)
{
    return processorEngine->IsKeyEaten(pThreadMgr, tfClientId, code, pwch, isComposing, candidateMode, isCandidateWithWildcard, pKeyState);
}

//+---------------------------------------------------------------------------
//
// CCompositionProcessorEngine::IsVirtualKeyNeed
//
// Test virtual key code need to the Composition Processor Engine.
// param
//     [in] uCode - Specify virtual key code.
//     [in/out] pwch       - char code
//     [in] fComposing     - Specified composing.
//     [in] fCandidateMode - Specified candidate mode.
//     [out] pKeyState     - Returns function regarding virtual key.
// returns
//     If engine need this virtual key code, returns true. Otherwise returns false.
//----------------------------------------------------------------------------

//BOOL CCompositionProcessorEngine::IsVirtualKeyNeed(UINT uCode, _In_reads_(1) WCHAR *pwch, BOOL fComposing, CANDIDATE_MODE candidateMode, BOOL hasCandidateWithWildcard, _Out_opt_ _KEYSTROKE_STATE *pKeyState)
//{
//    return processorEngine->IsVirtualKeyNeed(uCode, pwch, fComposing, candidateMode, hasCandidateWithWildcard, pKeyState);
//    if (pKeyState)
//    {
//        pKeyState->Category = CATEGORY_NONE;
//        pKeyState->Function = FUNCTION_NONE;
//    }
//
//    if (candidateMode == CANDIDATE_ORIGINAL || candidateMode == CANDIDATE_PHRASE || candidateMode == CANDIDATE_WITH_NEXT_COMPOSITION)
//    {
//        fComposing = FALSE;
//    }
//
//    if (fComposing || candidateMode == CANDIDATE_INCREMENTAL || candidateMode == CANDIDATE_NONE)
//    {
//        if (IsVirtualKeyKeystrokeComposition(uCode, pKeyState, FUNCTION_NONE))
//        {
//            return TRUE;
//        }
//        else if ((IsWildcard() && IsWildcardChar(*pwch) && !IsDisableWildcardAtFirst()) ||
//            (IsWildcard() && IsWildcardChar(*pwch) &&  IsDisableWildcardAtFirst() && _keystrokeBuffer.GetLength()))
//        {
//            if (pKeyState)
//            {
//                pKeyState->Category = CATEGORY_COMPOSING;
//                pKeyState->Function = FUNCTION_INPUT;
//            }
//            return TRUE;
//        }
//        else if (_hasWildcardIncludedInKeystrokeBuffer && uCode == VK_SPACE)
//        {
//            if (pKeyState) { pKeyState->Category = CATEGORY_COMPOSING; pKeyState->Function = FUNCTION_CONVERT_WILDCARD; } return TRUE;
//        }
//    }
//
//    if (candidateMode == CANDIDATE_ORIGINAL || candidateMode == CANDIDATE_PHRASE || candidateMode == CANDIDATE_WITH_NEXT_COMPOSITION)
//    {
//        BOOL isRetCode = TRUE;
//        if (IsVirtualKeyKeystrokeCandidate(uCode, pKeyState, candidateMode, &isRetCode, &_KeystrokeCandidate))
//        {
//            return isRetCode;
//        }
//
//        if (hasCandidateWithWildcard)
//        {
//            if (IsVirtualKeyKeystrokeCandidate(uCode, pKeyState, candidateMode, &isRetCode, &_KeystrokeCandidateWildcard))
//            {
//                return isRetCode;
//            }
//        }
//
//        // Candidate list could not handle key. We can try to restart the composition.
//        if (IsVirtualKeyKeystrokeComposition(uCode, pKeyState, FUNCTION_INPUT))
//        {
//            if (candidateMode != CANDIDATE_ORIGINAL)
//            {
//                return TRUE;
//            }
//            else
//            {
//                if (pKeyState) { pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_FINALIZE_CANDIDATELIST_AND_INPUT; } 
//                return TRUE;
//            }
//        }
//    } 
//
//    // CANDIDATE_INCREMENTAL should process Keystroke.Candidate virtual keys.
//    else if (candidateMode == CANDIDATE_INCREMENTAL)
//    {
//        BOOL isRetCode = TRUE;
//        if (IsVirtualKeyKeystrokeCandidate(uCode, pKeyState, candidateMode, &isRetCode, &_KeystrokeCandidate))
//        {
//            return isRetCode;
//        }
//    }
//
//    if (!fComposing && candidateMode != CANDIDATE_ORIGINAL && candidateMode != CANDIDATE_PHRASE && candidateMode != CANDIDATE_WITH_NEXT_COMPOSITION) 
//    {
//        if (IsVirtualKeyKeystrokeComposition(uCode, pKeyState, FUNCTION_INPUT))
//        {
//            return TRUE;
//        }
//    }
//
//    // System pre-defined keystroke
//    if (fComposing)
//    {
//        if ((candidateMode != CANDIDATE_INCREMENTAL))
//        {
//            switch (uCode)
//            {
//            case VK_LEFT:   if (pKeyState) { pKeyState->Category = CATEGORY_COMPOSING; pKeyState->Function = FUNCTION_MOVE_LEFT; } return TRUE;
//            case VK_RIGHT:  if (pKeyState) { pKeyState->Category = CATEGORY_COMPOSING; pKeyState->Function = FUNCTION_MOVE_RIGHT; } return TRUE;
//            case VK_RETURN: if (pKeyState) { pKeyState->Category = CATEGORY_COMPOSING; pKeyState->Function = FUNCTION_FINALIZE_CANDIDATELIST; } return TRUE;
//            case VK_ESCAPE: if (pKeyState) { pKeyState->Category = CATEGORY_COMPOSING; pKeyState->Function = FUNCTION_CANCEL; } return TRUE;
//            case VK_BACK:   if (pKeyState) { pKeyState->Category = CATEGORY_COMPOSING; pKeyState->Function = FUNCTION_BACKSPACE; } return TRUE;
//
//            case VK_UP:     if (pKeyState) { pKeyState->Category = CATEGORY_COMPOSING; pKeyState->Function = FUNCTION_MOVE_UP; } return TRUE;
//            case VK_DOWN:   if (pKeyState) { pKeyState->Category = CATEGORY_COMPOSING; pKeyState->Function = FUNCTION_MOVE_DOWN; } return TRUE;
//            case VK_PRIOR:  if (pKeyState) { pKeyState->Category = CATEGORY_COMPOSING; pKeyState->Function = FUNCTION_MOVE_PAGE_UP; } return TRUE;
//            case VK_NEXT:   if (pKeyState) { pKeyState->Category = CATEGORY_COMPOSING; pKeyState->Function = FUNCTION_MOVE_PAGE_DOWN; } return TRUE;
//
//            case VK_HOME:   if (pKeyState) { pKeyState->Category = CATEGORY_COMPOSING; pKeyState->Function = FUNCTION_MOVE_PAGE_TOP; } return TRUE;
//            case VK_END:    if (pKeyState) { pKeyState->Category = CATEGORY_COMPOSING; pKeyState->Function = FUNCTION_MOVE_PAGE_BOTTOM; } return TRUE;
//
//            case VK_SPACE:  if (pKeyState) { pKeyState->Category = CATEGORY_COMPOSING; pKeyState->Function = FUNCTION_CONVERT; } return TRUE;
//            }
//        }
//        else if ((candidateMode == CANDIDATE_INCREMENTAL))
//        {
//            switch (uCode)
//            {
//                // VK_LEFT, VK_RIGHT - set *pIsEaten = FALSE for application could move caret left or right.
//                // and for CUAS, invoke _HandleCompositionCancel() edit session due to ignore CUAS default key handler for send out terminate composition
//            case VK_LEFT:
//            case VK_RIGHT:
//                {
//                    if (pKeyState)
//                    {
//                        pKeyState->Category = CATEGORY_INVOKE_COMPOSITION_EDIT_SESSION;
//                        pKeyState->Function = FUNCTION_CANCEL;
//                    }
//                }
//                return FALSE;
//
//            case VK_RETURN: if (pKeyState) { pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_FINALIZE_CANDIDATELIST; } return TRUE;
//            case VK_ESCAPE: if (pKeyState) { pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_CANCEL; } return TRUE;
//
//                // VK_BACK - remove one char from reading string.
//            case VK_BACK:   if (pKeyState) { pKeyState->Category = CATEGORY_COMPOSING; pKeyState->Function = FUNCTION_BACKSPACE; } return TRUE;
//
//            case VK_UP:     if (pKeyState) { pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_MOVE_UP; } return TRUE;
//            case VK_DOWN:   if (pKeyState) { pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_MOVE_DOWN; } return TRUE;
//            case VK_PRIOR:  if (pKeyState) { pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_MOVE_PAGE_UP; } return TRUE;
//            case VK_NEXT:   if (pKeyState) { pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_MOVE_PAGE_DOWN; } return TRUE;
//
//            case VK_HOME:   if (pKeyState) { pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_MOVE_PAGE_TOP; } return TRUE;
//            case VK_END:    if (pKeyState) { pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_MOVE_PAGE_BOTTOM; } return TRUE;
//
//            case VK_SPACE:
//                {
//                    if (candidateMode == CANDIDATE_INCREMENTAL)
//                    {
//                        if (pKeyState) { pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_CONVERT; } return TRUE;
//                    }
//                    else
//                    {
//                        if (pKeyState) { pKeyState->Category = CATEGORY_COMPOSING; pKeyState->Function = FUNCTION_CONVERT; } return TRUE;
//                    }
//                }
//            }
//        }
//    }
//
//    if ((candidateMode == CANDIDATE_ORIGINAL) || (candidateMode == CANDIDATE_WITH_NEXT_COMPOSITION))
//    {
//        switch (uCode)
//        {
//        case VK_UP:     if (pKeyState) { pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_MOVE_UP; } return TRUE;
//        case VK_DOWN:   if (pKeyState) { pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_MOVE_DOWN; } return TRUE;
//        case VK_PRIOR:  if (pKeyState) { pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_MOVE_PAGE_UP; } return TRUE;
//        case VK_NEXT:   if (pKeyState) { pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_MOVE_PAGE_DOWN; } return TRUE;
//        case VK_HOME:   if (pKeyState) { pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_MOVE_PAGE_TOP; } return TRUE;
//        case VK_END:    if (pKeyState) { pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_MOVE_PAGE_BOTTOM; } return TRUE;
//        case VK_RETURN: if (pKeyState) { pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_FINALIZE_CANDIDATELIST; } return TRUE;
//        case VK_SPACE:  if (pKeyState) { pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_CONVERT; } return TRUE;
//        case VK_BACK:   if (pKeyState) { pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_CANCEL; } return TRUE;
//
//        case VK_ESCAPE:
//            {
//                if (candidateMode == CANDIDATE_WITH_NEXT_COMPOSITION)
//                {
//                    if (pKeyState)
//                    {
//                        pKeyState->Category = CATEGORY_INVOKE_COMPOSITION_EDIT_SESSION;
//                        pKeyState->Function = FUNCTION_FINALIZE_TEXTSTORE;
//                    }
//                    return TRUE;
//                }
//                else
//                {
//                    if (pKeyState)
//                    {
//                        pKeyState->Category = CATEGORY_CANDIDATE;
//                        pKeyState->Function = FUNCTION_CANCEL;
//                    }
//                    return TRUE;
//                }
//            }
//        }
//
//        if (candidateMode == CANDIDATE_WITH_NEXT_COMPOSITION)
//        {
//            if (IsVirtualKeyKeystrokeComposition(uCode, NULL, FUNCTION_NONE))
//            {
//                if (pKeyState) { pKeyState->Category = CATEGORY_COMPOSING; pKeyState->Function = FUNCTION_FINALIZE_TEXTSTORE_AND_INPUT; } return TRUE;
//            }
//        }
//    }
//
//    if (candidateMode == CANDIDATE_PHRASE)
//    {
//        switch (uCode)
//        {
//        case VK_UP:     if (pKeyState) { pKeyState->Category = CATEGORY_PHRASE; pKeyState->Function = FUNCTION_MOVE_UP; } return TRUE;
//        case VK_DOWN:   if (pKeyState) { pKeyState->Category = CATEGORY_PHRASE; pKeyState->Function = FUNCTION_MOVE_DOWN; } return TRUE;
//        case VK_PRIOR:  if (pKeyState) { pKeyState->Category = CATEGORY_PHRASE; pKeyState->Function = FUNCTION_MOVE_PAGE_UP; } return TRUE;
//        case VK_NEXT:   if (pKeyState) { pKeyState->Category = CATEGORY_PHRASE; pKeyState->Function = FUNCTION_MOVE_PAGE_DOWN; } return TRUE;
//        case VK_HOME:   if (pKeyState) { pKeyState->Category = CATEGORY_PHRASE; pKeyState->Function = FUNCTION_MOVE_PAGE_TOP; } return TRUE;
//        case VK_END:    if (pKeyState) { pKeyState->Category = CATEGORY_PHRASE; pKeyState->Function = FUNCTION_MOVE_PAGE_BOTTOM; } return TRUE;
//        case VK_RETURN: if (pKeyState) { pKeyState->Category = CATEGORY_PHRASE; pKeyState->Function = FUNCTION_FINALIZE_CANDIDATELIST; } return TRUE;
//        case VK_SPACE:  if (pKeyState) { pKeyState->Category = CATEGORY_PHRASE; pKeyState->Function = FUNCTION_CONVERT; } return TRUE;
//        case VK_ESCAPE: if (pKeyState) { pKeyState->Category = CATEGORY_PHRASE; pKeyState->Function = FUNCTION_CANCEL; } return TRUE;
//        case VK_BACK:   if (pKeyState) { pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_CANCEL; } return TRUE;
//        }
//    }
//
//    if (IsKeystrokeRange(uCode, pKeyState, candidateMode))
//    {
//        return TRUE;
//    }
//    else if (pKeyState && pKeyState->Category != CATEGORY_NONE)
//    {
//        return FALSE;
//    }
//
//    if (*pwch && !IsVirtualKeyKeystrokeComposition(uCode, pKeyState, FUNCTION_NONE))
//    {
//        if (pKeyState)
//        {
//            pKeyState->Category = CATEGORY_INVOKE_COMPOSITION_EDIT_SESSION;
//            pKeyState->Function = FUNCTION_FINALIZE_TEXTSTORE;
//        }
//        return FALSE;
//    }
//
//    return FALSE;
//}

//+---------------------------------------------------------------------------
//
// AddVirtualKey
// Add virtual key code to Composition Processor Engine for used to parse keystroke data.
// param
//     [in] uCode - Specify virtual key code.
// returns
//     State of Text Processor Engine.
//----------------------------------------------------------------------------

BOOL CCompositionProcessorEngine::AddVirtualKey(WCHAR wch)
{
    return processorEngine->AddVirtualKey(wch);
//    if (!wch)
//    {
//        return FALSE;
//    }
//
//    //
//    // append one keystroke in buffer.
//    //
//    DWORD_PTR srgKeystrokeBufLen = _keystrokeBuffer.GetLength();
//    PWCHAR pwch = new (std::nothrow) WCHAR[ srgKeystrokeBufLen + 1 ];
//    if (!pwch)
//    {
//        return FALSE;
//    }
//
//    memcpy(pwch, _keystrokeBuffer.Get(), srgKeystrokeBufLen * sizeof(WCHAR));
//    pwch[ srgKeystrokeBufLen ] = wch;
//
//    if (_keystrokeBuffer.Get())
//    {
//        delete [] _keystrokeBuffer.Get();
//    }
//
//    _keystrokeBuffer.Set(pwch, srgKeystrokeBufLen + 1);
//
//    return TRUE;
}

//+---------------------------------------------------------------------------
//
// RemoveVirtualKey
// Remove stored virtual key code.
// param
//     [in] dwIndex   - Specified index.
// returns
//     none.
//----------------------------------------------------------------------------

void CCompositionProcessorEngine::RemoveVirtualKey(DWORD_PTR dwIndex)
{
    return processorEngine->RemoveVirtualKey(dwIndex);

//    DWORD_PTR srgKeystrokeBufLen = _keystrokeBuffer.GetLength();
//
//    if (dwIndex + 1 < srgKeystrokeBufLen)
//    {
//        // shift following eles left
//        memmove((BYTE*)_keystrokeBuffer.Get() + (dwIndex * sizeof(WCHAR)),
//            (BYTE*)_keystrokeBuffer.Get() + ((dwIndex + 1) * sizeof(WCHAR)),
//            (srgKeystrokeBufLen - dwIndex - 1) * sizeof(WCHAR));
//    }
//
//    _keystrokeBuffer.Set(_keystrokeBuffer.Get(), srgKeystrokeBufLen - 1);
}

//+---------------------------------------------------------------------------
//
// PurgeVirtualKey
// Purge stored virtual key code.
// param
//     none.
// returns
//     none.
//----------------------------------------------------------------------------

void CCompositionProcessorEngine::PurgeVirtualKey()
{
    return processorEngine->PurgeVirtualKey();

//    if (_keystrokeBuffer.Get())
//    {
//        delete [] _keystrokeBuffer.Get();
//        _keystrokeBuffer.Set(NULL, 0);
//    }
}

DWORD_PTR CCompositionProcessorEngine::GetVirtualKeyLength()
{
    return processorEngine->GetVirtualKeyLength();
    // _keystrokeBuffer.GetLength();
}

// WCHAR CCompositionProcessorEngine::GetVirtualKey(DWORD_PTR dwIndex) 
// { 
//     if (dwIndex < _keystrokeBuffer.GetLength())
//     {
//         return *(_keystrokeBuffer.Get() + dwIndex);
//     }
//     return 0;
// }

//+---------------------------------------------------------------------------
//
// GetReadingStrings
// Retrieves string from Composition Processor Engine.
// param
//     [out] pReadingStrings - Specified returns pointer of CUnicodeString.
// returns
//     none
//
//----------------------------------------------------------------------------

void CCompositionProcessorEngine::GetReadingStrings(_Inout_ std::vector<CStringRange> *pReadingStrings, _Out_ BOOL *pIsWildcardIncluded)
{
    return processorEngine->GetReadingStrings(pReadingStrings, pIsWildcardIncluded);
//     CStringRange oneKeystroke;
// 
//     _hasWildcardIncludedInKeystrokeBuffer = FALSE;
// 
//     if (pReadingStrings->Count() == 0 && _keystrokeBuffer.GetLength())
//     {
//         CStringRange* pNewString = nullptr;
// 
//         pNewString = pReadingStrings->Append();
//         if (pNewString)
//         {
//             *pNewString = _keystrokeBuffer;
//         }
// 
//         for (DWORD index = 0; index < _keystrokeBuffer.GetLength(); index++)
//         {
//             oneKeystroke.Set(_keystrokeBuffer.Get() + index, 1);
// 
//             if (IsWildcard() && IsWildcardChar(*oneKeystroke.Get()))
//             {
//                 _hasWildcardIncludedInKeystrokeBuffer = TRUE;
//             }
//         }
//     }
// 
//     *pIsWildcardIncluded = _hasWildcardIncludedInKeystrokeBuffer;
}

//+---------------------------------------------------------------------------
//
// GetCandidateList
//
//----------------------------------------------------------------------------

void CCompositionProcessorEngine::GetCandidateList(_Inout_ std::vector<CCandidateListItem> *pCandidateList, BOOL isIncrementalWordSearch, BOOL isWildcardSearch)
{
    return processorEngine->GetCandidateList(pCandidateList, isIncrementalWordSearch, isWildcardSearch);

//     if (!IsDictionaryAvailable())
//     {
//         return;
//     }
// 
//     if (isIncrementalWordSearch)
//     {
//         CStringRange wildcardSearch;
//         DWORD_PTR keystrokeBufLen = _keystrokeBuffer.GetLength() + 2;
//         PWCHAR pwch = new (std::nothrow) WCHAR[ keystrokeBufLen ];
//         if (!pwch)
//         {
//             return;
//         }
// 
//         // check keystroke buffer already has wildcard char which end user want wildcard serach
//         DWORD wildcardIndex = 0;
//         BOOL isFindWildcard = FALSE;
// 
//         if (IsWildcard())
//         {
//             for (wildcardIndex = 0; wildcardIndex < _keystrokeBuffer.GetLength(); wildcardIndex++)
//             {
//                 if (IsWildcardChar(*(_keystrokeBuffer.Get() + wildcardIndex)))
//                 {
//                     isFindWildcard = TRUE;
//                     break;
//                 }
//             }
//         }
// 
//         StringCchCopyN(pwch, keystrokeBufLen, _keystrokeBuffer.Get(), _keystrokeBuffer.GetLength());
// 
//         if (!isFindWildcard)
//         {
//             // add wildcard char for incremental search
//             StringCchCat(pwch, keystrokeBufLen, L"*");
//         }
// 
//         size_t len = 0;
//         if (StringCchLength(pwch, STRSAFE_MAX_CCH, &len) == S_OK)
//         {
//             wildcardSearch.Set(pwch, len);
//         }
//         else
//         {
//             return;
//         }
// 
//         _pTableDictionaryEngine->CollectWordForWildcard(&wildcardSearch, pCandidateList);
// 
//         if (0 >= pCandidateList->Count())
//         {
//             return;
//         }
// 
//         if (IsKeystrokeSort())
//         {
//             _pTableDictionaryEngine->SortListItemByFindKeyCode(pCandidateList);
//         }
// 
//         // Incremental search would show keystroke data from all candidate list items
//         // but wont show identical keystroke data for user inputted.
//         for (UINT index = 0; index < pCandidateList->Count(); index++)
//         {
//             CCandidateListItem *pLI = pCandidateList->GetAt(index);
//             DWORD_PTR keystrokeBufferLen = 0;
// 
//             if (IsWildcard())
//             {
//                 keystrokeBufferLen = wildcardIndex;
//             }
//             else
//             {
//                 keystrokeBufferLen = _keystrokeBuffer.GetLength();
//             }
// 
//             CStringRange newFindKeyCode;
//             newFindKeyCode.Set(pLI->_FindKeyCode.Get() + keystrokeBufferLen, pLI->_FindKeyCode.GetLength() - keystrokeBufferLen);
//             pLI->_FindKeyCode.Set(newFindKeyCode);
//         }
// 
//         delete [] pwch;
//     }
//     else if (isWildcardSearch)
//     {
//         _pTableDictionaryEngine->CollectWordForWildcard(&_keystrokeBuffer, pCandidateList);
//     }
//     else
//     {
//         _pTableDictionaryEngine->CollectWord(&_keystrokeBuffer, pCandidateList);
//     }
// 
//     for (UINT index = 0; index < pCandidateList->Count();)
//     {
//         CCandidateListItem *pLI = pCandidateList->GetAt(index);
//         CStringRange startItemString;
//         CStringRange endItemString;
// 
//         startItemString.Set(pLI->_ItemString.Get(), 1);
//         endItemString.Set(pLI->_ItemString.Get() + pLI->_ItemString.GetLength() - 1, 1);
// 
//         index++;
//     }
}

//+---------------------------------------------------------------------------
//
// GetCandidateStringInConverted
//
//----------------------------------------------------------------------------

void CCompositionProcessorEngine::GetCandidateStringInConverted(CStringRange &searchString, _In_ std::vector<CCandidateListItem> *pCandidateList)
{
    return processorEngine->GetCandidateStringInConverted(searchString, pCandidateList);

//     if (!IsDictionaryAvailable())
//     {
//         return;
//     }
// 
//     // Search phrase from SECTION_TEXT's converted string list
//     CStringRange wildcardSearch;
//     DWORD_PTR srgKeystrokeBufLen = searchString.GetLength() + 2;
//     PWCHAR pwch = new (std::nothrow) WCHAR[ srgKeystrokeBufLen ];
//     if (!pwch)
//     {
//         return;
//     }
// 
//     StringCchCopyN(pwch, srgKeystrokeBufLen, searchString.Get(), searchString.GetLength());
//     StringCchCat(pwch, srgKeystrokeBufLen, L"*");
// 
//     // add wildcard char
//     size_t len = 0;
//     if (StringCchLength(pwch, STRSAFE_MAX_CCH, &len) != S_OK)
//     {
//         return;
//     }
//     wildcardSearch.Set(pwch, len);
// 
//     _pTableDictionaryEngine->CollectWordFromConvertedStringForWildcard(&wildcardSearch, pCandidateList);
// 
//     if (IsKeystrokeSort())
//     {
//         _pTableDictionaryEngine->SortListItemByFindKeyCode(pCandidateList);
//     }
// 
//     wildcardSearch.Clear();
//     delete [] pwch;
}

//+---------------------------------------------------------------------------
//
// OnPreservedKey
//
//----------------------------------------------------------------------------

void CCompositionProcessorEngine::OnPreservedKey(REFGUID rguid, _Out_ BOOL *pIsEaten, _In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId)
{
    return processorEngine->OnPreservedKey(rguid, pIsEaten, pThreadMgr, tfClientId);

//     if (IsEqualGUID(rguid, _PreservedKey_IMEMode.Guid))
//     {
//         if (!CheckShiftKeyOnly(&_PreservedKey_IMEMode.TSFPreservedKeyTable))
//         {
//             *pIsEaten = FALSE;
//             return;
//         }
//         BOOL isOpen = FALSE;
//         CCompartment CompartmentKeyboardOpen(pThreadMgr, tfClientId, GUID_COMPARTMENT_KEYBOARD_OPENCLOSE);
//         CompartmentKeyboardOpen._GetCompartmentBOOL(isOpen);
//         CompartmentKeyboardOpen._SetCompartmentBOOL(isOpen ? FALSE : TRUE);
// 
//         *pIsEaten = TRUE;
//     }
//     else if (IsEqualGUID(rguid, _PreservedKey_DoubleSingleByte.Guid))
//     {
//         if (!CheckShiftKeyOnly(&_PreservedKey_DoubleSingleByte.TSFPreservedKeyTable))
//         {
//             *pIsEaten = FALSE;
//             return;
//         }
//         BOOL isDouble = FALSE;
//         CCompartment CompartmentDoubleSingleByte(pThreadMgr, tfClientId, Global::SampleIMEGuidCompartmentDoubleSingleByte);
//         CompartmentDoubleSingleByte._GetCompartmentBOOL(isDouble);
//         CompartmentDoubleSingleByte._SetCompartmentBOOL(isDouble ? FALSE : TRUE);
//         *pIsEaten = TRUE;
//     }
//     else if (IsEqualGUID(rguid, _PreservedKey_Punctuation.Guid))
//     {
//         if (!CheckShiftKeyOnly(&_PreservedKey_Punctuation.TSFPreservedKeyTable))
//         {
//             *pIsEaten = FALSE;
//             return;
//         }
//         BOOL isPunctuation = FALSE;
//         CCompartment CompartmentPunctuation(pThreadMgr, tfClientId, Global::SampleIMEGuidCompartmentPunctuation);
//         CompartmentPunctuation._GetCompartmentBOOL(isPunctuation);
//         CompartmentPunctuation._SetCompartmentBOOL(isPunctuation ? FALSE : TRUE);
//         *pIsEaten = TRUE;
//     }
//     else
//     {
//         *pIsEaten = FALSE;
//     }
//     *pIsEaten = TRUE;
}

//+---------------------------------------------------------------------------
//
// IsPunctuation
//
//----------------------------------------------------------------------------

BOOL CCompositionProcessorEngine::IsPunctuation(WCHAR wch)
{
    return processorEngine->IsPunctuation(wch);

//     for (int i = 0; i < ARRAYSIZE(Global::PunctuationTable); i++)
//     {
//         if (Global::PunctuationTable[i]._Code == wch)
//         {
//             return TRUE;
//         }
//     }
// 
//     for (UINT j = 0; j < _PunctuationPair.Count(); j++)
//     {
//         CPunctuationPair* pPuncPair = _PunctuationPair.GetAt(j);
// 
//         if (pPuncPair->_punctuation._Code == wch)
//         {
//             return TRUE;
//         }
//     }
// 
//     for (UINT k = 0; k < _PunctuationNestPair.Count(); k++)
//     {
//         CPunctuationNestPair* pPuncNestPair = _PunctuationNestPair.GetAt(k);
// 
//         if (pPuncNestPair->_punctuation_begin._Code == wch)
//         {
//             return TRUE;
//         }
//         if (pPuncNestPair->_punctuation_end._Code == wch)
//         {
//             return TRUE;
//         }
//     }
//     return FALSE;
}

//+---------------------------------------------------------------------------
//
// GetPunctuationPair
//
//----------------------------------------------------------------------------

WCHAR CCompositionProcessorEngine::GetPunctuation(WCHAR wch)
{
    return processorEngine->GetPunctuation(wch);

//     for (int i = 0; i < ARRAYSIZE(Global::PunctuationTable); i++)
//     {
//         if (Global::PunctuationTable[i]._Code == wch)
//         {
//             return Global::PunctuationTable[i]._Punctuation;
//         }
//     }
// 
//     for (UINT j = 0; j < _PunctuationPair.Count(); j++)
//     {
//         CPunctuationPair* pPuncPair = _PunctuationPair.GetAt(j);
// 
//         if (pPuncPair->_punctuation._Code == wch)
//         {
//             if (! pPuncPair->_isPairToggle)
//             {
//                 pPuncPair->_isPairToggle = TRUE;
//                 return pPuncPair->_punctuation._Punctuation;
//             }
//             else
//             {
//                 pPuncPair->_isPairToggle = FALSE;
//                 return pPuncPair->_pairPunctuation;
//             }
//         }
//     }
// 
//     for (UINT k = 0; k < _PunctuationNestPair.Count(); k++)
//     {
//         CPunctuationNestPair* pPuncNestPair = _PunctuationNestPair.GetAt(k);
// 
//         if (pPuncNestPair->_punctuation_begin._Code == wch)
//         {
//             if (pPuncNestPair->_nestCount++ == 0)
//             {
//                 return pPuncNestPair->_punctuation_begin._Punctuation;
//             }
//             else
//             {
//                 return pPuncNestPair->_pairPunctuation_begin;
//             }
//         }
//         if (pPuncNestPair->_punctuation_end._Code == wch)
//         {
//             if (--pPuncNestPair->_nestCount == 0)
//             {
//                 return pPuncNestPair->_punctuation_end._Punctuation;
//             }
//             else
//             {
//                 return pPuncNestPair->_pairPunctuation_end;
//             }
//         }
//     }
//     return 0;
}

//+---------------------------------------------------------------------------
//
// IsDoubleSingleByte
//
//----------------------------------------------------------------------------

BOOL CCompositionProcessorEngine::IsDoubleSingleByte(WCHAR wch)
{
    return processorEngine->IsDoubleSingleByte(wch);

//     if (L' ' <= wch && wch <= L'~')
//     {
//         return TRUE;
//     }
//     return FALSE;
}

BOOL CCompositionProcessorEngine::IsMakePhraseFromText()
{
    return processorEngine->IsMakePhraseFromText();
}

//+---------------------------------------------------------------------------
//
// CCompositionProcessorEngine::SetLanguageBarStatus
//
//----------------------------------------------------------------------------

VOID CCompositionProcessorEngine::SetLanguageBarStatus(DWORD status, BOOL isSet)
{
    for (auto&& langBarButton: m_langaugeBarButtons)
    {
        if (langBarButton)
        {
            langBarButton->SetStatus(status, isSet);
            langBarButton.reset();
        }
    }
}

//+---------------------------------------------------------------------------
//
// UpdatePrivateCompartments
//
//----------------------------------------------------------------------------

void CCompositionProcessorEngine::ConversionModeCompartmentUpdated(_In_ ITfThreadMgr *pThreadMgr)
{
    processorEngine->ConversionModeCompartmentUpdated(pThreadMgr);

//    if (!_pCompartmentConversion)
//    {
//        return;
//    }
//
//    DWORD conversionMode = 0;
//    if (FAILED(_pCompartmentConversion->_GetCompartmentDWORD(conversionMode)))
//    {
//        return;
//    }
//
//    BOOL isDouble = FALSE;
//    CCompartment CompartmentDoubleSingleByte(pThreadMgr, _tfClientId, Global::SampleIMEGuidCompartmentDoubleSingleByte);
//    if (SUCCEEDED(CompartmentDoubleSingleByte._GetCompartmentBOOL(isDouble)))
//    {
//        if (!isDouble && (conversionMode & TF_CONVERSIONMODE_FULLSHAPE))
//        {
//            CompartmentDoubleSingleByte._SetCompartmentBOOL(TRUE);
//        }
//        else if (isDouble && !(conversionMode & TF_CONVERSIONMODE_FULLSHAPE))
//        {
//            CompartmentDoubleSingleByte._SetCompartmentBOOL(FALSE);
//        }
//    }
//    BOOL isPunctuation = FALSE;
//    CCompartment CompartmentPunctuation(pThreadMgr, _tfClientId, Global::SampleIMEGuidCompartmentPunctuation);
//    if (SUCCEEDED(CompartmentPunctuation._GetCompartmentBOOL(isPunctuation)))
//    {
//        if (!isPunctuation && (conversionMode & TF_CONVERSIONMODE_SYMBOL))
//        {
//            CompartmentPunctuation._SetCompartmentBOOL(TRUE);
//        }
//        else if (isPunctuation && !(conversionMode & TF_CONVERSIONMODE_SYMBOL))
//        {
//            CompartmentPunctuation._SetCompartmentBOOL(FALSE);
//        }
//    }
//
//    BOOL fOpen = FALSE;
//    CCompartment CompartmentKeyboardOpen(pThreadMgr, _tfClientId, GUID_COMPARTMENT_KEYBOARD_OPENCLOSE);
//    if (SUCCEEDED(CompartmentKeyboardOpen._GetCompartmentBOOL(fOpen)))
//    {
//        if (fOpen && !(conversionMode & TF_CONVERSIONMODE_NATIVE))
//        {
//            CompartmentKeyboardOpen._SetCompartmentBOOL(FALSE);
//        }
//        else if (!fOpen && (conversionMode & TF_CONVERSIONMODE_NATIVE))
//        {
//            CompartmentKeyboardOpen._SetCompartmentBOOL(TRUE);
//        }
//    }
}

void CCompositionProcessorEngine::ShowAllLanguageBarIcons()
{
	SetLanguageBarStatus(TF_LBI_STATUS_HIDDEN, FALSE);
}

void CCompositionProcessorEngine::HideAllLanguageBarIcons()
{
	SetLanguageBarStatus(TF_LBI_STATUS_HIDDEN, TRUE);
}

std::vector<DWORD>* CCompositionProcessorEngine::GetCandidateListIndexRange()
{
    return processorEngine->GetCandidateListIndexRange();
}

UINT CCompositionProcessorEngine::GetCandidateWindowWidth()
{
    return processorEngine->GetCandidateWindowWidth();
}


//////////////////////////////////////////////////////////////////////
//
// CompositionProcessorEngine implementation.
//
//////////////////////////////////////////////////////////////////////

//+---------------------------------------------------------------------------
//
// ctor
//
//----------------------------------------------------------------------------

//CompositionProcessorEngine::CompositionProcessorEngine()
//{
//    _pTableDictionaryEngine = nullptr;
//    _pDictionaryFile = nullptr;
//
//    _langid = 0xffff;
//    _guidProfile = GUID_NULL;
//    _tfClientId = TF_CLIENTID_NULL;
//
////    _pLanguageBar_IMEMode = nullptr;
//    _pLanguageBar_DoubleSingleByte = nullptr;
//    _pLanguageBar_Punctuation = nullptr;
//
//    _pCompartmentConversion = nullptr;
//    _pCompartmentKeyboardOpenEventSink = nullptr;
//    _pCompartmentConversionEventSink = nullptr;
//    _pCompartmentDoubleSingleByteEventSink = nullptr;
//    _pCompartmentPunctuationEventSink = nullptr;
//
//    _hasWildcardIncludedInKeystrokeBuffer = FALSE;
//
//    _isWildcard = FALSE;
//    _isDisableWildcardAtFirst = FALSE;
//    _hasMakePhraseFromText = FALSE;
//    _isKeystrokeSort = FALSE;
//
//    _candidateListPhraseModifier = 0;
//
//    _candidateWndWidth = CAND_WIDTH;
//
//    InitKeyStrokeTable();
//}
//
////+---------------------------------------------------------------------------
////
//// dtor
////
////----------------------------------------------------------------------------
//
//CompositionProcessorEngine::~CompositionProcessorEngine()
//{
//    if (_pTableDictionaryEngine)
//    {
//        delete _pTableDictionaryEngine;
//        _pTableDictionaryEngine = nullptr;
//    }
//
//    if (_pLanguageBar_IMEMode)
//    {
//        _pLanguageBar_IMEMode->CleanUp();
//        _pLanguageBar_IMEMode->Release();
//        _pLanguageBar_IMEMode = nullptr;
//    }
//    if (_pLanguageBar_DoubleSingleByte)
//    {
//        _pLanguageBar_DoubleSingleByte->CleanUp();
//        _pLanguageBar_DoubleSingleByte->Release();
//        _pLanguageBar_DoubleSingleByte = nullptr;
//    }
//    if (_pLanguageBar_Punctuation)
//    {
//        _pLanguageBar_Punctuation->CleanUp();
//        _pLanguageBar_Punctuation->Release();
//        _pLanguageBar_Punctuation = nullptr;
//    }
//
//    if (_pCompartmentConversion)
//    {
//        delete _pCompartmentConversion;
//        _pCompartmentConversion = nullptr;
//    }
//    if (_pCompartmentKeyboardOpenEventSink)
//    {
//        _pCompartmentKeyboardOpenEventSink->_Unadvise();
//        delete _pCompartmentKeyboardOpenEventSink;
//        _pCompartmentKeyboardOpenEventSink = nullptr;
//    }
//    if (_pCompartmentConversionEventSink)
//    {
//        _pCompartmentConversionEventSink->_Unadvise();
//        delete _pCompartmentConversionEventSink;
//        _pCompartmentConversionEventSink = nullptr;
//    }
//    if (_pCompartmentDoubleSingleByteEventSink)
//    {
//        _pCompartmentDoubleSingleByteEventSink->_Unadvise();
//        delete _pCompartmentDoubleSingleByteEventSink;
//        _pCompartmentDoubleSingleByteEventSink = nullptr;
//    }
//    if (_pCompartmentPunctuationEventSink)
//    {
//        _pCompartmentPunctuationEventSink->_Unadvise();
//        delete _pCompartmentPunctuationEventSink;
//        _pCompartmentPunctuationEventSink = nullptr;
//    }
//
//    if (_pDictionaryFile)
//    {
//        delete _pDictionaryFile;
//        _pDictionaryFile = nullptr;
//    }
//}
//
////+---------------------------------------------------------------------------
////
//// AddVirtualKey
//// Add virtual key code to Composition Processor Engine for used to parse keystroke data.
//// param
////     [in] uCode - Specify virtual key code.
//// returns
////     State of Text Processor Engine.
////----------------------------------------------------------------------------
//
//BOOL CompositionProcessorEngine::AddVirtualKey(WCHAR wch)
//{
//    if (!wch)
//    {
//        return FALSE;
//    }
//
//    //
//    // append one keystroke in buffer.
//    //
//    DWORD_PTR srgKeystrokeBufLen = _keystrokeBuffer.GetLength();
//    PWCHAR pwch = new (std::nothrow) WCHAR[ srgKeystrokeBufLen + 1 ];
//    if (!pwch)
//    {
//        return FALSE;
//    }
//
//    memcpy(pwch, _keystrokeBuffer.Get(), srgKeystrokeBufLen * sizeof(WCHAR));
//    pwch[ srgKeystrokeBufLen ] = wch;
//
//    if (_keystrokeBuffer.Get())
//    {
//        delete [] _keystrokeBuffer.Get();
//    }
//
//    _keystrokeBuffer.Set(pwch, srgKeystrokeBufLen + 1);
//
//    return TRUE;
//}
//
////+---------------------------------------------------------------------------
////
//// RemoveVirtualKey
//// Remove stored virtual key code.
//// param
////     [in] dwIndex   - Specified index.
//// returns
////     none.
////----------------------------------------------------------------------------
//
//void CompositionProcessorEngine::RemoveVirtualKey(DWORD_PTR dwIndex)
//{
//    DWORD_PTR srgKeystrokeBufLen = _keystrokeBuffer.GetLength();
//
//    if (dwIndex + 1 < srgKeystrokeBufLen)
//    {
//        // shift following eles left
//        memmove((BYTE*)_keystrokeBuffer.Get() + (dwIndex * sizeof(WCHAR)),
//            (BYTE*)_keystrokeBuffer.Get() + ((dwIndex + 1) * sizeof(WCHAR)),
//            (srgKeystrokeBufLen - dwIndex - 1) * sizeof(WCHAR));
//    }
//
//    _keystrokeBuffer.Set(_keystrokeBuffer.Get(), srgKeystrokeBufLen - 1);
//}
//
////+---------------------------------------------------------------------------
////
//// PurgeVirtualKey
//// Purge stored virtual key code.
//// param
////     none.
//// returns
////     none.
////----------------------------------------------------------------------------
//
//void CompositionProcessorEngine::PurgeVirtualKey()
//{
//    if (_keystrokeBuffer.Get())
//    {
//        delete [] _keystrokeBuffer.Get();
//        _keystrokeBuffer.Set(NULL, 0);
//    }
//}
//
//WCHAR CompositionProcessorEngine::GetVirtualKey(DWORD_PTR dwIndex) 
//{ 
//    if (dwIndex < _keystrokeBuffer.GetLength())
//    {
//        return *(_keystrokeBuffer.Get() + dwIndex);
//    }
//    return 0;
//}
////+---------------------------------------------------------------------------
////
//// GetReadingStrings
//// Retrieves string from Composition Processor Engine.
//// param
////     [out] pReadingStrings - Specified returns pointer of CUnicodeString.
//// returns
////     none
////
////----------------------------------------------------------------------------
//
//void CompositionProcessorEngine::GetReadingStrings(_Inout_ std::vector<CStringRange> *pReadingStrings, _Out_ BOOL *pIsWildcardIncluded)
//{
//    CStringRange oneKeystroke;
//
//    _hasWildcardIncludedInKeystrokeBuffer = FALSE;
//
//    if (pReadingStrings->Count() == 0 && _keystrokeBuffer.GetLength())
//    {
//        CStringRange* pNewString = nullptr;
//
//        pNewString = pReadingStrings->Append();
//        if (pNewString)
//        {
//            *pNewString = _keystrokeBuffer;
//        }
//
//        for (DWORD index = 0; index < _keystrokeBuffer.GetLength(); index++)
//        {
//            oneKeystroke.Set(_keystrokeBuffer.Get() + index, 1);
//
//            if (IsWildcard() && IsWildcardChar(*oneKeystroke.Get()))
//            {
//                _hasWildcardIncludedInKeystrokeBuffer = TRUE;
//            }
//        }
//    }
//
//    *pIsWildcardIncluded = _hasWildcardIncludedInKeystrokeBuffer;
//}
//
////+---------------------------------------------------------------------------
////
//// GetCandidateList
////
////----------------------------------------------------------------------------
//
//void CompositionProcessorEngine::GetCandidateList(_Inout_ std::vector<CCandidateListItem> *pCandidateList, BOOL isIncrementalWordSearch, BOOL isWildcardSearch)
//{
//    if (!IsDictionaryAvailable())
//    {
//        return;
//    }
//
//    if (isIncrementalWordSearch)
//    {
//        CStringRange wildcardSearch;
//        DWORD_PTR keystrokeBufLen = _keystrokeBuffer.GetLength() + 2;
//        PWCHAR pwch = new (std::nothrow) WCHAR[ keystrokeBufLen ];
//        if (!pwch)
//        {
//            return;
//        }
//
//        // check keystroke buffer already has wildcard char which end user want wildcard serach
//        DWORD wildcardIndex = 0;
//        BOOL isFindWildcard = FALSE;
//
//        if (IsWildcard())
//        {
//            for (wildcardIndex = 0; wildcardIndex < _keystrokeBuffer.GetLength(); wildcardIndex++)
//            {
//                if (IsWildcardChar(*(_keystrokeBuffer.Get() + wildcardIndex)))
//                {
//                    isFindWildcard = TRUE;
//                    break;
//                }
//            }
//        }
//
//        StringCchCopyN(pwch, keystrokeBufLen, _keystrokeBuffer.Get(), _keystrokeBuffer.GetLength());
//
//        if (!isFindWildcard)
//        {
//            // add wildcard char for incremental search
//            StringCchCat(pwch, keystrokeBufLen, L"*");
//        }
//
//        size_t len = 0;
//        if (StringCchLength(pwch, STRSAFE_MAX_CCH, &len) == S_OK)
//        {
//            wildcardSearch.Set(pwch, len);
//        }
//        else
//        {
//            return;
//        }
//
//        _pTableDictionaryEngine->CollectWordForWildcard(&wildcardSearch, pCandidateList);
//
//        if (0 >= pCandidateList->Count())
//        {
//            return;
//        }
//
//        if (IsKeystrokeSort())
//        {
//            _pTableDictionaryEngine->SortListItemByFindKeyCode(pCandidateList);
//        }
//
//        // Incremental search would show keystroke data from all candidate list items
//        // but wont show identical keystroke data for user inputted.
//        for (UINT index = 0; index < pCandidateList->Count(); index++)
//        {
//            CCandidateListItem *pLI = pCandidateList->GetAt(index);
//            DWORD_PTR keystrokeBufferLen = 0;
//
//            if (IsWildcard())
//            {
//                keystrokeBufferLen = wildcardIndex;
//            }
//            else
//            {
//                keystrokeBufferLen = _keystrokeBuffer.GetLength();
//            }
//
//            CStringRange newFindKeyCode;
//            newFindKeyCode.Set(pLI->_FindKeyCode.Get() + keystrokeBufferLen, pLI->_FindKeyCode.GetLength() - keystrokeBufferLen);
//            pLI->_FindKeyCode.Set(newFindKeyCode);
//        }
//
//        delete [] pwch;
//    }
//    else if (isWildcardSearch)
//    {
//        _pTableDictionaryEngine->CollectWordForWildcard(&_keystrokeBuffer, pCandidateList);
//    }
//    else
//    {
//        _pTableDictionaryEngine->CollectWord(&_keystrokeBuffer, pCandidateList);
//    }
//
//    for (UINT index = 0; index < pCandidateList->Count();)
//    {
//        CCandidateListItem *pLI = pCandidateList->GetAt(index);
//        CStringRange startItemString;
//        CStringRange endItemString;
//
//        startItemString.Set(pLI->_ItemString.Get(), 1);
//        endItemString.Set(pLI->_ItemString.Get() + pLI->_ItemString.GetLength() - 1, 1);
//
//        index++;
//    }
//}
//
////+---------------------------------------------------------------------------
////
//// GetCandidateStringInConverted
////
////----------------------------------------------------------------------------
//
//void CompositionProcessorEngine::GetCandidateStringInConverted(CStringRange &searchString, _In_ std::vector<CCandidateListItem> *pCandidateList)
//{
//    if (!IsDictionaryAvailable())
//    {
//        return;
//    }
//
//    // Search phrase from SECTION_TEXT's converted string list
//    CStringRange wildcardSearch;
//    DWORD_PTR srgKeystrokeBufLen = searchString.GetLength() + 2;
//    PWCHAR pwch = new (std::nothrow) WCHAR[ srgKeystrokeBufLen ];
//    if (!pwch)
//    {
//        return;
//    }
//
//    StringCchCopyN(pwch, srgKeystrokeBufLen, searchString.Get(), searchString.GetLength());
//    StringCchCat(pwch, srgKeystrokeBufLen, L"*");
//
//    // add wildcard char
//    size_t len = 0;
//    if (StringCchLength(pwch, STRSAFE_MAX_CCH, &len) != S_OK)
//    {
//        return;
//    }
//    wildcardSearch.Set(pwch, len);
//
//    _pTableDictionaryEngine->CollectWordFromConvertedStringForWildcard(&wildcardSearch, pCandidateList);
//
//    if (IsKeystrokeSort())
//    {
//        _pTableDictionaryEngine->SortListItemByFindKeyCode(pCandidateList);
//    }
//
//    wildcardSearch.Clear();
//    delete [] pwch;
//}
//
////+---------------------------------------------------------------------------
////
//// IsPunctuation
////
////----------------------------------------------------------------------------
//
//BOOL CompositionProcessorEngine::IsPunctuation(WCHAR wch)
//{
//    for (int i = 0; i < ARRAYSIZE(Global::PunctuationTable); i++)
//    {
//        if (Global::PunctuationTable[i]._Code == wch)
//        {
//            return TRUE;
//        }
//    }
//
//    for (UINT j = 0; j < _PunctuationPair.Count(); j++)
//    {
//        CPunctuationPair* pPuncPair = _PunctuationPair.GetAt(j);
//
//        if (pPuncPair->_punctuation._Code == wch)
//        {
//            return TRUE;
//        }
//    }
//
//    for (UINT k = 0; k < _PunctuationNestPair.Count(); k++)
//    {
//        CPunctuationNestPair* pPuncNestPair = _PunctuationNestPair.GetAt(k);
//
//        if (pPuncNestPair->_punctuation_begin._Code == wch)
//        {
//            return TRUE;
//        }
//        if (pPuncNestPair->_punctuation_end._Code == wch)
//        {
//            return TRUE;
//        }
//    }
//    return FALSE;
//}
//
////+---------------------------------------------------------------------------
////
//// GetPunctuationPair
////
////----------------------------------------------------------------------------
//
//WCHAR CompositionProcessorEngine::GetPunctuation(WCHAR wch)
//{
//    for (int i = 0; i < ARRAYSIZE(Global::PunctuationTable); i++)
//    {
//        if (Global::PunctuationTable[i]._Code == wch)
//        {
//            return Global::PunctuationTable[i]._Punctuation;
//        }
//    }
//
//    for (UINT j = 0; j < _PunctuationPair.Count(); j++)
//    {
//        CPunctuationPair* pPuncPair = _PunctuationPair.GetAt(j);
//
//        if (pPuncPair->_punctuation._Code == wch)
//        {
//            if (! pPuncPair->_isPairToggle)
//            {
//                pPuncPair->_isPairToggle = TRUE;
//                return pPuncPair->_punctuation._Punctuation;
//            }
//            else
//            {
//                pPuncPair->_isPairToggle = FALSE;
//                return pPuncPair->_pairPunctuation;
//            }
//        }
//    }
//
//    for (UINT k = 0; k < _PunctuationNestPair.Count(); k++)
//    {
//        CPunctuationNestPair* pPuncNestPair = _PunctuationNestPair.GetAt(k);
//
//        if (pPuncNestPair->_punctuation_begin._Code == wch)
//        {
//            if (pPuncNestPair->_nestCount++ == 0)
//            {
//                return pPuncNestPair->_punctuation_begin._Punctuation;
//            }
//            else
//            {
//                return pPuncNestPair->_pairPunctuation_begin;
//            }
//        }
//        if (pPuncNestPair->_punctuation_end._Code == wch)
//        {
//            if (--pPuncNestPair->_nestCount == 0)
//            {
//                return pPuncNestPair->_punctuation_end._Punctuation;
//            }
//            else
//            {
//                return pPuncNestPair->_pairPunctuation_end;
//            }
//        }
//    }
//    return 0;
//}
//
////+---------------------------------------------------------------------------
////
//// IsDoubleSingleByte
////
////----------------------------------------------------------------------------
//
//BOOL CompositionProcessorEngine::IsDoubleSingleByte(WCHAR wch)
//{
//    if (L' ' <= wch && wch <= L'~')
//    {
//        return TRUE;
//    }
//    return FALSE;
//}
//
////+---------------------------------------------------------------------------
////
//// SetupKeystroke
////
////----------------------------------------------------------------------------
//
//void CompositionProcessorEngine::SetupKeystroke()
//{
//    SetKeystrokeTable(&_KeystrokeComposition);
//    return;
//}
//
////+---------------------------------------------------------------------------
////
//// SetKeystrokeTable
////
////----------------------------------------------------------------------------
//
//void CompositionProcessorEngine::SetKeystrokeTable(_Inout_ std::vector<_KEYSTROKE> *pKeystroke)
//{
//    for (int i = 0; i < 26; i++)
//    {
//        _KEYSTROKE* pKS = nullptr;
//
//        pKS = pKeystroke->Append();
//        if (!pKS)
//        {
//            break;
//        }
//        *pKS = _keystrokeTable[i];
//    }
//}
//
////+---------------------------------------------------------------------------
////
//// SetupPreserved
////
////----------------------------------------------------------------------------
//
//void CompositionProcessorEngine::SetupPreserved(_In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId)
//{
//    TF_PRESERVEDKEY preservedKeyImeMode = {};
//    preservedKeyImeMode.uVKey = VK_SHIFT;
//    preservedKeyImeMode.uModifiers = _TF_MOD_ON_KEYUP_SHIFT_ONLY;
//    SetPreservedKey(Global::SampleIMEGuidImeModePreserveKey, preservedKeyImeMode, Global::ImeModeDescription, &_PreservedKey_IMEMode);
//
//    TF_PRESERVEDKEY preservedKeyDoubleSingleByte = {};
//    preservedKeyDoubleSingleByte.uVKey = VK_SPACE;
//    preservedKeyDoubleSingleByte.uModifiers = TF_MOD_SHIFT;
//    SetPreservedKey(Global::SampleIMEGuidDoubleSingleBytePreserveKey, preservedKeyDoubleSingleByte, Global::DoubleSingleByteDescription, &_PreservedKey_DoubleSingleByte);
//
//    TF_PRESERVEDKEY preservedKeyPunctuation = {};
//    preservedKeyPunctuation.uVKey = VK_OEM_PERIOD;
//    preservedKeyPunctuation.uModifiers = TF_MOD_CONTROL;
//    SetPreservedKey(Global::SampleIMEGuidPunctuationPreserveKey, preservedKeyPunctuation, Global::PunctuationDescription, &_PreservedKey_Punctuation);
//
//    InitPreservedKey(&_PreservedKey_IMEMode, pThreadMgr, tfClientId);
//    InitPreservedKey(&_PreservedKey_DoubleSingleByte, pThreadMgr, tfClientId);
//    InitPreservedKey(&_PreservedKey_Punctuation, pThreadMgr, tfClientId);
//
//    return;
//}
//
////+---------------------------------------------------------------------------
////
//// SetKeystrokeTable
////
////----------------------------------------------------------------------------
//
//void CompositionProcessorEngine::SetPreservedKey(const CLSID clsid, TF_PRESERVEDKEY & tfPreservedKey, _In_z_ LPCWSTR pwszDescription, _Out_ XPreservedKey *pXPreservedKey)
//{
//    pXPreservedKey->Guid = clsid;
//
//    TF_PRESERVEDKEY *ptfPsvKey1 = pXPreservedKey->TSFPreservedKeyTable.Append();
//    if (!ptfPsvKey1)
//    {
//        return;
//    }
//    *ptfPsvKey1 = tfPreservedKey;
//
//    size_t srgKeystrokeBufLen = 0;
//    if (StringCchLength(pwszDescription, STRSAFE_MAX_CCH, &srgKeystrokeBufLen) != S_OK)
//    {
//        return;
//    }
//    pXPreservedKey->Description = new (std::nothrow) WCHAR[srgKeystrokeBufLen + 1];
//    if (!pXPreservedKey->Description)
//    {
//        return;
//    }
//
//    StringCchCopy((LPWSTR)pXPreservedKey->Description, srgKeystrokeBufLen, pwszDescription);
//
//    return;
//}
////+---------------------------------------------------------------------------
////
//// InitPreservedKey
////
//// Register a hot key.
////
////----------------------------------------------------------------------------
//
//BOOL CompositionProcessorEngine::InitPreservedKey(_In_ XPreservedKey *pXPreservedKey, _In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId)
//{
//    ITfKeystrokeMgr *pKeystrokeMgr = nullptr;
//
//    if (IsEqualGUID(pXPreservedKey->Guid, GUID_NULL))
//    {
//        return FALSE;
//    }
//
//    if (pThreadMgr->QueryInterface(IID_ITfKeystrokeMgr, (void **)&pKeystrokeMgr) != S_OK)
//    {
//        return FALSE;
//    }
//
//    for (UINT i = 0; i < pXPreservedKey->TSFPreservedKeyTable.Count(); i++)
//    {
//        TF_PRESERVEDKEY preservedKey = *pXPreservedKey->TSFPreservedKeyTable.GetAt(i);
//        preservedKey.uModifiers &= 0xffff;
//
//        size_t lenOfDesc = 0;
//        if (StringCchLength(pXPreservedKey->Description, STRSAFE_MAX_CCH, &lenOfDesc) != S_OK)
//        {
//            return FALSE;
//        }
//        pKeystrokeMgr->PreserveKey(tfClientId, pXPreservedKey->Guid, &preservedKey, pXPreservedKey->Description, static_cast<ULONG>(lenOfDesc));
//    }
//
//    pKeystrokeMgr->Release();
//
//    return TRUE;
//}
//
////+---------------------------------------------------------------------------
////
//// CheckShiftKeyOnly
////
////----------------------------------------------------------------------------
//
//BOOL CompositionProcessorEngine::CheckShiftKeyOnly(_In_ std::vector<TF_PRESERVEDKEY> *pTSFPreservedKeyTable)
//{
//    for (UINT i = 0; i < pTSFPreservedKeyTable->Count(); i++)
//    {
//        TF_PRESERVEDKEY *ptfPsvKey = pTSFPreservedKeyTable->GetAt(i);
//
//        if (((ptfPsvKey->uModifiers & (_TF_MOD_ON_KEYUP_SHIFT_ONLY & 0xffff0000)) && !WindowsImeLib::IsShiftKeyDownOnly) ||
//            ((ptfPsvKey->uModifiers & (_TF_MOD_ON_KEYUP_CONTROL_ONLY & 0xffff0000)) && !WindowsImeLib::IsControlKeyDownOnly) ||
//            ((ptfPsvKey->uModifiers & (_TF_MOD_ON_KEYUP_ALT_ONLY & 0xffff0000)) && !WindowsImeLib::IsAltKeyDownOnly)         )
//        {
//            return FALSE;
//        }
//    }
//
//    return TRUE;
//}
//
////+---------------------------------------------------------------------------
////
//// OnPreservedKey
////
////----------------------------------------------------------------------------
//
//void CompositionProcessorEngine::OnPreservedKey(REFGUID rguid, _Out_ BOOL *pIsEaten, _In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId)
//{
//    if (IsEqualGUID(rguid, _PreservedKey_IMEMode.Guid))
//    {
//        if (!CheckShiftKeyOnly(&_PreservedKey_IMEMode.TSFPreservedKeyTable))
//        {
//            *pIsEaten = FALSE;
//            return;
//        }
//        BOOL isOpen = FALSE;
//        CCompartment CompartmentKeyboardOpen(pThreadMgr, tfClientId, GUID_COMPARTMENT_KEYBOARD_OPENCLOSE);
//        CompartmentKeyboardOpen._GetCompartmentBOOL(isOpen);
//        CompartmentKeyboardOpen._SetCompartmentBOOL(isOpen ? FALSE : TRUE);
//
//        *pIsEaten = TRUE;
//    }
//    else if (IsEqualGUID(rguid, _PreservedKey_DoubleSingleByte.Guid))
//    {
//        if (!CheckShiftKeyOnly(&_PreservedKey_DoubleSingleByte.TSFPreservedKeyTable))
//        {
//            *pIsEaten = FALSE;
//            return;
//        }
//        BOOL isDouble = FALSE;
//        CCompartment CompartmentDoubleSingleByte(pThreadMgr, tfClientId, Global::SampleIMEGuidCompartmentDoubleSingleByte);
//        CompartmentDoubleSingleByte._GetCompartmentBOOL(isDouble);
//        CompartmentDoubleSingleByte._SetCompartmentBOOL(isDouble ? FALSE : TRUE);
//        *pIsEaten = TRUE;
//    }
//    else if (IsEqualGUID(rguid, _PreservedKey_Punctuation.Guid))
//    {
//        if (!CheckShiftKeyOnly(&_PreservedKey_Punctuation.TSFPreservedKeyTable))
//        {
//            *pIsEaten = FALSE;
//            return;
//        }
//        BOOL isPunctuation = FALSE;
//        CCompartment CompartmentPunctuation(pThreadMgr, tfClientId, Global::SampleIMEGuidCompartmentPunctuation);
//        CompartmentPunctuation._GetCompartmentBOOL(isPunctuation);
//        CompartmentPunctuation._SetCompartmentBOOL(isPunctuation ? FALSE : TRUE);
//        *pIsEaten = TRUE;
//    }
//    else
//    {
//        *pIsEaten = FALSE;
//    }
//    *pIsEaten = TRUE;
//}
//
////+---------------------------------------------------------------------------
////
//// SetupConfiguration
////
////----------------------------------------------------------------------------
//
//void CompositionProcessorEngine::SetupConfiguration()
//{
//    _isWildcard = TRUE;
//    _isDisableWildcardAtFirst = TRUE;
//    _hasMakePhraseFromText = TRUE;
//    _isKeystrokeSort = TRUE;
//    _candidateWndWidth = CAND_WIDTH;
//
//    SetInitialCandidateListRange();
//
//    SetDefaultCandidateTextFont();
//
//    return;
//}

//+---------------------------------------------------------------------------
//
// SetupLanguageBar
//
//----------------------------------------------------------------------------

void CCompositionProcessorEngine::SetupLanguageBar(
	_In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId, BOOL isSecureMode,
	_In_reads_(countButtons) const WindowsImeLib::LanguageBarButtonProperty* properties, UINT countButtons)
{

	// INPUT MODE CONVERSION is as default.
    _pCompartmentConversion = std::make_shared<CCompartment>(pThreadMgr, tfClientId, GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION);
    _pCompartmentConversionEventSink.attach(new (std::nothrow) CCompartmentEventSink(CompartmentCallback, this));
    if (_pCompartmentConversionEventSink)
    {
        _pCompartmentConversionEventSink->_Advise(pThreadMgr, GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION);
	}

	for (UINT i = 0; i < countButtons; ++i)
	{
    	const auto& buttonProp = properties[i];

    	wil::com_ptr<CLangBarItemButton> langBarButton;
    	DWORD dwEnable = 1;
    	CreateLanguageBarButton(dwEnable,
    		buttonProp.id, buttonProp.langBarDescription, buttonProp.description, buttonProp.onIconResourceIndex, buttonProp.offIconResourceIndex,
    		&langBarButton, isSecureMode);

    	InitLanguageBar(langBarButton.get(), pThreadMgr, tfClientId, buttonProp.compartmentId);

    	wil::com_ptr<CCompartmentEventSink> compartmentEventSink;
    	compartmentEventSink.attach(new (std::nothrow) CCompartmentEventSink(CompartmentCallback, this));
    	compartmentEventSink->_Advise(pThreadMgr, buttonProp.compartmentId);

		m_langaugeBarButtons.emplace_back(langBarButton);
		m_langaugeBarButtonEventSinks.emplace_back(compartmentEventSink);
	}
}

//+---------------------------------------------------------------------------
//
// CreateLanguageBarButton
//
//----------------------------------------------------------------------------

void CCompositionProcessorEngine::CreateLanguageBarButton(DWORD dwEnable, GUID guidLangBar, _In_z_ LPCWSTR pwszDescriptionValue, _In_z_ LPCWSTR pwszTooltipValue, DWORD dwOnIconIndex, DWORD dwOffIconIndex, _Outptr_result_maybenull_ CLangBarItemButton **ppLangBarItemButton, BOOL isSecureMode)
{
    dwEnable;

    if (ppLangBarItemButton)
    {
        *ppLangBarItemButton = new (std::nothrow) CLangBarItemButton(guidLangBar, pwszDescriptionValue, pwszTooltipValue, dwOnIconIndex, dwOffIconIndex, isSecureMode);
    }

    return;
}

//+---------------------------------------------------------------------------
//
// InitLanguageBar
//
//----------------------------------------------------------------------------

BOOL CCompositionProcessorEngine::InitLanguageBar(_In_ CLangBarItemButton *pLangBarItemButton, _In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId, REFGUID guidCompartment)
{
    if (pLangBarItemButton)
    {
        if (pLangBarItemButton->_AddItem(pThreadMgr) == S_OK)
        {
            if (pLangBarItemButton->_RegisterCompartment(pThreadMgr, tfClientId, guidCompartment))
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

////+---------------------------------------------------------------------------
////
//// SetupDictionaryFile
////
////----------------------------------------------------------------------------
//
//BOOL CompositionProcessorEngine::SetupDictionaryFile()
//{   
//    // Not yet registered
//    // Register CFileMapping
//    WCHAR wszFileName[MAX_PATH] = {'\0'};
//    DWORD cchA = GetModuleFileName(Global::dllInstanceHandle, wszFileName, ARRAYSIZE(wszFileName));
//    size_t iDicFileNameLen = cchA + wcslen(TEXTSERVICE_DIC);
//    WCHAR *pwszFileName = new (std::nothrow) WCHAR[iDicFileNameLen + 1];
//    if (!pwszFileName)
//    {
//        goto ErrorExit;
//    }
//    *pwszFileName = L'\0';
//
//    // find the last '/'
//    while (cchA--)
//    {
//        WCHAR wszChar = wszFileName[cchA];
//        if (wszChar == '\\' || wszChar == '/')
//        {
//            StringCchCopyN(pwszFileName, iDicFileNameLen + 1, wszFileName, static_cast<size_t>(cchA) + 1);
//            StringCchCatN(pwszFileName, iDicFileNameLen + 1, TEXTSERVICE_DIC, wcslen(TEXTSERVICE_DIC));
//            break;
//        }
//    }
//
//    // create CFileMapping object
//    if (_pDictionaryFile == nullptr)
//    {
//        _pDictionaryFile = new (std::nothrow) CFileMapping();
//        if (!_pDictionaryFile)
//        {
//            goto ErrorExit;
//        }
//    }
//    if (!(_pDictionaryFile)->CreateFile(pwszFileName, GENERIC_READ, OPEN_EXISTING, FILE_SHARE_READ))
//    {
//        goto ErrorExit;
//    }
//
//    _pTableDictionaryEngine = new (std::nothrow) CTableDictionaryEngine(GetLocale(), _pDictionaryFile);
//    if (!_pTableDictionaryEngine)
//    {
//        goto ErrorExit;
//    }
//
//    delete []pwszFileName;
//    return TRUE;
//ErrorExit:
//    if (pwszFileName)
//    {
//        delete []pwszFileName;
//    }
//    return FALSE;
//}
//
////+---------------------------------------------------------------------------
////
//// GetDictionaryFile
////
////----------------------------------------------------------------------------
//
//CFile* CompositionProcessorEngine::GetDictionaryFile()
//{
//    return _pDictionaryFile;
//}
//
////+---------------------------------------------------------------------------
////
//// SetupPunctuationPair
////
////----------------------------------------------------------------------------
//
//void CompositionProcessorEngine::SetupPunctuationPair()
//{
//    // Punctuation pair
//    const int pair_count = 2;
//    CPunctuationPair punc_quotation_mark(L'"', 0x201C, 0x201D);
//    CPunctuationPair punc_apostrophe(L'\'', 0x2018, 0x2019);
//
//    CPunctuationPair puncPairs[pair_count] = {
//        punc_quotation_mark,
//        punc_apostrophe,
//    };
//
//    for (int i = 0; i < pair_count; ++i)
//    {
//        CPunctuationPair *pPuncPair = _PunctuationPair.Append();
//        *pPuncPair = puncPairs[i];
//    }
//
//    // Punctuation nest pair
//    CPunctuationNestPair punc_angle_bracket(L'<', 0x300A, 0x3008, L'>', 0x300B, 0x3009);
//
//    CPunctuationNestPair* pPuncNestPair = _PunctuationNestPair.Append();
//    *pPuncNestPair = punc_angle_bracket;
//}
//
//void CompositionProcessorEngine::InitializeSampleIMECompartment(_In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId)
//{
//    // set initial mode
//    CCompartment CompartmentKeyboardOpen(pThreadMgr, tfClientId, GUID_COMPARTMENT_KEYBOARD_OPENCLOSE);
//    CompartmentKeyboardOpen._SetCompartmentBOOL(TRUE);
//
//    CCompartment CompartmentDoubleSingleByte(pThreadMgr, tfClientId, Global::SampleIMEGuidCompartmentDoubleSingleByte);
//    CompartmentDoubleSingleByte._SetCompartmentBOOL(FALSE);
//
//    CCompartment CompartmentPunctuation(pThreadMgr, tfClientId, Global::SampleIMEGuidCompartmentPunctuation);
//    CompartmentPunctuation._SetCompartmentBOOL(TRUE);
//
//    PrivateCompartmentsUpdated(pThreadMgr);
//}

//+---------------------------------------------------------------------------
//
// CompartmentCallback
//
//----------------------------------------------------------------------------

// static
HRESULT CCompositionProcessorEngine::CompartmentCallback(_In_ void *pv, REFGUID guidCompartment)
{
    CCompositionProcessorEngine* fakeThis = (CCompositionProcessorEngine*)pv;

    if (fakeThis && fakeThis->processorEngine)
    {
        return fakeThis->processorEngine->CompartmentCallback(guidCompartment);
    }

    return S_OK;

//    ITfThreadMgr* pThreadMgr = nullptr;
//    HRESULT hr = CoCreateInstance(CLSID_TF_ThreadMgr, nullptr, CLSCTX_INPROC_SERVER, IID_ITfThreadMgr, (void**)&pThreadMgr);
//    if (FAILED(hr))
//    {
//        return E_FAIL;
//    }
//
//    if (IsEqualGUID(guidCompartment, Global::SampleIMEGuidCompartmentDoubleSingleByte) ||
//        IsEqualGUID(guidCompartment, Global::SampleIMEGuidCompartmentPunctuation))
//    {
//        fakeThis->PrivateCompartmentsUpdated(pThreadMgr);
//    }
//    else if (IsEqualGUID(guidCompartment, GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION) ||
//        IsEqualGUID(guidCompartment, GUID_COMPARTMENT_KEYBOARD_INPUTMODE_SENTENCE))
//    {
//        fakeThis->ConversionModeCompartmentUpdated(pThreadMgr);
//    }
//    else if (IsEqualGUID(guidCompartment, GUID_COMPARTMENT_KEYBOARD_OPENCLOSE))
//    {
//        fakeThis->KeyboardOpenCompartmentUpdated(pThreadMgr);
//    }
//
//    pThreadMgr->Release();
//    pThreadMgr = nullptr;
//
//    return S_OK;
}

////+---------------------------------------------------------------------------
////
//// UpdatePrivateCompartments
////
////----------------------------------------------------------------------------
//
//void CompositionProcessorEngine::ConversionModeCompartmentUpdated(_In_ ITfThreadMgr *pThreadMgr)
//{
//    if (!_pCompartmentConversion)
//    {
//        return;
//    }
//
//    DWORD conversionMode = 0;
//    if (FAILED(_pCompartmentConversion->_GetCompartmentDWORD(conversionMode)))
//    {
//        return;
//    }
//
//    BOOL isDouble = FALSE;
//    CCompartment CompartmentDoubleSingleByte(pThreadMgr, _tfClientId, Global::SampleIMEGuidCompartmentDoubleSingleByte);
//    if (SUCCEEDED(CompartmentDoubleSingleByte._GetCompartmentBOOL(isDouble)))
//    {
//        if (!isDouble && (conversionMode & TF_CONVERSIONMODE_FULLSHAPE))
//        {
//            CompartmentDoubleSingleByte._SetCompartmentBOOL(TRUE);
//        }
//        else if (isDouble && !(conversionMode & TF_CONVERSIONMODE_FULLSHAPE))
//        {
//            CompartmentDoubleSingleByte._SetCompartmentBOOL(FALSE);
//        }
//    }
//    BOOL isPunctuation = FALSE;
//    CCompartment CompartmentPunctuation(pThreadMgr, _tfClientId, Global::SampleIMEGuidCompartmentPunctuation);
//    if (SUCCEEDED(CompartmentPunctuation._GetCompartmentBOOL(isPunctuation)))
//    {
//        if (!isPunctuation && (conversionMode & TF_CONVERSIONMODE_SYMBOL))
//        {
//            CompartmentPunctuation._SetCompartmentBOOL(TRUE);
//        }
//        else if (isPunctuation && !(conversionMode & TF_CONVERSIONMODE_SYMBOL))
//        {
//            CompartmentPunctuation._SetCompartmentBOOL(FALSE);
//        }
//    }
//
//    BOOL fOpen = FALSE;
//    CCompartment CompartmentKeyboardOpen(pThreadMgr, _tfClientId, GUID_COMPARTMENT_KEYBOARD_OPENCLOSE);
//    if (SUCCEEDED(CompartmentKeyboardOpen._GetCompartmentBOOL(fOpen)))
//    {
//        if (fOpen && !(conversionMode & TF_CONVERSIONMODE_NATIVE))
//        {
//            CompartmentKeyboardOpen._SetCompartmentBOOL(FALSE);
//        }
//        else if (!fOpen && (conversionMode & TF_CONVERSIONMODE_NATIVE))
//        {
//            CompartmentKeyboardOpen._SetCompartmentBOOL(TRUE);
//        }
//    }
//}
//
////+---------------------------------------------------------------------------
////
//// PrivateCompartmentsUpdated()
////
////----------------------------------------------------------------------------
//
//void CompositionProcessorEngine::PrivateCompartmentsUpdated(_In_ ITfThreadMgr *pThreadMgr)
//{
//    if (!_pCompartmentConversion)
//    {
//        return;
//    }
//
//    DWORD conversionMode = 0;
//    DWORD conversionModePrev = 0;
//    if (FAILED(_pCompartmentConversion->_GetCompartmentDWORD(conversionMode)))
//    {
//        return;
//    }
//
//    conversionModePrev = conversionMode;
//
//    BOOL isDouble = FALSE;
//    CCompartment CompartmentDoubleSingleByte(pThreadMgr, _tfClientId, Global::SampleIMEGuidCompartmentDoubleSingleByte);
//    if (SUCCEEDED(CompartmentDoubleSingleByte._GetCompartmentBOOL(isDouble)))
//    {
//        if (!isDouble && (conversionMode & TF_CONVERSIONMODE_FULLSHAPE))
//        {
//            conversionMode &= ~TF_CONVERSIONMODE_FULLSHAPE;
//        }
//        else if (isDouble && !(conversionMode & TF_CONVERSIONMODE_FULLSHAPE))
//        {
//            conversionMode |= TF_CONVERSIONMODE_FULLSHAPE;
//        }
//    }
//
//    BOOL isPunctuation = FALSE;
//    CCompartment CompartmentPunctuation(pThreadMgr, _tfClientId, Global::SampleIMEGuidCompartmentPunctuation);
//    if (SUCCEEDED(CompartmentPunctuation._GetCompartmentBOOL(isPunctuation)))
//    {
//        if (!isPunctuation && (conversionMode & TF_CONVERSIONMODE_SYMBOL))
//        {
//            conversionMode &= ~TF_CONVERSIONMODE_SYMBOL;
//        }
//        else if (isPunctuation && !(conversionMode & TF_CONVERSIONMODE_SYMBOL))
//        {
//            conversionMode |= TF_CONVERSIONMODE_SYMBOL;
//        }
//    }
//
//    if (conversionMode != conversionModePrev)
//    {
//        _pCompartmentConversion->_SetCompartmentDWORD(conversionMode);
//    }
//}
//
////+---------------------------------------------------------------------------
////
//// KeyboardOpenCompartmentUpdated
////
////----------------------------------------------------------------------------
//
//void CompositionProcessorEngine::KeyboardOpenCompartmentUpdated(_In_ ITfThreadMgr *pThreadMgr)
//{
//    if (!_pCompartmentConversion)
//    {
//        return;
//    }
//
//    DWORD conversionMode = 0;
//    DWORD conversionModePrev = 0;
//    if (FAILED(_pCompartmentConversion->_GetCompartmentDWORD(conversionMode)))
//    {
//        return;
//    }
//
//    conversionModePrev = conversionMode;
//
//    BOOL isOpen = FALSE;
//    CCompartment CompartmentKeyboardOpen(pThreadMgr, _tfClientId, GUID_COMPARTMENT_KEYBOARD_OPENCLOSE);
//    if (SUCCEEDED(CompartmentKeyboardOpen._GetCompartmentBOOL(isOpen)))
//    {
//        if (isOpen && !(conversionMode & TF_CONVERSIONMODE_NATIVE))
//        {
//            conversionMode |= TF_CONVERSIONMODE_NATIVE;
//        }
//        else if (!isOpen && (conversionMode & TF_CONVERSIONMODE_NATIVE))
//        {
//            conversionMode &= ~TF_CONVERSIONMODE_NATIVE;
//        }
//    }
//
//    if (conversionMode != conversionModePrev)
//    {
//        _pCompartmentConversion->_SetCompartmentDWORD(conversionMode);
//    }
//}
//
//
////////////////////////////////////////////////////////////////////////
////
//// XPreservedKey implementation.
////
////////////////////////////////////////////////////////////////////////
//
////+---------------------------------------------------------------------------
////
//// UninitPreservedKey
////
////----------------------------------------------------------------------------
//
//BOOL CompositionProcessorEngine::XPreservedKey::UninitPreservedKey(_In_ ITfThreadMgr *pThreadMgr)
//{
//    ITfKeystrokeMgr* pKeystrokeMgr = nullptr;
//
//    if (IsEqualGUID(Guid, GUID_NULL))
//    {
//        return FALSE;
//    }
//
//    if (FAILED(pThreadMgr->QueryInterface(IID_ITfKeystrokeMgr, (void **)&pKeystrokeMgr)))
//    {
//        return FALSE;
//    }
//
//    for (UINT i = 0; i < TSFPreservedKeyTable.Count(); i++)
//    {
//        TF_PRESERVEDKEY pPreservedKey = *TSFPreservedKeyTable.GetAt(i);
//        pPreservedKey.uModifiers &= 0xffff;
//
//        pKeystrokeMgr->UnpreserveKey(Guid, &pPreservedKey);
//    }
//
//    pKeystrokeMgr->Release();
//
//    return TRUE;
//}
//
//CompositionProcessorEngine::XPreservedKey::XPreservedKey()
//{
//    Guid = GUID_NULL;
//    Description = nullptr;
//}
//
//CompositionProcessorEngine::XPreservedKey::~XPreservedKey()
//{
//    ITfThreadMgr* pThreadMgr = nullptr;
//
//    HRESULT hr = CoCreateInstance(CLSID_TF_ThreadMgr, NULL, CLSCTX_INPROC_SERVER, IID_ITfThreadMgr, (void**)&pThreadMgr);
//    if (SUCCEEDED(hr))
//    {
//        UninitPreservedKey(pThreadMgr);
//        pThreadMgr->Release();
//        pThreadMgr = nullptr;
//    }
//
//    if (Description)
//    {
//        delete [] Description;
//    }
//}
//
//void CompositionProcessorEngine::InitKeyStrokeTable()
//{
//    for (int i = 0; i < 26; i++)
//    {
//        _keystrokeTable[i].VirtualKey = 'A' + i;
//        _keystrokeTable[i].Modifiers = 0;
//        _keystrokeTable[i].Function = FUNCTION_INPUT;
//    }
//}
//
//void CompositionProcessorEngine::SetInitialCandidateListRange()
//{
//    for (DWORD i = 1; i <= 10; i++)
//    {
//        DWORD* pNewIndexRange = nullptr;
//
//        pNewIndexRange = _candidateListIndexRange.Append();
//        if (pNewIndexRange != nullptr)
//        {
//            if (i != 10)
//            {
//                *pNewIndexRange = i;
//            }
//            else
//            {
//                *pNewIndexRange = 0;
//            }
//        }
//    }
//}

void CCompositionProcessorEngine::SetDefaultCandidateTextFont(int idsDefaultFont)
{
    // Candidate Text Font
    if (Global::defaultlFontHandle == nullptr)
    {
        WCHAR fontName[50] = {'\0'}; 
        LoadString(Global::dllInstanceHandle, idsDefaultFont, fontName, 50);
        Global::defaultlFontHandle = CreateFont(-MulDiv(10, GetDeviceCaps(GetDC(NULL), LOGPIXELSY), 72), 0, 0, 0, FW_MEDIUM, 0, 0, 0, 0, 0, 0, 0, 0, fontName);
        if (!Global::defaultlFontHandle)
        {
            LOGFONT lf = {};
            SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &lf, 0);
            // Fall back to the default GUI font on failure.
            Global::defaultlFontHandle = CreateFont(-MulDiv(10, GetDeviceCaps(GetDC(NULL), LOGPIXELSY), 72), 0, 0, 0, FW_MEDIUM, 0, 0, 0, 0, 0, 0, 0, 0, lf.lfFaceName);
        }
    }
}

////////////////////////////////////////////////////////////////////////
////
////    CompositionProcessorEngine
////
////////////////////////////////////////////////////////////////////////
//
////+---------------------------------------------------------------------------
////
//// CompositionProcessorEngine::IsVirtualKeyNeed
////
//// Test virtual key code need to the Composition Processor Engine.
//// param
////     [in] uCode - Specify virtual key code.
////     [in/out] pwch       - char code
////     [in] fComposing     - Specified composing.
////     [in] fCandidateMode - Specified candidate mode.
////     [out] pKeyState     - Returns function regarding virtual key.
//// returns
////     If engine need this virtual key code, returns true. Otherwise returns false.
////----------------------------------------------------------------------------
//
//BOOL CompositionProcessorEngine::IsVirtualKeyNeed(UINT uCode, _In_reads_(1) WCHAR *pwch, BOOL fComposing, CANDIDATE_MODE candidateMode, BOOL hasCandidateWithWildcard, _Out_opt_ _KEYSTROKE_STATE *pKeyState)
//{
//    if (pKeyState)
//    {
//        pKeyState->Category = CATEGORY_NONE;
//        pKeyState->Function = FUNCTION_NONE;
//    }
//
//    if (candidateMode == CANDIDATE_ORIGINAL || candidateMode == CANDIDATE_PHRASE || candidateMode == CANDIDATE_WITH_NEXT_COMPOSITION)
//    {
//        fComposing = FALSE;
//    }
//
//    if (fComposing || candidateMode == CANDIDATE_INCREMENTAL || candidateMode == CANDIDATE_NONE)
//    {
//        if (IsVirtualKeyKeystrokeComposition(uCode, pKeyState, FUNCTION_NONE))
//        {
//            return TRUE;
//        }
//        else if ((IsWildcard() && IsWildcardChar(*pwch) && !IsDisableWildcardAtFirst()) ||
//            (IsWildcard() && IsWildcardChar(*pwch) &&  IsDisableWildcardAtFirst() && _keystrokeBuffer.GetLength()))
//        {
//            if (pKeyState)
//            {
//                pKeyState->Category = CATEGORY_COMPOSING;
//                pKeyState->Function = FUNCTION_INPUT;
//            }
//            return TRUE;
//        }
//        else if (_hasWildcardIncludedInKeystrokeBuffer && uCode == VK_SPACE)
//        {
//            if (pKeyState) { pKeyState->Category = CATEGORY_COMPOSING; pKeyState->Function = FUNCTION_CONVERT_WILDCARD; } return TRUE;
//        }
//    }
//
//    if (candidateMode == CANDIDATE_ORIGINAL || candidateMode == CANDIDATE_PHRASE || candidateMode == CANDIDATE_WITH_NEXT_COMPOSITION)
//    {
//        BOOL isRetCode = TRUE;
//        if (IsVirtualKeyKeystrokeCandidate(uCode, pKeyState, candidateMode, &isRetCode, &_KeystrokeCandidate))
//        {
//            return isRetCode;
//        }
//
//        if (hasCandidateWithWildcard)
//        {
//            if (IsVirtualKeyKeystrokeCandidate(uCode, pKeyState, candidateMode, &isRetCode, &_KeystrokeCandidateWildcard))
//            {
//                return isRetCode;
//            }
//        }
//
//        // Candidate list could not handle key. We can try to restart the composition.
//        if (IsVirtualKeyKeystrokeComposition(uCode, pKeyState, FUNCTION_INPUT))
//        {
//            if (candidateMode != CANDIDATE_ORIGINAL)
//            {
//                return TRUE;
//            }
//            else
//            {
//                if (pKeyState) { pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_FINALIZE_CANDIDATELIST_AND_INPUT; } 
//                return TRUE;
//            }
//        }
//    } 
//
//    // CANDIDATE_INCREMENTAL should process Keystroke.Candidate virtual keys.
//    else if (candidateMode == CANDIDATE_INCREMENTAL)
//    {
//        BOOL isRetCode = TRUE;
//        if (IsVirtualKeyKeystrokeCandidate(uCode, pKeyState, candidateMode, &isRetCode, &_KeystrokeCandidate))
//        {
//            return isRetCode;
//        }
//    }
//
//    if (!fComposing && candidateMode != CANDIDATE_ORIGINAL && candidateMode != CANDIDATE_PHRASE && candidateMode != CANDIDATE_WITH_NEXT_COMPOSITION) 
//    {
//        if (IsVirtualKeyKeystrokeComposition(uCode, pKeyState, FUNCTION_INPUT))
//        {
//            return TRUE;
//        }
//    }
//
//    // System pre-defined keystroke
//    if (fComposing)
//    {
//        if ((candidateMode != CANDIDATE_INCREMENTAL))
//        {
//            switch (uCode)
//            {
//            case VK_LEFT:   if (pKeyState) { pKeyState->Category = CATEGORY_COMPOSING; pKeyState->Function = FUNCTION_MOVE_LEFT; } return TRUE;
//            case VK_RIGHT:  if (pKeyState) { pKeyState->Category = CATEGORY_COMPOSING; pKeyState->Function = FUNCTION_MOVE_RIGHT; } return TRUE;
//            case VK_RETURN: if (pKeyState) { pKeyState->Category = CATEGORY_COMPOSING; pKeyState->Function = FUNCTION_FINALIZE_CANDIDATELIST; } return TRUE;
//            case VK_ESCAPE: if (pKeyState) { pKeyState->Category = CATEGORY_COMPOSING; pKeyState->Function = FUNCTION_CANCEL; } return TRUE;
//            case VK_BACK:   if (pKeyState) { pKeyState->Category = CATEGORY_COMPOSING; pKeyState->Function = FUNCTION_BACKSPACE; } return TRUE;
//
//            case VK_UP:     if (pKeyState) { pKeyState->Category = CATEGORY_COMPOSING; pKeyState->Function = FUNCTION_MOVE_UP; } return TRUE;
//            case VK_DOWN:   if (pKeyState) { pKeyState->Category = CATEGORY_COMPOSING; pKeyState->Function = FUNCTION_MOVE_DOWN; } return TRUE;
//            case VK_PRIOR:  if (pKeyState) { pKeyState->Category = CATEGORY_COMPOSING; pKeyState->Function = FUNCTION_MOVE_PAGE_UP; } return TRUE;
//            case VK_NEXT:   if (pKeyState) { pKeyState->Category = CATEGORY_COMPOSING; pKeyState->Function = FUNCTION_MOVE_PAGE_DOWN; } return TRUE;
//
//            case VK_HOME:   if (pKeyState) { pKeyState->Category = CATEGORY_COMPOSING; pKeyState->Function = FUNCTION_MOVE_PAGE_TOP; } return TRUE;
//            case VK_END:    if (pKeyState) { pKeyState->Category = CATEGORY_COMPOSING; pKeyState->Function = FUNCTION_MOVE_PAGE_BOTTOM; } return TRUE;
//
//            case VK_SPACE:  if (pKeyState) { pKeyState->Category = CATEGORY_COMPOSING; pKeyState->Function = FUNCTION_CONVERT; } return TRUE;
//            }
//        }
//        else if ((candidateMode == CANDIDATE_INCREMENTAL))
//        {
//            switch (uCode)
//            {
//                // VK_LEFT, VK_RIGHT - set *pIsEaten = FALSE for application could move caret left or right.
//                // and for CUAS, invoke _HandleCompositionCancel() edit session due to ignore CUAS default key handler for send out terminate composition
//            case VK_LEFT:
//            case VK_RIGHT:
//                {
//                    if (pKeyState)
//                    {
//                        pKeyState->Category = CATEGORY_INVOKE_COMPOSITION_EDIT_SESSION;
//                        pKeyState->Function = FUNCTION_CANCEL;
//                    }
//                }
//                return FALSE;
//
//            case VK_RETURN: if (pKeyState) { pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_FINALIZE_CANDIDATELIST; } return TRUE;
//            case VK_ESCAPE: if (pKeyState) { pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_CANCEL; } return TRUE;
//
//                // VK_BACK - remove one char from reading string.
//            case VK_BACK:   if (pKeyState) { pKeyState->Category = CATEGORY_COMPOSING; pKeyState->Function = FUNCTION_BACKSPACE; } return TRUE;
//
//            case VK_UP:     if (pKeyState) { pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_MOVE_UP; } return TRUE;
//            case VK_DOWN:   if (pKeyState) { pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_MOVE_DOWN; } return TRUE;
//            case VK_PRIOR:  if (pKeyState) { pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_MOVE_PAGE_UP; } return TRUE;
//            case VK_NEXT:   if (pKeyState) { pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_MOVE_PAGE_DOWN; } return TRUE;
//
//            case VK_HOME:   if (pKeyState) { pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_MOVE_PAGE_TOP; } return TRUE;
//            case VK_END:    if (pKeyState) { pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_MOVE_PAGE_BOTTOM; } return TRUE;
//
//            case VK_SPACE:
//                {
//                    if (candidateMode == CANDIDATE_INCREMENTAL)
//                    {
//                        if (pKeyState) { pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_CONVERT; } return TRUE;
//                    }
//                    else
//                    {
//                        if (pKeyState) { pKeyState->Category = CATEGORY_COMPOSING; pKeyState->Function = FUNCTION_CONVERT; } return TRUE;
//                    }
//                }
//            }
//        }
//    }
//
//    if ((candidateMode == CANDIDATE_ORIGINAL) || (candidateMode == CANDIDATE_WITH_NEXT_COMPOSITION))
//    {
//        switch (uCode)
//        {
//        case VK_UP:     if (pKeyState) { pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_MOVE_UP; } return TRUE;
//        case VK_DOWN:   if (pKeyState) { pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_MOVE_DOWN; } return TRUE;
//        case VK_PRIOR:  if (pKeyState) { pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_MOVE_PAGE_UP; } return TRUE;
//        case VK_NEXT:   if (pKeyState) { pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_MOVE_PAGE_DOWN; } return TRUE;
//        case VK_HOME:   if (pKeyState) { pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_MOVE_PAGE_TOP; } return TRUE;
//        case VK_END:    if (pKeyState) { pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_MOVE_PAGE_BOTTOM; } return TRUE;
//        case VK_RETURN: if (pKeyState) { pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_FINALIZE_CANDIDATELIST; } return TRUE;
//        case VK_SPACE:  if (pKeyState) { pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_CONVERT; } return TRUE;
//        case VK_BACK:   if (pKeyState) { pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_CANCEL; } return TRUE;
//
//        case VK_ESCAPE:
//            {
//                if (candidateMode == CANDIDATE_WITH_NEXT_COMPOSITION)
//                {
//                    if (pKeyState)
//                    {
//                        pKeyState->Category = CATEGORY_INVOKE_COMPOSITION_EDIT_SESSION;
//                        pKeyState->Function = FUNCTION_FINALIZE_TEXTSTORE;
//                    }
//                    return TRUE;
//                }
//                else
//                {
//                    if (pKeyState)
//                    {
//                        pKeyState->Category = CATEGORY_CANDIDATE;
//                        pKeyState->Function = FUNCTION_CANCEL;
//                    }
//                    return TRUE;
//                }
//            }
//        }
//
//        if (candidateMode == CANDIDATE_WITH_NEXT_COMPOSITION)
//        {
//            if (IsVirtualKeyKeystrokeComposition(uCode, NULL, FUNCTION_NONE))
//            {
//                if (pKeyState) { pKeyState->Category = CATEGORY_COMPOSING; pKeyState->Function = FUNCTION_FINALIZE_TEXTSTORE_AND_INPUT; } return TRUE;
//            }
//        }
//    }
//
//    if (candidateMode == CANDIDATE_PHRASE)
//    {
//        switch (uCode)
//        {
//        case VK_UP:     if (pKeyState) { pKeyState->Category = CATEGORY_PHRASE; pKeyState->Function = FUNCTION_MOVE_UP; } return TRUE;
//        case VK_DOWN:   if (pKeyState) { pKeyState->Category = CATEGORY_PHRASE; pKeyState->Function = FUNCTION_MOVE_DOWN; } return TRUE;
//        case VK_PRIOR:  if (pKeyState) { pKeyState->Category = CATEGORY_PHRASE; pKeyState->Function = FUNCTION_MOVE_PAGE_UP; } return TRUE;
//        case VK_NEXT:   if (pKeyState) { pKeyState->Category = CATEGORY_PHRASE; pKeyState->Function = FUNCTION_MOVE_PAGE_DOWN; } return TRUE;
//        case VK_HOME:   if (pKeyState) { pKeyState->Category = CATEGORY_PHRASE; pKeyState->Function = FUNCTION_MOVE_PAGE_TOP; } return TRUE;
//        case VK_END:    if (pKeyState) { pKeyState->Category = CATEGORY_PHRASE; pKeyState->Function = FUNCTION_MOVE_PAGE_BOTTOM; } return TRUE;
//        case VK_RETURN: if (pKeyState) { pKeyState->Category = CATEGORY_PHRASE; pKeyState->Function = FUNCTION_FINALIZE_CANDIDATELIST; } return TRUE;
//        case VK_SPACE:  if (pKeyState) { pKeyState->Category = CATEGORY_PHRASE; pKeyState->Function = FUNCTION_CONVERT; } return TRUE;
//        case VK_ESCAPE: if (pKeyState) { pKeyState->Category = CATEGORY_PHRASE; pKeyState->Function = FUNCTION_CANCEL; } return TRUE;
//        case VK_BACK:   if (pKeyState) { pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_CANCEL; } return TRUE;
//        }
//    }
//
//    if (IsKeystrokeRange(uCode, pKeyState, candidateMode))
//    {
//        return TRUE;
//    }
//    else if (pKeyState && pKeyState->Category != CATEGORY_NONE)
//    {
//        return FALSE;
//    }
//
//    if (*pwch && !IsVirtualKeyKeystrokeComposition(uCode, pKeyState, FUNCTION_NONE))
//    {
//        if (pKeyState)
//        {
//            pKeyState->Category = CATEGORY_INVOKE_COMPOSITION_EDIT_SESSION;
//            pKeyState->Function = FUNCTION_FINALIZE_TEXTSTORE;
//        }
//        return FALSE;
//    }
//
//    return FALSE;
//}
//
////+---------------------------------------------------------------------------
////
//// CompositionProcessorEngine::IsVirtualKeyKeystrokeComposition
////
////----------------------------------------------------------------------------
//
//BOOL CompositionProcessorEngine::IsVirtualKeyKeystrokeComposition(UINT uCode, _Out_opt_ _KEYSTROKE_STATE *pKeyState, KEYSTROKE_FUNCTION function)
//{
//    if (pKeyState == nullptr)
//    {
//        return FALSE;
//    }
//
//    pKeyState->Category = CATEGORY_NONE;
//    pKeyState->Function = FUNCTION_NONE;
//
//    for (UINT i = 0; i < _KeystrokeComposition.Count(); i++)
//    {
//        _KEYSTROKE *pKeystroke = nullptr;
//
//        pKeystroke = _KeystrokeComposition.GetAt(i);
//
//        if ((pKeystroke->VirtualKey == uCode) && Global::CheckModifiers(WindowsImeLib::ModifiersValue, pKeystroke->Modifiers))
//        {
//            if (function == FUNCTION_NONE)
//            {
//                pKeyState->Category = CATEGORY_COMPOSING;
//                pKeyState->Function = pKeystroke->Function;
//                return TRUE;
//            }
//            else if (function == pKeystroke->Function)
//            {
//                pKeyState->Category = CATEGORY_COMPOSING;
//                pKeyState->Function = pKeystroke->Function;
//                return TRUE;
//            }
//        }
//    }
//
//    return FALSE;
//}
//
////+---------------------------------------------------------------------------
////
//// CompositionProcessorEngine::IsVirtualKeyKeystrokeCandidate
////
////----------------------------------------------------------------------------
//
//BOOL CompositionProcessorEngine::IsVirtualKeyKeystrokeCandidate(UINT uCode, _In_ _KEYSTROKE_STATE *pKeyState, CANDIDATE_MODE candidateMode, _Out_ BOOL *pfRetCode, _In_ std::vector<_KEYSTROKE> *pKeystrokeMetric)
//{
//    if (pfRetCode == nullptr)
//    {
//        return FALSE;
//    }
//    *pfRetCode = FALSE;
//
//    for (UINT i = 0; i < pKeystrokeMetric->Count(); i++)
//    {
//        _KEYSTROKE *pKeystroke = nullptr;
//
//        pKeystroke = pKeystrokeMetric->GetAt(i);
//
//        if ((pKeystroke->VirtualKey == uCode) && Global::CheckModifiers(WindowsImeLib::ModifiersValue, pKeystroke->Modifiers))
//        {
//            *pfRetCode = TRUE;
//            if (pKeyState)
//            {
//                pKeyState->Category = (candidateMode == CANDIDATE_ORIGINAL ? CATEGORY_CANDIDATE :
//                    candidateMode == CANDIDATE_PHRASE ? CATEGORY_PHRASE : CATEGORY_CANDIDATE);
//
//                pKeyState->Function = pKeystroke->Function;
//            }
//            return TRUE;
//        }
//    }
//
//    return FALSE;
//}
//
////+---------------------------------------------------------------------------
////
//// CompositionProcessorEngine::IsKeyKeystrokeRange
////
////----------------------------------------------------------------------------
//
//BOOL CompositionProcessorEngine::IsKeystrokeRange(UINT uCode, _Out_ _KEYSTROKE_STATE *pKeyState, CANDIDATE_MODE candidateMode)
//{
//    if (pKeyState == nullptr)
//    {
//        return FALSE;
//    }
//
//    pKeyState->Category = CATEGORY_NONE;
//    pKeyState->Function = FUNCTION_NONE;
//
//    if (_candidateListIndexRange.IsRange(uCode))
//    {
//        if (candidateMode == CANDIDATE_PHRASE)
//        {
//            // Candidate phrase could specify modifier
//            if ((GetCandidateListPhraseModifier() == 0 && WindowsImeLib::ModifiersValue == 0) ||
//                (GetCandidateListPhraseModifier() != 0 && Global::CheckModifiers(WindowsImeLib::ModifiersValue, GetCandidateListPhraseModifier())))
//            {
//                pKeyState->Category = CATEGORY_PHRASE; pKeyState->Function = FUNCTION_SELECT_BY_NUMBER;
//                return TRUE;
//            }
//            else
//            {
//                pKeyState->Category = CATEGORY_INVOKE_COMPOSITION_EDIT_SESSION; pKeyState->Function = FUNCTION_FINALIZE_TEXTSTORE_AND_INPUT;
//                return FALSE;
//            }
//        }
//        else if (candidateMode == CANDIDATE_WITH_NEXT_COMPOSITION)
//        {
//            // Candidate phrase could specify modifier
//            if ((GetCandidateListPhraseModifier() == 0 && WindowsImeLib::ModifiersValue == 0) ||
//                (GetCandidateListPhraseModifier() != 0 && Global::CheckModifiers(WindowsImeLib::ModifiersValue, GetCandidateListPhraseModifier())))
//            {
//                pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_SELECT_BY_NUMBER;
//                return TRUE;
//            }
//            // else next composition
//        }
//        else if (candidateMode != CANDIDATE_NONE)
//        {
//            pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_SELECT_BY_NUMBER;
//            return TRUE;
//        }
//    }
//    return FALSE;
//}


BOOL CCompositionProcessorEngine::GetCompartmentBool(ITfThreadMgr *pThreadMgr, TfClientId tfClientId, REFGUID guidCompartment)
{
    BOOL value = FALSE;
    CCompartment compartment(pThreadMgr, tfClientId, guidCompartment);
    LOG_IF_FAILED(compartment._GetCompartmentBOOL(value));
    return value;
}

void CCompositionProcessorEngine::SetCompartmentBool(ITfThreadMgr *pThreadMgr, TfClientId tfClientId, REFGUID guidCompartment, BOOL value)
{
    CCompartment compartment(pThreadMgr, tfClientId, guidCompartment);
    LOG_IF_FAILED(compartment._SetCompartmentBOOL(value));
}

DWORD CCompositionProcessorEngine::GetCompartmentDword(ITfThreadMgr *pThreadMgr, TfClientId tfClientId, REFGUID guidCompartment)
{
    DWORD value = 0;
    CCompartment compartment(pThreadMgr, tfClientId, guidCompartment);
    LOG_IF_FAILED(compartment._GetCompartmentDWORD(value));
    return value;
}

void CCompositionProcessorEngine::SetCompartmentDword(ITfThreadMgr *pThreadMgr, TfClientId tfClientId, REFGUID guidCompartment, DWORD value)
{
    CCompartment compartment(pThreadMgr, tfClientId, guidCompartment);
    LOG_IF_FAILED(compartment._SetCompartmentDWORD(value));
}

void CCompositionProcessorEngine::ClearCompartment(ITfThreadMgr *pThreadMgr, TfClientId tfClientId, REFGUID guidCompartment)
{
    CCompartment compartment(pThreadMgr, tfClientId, guidCompartment);
    LOG_IF_FAILED(compartment._ClearCompartment());
}
