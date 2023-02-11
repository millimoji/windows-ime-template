// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include "SampleIMEDefine.h"
#include "SampleIMEBaseStructure.h"

namespace SampleIMENS
{
namespace Global
{
inline USHORT ModifiersValue = 0;
inline USHORT UniqueModifiersValue = 0;
inline BOOL   IsShiftKeyDownOnly = FALSE;
inline BOOL   IsControlKeyDownOnly = FALSE;
inline BOOL   IsAltKeyDownOnly = FALSE;

extern const CLSID SampleIMEGuidImeModePreserveKey;
extern const CLSID SampleIMEGuidDoubleSingleBytePreserveKey;
extern const CLSID SampleIMEGuidPunctuationPreserveKey;

BOOL CheckModifiers(UINT uModCurrent, UINT uMod);

extern const struct _PUNCTUATION PunctuationTable[14];

extern const GUID SampleIMEGuidLangBarDoubleSingleByte;
extern const GUID SampleIMEGuidLangBarPunctuation;
extern const GUID SampleIMEGuidCompartmentDoubleSingleByte;
extern const GUID SampleIMEGuidCompartmentPunctuation;

extern const WCHAR UnicodeByteOrderMark;
extern const WCHAR KeywordDelimiter;
extern const WCHAR StringDelimiter;

extern const WCHAR ImeModeDescription[];
extern const int ImeModeOnIcoIndex;
extern const int ImeModeOffIcoIndex;

extern const WCHAR DoubleSingleByteDescription[];
extern const int DoubleSingleByteOnIcoIndex;
extern const int DoubleSingleByteOffIcoIndex;

extern const WCHAR PunctuationDescription[];
extern const int PunctuationOnIcoIndex;
extern const int PunctuationOffIcoIndex;

extern const WCHAR LangbarImeModeDescription[];
extern const WCHAR LangbarDoubleSingleByteDescription[];
extern const WCHAR LangbarPunctuationDescription[];

class ConstantProvider :
    public WindowsImeLib::IConstantProvider,
    public std::enable_shared_from_this<ConstantProvider>
{
    const CLSID& IMECLSID() noexcept override
    {
        // {D2291A80-84D8-4641-9AB2-BDD1472C846B}
        static const CLSID SampleIMECLSID = { 
            0xd2291a80,
            0x84d8,
            0x4641,
            { 0x9a, 0xb2, 0xbd, 0xd1, 0x47, 0x2c, 0x84, 0x6b } };
        return SampleIMECLSID;
    }
    const GUID& IMEProfileGuid() noexcept override
    {
        // {83955C0E-2C09-47a5-BCF3-F2B98E11EE8B}
        static const GUID SampleIMEGuidProfile = {
            0x83955c0e,
            0x2c09,
            0x47a5,
            { 0xbc, 0xf3, 0xf2, 0xb9, 0x8e, 0x11, 0xee, 0x8b } };
        return SampleIMEGuidProfile;
    }
    const GUID& DisplayAttributeInput() noexcept override
    {
        // {4C802E2C-8140-4436-A5E5-F7C544EBC9CD}
        static const GUID SampleIMEGuidDisplayAttributeInput = {
            0x4c802e2c,
            0x8140,
            0x4436,
            { 0xa5, 0xe5, 0xf7, 0xc5, 0x44, 0xeb, 0xc9, 0xcd } };
        return SampleIMEGuidDisplayAttributeInput;
    }
    const GUID& DisplayAttributeConverted() noexcept override
    {
        // {9A1CC683-F2A7-4701-9C6E-2DA69A5CD474}
        static const GUID SampleIMEGuidDisplayAttributeConverted = {
            0x9a1cc683,
            0xf2a7,
            0x4701,
            { 0x9c, 0x6e, 0x2d, 0xa6, 0x9a, 0x5c, 0xd4, 0x74 } };
        return SampleIMEGuidDisplayAttributeConverted;
    }
    const GUID& CandUIElement() noexcept override
    {
        // {84B0749F-8DE7-4732-907A-3BCB150A01A8}
        static const GUID SampleIMEGuidCandUIElement = {
            0x84b0749f,
            0x8de7,
            0x4732,
            { 0x90, 0x7a, 0x3b, 0xcb, 0x15, 0xa, 0x1, 0xa8 } };
        return SampleIMEGuidCandUIElement;
    }
    const LANGID GetLangID() noexcept override
    {
        return MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED);
    }
    const LCID GetLocale() noexcept override
    {
        return MAKELCID(GetLangID(), SORT_DEFAULT);
    }
    const CLSID& ServerCLSID() noexcept override
    {
        // {A0F8CCF8-8613-472D-8A15-67568DDD9C21}
        static const GUID serverCLSID = { 0xa0f8ccf8, 0x8613, 0x472d, { 0x8a, 0x15, 0x67, 0x56, 0x8d, 0xdd, 0x9c, 0x21 } };
        return serverCLSID;
    }
    const GUID& ServerAppID() noexcept override
    {
        // {7DCD1161-73BA-42F4-80ED-DDC1CC9E59AD}
        static const GUID sererAppID = { 0x7dcd1161, 0x73ba, 0x42f4, { 0x80, 0xed, 0xdd, 0xc1, 0xcc, 0x9e, 0x59, 0xad } };
        return sererAppID;
    }
    const wchar_t* ServerName() noexcept override
    {
        return L"Sample IME Singleton Sever";
    }
    void GetPreferredTouchKeyboardLayout(_Out_ TKBLayoutType* layoutType, _Out_ WORD* preferredLayoutId) noexcept override
    {
        *layoutType = TKBLT_OPTIMIZED;
        *preferredLayoutId = TKBL_OPT_SIMPLIFIED_CHINESE_PINYIN;
    }
    UINT GetCandidateWindowWidth() noexcept override
    {
        return CAND_WIDTH;
    }
    const int GetDefaultCandidateTextFontResourceID()  override
    {
        return IDS_DEFAULT_FONT;
    }
};

extern const WCHAR FullWidthCharTable[];
}
}
using namespace SampleIMENS;
