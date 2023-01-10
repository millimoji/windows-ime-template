// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "pch.h"
#include "ProcessorEngine.h"
#include "TableDictionaryEngine.h"
#include "DictionarySearch.h"
// #include "TfInputProcessorProfile.h"
#include "SampleIMEGlobals.h"
// #include "RegKey.h"

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

CompositionProcessorEngine::CompositionProcessorEngine(const std::weak_ptr<WindowsImeLib::ICompositionProcessorEngineOwner>& owner) :
    m_owner(owner)
{
    _pTableDictionaryEngine = nullptr;
    _pDictionaryFile = nullptr;

//    _langid = 0xffff;
//    _guidProfile = GUID_NULL;
//    _tfClientId = TF_CLIENTID_NULL;

//    _pLanguageBar_IMEMode = nullptr;
//    _pLanguageBar_DoubleSingleByte = nullptr;
//    _pLanguageBar_Punctuation = nullptr;

//    _pCompartmentConversion = nullptr;
//    _pCompartmentKeyboardOpenEventSink = nullptr;
//    _pCompartmentConversionEventSink = nullptr;
//    _pCompartmentDoubleSingleByteEventSink = nullptr;
//    _pCompartmentPunctuationEventSink = nullptr;

    _hasWildcardIncludedInKeystrokeBuffer = FALSE;

    _isWildcard = FALSE;
    _isDisableWildcardAtFirst = FALSE;
    _hasMakePhraseFromText = FALSE;
    _isKeystrokeSort = FALSE;

    _candidateListPhraseModifier = 0;

    _candidateWndWidth = CAND_WIDTH;

    InitKeyStrokeTable();
}

//+---------------------------------------------------------------------------
//
// dtor
//
//----------------------------------------------------------------------------

CompositionProcessorEngine::~CompositionProcessorEngine()
{
    if (_pTableDictionaryEngine)
    {
        delete _pTableDictionaryEngine;
        _pTableDictionaryEngine = nullptr;
    }

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

    if (_pDictionaryFile)
    {
        delete _pDictionaryFile;
        _pDictionaryFile = nullptr;
    }
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

BOOL CompositionProcessorEngine::Initialize()
{
    BOOL ret = TRUE;

//    if ((tfClientId == 0) && (pThreadMgr == nullptr))
//    {
//        ret = FALSE;
//        goto Exit;
//    }

//    _isComLessMode = isComLessMode;
//    _langid = langid;
//    _guidProfile = guidLanguageProfile;
//    _tfClientId = tfClientId;

    // SetupPreserved(pThreadMgr, tfClientId); 
    // InitializeSampleIMECompartment(pThreadMgr, tfClientId);
    SetupPunctuationPair();
    // SetupLanguageBar(pThreadMgr, tfClientId, isSecureMode);
    SetupKeystroke();
    SetupConfiguration();
    SetupDictionaryFile();
// Exit:
    return ret;
}

//+---------------------------------------------------------------------------
//
// AddVirtualKey
// Add virtual key code to Composition Processor Engine for used to parse keystroke data.
// param
//     [in] uCode - Specify virtual key code.
// returns
//     State of Text Processor Engine.
//----------------------------------------------------------------------------

BOOL CompositionProcessorEngine::AddVirtualKey(WCHAR wch)
{
    if (!wch)
    {
        return FALSE;
    }

    //
    // append one keystroke in buffer.
    //
    DWORD_PTR srgKeystrokeBufLen = _keystrokeBuffer.GetLength();
    PWCHAR pwch = new (std::nothrow) WCHAR[ srgKeystrokeBufLen + 1 ];
    if (!pwch)
    {
        return FALSE;
    }

    memcpy(pwch, _keystrokeBuffer.Get(), srgKeystrokeBufLen * sizeof(WCHAR));
    pwch[ srgKeystrokeBufLen ] = wch;

    if (_keystrokeBuffer.Get())
    {
        delete [] _keystrokeBuffer.Get();
    }

    _keystrokeBuffer.Set(pwch, srgKeystrokeBufLen + 1);

    return TRUE;
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

void CompositionProcessorEngine::RemoveVirtualKey(DWORD_PTR dwIndex)
{
    DWORD_PTR srgKeystrokeBufLen = _keystrokeBuffer.GetLength();

    if (dwIndex + 1 < srgKeystrokeBufLen)
    {
        // shift following eles left
        memmove((BYTE*)_keystrokeBuffer.Get() + (dwIndex * sizeof(WCHAR)),
            (BYTE*)_keystrokeBuffer.Get() + ((dwIndex + 1) * sizeof(WCHAR)),
            (srgKeystrokeBufLen - dwIndex - 1) * sizeof(WCHAR));
    }

    _keystrokeBuffer.Set(_keystrokeBuffer.Get(), srgKeystrokeBufLen - 1);
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

void CompositionProcessorEngine::PurgeVirtualKey()
{
    if (_keystrokeBuffer.Get())
    {
        delete [] _keystrokeBuffer.Get();
        _keystrokeBuffer.Set(NULL, 0);
    }
}

WCHAR CompositionProcessorEngine::GetVirtualKey(DWORD_PTR dwIndex) 
{ 
    if (dwIndex < _keystrokeBuffer.GetLength())
    {
        return *(_keystrokeBuffer.Get() + dwIndex);
    }
    return 0;
}
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

void CompositionProcessorEngine::GetReadingStrings(_Inout_ std::vector<CStringRange> *pReadingStrings, _Out_ BOOL *pIsWildcardIncluded)
{
    CStringRange oneKeystroke;

    _hasWildcardIncludedInKeystrokeBuffer = FALSE;

    if (pReadingStrings->size() == 0 && _keystrokeBuffer.GetLength())
    {
        CStringRange* pNewString = nullptr;

        pReadingStrings->push_back(CStringRange());
        pNewString = &pReadingStrings->back();
        if (pNewString)
        {
            *pNewString = _keystrokeBuffer;
        }

        for (DWORD index = 0; index < _keystrokeBuffer.GetLength(); index++)
        {
            oneKeystroke.Set(_keystrokeBuffer.Get() + index, 1);

            if (IsWildcard() && IsWildcardChar(*oneKeystroke.Get()))
            {
                _hasWildcardIncludedInKeystrokeBuffer = TRUE;
            }
        }
    }

    *pIsWildcardIncluded = _hasWildcardIncludedInKeystrokeBuffer;
}

//+---------------------------------------------------------------------------
//
// GetCandidateList
//
//----------------------------------------------------------------------------

void CompositionProcessorEngine::GetCandidateList(_Inout_ std::vector<CCandidateListItem> *pCandidateList, BOOL isIncrementalWordSearch, BOOL isWildcardSearch)
{
    if (!IsDictionaryAvailable())
    {
        return;
    }

    if (isIncrementalWordSearch)
    {
        CStringRange wildcardSearch;
        DWORD_PTR keystrokeBufLen = _keystrokeBuffer.GetLength() + 2;
        PWCHAR pwch = new (std::nothrow) WCHAR[ keystrokeBufLen ];
        if (!pwch)
        {
            return;
        }

        // check keystroke buffer already has wildcard char which end user want wildcard serach
        DWORD wildcardIndex = 0;
        BOOL isFindWildcard = FALSE;

        if (IsWildcard())
        {
            for (wildcardIndex = 0; wildcardIndex < _keystrokeBuffer.GetLength(); wildcardIndex++)
            {
                if (IsWildcardChar(*(_keystrokeBuffer.Get() + wildcardIndex)))
                {
                    isFindWildcard = TRUE;
                    break;
                }
            }
        }

        StringCchCopyN(pwch, keystrokeBufLen, _keystrokeBuffer.Get(), _keystrokeBuffer.GetLength());

        if (!isFindWildcard)
        {
            // add wildcard char for incremental search
            StringCchCat(pwch, keystrokeBufLen, L"*");
        }

        size_t len = 0;
        if (StringCchLength(pwch, STRSAFE_MAX_CCH, &len) == S_OK)
        {
            wildcardSearch.Set(pwch, len);
        }
        else
        {
            return;
        }

        _pTableDictionaryEngine->CollectWordForWildcard(&wildcardSearch, pCandidateList);

        if (0 >= pCandidateList->size())
        {
            return;
        }

        if (IsKeystrokeSort())
        {
            _pTableDictionaryEngine->SortListItemByFindKeyCode(pCandidateList);
        }

        // Incremental search would show keystroke data from all candidate list items
        // but wont show identical keystroke data for user inputted.
        for (UINT index = 0; index < pCandidateList->size(); index++)
        {
            CCandidateListItem *pLI = &pCandidateList->at(index);
            DWORD_PTR keystrokeBufferLen = 0;

            if (IsWildcard())
            {
                keystrokeBufferLen = wildcardIndex;
            }
            else
            {
                keystrokeBufferLen = _keystrokeBuffer.GetLength();
            }

            CStringRange newFindKeyCode;
            newFindKeyCode.Set(pLI->_FindKeyCode.Get() + keystrokeBufferLen, pLI->_FindKeyCode.GetLength() - keystrokeBufferLen);
            pLI->_FindKeyCode.Set(newFindKeyCode);
        }

        delete [] pwch;
    }
    else if (isWildcardSearch)
    {
        _pTableDictionaryEngine->CollectWordForWildcard(&_keystrokeBuffer, pCandidateList);
    }
    else
    {
        _pTableDictionaryEngine->CollectWord(&_keystrokeBuffer, pCandidateList);
    }

    for (UINT index = 0; index < pCandidateList->size();)
    {
        CCandidateListItem *pLI = &pCandidateList->at(index);
        CStringRange startItemString;
        CStringRange endItemString;

        startItemString.Set(pLI->_ItemString.Get(), 1);
        endItemString.Set(pLI->_ItemString.Get() + pLI->_ItemString.GetLength() - 1, 1);

        index++;
    }
}

//+---------------------------------------------------------------------------
//
// GetCandidateStringInConverted
//
//----------------------------------------------------------------------------

void CompositionProcessorEngine::GetCandidateStringInConverted(CStringRange &searchString, _In_ std::vector<CCandidateListItem> *pCandidateList)
{
    if (!IsDictionaryAvailable())
    {
        return;
    }

    // Search phrase from SECTION_TEXT's converted string list
    CStringRange wildcardSearch;
    DWORD_PTR srgKeystrokeBufLen = searchString.GetLength() + 2;
    PWCHAR pwch = new (std::nothrow) WCHAR[ srgKeystrokeBufLen ];
    if (!pwch)
    {
        return;
    }

    StringCchCopyN(pwch, srgKeystrokeBufLen, searchString.Get(), searchString.GetLength());
    StringCchCat(pwch, srgKeystrokeBufLen, L"*");

    // add wildcard char
    size_t len = 0;
    if (StringCchLength(pwch, STRSAFE_MAX_CCH, &len) != S_OK)
    {
        return;
    }
    wildcardSearch.Set(pwch, len);

    _pTableDictionaryEngine->CollectWordFromConvertedStringForWildcard(&wildcardSearch, pCandidateList);

    if (IsKeystrokeSort())
    {
        _pTableDictionaryEngine->SortListItemByFindKeyCode(pCandidateList);
    }

    wildcardSearch.Clear();
    delete [] pwch;
}

//+---------------------------------------------------------------------------
//
// IsPunctuation
//
//----------------------------------------------------------------------------

BOOL CompositionProcessorEngine::IsPunctuation(WCHAR wch)
{
    for (int i = 0; i < ARRAYSIZE(Global::PunctuationTable); i++)
    {
        if (Global::PunctuationTable[i]._Code == wch)
        {
            return TRUE;
        }
    }

    for (UINT j = 0; j < _PunctuationPair.size(); j++)
    {
        CPunctuationPair* pPuncPair = &_PunctuationPair.at(j);

        if (pPuncPair->_punctuation._Code == wch)
        {
            return TRUE;
        }
    }

    for (UINT k = 0; k < _PunctuationNestPair.size(); k++)
    {
        CPunctuationNestPair* pPuncNestPair = &_PunctuationNestPair.at(k);

        if (pPuncNestPair->_punctuation_begin._Code == wch)
        {
            return TRUE;
        }
        if (pPuncNestPair->_punctuation_end._Code == wch)
        {
            return TRUE;
        }
    }
    return FALSE;
}

//+---------------------------------------------------------------------------
//
// GetPunctuationPair
//
//----------------------------------------------------------------------------

WCHAR CompositionProcessorEngine::GetPunctuation(WCHAR wch)
{
    for (int i = 0; i < ARRAYSIZE(Global::PunctuationTable); i++)
    {
        if (Global::PunctuationTable[i]._Code == wch)
        {
            return Global::PunctuationTable[i]._Punctuation;
        }
    }

    for (UINT j = 0; j < _PunctuationPair.size(); j++)
    {
        CPunctuationPair* pPuncPair = &_PunctuationPair.at(j);

        if (pPuncPair->_punctuation._Code == wch)
        {
            if (! pPuncPair->_isPairToggle)
            {
                pPuncPair->_isPairToggle = TRUE;
                return pPuncPair->_punctuation._Punctuation;
            }
            else
            {
                pPuncPair->_isPairToggle = FALSE;
                return pPuncPair->_pairPunctuation;
            }
        }
    }

    for (UINT k = 0; k < _PunctuationNestPair.size(); k++)
    {
        CPunctuationNestPair* pPuncNestPair = &_PunctuationNestPair.at(k);

        if (pPuncNestPair->_punctuation_begin._Code == wch)
        {
            if (pPuncNestPair->_nestCount++ == 0)
            {
                return pPuncNestPair->_punctuation_begin._Punctuation;
            }
            else
            {
                return pPuncNestPair->_pairPunctuation_begin;
            }
        }
        if (pPuncNestPair->_punctuation_end._Code == wch)
        {
            if (--pPuncNestPair->_nestCount == 0)
            {
                return pPuncNestPair->_punctuation_end._Punctuation;
            }
            else
            {
                return pPuncNestPair->_pairPunctuation_end;
            }
        }
    }
    return 0;
}

//+---------------------------------------------------------------------------
//
// IsDoubleSingleByte
//
//----------------------------------------------------------------------------

BOOL CompositionProcessorEngine::IsDoubleSingleByte(WCHAR wch)
{
    if (L' ' <= wch && wch <= L'~')
    {
        return TRUE;
    }
    return FALSE;
}

//+---------------------------------------------------------------------------
//
// SetupKeystroke
//
//----------------------------------------------------------------------------

void CompositionProcessorEngine::SetupKeystroke()
{
    SetKeystrokeTable(&_KeystrokeComposition);
    return;
}

//+---------------------------------------------------------------------------
//
// SetKeystrokeTable
//
//----------------------------------------------------------------------------

void CompositionProcessorEngine::SetKeystrokeTable(_Inout_ std::vector<_KEYSTROKE> *pKeystroke)
{
    for (int i = 0; i < 26; i++)
    {
        _KEYSTROKE* pKS = nullptr;

        pKeystroke->push_back(_KEYSTROKE());
        pKS = &pKeystroke->back();
        if (!pKS)
        {
            break;
        }
        *pKS = _keystrokeTable[i];
    }
}

// //+---------------------------------------------------------------------------
// //
// // SetupPreserved
// //
// //----------------------------------------------------------------------------
// 
// void CompositionProcessorEngine::SetupPreserved(_In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId)
// {
//     TF_PRESERVEDKEY preservedKeyImeMode = {};
//     preservedKeyImeMode.uVKey = VK_SHIFT;
//     preservedKeyImeMode.uModifiers = _TF_MOD_ON_KEYUP_SHIFT_ONLY;
//     SetPreservedKey(Global::SampleIMEGuidImeModePreserveKey, preservedKeyImeMode, Global::ImeModeDescription, &_PreservedKey_IMEMode);
// 
//     TF_PRESERVEDKEY preservedKeyDoubleSingleByte = {};
//     preservedKeyDoubleSingleByte.uVKey = VK_SPACE;
//     preservedKeyDoubleSingleByte.uModifiers = TF_MOD_SHIFT;
//     SetPreservedKey(Global::SampleIMEGuidDoubleSingleBytePreserveKey, preservedKeyDoubleSingleByte, Global::DoubleSingleByteDescription, &_PreservedKey_DoubleSingleByte);
// 
//     TF_PRESERVEDKEY preservedKeyPunctuation = {};
//     preservedKeyPunctuation.uVKey = VK_OEM_PERIOD;
//     preservedKeyPunctuation.uModifiers = TF_MOD_CONTROL;
//     SetPreservedKey(Global::SampleIMEGuidPunctuationPreserveKey, preservedKeyPunctuation, Global::PunctuationDescription, &_PreservedKey_Punctuation);
// 
//     InitPreservedKey(&_PreservedKey_IMEMode, pThreadMgr, tfClientId);
//     InitPreservedKey(&_PreservedKey_DoubleSingleByte, pThreadMgr, tfClientId);
//     InitPreservedKey(&_PreservedKey_Punctuation, pThreadMgr, tfClientId);
// 
//     return;
// }
// 
// //+---------------------------------------------------------------------------
// //
// // SetKeystrokeTable
// //
// //----------------------------------------------------------------------------
// 
// void CompositionProcessorEngine::SetPreservedKey(const CLSID clsid, TF_PRESERVEDKEY & tfPreservedKey, _In_z_ LPCWSTR pwszDescription, _Out_ XPreservedKey *pXPreservedKey)
// {
//     pXPreservedKey->Guid = clsid;
// 
//     pXPreservedKey->TSFPreservedKeyTable.push_back(TF_PRESERVEDKEY());
//     TF_PRESERVEDKEY *ptfPsvKey1 = &pXPreservedKey->TSFPreservedKeyTable.back();
//     if (!ptfPsvKey1)
//     {
//         return;
//     }
//     *ptfPsvKey1 = tfPreservedKey;
// 
//     size_t srgKeystrokeBufLen = 0;
//     if (StringCchLength(pwszDescription, STRSAFE_MAX_CCH, &srgKeystrokeBufLen) != S_OK)
//     {
//         return;
//     }
//     pXPreservedKey->Description = new (std::nothrow) WCHAR[srgKeystrokeBufLen + 1];
//     if (!pXPreservedKey->Description)
//     {
//         return;
//     }
// 
//     StringCchCopy((LPWSTR)pXPreservedKey->Description, srgKeystrokeBufLen, pwszDescription);
// 
//     return;
// }
// 
// //+---------------------------------------------------------------------------
// //
// // InitPreservedKey
// //
// // Register a hot key.
// //
// //----------------------------------------------------------------------------
// 
// BOOL CompositionProcessorEngine::InitPreservedKey(_In_ XPreservedKey *pXPreservedKey, _In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId)
// {
//     ITfKeystrokeMgr *pKeystrokeMgr = nullptr;
// 
//     if (IsEqualGUID(pXPreservedKey->Guid, GUID_NULL))
//     {
//         return FALSE;
//     }
// 
//     if (pThreadMgr->QueryInterface(IID_ITfKeystrokeMgr, (void **)&pKeystrokeMgr) != S_OK)
//     {
//         return FALSE;
//     }
// 
//     for (UINT i = 0; i < pXPreservedKey->TSFPreservedKeyTable.size(); i++)
//     {
//         TF_PRESERVEDKEY preservedKey = pXPreservedKey->TSFPreservedKeyTable.at(i);
//         preservedKey.uModifiers &= 0xffff;
// 
//         size_t lenOfDesc = 0;
//         if (StringCchLength(pXPreservedKey->Description, STRSAFE_MAX_CCH, &lenOfDesc) != S_OK)
//         {
//             return FALSE;
//         }
//         pKeystrokeMgr->PreserveKey(tfClientId, pXPreservedKey->Guid, &preservedKey, pXPreservedKey->Description, static_cast<ULONG>(lenOfDesc));
//     }
// 
//     pKeystrokeMgr->Release();
// 
//     return TRUE;
// }
// 
// //+---------------------------------------------------------------------------
// //
// // CheckShiftKeyOnly
// //
// //----------------------------------------------------------------------------
// 
// BOOL CompositionProcessorEngine::CheckShiftKeyOnly(_In_ std::vector<TF_PRESERVEDKEY> *pTSFPreservedKeyTable)
// {
//     for (UINT i = 0; i < pTSFPreservedKeyTable->size(); i++)
//     {
//         TF_PRESERVEDKEY *ptfPsvKey = &pTSFPreservedKeyTable->at(i);
// 
//         if (((ptfPsvKey->uModifiers & (_TF_MOD_ON_KEYUP_SHIFT_ONLY & 0xffff0000)) && !WindowsImeLib::IsShiftKeyDownOnly) ||
//             ((ptfPsvKey->uModifiers & (_TF_MOD_ON_KEYUP_CONTROL_ONLY & 0xffff0000)) && !WindowsImeLib::IsControlKeyDownOnly) ||
//             ((ptfPsvKey->uModifiers & (_TF_MOD_ON_KEYUP_ALT_ONLY & 0xffff0000)) && !WindowsImeLib::IsAltKeyDownOnly)         )
//         {
//             return FALSE;
//         }
//     }
// 
//     return TRUE;
// }
// 
// //+---------------------------------------------------------------------------
// //
// // OnPreservedKey
// //
// //----------------------------------------------------------------------------
// 
// void CompositionProcessorEngine::OnPreservedKey(REFGUID rguid, _Out_ BOOL *pIsEaten, _In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId)
// {
//     *pIsEaten = FALSE;
//     const auto owner = m_owner.lock();
//     if (!owner)
//     {
//         return;
//     }
// 
//     if (IsEqualGUID(rguid, _PreservedKey_IMEMode.Guid))
//     {
//         if (!CheckShiftKeyOnly(&_PreservedKey_IMEMode.TSFPreservedKeyTable))
//         {
//             *pIsEaten = FALSE;
//             return;
//         }
//         BOOL isOpen = owner->GetCompartmentBool(pThreadMgr, tfClientId, GUID_COMPARTMENT_KEYBOARD_OPENCLOSE);
//         owner->SetCompartmentBool(pThreadMgr, tfClientId, GUID_COMPARTMENT_KEYBOARD_OPENCLOSE, isOpen ? FALSE : TRUE);
//         *pIsEaten = TRUE;
//     }
//     else if (IsEqualGUID(rguid, _PreservedKey_DoubleSingleByte.Guid))
//     {
//         if (!CheckShiftKeyOnly(&_PreservedKey_DoubleSingleByte.TSFPreservedKeyTable))
//         {
//             *pIsEaten = FALSE;
//             return;
//         }
//         BOOL isDouble = owner->GetCompartmentBool(pThreadMgr, tfClientId, Global::SampleIMEGuidCompartmentDoubleSingleByte);
//         owner->SetCompartmentBool(pThreadMgr, tfClientId, Global::SampleIMEGuidCompartmentDoubleSingleByte, isDouble ? FALSE : TRUE);
//         *pIsEaten = TRUE;
//     }
//     else if (IsEqualGUID(rguid, _PreservedKey_Punctuation.Guid))
//     {
//         if (!CheckShiftKeyOnly(&_PreservedKey_Punctuation.TSFPreservedKeyTable))
//         {
//             *pIsEaten = FALSE;
//             return;
//         }
//         BOOL isPunctuation = owner->GetCompartmentBool(pThreadMgr, tfClientId, Global::SampleIMEGuidCompartmentPunctuation);
//         owner->SetCompartmentBool(pThreadMgr, tfClientId, Global::SampleIMEGuidCompartmentPunctuation, isPunctuation ? FALSE : TRUE);
//         *pIsEaten = TRUE;
//     }
//     else
//     {
//         *pIsEaten = FALSE;
//     }
//     *pIsEaten = TRUE;
// }

//+---------------------------------------------------------------------------
//
// SetupConfiguration
//
//----------------------------------------------------------------------------

void CompositionProcessorEngine::SetupConfiguration()
{
    _isWildcard = TRUE;
    _isDisableWildcardAtFirst = TRUE;
    _hasMakePhraseFromText = TRUE;
    _isKeystrokeSort = TRUE;
    _candidateWndWidth = CAND_WIDTH;

    SetInitialCandidateListRange();

    SetDefaultCandidateTextFont();

    return;
}

// //+---------------------------------------------------------------------------
// //
// // SetupLanguageBar
// //
// //----------------------------------------------------------------------------
// 
// void CompositionProcessorEngine::SetupLanguageBar(_In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId, BOOL isSecureMode)
// {
//     static const WindowsImeLib::LanguageBarButtonProperty buttonProperties[] = {
//         {
//             GUID_LBI_INPUTMODE,
//             GUID_COMPARTMENT_KEYBOARD_OPENCLOSE,
//             Global::LangbarImeModeDescription,
//             Global::ImeModeDescription,
//             Global::ImeModeOnIcoIndex,
//             Global::ImeModeOffIcoIndex,
//         },
//         {
//             Global::SampleIMEGuidLangBarDoubleSingleByte,
//             Global::SampleIMEGuidCompartmentDoubleSingleByte,
//             Global::LangbarDoubleSingleByteDescription,
//             Global::DoubleSingleByteDescription,
//             Global::DoubleSingleByteOnIcoIndex,
//             Global::DoubleSingleByteOffIcoIndex,
//         },
//         {
//             Global::SampleIMEGuidLangBarPunctuation,
//             Global::SampleIMEGuidCompartmentPunctuation,
//             Global::LangbarPunctuationDescription,
//             Global::PunctuationDescription,
//             Global::PunctuationOnIcoIndex,
//             Global::PunctuationOffIcoIndex,
//         }
//         
//     };
// 
//     if (const auto owner = m_owner.lock())
//     {
//         owner->SetupLanguageBar(pThreadMgr, tfClientId, isSecureMode, buttonProperties, ARRAYSIZE(buttonProperties));
//     }
// 
//    DWORD dwEnable = 1;
//    CreateLanguageBarButton(dwEnable, GUID_LBI_INPUTMODE, Global::LangbarImeModeDescription, Global::ImeModeDescription, Global::ImeModeOnIcoIndex, Global::ImeModeOffIcoIndex, &_pLanguageBar_IMEMode, isSecureMode);
//    CreateLanguageBarButton(dwEnable, Global::SampleIMEGuidLangBarDoubleSingleByte, Global::LangbarDoubleSingleByteDescription, Global::DoubleSingleByteDescription, Global::DoubleSingleByteOnIcoIndex, Global::DoubleSingleByteOffIcoIndex, &_pLanguageBar_DoubleSingleByte, isSecureMode);
//    CreateLanguageBarButton(dwEnable, Global::SampleIMEGuidLangBarPunctuation, Global::LangbarPunctuationDescription, Global::PunctuationDescription, Global::PunctuationOnIcoIndex, Global::PunctuationOffIcoIndex, &_pLanguageBar_Punctuation, isSecureMode);
//
//    InitLanguageBar(_pLanguageBar_IMEMode, pThreadMgr, tfClientId, GUID_COMPARTMENT_KEYBOARD_OPENCLOSE);
//    InitLanguageBar(_pLanguageBar_DoubleSingleByte, pThreadMgr, tfClientId, Global::SampleIMEGuidCompartmentDoubleSingleByte);
//    InitLanguageBar(_pLanguageBar_Punctuation, pThreadMgr, tfClientId, Global::SampleIMEGuidCompartmentPunctuation);
//
//    _pCompartmentConversion = new (std::nothrow) CCompartment(pThreadMgr, tfClientId, GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION);
//    _pCompartmentKeyboardOpenEventSink = new (std::nothrow) CCompartmentEventSink(CompartmentCallback, this);
//    _pCompartmentConversionEventSink = new (std::nothrow) CCompartmentEventSink(CompartmentCallback, this);
//    _pCompartmentDoubleSingleByteEventSink = new (std::nothrow) CCompartmentEventSink(CompartmentCallback, this);
//    _pCompartmentPunctuationEventSink = new (std::nothrow) CCompartmentEventSink(CompartmentCallback, this);
//
//    if (_pCompartmentKeyboardOpenEventSink)
//    {
//        _pCompartmentKeyboardOpenEventSink->_Advise(pThreadMgr, GUID_COMPARTMENT_KEYBOARD_OPENCLOSE);
//    }
//    if (_pCompartmentConversionEventSink)
//    {
//        _pCompartmentConversionEventSink->_Advise(pThreadMgr, GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION);
//    }
//    if (_pCompartmentDoubleSingleByteEventSink)
//    {
//        _pCompartmentDoubleSingleByteEventSink->_Advise(pThreadMgr, Global::SampleIMEGuidCompartmentDoubleSingleByte);
//    }
//    if (_pCompartmentPunctuationEventSink)
//    {
//        _pCompartmentPunctuationEventSink->_Advise(pThreadMgr, Global::SampleIMEGuidCompartmentPunctuation);
//    }
//
//    return;
// }

//+---------------------------------------------------------------------------
//
// CreateLanguageBarButton
//
//----------------------------------------------------------------------------

//void CompositionProcessorEngine::CreateLanguageBarButton(DWORD dwEnable, GUID guidLangBar, _In_z_ LPCWSTR pwszDescriptionValue, _In_z_ LPCWSTR pwszTooltipValue, DWORD dwOnIconIndex, DWORD dwOffIconIndex, _Outptr_result_maybenull_ CLangBarItemButton **ppLangBarItemButton, BOOL isSecureMode)
//{
//    dwEnable;
//
//    if (ppLangBarItemButton)
//    {
//        *ppLangBarItemButton = new (std::nothrow) CLangBarItemButton(guidLangBar, pwszDescriptionValue, pwszTooltipValue, dwOnIconIndex, dwOffIconIndex, isSecureMode);
//    }
//
//    return;
//}

//+---------------------------------------------------------------------------
//
// InitLanguageBar
//
//----------------------------------------------------------------------------

//BOOL CompositionProcessorEngine::InitLanguageBar(_In_ CLangBarItemButton *pLangBarItemButton, _In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId, REFGUID guidCompartment)
//{
//    if (pLangBarItemButton)
//    {
//        if (pLangBarItemButton->_AddItem(pThreadMgr) == S_OK)
//        {
//            if (pLangBarItemButton->_RegisterCompartment(pThreadMgr, tfClientId, guidCompartment))
//            {
//                return TRUE;
//            }
//        }
//    }
//    return FALSE;
//}

//+---------------------------------------------------------------------------
//
// SetupDictionaryFile
//
//----------------------------------------------------------------------------

HMODULE GetThisModuleHandle()
{
    static HMODULE s_module = ([]() -> HMODULE
        {
            HMODULE module;
            LOG_IF_WIN32_BOOL_FALSE(GetModuleHandleEx(
                GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                reinterpret_cast<LPCWSTR>(GetThisModuleHandle),
                &module));
            return module;
        })();
    return s_module;
}


BOOL CompositionProcessorEngine::SetupDictionaryFile()
{   
    // Not yet registered
    // Register CFileMapping
    WCHAR wszFileName[MAX_PATH] = {'\0'};
    DWORD cchA = GetModuleFileName(GetThisModuleHandle(), wszFileName, ARRAYSIZE(wszFileName));
    size_t iDicFileNameLen = cchA + wcslen(TEXTSERVICE_DIC);
    WCHAR *pwszFileName = new (std::nothrow) WCHAR[iDicFileNameLen + 1];
    if (!pwszFileName)
    {
        goto ErrorExit;
    }
    *pwszFileName = L'\0';

    // find the last '/'
    while (cchA--)
    {
        WCHAR wszChar = wszFileName[cchA];
        if (wszChar == '\\' || wszChar == '/')
        {
            StringCchCopyN(pwszFileName, iDicFileNameLen + 1, wszFileName, static_cast<size_t>(cchA) + 1);
            StringCchCatN(pwszFileName, iDicFileNameLen + 1, TEXTSERVICE_DIC, wcslen(TEXTSERVICE_DIC));
            break;
        }
    }

    // create CFileMapping object
    if (_pDictionaryFile == nullptr)
    {
        _pDictionaryFile = new (std::nothrow) CFileMapping();
        if (!_pDictionaryFile)
        {
            goto ErrorExit;
        }
    }
    if (!(_pDictionaryFile)->CreateFile(pwszFileName, GENERIC_READ, OPEN_EXISTING, FILE_SHARE_READ))
    {
        goto ErrorExit;
    }

    _pTableDictionaryEngine = new (std::nothrow) CTableDictionaryEngine(
        WindowsImeLib::g_processorFactory->GetConstantProvider()->GetLocale(), _pDictionaryFile);

    if (!_pTableDictionaryEngine)
    {
        goto ErrorExit;
    }

    delete []pwszFileName;
    return TRUE;
ErrorExit:
    if (pwszFileName)
    {
        delete []pwszFileName;
    }
    return FALSE;
}

//+---------------------------------------------------------------------------
//
// GetDictionaryFile
//
//----------------------------------------------------------------------------

CFile* CompositionProcessorEngine::GetDictionaryFile()
{
    return _pDictionaryFile;
}

//+---------------------------------------------------------------------------
//
// SetupPunctuationPair
//
//----------------------------------------------------------------------------

void CompositionProcessorEngine::SetupPunctuationPair()
{
    // Punctuation pair
    const int pair_count = 2;
    CPunctuationPair punc_quotation_mark(L'"', 0x201C, 0x201D);
    CPunctuationPair punc_apostrophe(L'\'', 0x2018, 0x2019);

    CPunctuationPair puncPairs[pair_count] = {
        punc_quotation_mark,
        punc_apostrophe,
    };

    for (int i = 0; i < pair_count; ++i)
    {
        _PunctuationPair.push_back(CPunctuationPair());
        CPunctuationPair *pPuncPair = &_PunctuationPair.back();
        *pPuncPair = puncPairs[i];
    }

    // Punctuation nest pair
    CPunctuationNestPair punc_angle_bracket(L'<', 0x300A, 0x3008, L'>', 0x300B, 0x3009);

    _PunctuationNestPair.push_back(CPunctuationNestPair());
    CPunctuationNestPair* pPuncNestPair = &_PunctuationNestPair.back();
    *pPuncNestPair = punc_angle_bracket;
}

// void CompositionProcessorEngine::InitializeSampleIMECompartment(_In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId)
// {
//     const auto owner = m_owner.lock();
//     if (owner)
//     {
//         owner->SetCompartmentBool(pThreadMgr, tfClientId, GUID_COMPARTMENT_KEYBOARD_OPENCLOSE, TRUE);
//         owner->SetCompartmentBool(pThreadMgr, tfClientId, Global::SampleIMEGuidCompartmentDoubleSingleByte, FALSE);
//         owner->SetCompartmentBool(pThreadMgr, tfClientId, Global::SampleIMEGuidCompartmentPunctuation, TRUE);
//     }
// 
//     PrivateCompartmentsUpdated(pThreadMgr);
// }
//
// //+---------------------------------------------------------------------------
// //
// // CompartmentCallback
// //
// //----------------------------------------------------------------------------
// 
// HRESULT CompositionProcessorEngine::CompartmentCallback(REFGUID guidCompartment) noexcept try
// {
//     ITfThreadMgr* pThreadMgr = nullptr;
//     HRESULT hr = CoCreateInstance(CLSID_TF_ThreadMgr, nullptr, CLSCTX_INPROC_SERVER, IID_ITfThreadMgr, (void**)&pThreadMgr);
//     if (FAILED(hr))
//     {
//         return E_FAIL;
//     }
// 
//     if (IsEqualGUID(guidCompartment, Global::SampleIMEGuidCompartmentDoubleSingleByte) ||
//         IsEqualGUID(guidCompartment, Global::SampleIMEGuidCompartmentPunctuation))
//     {
//         PrivateCompartmentsUpdated(pThreadMgr);
//     }
//     else if (IsEqualGUID(guidCompartment, GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION) ||
//         IsEqualGUID(guidCompartment, GUID_COMPARTMENT_KEYBOARD_INPUTMODE_SENTENCE))
//     {
//         ConversionModeCompartmentUpdated(pThreadMgr);
//     }
//     else if (IsEqualGUID(guidCompartment, GUID_COMPARTMENT_KEYBOARD_OPENCLOSE))
//     {
//         KeyboardOpenCompartmentUpdated(pThreadMgr);
//     }
// 
//     pThreadMgr->Release();
//     pThreadMgr = nullptr;
// 
//     return S_OK;
// }
// CATCH_RETURN()

// //+---------------------------------------------------------------------------
// //
// // UpdatePrivateCompartments
// //
// //----------------------------------------------------------------------------
// 
// void CompositionProcessorEngine::ConversionModeCompartmentUpdated(_In_ ITfThreadMgr* /*pThreadMgr*/)
// {
//     const auto owner = m_owner.lock();
//     if (!owner)
//     {
//         return;
//     }
// 
// //     DWORD conversionMode = owner->GetCompartmentDword(pThreadMgr, _tfClientId, GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION);
// // 
// //     BOOL isDouble = owner->GetCompartmentBool(pThreadMgr, _tfClientId, Global::SampleIMEGuidCompartmentDoubleSingleByte);
// // 
// //     if (!isDouble && (conversionMode & TF_CONVERSIONMODE_FULLSHAPE))
// //     {
// //         owner->SetCompartmentBool(pThreadMgr, _tfClientId, Global::SampleIMEGuidCompartmentDoubleSingleByte, TRUE);
// //     }
// //     else if (isDouble && !(conversionMode & TF_CONVERSIONMODE_FULLSHAPE))
// //     {
// //         owner->SetCompartmentBool(pThreadMgr, _tfClientId, Global::SampleIMEGuidCompartmentDoubleSingleByte, FALSE);
// //     }
// // 
// //     BOOL isPunctuation = owner->GetCompartmentBool(pThreadMgr, _tfClientId, Global::SampleIMEGuidCompartmentPunctuation);
// // 
// //     if (!isPunctuation && (conversionMode & TF_CONVERSIONMODE_SYMBOL))
// //     {
// //         owner->SetCompartmentBool(pThreadMgr, _tfClientId, Global::SampleIMEGuidCompartmentPunctuation, TRUE);
// //     }
// //     else if (isPunctuation && !(conversionMode & TF_CONVERSIONMODE_SYMBOL))
// //     {
// //         owner->SetCompartmentBool(pThreadMgr, _tfClientId, Global::SampleIMEGuidCompartmentPunctuation, FALSE);
// //     }
// // 
// //     BOOL fOpen = owner->GetCompartmentBool(pThreadMgr, _tfClientId, GUID_COMPARTMENT_KEYBOARD_OPENCLOSE);
// // 
// //     if (fOpen && !(conversionMode & TF_CONVERSIONMODE_NATIVE))
// //     {
// //         owner->SetCompartmentBool(pThreadMgr, _tfClientId, GUID_COMPARTMENT_KEYBOARD_OPENCLOSE, FALSE);
// //     }
// //     else if (!fOpen && (conversionMode & TF_CONVERSIONMODE_NATIVE))
// //     {
// //         owner->SetCompartmentBool(pThreadMgr, _tfClientId, GUID_COMPARTMENT_KEYBOARD_OPENCLOSE, TRUE);
// //     }
// }
// 
// //+---------------------------------------------------------------------------
// //
// // PrivateCompartmentsUpdated()
// //
// //----------------------------------------------------------------------------
// 
// void CompositionProcessorEngine::PrivateCompartmentsUpdated(_In_ ITfThreadMgr* /*pThreadMgr*/)
// {
//    const auto owner = m_owner.lock();
//     if (!owner)
//     {
//         return;
//     }
// 
//     DWORD conversionMode = owner->GetCompartmentDword(pThreadMgr, _tfClientId, GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION);
//     DWORD conversionModePrev = conversionMode;
// 
//     BOOL isDouble = owner->GetCompartmentBool(pThreadMgr, _tfClientId, Global::SampleIMEGuidCompartmentDoubleSingleByte);
//     if (!isDouble && (conversionMode & TF_CONVERSIONMODE_FULLSHAPE))
//     {
//         conversionMode &= ~TF_CONVERSIONMODE_FULLSHAPE;
//     }
//     else if (isDouble && !(conversionMode & TF_CONVERSIONMODE_FULLSHAPE))
//     {
//         conversionMode |= TF_CONVERSIONMODE_FULLSHAPE;
//     }
// 
//     BOOL isPunctuation = owner->GetCompartmentBool(pThreadMgr, _tfClientId, Global::SampleIMEGuidCompartmentPunctuation);
//     if (!isPunctuation && (conversionMode & TF_CONVERSIONMODE_SYMBOL))
//     {
//         conversionMode &= ~TF_CONVERSIONMODE_SYMBOL;
//     }
//     else if (isPunctuation && !(conversionMode & TF_CONVERSIONMODE_SYMBOL))
//     {
//         conversionMode |= TF_CONVERSIONMODE_SYMBOL;
//     }
// 
//     if (conversionMode != conversionModePrev)
//     {
//         owner->SetCompartmentDword(pThreadMgr, _tfClientId, GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION, conversionMode);
//     }
// }

//+---------------------------------------------------------------------------
//
// KeyboardOpenCompartmentUpdated
//
//----------------------------------------------------------------------------

// void CompositionProcessorEngine::KeyboardOpenCompartmentUpdated(_In_ ITfThreadMgr* /*pThreadMgr*/)
// {
//     const auto owner = m_owner.lock();
//     if (!owner)
//     {
//         return;
//     }
// 
// //     DWORD conversionMode = owner->GetCompartmentDword(pThreadMgr, _tfClientId, GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION);
// //     DWORD conversionModePrev = conversionMode;
// // 
// //     BOOL isOpen = owner->GetCompartmentBool(pThreadMgr, _tfClientId, GUID_COMPARTMENT_KEYBOARD_OPENCLOSE);
// // 
// //     if (isOpen && !(conversionMode & TF_CONVERSIONMODE_NATIVE))
// //     {
// //         conversionMode |= TF_CONVERSIONMODE_NATIVE;
// //     }
// //     else if (!isOpen && (conversionMode & TF_CONVERSIONMODE_NATIVE))
// //     {
// //         conversionMode &= ~TF_CONVERSIONMODE_NATIVE;
// //     }
// // 
// //     if (conversionMode != conversionModePrev)
// //     {
// //         owner->SetCompartmentDword(pThreadMgr, _tfClientId, GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION, conversionMode);
// //     }
// }
// 
// 
// //////////////////////////////////////////////////////////////////////
// //
// // XPreservedKey implementation.
// //
// //////////////////////////////////////////////////////////////////////
// 
// //+---------------------------------------------------------------------------
// //
// // UninitPreservedKey
// //
// //----------------------------------------------------------------------------
// 
// BOOL CompositionProcessorEngine::XPreservedKey::UninitPreservedKey(_In_ ITfThreadMgr *pThreadMgr)
// {
//     ITfKeystrokeMgr* pKeystrokeMgr = nullptr;
// 
//     if (IsEqualGUID(Guid, GUID_NULL))
//     {
//         return FALSE;
//     }
// 
//     if (FAILED(pThreadMgr->QueryInterface(IID_ITfKeystrokeMgr, (void **)&pKeystrokeMgr)))
//     {
//         return FALSE;
//     }
// 
//     for (UINT i = 0; i < TSFPreservedKeyTable.size(); i++)
//     {
//         TF_PRESERVEDKEY pPreservedKey = TSFPreservedKeyTable.at(i);
//         pPreservedKey.uModifiers &= 0xffff;
// 
//         pKeystrokeMgr->UnpreserveKey(Guid, &pPreservedKey);
//     }
// 
//     pKeystrokeMgr->Release();
// 
//     return TRUE;
// }
// 
// CompositionProcessorEngine::XPreservedKey::XPreservedKey()
// {
//     Guid = GUID_NULL;
//     Description = nullptr;
// }
// 
// CompositionProcessorEngine::XPreservedKey::~XPreservedKey()
// {
//     ITfThreadMgr* pThreadMgr = nullptr;
// 
//     HRESULT hr = CoCreateInstance(CLSID_TF_ThreadMgr, NULL, CLSCTX_INPROC_SERVER, IID_ITfThreadMgr, (void**)&pThreadMgr);
//     if (SUCCEEDED(hr))
//     {
//         UninitPreservedKey(pThreadMgr);
//         pThreadMgr->Release();
//         pThreadMgr = nullptr;
//     }
// 
//     if (Description)
//     {
//         delete [] Description;
//     }
// }

void CompositionProcessorEngine::InitKeyStrokeTable()
{
    for (int i = 0; i < 26; i++)
    {
        _keystrokeTable[i].VirtualKey = 'A' + i;
        _keystrokeTable[i].Modifiers = 0;
        _keystrokeTable[i].Function = FUNCTION_INPUT;
    }
}

void CompositionProcessorEngine::SetInitialCandidateListRange()
{
    for (DWORD i = 1; i <= 10; i++)
    {
        _candidateListIndexRange.push_back(0);
        DWORD* pNewIndexRange = &_candidateListIndexRange.back();

        if (pNewIndexRange != nullptr)
        {
            if (i != 10)
            {
                *pNewIndexRange = i;
            }
            else
            {
                *pNewIndexRange = 0;
            }
        }
    }
}

void CompositionProcessorEngine::SetDefaultCandidateTextFont()
{
    if (const auto owner = m_owner.lock())
    {
        owner->SetDefaultCandidateTextFont(IDS_DEFAULT_FONT);
    }

//    // Candidate Text Font
//    if (WindowsImeLib::defaultlFontHandle == nullptr)
//    {
//        WCHAR fontName[50] = {'\0'}; 
//        LoadString(WindowsImeLib::dllInstanceHandle, IDS_DEFAULT_FONT, fontName, 50);
//        WindowsImeLib::defaultlFontHandle = CreateFont(-MulDiv(10, GetDeviceCaps(GetDC(NULL), LOGPIXELSY), 72), 0, 0, 0, FW_MEDIUM, 0, 0, 0, 0, 0, 0, 0, 0, fontName);
//        if (!WindowsImeLib::defaultlFontHandle)
//        {
//            LOGFONT lf = {};
//            SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &lf, 0);
//            // Fall back to the default GUI font on failure.
//            WindowsImeLib::defaultlFontHandle = CreateFont(-MulDiv(10, GetDeviceCaps(GetDC(NULL), LOGPIXELSY), 72), 0, 0, 0, FW_MEDIUM, 0, 0, 0, 0, 0, 0, 0, 0, lf.lfFaceName);
//        }
//    }
}

//////////////////////////////////////////////////////////////////////
//
//    CompositionProcessorEngine
//
//////////////////////////////////////////////////////////////////////

BOOL CompositionProcessorEngine::IsKeyEaten(
    _In_ ITfThreadMgr* /*pThreadMgr*/, TfClientId, UINT code, _Inout_updates_(1) WCHAR* pwch,
    BOOL isComposing, CANDIDATE_MODE candidateMode, BOOL isCandidateWithWildcard, _Out_opt_ _KEYSTROKE_STATE *pKeyState)
{
    if (pKeyState)
    {
        pKeyState->Category = CATEGORY_NONE;
        pKeyState->Function = FUNCTION_NONE;
    }

    BOOL isOpen = m_compartmentIsOpen;
    BOOL isDoubleSingleByte = m_compartmentIsDoubleSingleByte;
    BOOL isPunctuation = m_compartmentIsPunctuation;

    // if the keyboard is closed, we don't eat keys, with the exception of the touch keyboard specials keys
    if (!isOpen && !isDoubleSingleByte && !isPunctuation)
    {
        *pwch = L'\0';
        return FALSE;
    }

    //
    // Get composition engine
    //

    if (isOpen)
    {
        //
        // The candidate or phrase list handles the keys through ITfKeyEventSink.
        //
        // eat only keys that CKeyHandlerEditSession can handles.
        //
        if (IsVirtualKeyNeed(code, pwch, isComposing, candidateMode, isCandidateWithWildcard, pKeyState))
        {
            return TRUE;
        }
    }

    //
    // Punctuation
    //
    if (IsPunctuation(*pwch))
    {
        if ((candidateMode == CANDIDATE_NONE) && isPunctuation)
        {
            if (pKeyState)
            {
                pKeyState->Category = CATEGORY_COMPOSING;
                pKeyState->Function = FUNCTION_PUNCTUATION;
            }
            return TRUE;
        }
    }

    //
    // Double/Single byte
    //
    if (isDoubleSingleByte && IsDoubleSingleByte(*pwch))
    {
        if (candidateMode == CANDIDATE_NONE)
        {
            if (pKeyState)
            {
                pKeyState->Category = CATEGORY_COMPOSING;
                pKeyState->Function = FUNCTION_DOUBLE_SINGLE_BYTE;
            }
            return TRUE;
        }
    }

    return FALSE;
}

//+---------------------------------------------------------------------------
//
// CompositionProcessorEngine::IsVirtualKeyNeed
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

BOOL CompositionProcessorEngine::IsVirtualKeyNeed(UINT uCode, _In_reads_(1) WCHAR *pwch, BOOL fComposing, CANDIDATE_MODE candidateMode, BOOL hasCandidateWithWildcard, _Out_opt_ _KEYSTROKE_STATE *pKeyState)
{
    if (pKeyState)
    {
        pKeyState->Category = CATEGORY_NONE;
        pKeyState->Function = FUNCTION_NONE;
    }

    if (candidateMode == CANDIDATE_ORIGINAL || candidateMode == CANDIDATE_PHRASE || candidateMode == CANDIDATE_WITH_NEXT_COMPOSITION)
    {
        fComposing = FALSE;
    }

    if (fComposing || candidateMode == CANDIDATE_INCREMENTAL || candidateMode == CANDIDATE_NONE)
    {
        if (IsVirtualKeyKeystrokeComposition(uCode, pKeyState, FUNCTION_NONE))
        {
            return TRUE;
        }
        else if ((IsWildcard() && IsWildcardChar(*pwch) && !IsDisableWildcardAtFirst()) ||
            (IsWildcard() && IsWildcardChar(*pwch) &&  IsDisableWildcardAtFirst() && _keystrokeBuffer.GetLength()))
        {
            if (pKeyState)
            {
                pKeyState->Category = CATEGORY_COMPOSING;
                pKeyState->Function = FUNCTION_INPUT;
            }
            return TRUE;
        }
        else if (_hasWildcardIncludedInKeystrokeBuffer && uCode == VK_SPACE)
        {
            if (pKeyState) { pKeyState->Category = CATEGORY_COMPOSING; pKeyState->Function = FUNCTION_CONVERT_WILDCARD; } return TRUE;
        }
    }

    if (candidateMode == CANDIDATE_ORIGINAL || candidateMode == CANDIDATE_PHRASE || candidateMode == CANDIDATE_WITH_NEXT_COMPOSITION)
    {
        BOOL isRetCode = TRUE;
        if (IsVirtualKeyKeystrokeCandidate(uCode, pKeyState, candidateMode, &isRetCode, &_KeystrokeCandidate))
        {
            return isRetCode;
        }

        if (hasCandidateWithWildcard)
        {
            if (IsVirtualKeyKeystrokeCandidate(uCode, pKeyState, candidateMode, &isRetCode, &_KeystrokeCandidateWildcard))
            {
                return isRetCode;
            }
        }

        // Candidate list could not handle key. We can try to restart the composition.
        if (IsVirtualKeyKeystrokeComposition(uCode, pKeyState, FUNCTION_INPUT))
        {
            if (candidateMode != CANDIDATE_ORIGINAL)
            {
                return TRUE;
            }
            else
            {
                if (pKeyState) { pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_FINALIZE_CANDIDATELIST_AND_INPUT; } 
                return TRUE;
            }
        }
    } 

    // CANDIDATE_INCREMENTAL should process Keystroke.Candidate virtual keys.
    else if (candidateMode == CANDIDATE_INCREMENTAL)
    {
        BOOL isRetCode = TRUE;
        if (IsVirtualKeyKeystrokeCandidate(uCode, pKeyState, candidateMode, &isRetCode, &_KeystrokeCandidate))
        {
            return isRetCode;
        }
    }

    if (!fComposing && candidateMode != CANDIDATE_ORIGINAL && candidateMode != CANDIDATE_PHRASE && candidateMode != CANDIDATE_WITH_NEXT_COMPOSITION) 
    {
        if (IsVirtualKeyKeystrokeComposition(uCode, pKeyState, FUNCTION_INPUT))
        {
            return TRUE;
        }
    }

    // System pre-defined keystroke
    if (fComposing)
    {
        if ((candidateMode != CANDIDATE_INCREMENTAL))
        {
            switch (uCode)
            {
            case VK_LEFT:   if (pKeyState) { pKeyState->Category = CATEGORY_COMPOSING; pKeyState->Function = FUNCTION_MOVE_LEFT; } return TRUE;
            case VK_RIGHT:  if (pKeyState) { pKeyState->Category = CATEGORY_COMPOSING; pKeyState->Function = FUNCTION_MOVE_RIGHT; } return TRUE;
            case VK_RETURN: if (pKeyState) { pKeyState->Category = CATEGORY_COMPOSING; pKeyState->Function = FUNCTION_FINALIZE_CANDIDATELIST; } return TRUE;
            case VK_ESCAPE: if (pKeyState) { pKeyState->Category = CATEGORY_COMPOSING; pKeyState->Function = FUNCTION_CANCEL; } return TRUE;
            case VK_BACK:   if (pKeyState) { pKeyState->Category = CATEGORY_COMPOSING; pKeyState->Function = FUNCTION_BACKSPACE; } return TRUE;

            case VK_UP:     if (pKeyState) { pKeyState->Category = CATEGORY_COMPOSING; pKeyState->Function = FUNCTION_MOVE_UP; } return TRUE;
            case VK_DOWN:   if (pKeyState) { pKeyState->Category = CATEGORY_COMPOSING; pKeyState->Function = FUNCTION_MOVE_DOWN; } return TRUE;
            case VK_PRIOR:  if (pKeyState) { pKeyState->Category = CATEGORY_COMPOSING; pKeyState->Function = FUNCTION_MOVE_PAGE_UP; } return TRUE;
            case VK_NEXT:   if (pKeyState) { pKeyState->Category = CATEGORY_COMPOSING; pKeyState->Function = FUNCTION_MOVE_PAGE_DOWN; } return TRUE;

            case VK_HOME:   if (pKeyState) { pKeyState->Category = CATEGORY_COMPOSING; pKeyState->Function = FUNCTION_MOVE_PAGE_TOP; } return TRUE;
            case VK_END:    if (pKeyState) { pKeyState->Category = CATEGORY_COMPOSING; pKeyState->Function = FUNCTION_MOVE_PAGE_BOTTOM; } return TRUE;

            case VK_SPACE:  if (pKeyState) { pKeyState->Category = CATEGORY_COMPOSING; pKeyState->Function = FUNCTION_CONVERT; } return TRUE;
            }
        }
        else if ((candidateMode == CANDIDATE_INCREMENTAL))
        {
            switch (uCode)
            {
                // VK_LEFT, VK_RIGHT - set *pIsEaten = FALSE for application could move caret left or right.
                // and for CUAS, invoke _HandleCompositionCancel() edit session due to ignore CUAS default key handler for send out terminate composition
            case VK_LEFT:
            case VK_RIGHT:
                {
                    if (pKeyState)
                    {
                        pKeyState->Category = CATEGORY_INVOKE_COMPOSITION_EDIT_SESSION;
                        pKeyState->Function = FUNCTION_CANCEL;
                    }
                }
                return FALSE;

            case VK_RETURN: if (pKeyState) { pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_FINALIZE_CANDIDATELIST; } return TRUE;
            case VK_ESCAPE: if (pKeyState) { pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_CANCEL; } return TRUE;

                // VK_BACK - remove one char from reading string.
            case VK_BACK:   if (pKeyState) { pKeyState->Category = CATEGORY_COMPOSING; pKeyState->Function = FUNCTION_BACKSPACE; } return TRUE;

            case VK_UP:     if (pKeyState) { pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_MOVE_UP; } return TRUE;
            case VK_DOWN:   if (pKeyState) { pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_MOVE_DOWN; } return TRUE;
            case VK_PRIOR:  if (pKeyState) { pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_MOVE_PAGE_UP; } return TRUE;
            case VK_NEXT:   if (pKeyState) { pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_MOVE_PAGE_DOWN; } return TRUE;

            case VK_HOME:   if (pKeyState) { pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_MOVE_PAGE_TOP; } return TRUE;
            case VK_END:    if (pKeyState) { pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_MOVE_PAGE_BOTTOM; } return TRUE;

            case VK_SPACE:
                {
                    if (candidateMode == CANDIDATE_INCREMENTAL)
                    {
                        if (pKeyState) { pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_CONVERT; } return TRUE;
                    }
                    else
                    {
                        if (pKeyState) { pKeyState->Category = CATEGORY_COMPOSING; pKeyState->Function = FUNCTION_CONVERT; } return TRUE;
                    }
                }
            }
        }
    }

    if ((candidateMode == CANDIDATE_ORIGINAL) || (candidateMode == CANDIDATE_WITH_NEXT_COMPOSITION))
    {
        switch (uCode)
        {
        case VK_UP:     if (pKeyState) { pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_MOVE_UP; } return TRUE;
        case VK_DOWN:   if (pKeyState) { pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_MOVE_DOWN; } return TRUE;
        case VK_PRIOR:  if (pKeyState) { pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_MOVE_PAGE_UP; } return TRUE;
        case VK_NEXT:   if (pKeyState) { pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_MOVE_PAGE_DOWN; } return TRUE;
        case VK_HOME:   if (pKeyState) { pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_MOVE_PAGE_TOP; } return TRUE;
        case VK_END:    if (pKeyState) { pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_MOVE_PAGE_BOTTOM; } return TRUE;
        case VK_RETURN: if (pKeyState) { pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_FINALIZE_CANDIDATELIST; } return TRUE;
        case VK_SPACE:  if (pKeyState) { pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_CONVERT; } return TRUE;
        case VK_BACK:   if (pKeyState) { pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_CANCEL; } return TRUE;

        case VK_ESCAPE:
            {
                if (candidateMode == CANDIDATE_WITH_NEXT_COMPOSITION)
                {
                    if (pKeyState)
                    {
                        pKeyState->Category = CATEGORY_INVOKE_COMPOSITION_EDIT_SESSION;
                        pKeyState->Function = FUNCTION_FINALIZE_TEXTSTORE;
                    }
                    return TRUE;
                }
                else
                {
                    if (pKeyState)
                    {
                        pKeyState->Category = CATEGORY_CANDIDATE;
                        pKeyState->Function = FUNCTION_CANCEL;
                    }
                    return TRUE;
                }
            }
        }

        if (candidateMode == CANDIDATE_WITH_NEXT_COMPOSITION)
        {
            if (IsVirtualKeyKeystrokeComposition(uCode, NULL, FUNCTION_NONE))
            {
                if (pKeyState) { pKeyState->Category = CATEGORY_COMPOSING; pKeyState->Function = FUNCTION_FINALIZE_TEXTSTORE_AND_INPUT; } return TRUE;
            }
        }
    }

    if (candidateMode == CANDIDATE_PHRASE)
    {
        switch (uCode)
        {
        case VK_UP:     if (pKeyState) { pKeyState->Category = CATEGORY_PHRASE; pKeyState->Function = FUNCTION_MOVE_UP; } return TRUE;
        case VK_DOWN:   if (pKeyState) { pKeyState->Category = CATEGORY_PHRASE; pKeyState->Function = FUNCTION_MOVE_DOWN; } return TRUE;
        case VK_PRIOR:  if (pKeyState) { pKeyState->Category = CATEGORY_PHRASE; pKeyState->Function = FUNCTION_MOVE_PAGE_UP; } return TRUE;
        case VK_NEXT:   if (pKeyState) { pKeyState->Category = CATEGORY_PHRASE; pKeyState->Function = FUNCTION_MOVE_PAGE_DOWN; } return TRUE;
        case VK_HOME:   if (pKeyState) { pKeyState->Category = CATEGORY_PHRASE; pKeyState->Function = FUNCTION_MOVE_PAGE_TOP; } return TRUE;
        case VK_END:    if (pKeyState) { pKeyState->Category = CATEGORY_PHRASE; pKeyState->Function = FUNCTION_MOVE_PAGE_BOTTOM; } return TRUE;
        case VK_RETURN: if (pKeyState) { pKeyState->Category = CATEGORY_PHRASE; pKeyState->Function = FUNCTION_FINALIZE_CANDIDATELIST; } return TRUE;
        case VK_SPACE:  if (pKeyState) { pKeyState->Category = CATEGORY_PHRASE; pKeyState->Function = FUNCTION_CONVERT; } return TRUE;
        case VK_ESCAPE: if (pKeyState) { pKeyState->Category = CATEGORY_PHRASE; pKeyState->Function = FUNCTION_CANCEL; } return TRUE;
        case VK_BACK:   if (pKeyState) { pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_CANCEL; } return TRUE;
        }
    }

    if (IsKeystrokeRange(uCode, pKeyState, candidateMode))
    {
        return TRUE;
    }
    else if (pKeyState && pKeyState->Category != CATEGORY_NONE)
    {
        return FALSE;
    }

    if (*pwch && !IsVirtualKeyKeystrokeComposition(uCode, pKeyState, FUNCTION_NONE))
    {
        if (pKeyState)
        {
            pKeyState->Category = CATEGORY_INVOKE_COMPOSITION_EDIT_SESSION;
            pKeyState->Function = FUNCTION_FINALIZE_TEXTSTORE;
        }
        return FALSE;
    }

    return FALSE;
}

//+---------------------------------------------------------------------------
//
// CompositionProcessorEngine::IsVirtualKeyKeystrokeComposition
//
//----------------------------------------------------------------------------

BOOL CompositionProcessorEngine::IsVirtualKeyKeystrokeComposition(UINT uCode, _Out_opt_ _KEYSTROKE_STATE *pKeyState, KEYSTROKE_FUNCTION function)
{
    if (pKeyState == nullptr)
    {
        return FALSE;
    }

    pKeyState->Category = CATEGORY_NONE;
    pKeyState->Function = FUNCTION_NONE;

    for (UINT i = 0; i < _KeystrokeComposition.size(); i++)
    {
        _KEYSTROKE *pKeystroke = nullptr;

        pKeystroke = &_KeystrokeComposition.at(i);

        if ((pKeystroke->VirtualKey == uCode) && Global::CheckModifiers(WindowsImeLib::ModifiersValue, pKeystroke->Modifiers))
        {
            if (function == FUNCTION_NONE)
            {
                pKeyState->Category = CATEGORY_COMPOSING;
                pKeyState->Function = pKeystroke->Function;
                return TRUE;
            }
            else if (function == pKeystroke->Function)
            {
                pKeyState->Category = CATEGORY_COMPOSING;
                pKeyState->Function = pKeystroke->Function;
                return TRUE;
            }
        }
    }

    return FALSE;
}

//+---------------------------------------------------------------------------
//
// CompositionProcessorEngine::IsVirtualKeyKeystrokeCandidate
//
//----------------------------------------------------------------------------

BOOL CompositionProcessorEngine::IsVirtualKeyKeystrokeCandidate(UINT uCode, _In_ _KEYSTROKE_STATE *pKeyState, CANDIDATE_MODE candidateMode, _Out_ BOOL *pfRetCode, _In_ std::vector<_KEYSTROKE> *pKeystrokeMetric)
{
    if (pfRetCode == nullptr)
    {
        return FALSE;
    }
    *pfRetCode = FALSE;

    for (UINT i = 0; i < pKeystrokeMetric->size(); i++)
    {
        _KEYSTROKE *pKeystroke = nullptr;

        pKeystroke = &pKeystrokeMetric->at(i);

        if ((pKeystroke->VirtualKey == uCode) && Global::CheckModifiers(WindowsImeLib::ModifiersValue, pKeystroke->Modifiers))
        {
            *pfRetCode = TRUE;
            if (pKeyState)
            {
                pKeyState->Category = (candidateMode == CANDIDATE_ORIGINAL ? CATEGORY_CANDIDATE :
                    candidateMode == CANDIDATE_PHRASE ? CATEGORY_PHRASE : CATEGORY_CANDIDATE);

                pKeyState->Function = pKeystroke->Function;
            }
            return TRUE;
        }
    }

    return FALSE;
}

//+---------------------------------------------------------------------------
//
// CompositionProcessorEngine::IsKeyKeystrokeRange
//
//----------------------------------------------------------------------------

inline int FindVkInVector(const std::vector<DWORD>& srcVkList, UINT vk)
{
    bool isVkNumpad = (VK_NUMPAD0 <= vk) && (vk <= VK_NUMPAD9);

    for (auto it = srcVkList.begin(); it != srcVkList.end(); ++it)
    {
        if ((*it == vk) || (isVkNumpad && (*it == (vk - VK_NUMPAD0))))
        {
            return static_cast<int>(std::distance(srcVkList.begin(), it));
        }
    }
    return -1;
}

BOOL CompositionProcessorEngine::IsKeystrokeRange(UINT uCode, _Out_ _KEYSTROKE_STATE *pKeyState, CANDIDATE_MODE candidateMode)
{
    if (pKeyState == nullptr)
    {
        return FALSE;
    }

    pKeyState->Category = CATEGORY_NONE;
    pKeyState->Function = FUNCTION_NONE;

    if (FindVkInVector(_candidateListIndexRange, uCode) != -1)
    {
        if (candidateMode == CANDIDATE_PHRASE)
        {
            // Candidate phrase could specify modifier
            if ((GetCandidateListPhraseModifier() == 0 && WindowsImeLib::ModifiersValue == 0) ||
                (GetCandidateListPhraseModifier() != 0 && Global::CheckModifiers(WindowsImeLib::ModifiersValue, GetCandidateListPhraseModifier())))
            {
                pKeyState->Category = CATEGORY_PHRASE; pKeyState->Function = FUNCTION_SELECT_BY_NUMBER;
                return TRUE;
            }
            else
            {
                pKeyState->Category = CATEGORY_INVOKE_COMPOSITION_EDIT_SESSION; pKeyState->Function = FUNCTION_FINALIZE_TEXTSTORE_AND_INPUT;
                return FALSE;
            }
        }
        else if (candidateMode == CANDIDATE_WITH_NEXT_COMPOSITION)
        {
            // Candidate phrase could specify modifier
            if ((GetCandidateListPhraseModifier() == 0 && WindowsImeLib::ModifiersValue == 0) ||
                (GetCandidateListPhraseModifier() != 0 && Global::CheckModifiers(WindowsImeLib::ModifiersValue, GetCandidateListPhraseModifier())))
            {
                pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_SELECT_BY_NUMBER;
                return TRUE;
            }
            // else next composition
        }
        else if (candidateMode != CANDIDATE_NONE)
        {
            pKeyState->Category = CATEGORY_CANDIDATE; pKeyState->Function = FUNCTION_SELECT_BY_NUMBER;
            return TRUE;
        }
    }
    return FALSE;
}

// void CompositionProcessorEngine::ClearCompartment(ITfThreadMgr* /*pThreadMgr*/, TfClientId /*tfClientId*/)
// {
//     const auto owner = m_owner.lock();
//     if (!owner)
//     {
//         return;
//     }
// 
//     // owner->ClearCompartment(pThreadMgr, tfClientId, GUID_COMPARTMENT_KEYBOARD_OPENCLOSE);
//     // owner->ClearCompartment(pThreadMgr, tfClientId, Global::SampleIMEGuidCompartmentDoubleSingleByte);
//     // owner->ClearCompartment(pThreadMgr, tfClientId, Global::SampleIMEGuidCompartmentPunctuation);
// }

SampleIMEProcessor::SampleIMEProcessor(WindowsImeLib::ITextInputFramework* framework) :
    m_framework(framework)
{
}

SampleIMEProcessor::~SampleIMEProcessor()
{
}

std::wstring SampleIMEProcessor::TestMethod(const std::wstring& src)
{
    return src + L"-sample-suffix";
}

void SampleIMEProcessor::SetFocus(bool isGotten)
{
    WindowsImeLib::TraceLog("SampleIMEProcessor::SetFocus:%d", isGotten ? 1 : 0);
}

void SampleIMEProcessor::UpdateCustomState(const std::string& stateJson)
{
    WindowsImeLib::TraceLog("SampleIMEProcessor::UpdateSingletonEngine: %s", stateJson.c_str());
}
