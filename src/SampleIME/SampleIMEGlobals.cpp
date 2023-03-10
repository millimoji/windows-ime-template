// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "pch.h"
#include "resource.h"
#include "SampleIMEDefine.h"
#include "SampleIMEBaseStructure.h"

namespace SampleIMENS
{
namespace Global
{

//---------------------------------------------------------------------
// PreserveKey GUID
//---------------------------------------------------------------------
// {4B62B54B-F828-43B5-9095-A96DF9CBDF38}
extern const GUID SampleIMEGuidImeModePreserveKey = {
    0x4b62b54b, 
    0xf828, 
    0x43b5, 
    { 0x90, 0x95, 0xa9, 0x6d, 0xf9, 0xcb, 0xdf, 0x38 } 
};

// {5A08D6C4-4563-4E46-8DDB-65E75C4E73A3}
extern const GUID SampleIMEGuidDoubleSingleBytePreserveKey = {
    0x5a08d6c4, 
    0x4563, 
    0x4e46, 
    { 0x8d, 0xdb, 0x65, 0xe7, 0x5c, 0x4e, 0x73, 0xa3 } 
};

// {175F062E-B961-4AED-A3DF-59F78A02862D}
extern const GUID SampleIMEGuidPunctuationPreserveKey = {
    0x175f062e, 
    0xb961, 
    0x4aed, 
    { 0xa3, 0xdf, 0x59, 0xf7, 0x8a, 0x2, 0x86, 0x2d } 
};

// {6A11D9DE-46DB-455B-A257-2EB615746BF4}
extern const GUID SampleIMEGuidLangBarDoubleSingleByte = {
    0x6a11d9de,
    0x46db,
    0x455b,
    { 0xa2, 0x57, 0x2e, 0xb6, 0x15, 0x74, 0x6b, 0xf4 }
};

// {F29C731A-A51E-49FB-8A3C-EE51752912E2}
extern const GUID SampleIMEGuidLangBarPunctuation = {
    0xf29c731a,
    0xa51e,
    0x49fb,
    { 0x8a, 0x3c, 0xee, 0x51, 0x75, 0x29, 0x12, 0xe2 }
};

//---------------------------------------------------------------------
// Compartments
//---------------------------------------------------------------------
// {101011C5-CF72-4F0C-A515-153019593F10}
extern const GUID SampleIMEGuidCompartmentDoubleSingleByte = {
    0x101011c5,
    0xcf72,
    0x4f0c,
    { 0xa5, 0x15, 0x15, 0x30, 0x19, 0x59, 0x3f, 0x10 }
};

// {DD321BCC-A7F8-4561-9B61-9B3508C9BA97}
extern const GUID SampleIMEGuidCompartmentPunctuation = {
    0xdd321bcc,
    0xa7f8,
    0x4561,
    { 0x9b, 0x61, 0x9b, 0x35, 0x8, 0xc9, 0xba, 0x97 }
};

//---------------------------------------------------------------------
// Unicode byte order mark
//---------------------------------------------------------------------
extern const WCHAR UnicodeByteOrderMark = 0xFEFF;

//---------------------------------------------------------------------
// dictionary table delimiter
//---------------------------------------------------------------------
extern const WCHAR KeywordDelimiter = L'=';
extern const WCHAR StringDelimiter  = L'\"';

//---------------------------------------------------------------------
// defined item in setting file table [PreservedKey] section
//---------------------------------------------------------------------
extern const WCHAR ImeModeDescription[] = L"Chinese/English input (Shift)";
extern const int ImeModeOnIcoIndex = IME_MODE_ON_ICON_INDEX;
extern const int ImeModeOffIcoIndex = IME_MODE_OFF_ICON_INDEX;

extern const WCHAR DoubleSingleByteDescription[] = L"Double/Single byte (Shift+Space)";
extern const int DoubleSingleByteOnIcoIndex = IME_DOUBLE_ON_INDEX;
extern const int DoubleSingleByteOffIcoIndex = IME_DOUBLE_OFF_INDEX;

extern const WCHAR PunctuationDescription[] = L"Chinese/English punctuation (Ctrl+.)";
extern const int PunctuationOnIcoIndex = IME_PUNCTUATION_ON_INDEX;
extern const int PunctuationOffIcoIndex = IME_PUNCTUATION_OFF_INDEX;

//---------------------------------------------------------------------
// defined item in setting file table [LanguageBar] section
//---------------------------------------------------------------------
extern const WCHAR LangbarImeModeDescription[] = L"Conversion mode";
extern const WCHAR LangbarDoubleSingleByteDescription[] = L"Character width";
extern const WCHAR LangbarPunctuationDescription[] = L"Punctuation";

//---------------------------------------------------------------------
// defined punctuation characters
//---------------------------------------------------------------------
extern const struct _PUNCTUATION PunctuationTable[14] = {
    {L'!',  0xFF01},
    {L'$',  0xFFE5},
    {L'&',  0x2014},
    {L'(',  0xFF08},
    {L')',  0xFF09},
    {L',',  0xFF0C},
    {L'.',  0x3002},
    {L':',  0xFF1A},
    {L';',  0xFF1B},
    {L'?',  0xFF1F},
    {L'@',  0x00B7},
    {L'\\', 0x3001},
    {L'^',  0x2026},
    {L'_',  0x2014}
};

//+---------------------------------------------------------------------------
//
// CheckModifiers
//
//----------------------------------------------------------------------------

#define TF_MOD_RLALT      (TF_MOD_RALT | TF_MOD_LALT)
#define TF_MOD_RLCONTROL  (TF_MOD_RCONTROL | TF_MOD_LCONTROL)
#define TF_MOD_RLSHIFT    (TF_MOD_RSHIFT | TF_MOD_LSHIFT)

#define CheckMod(m0, m1, mod)        \
    if (m1 & TF_MOD_ ## mod ##)      \
    { \
        if (!(m0 & TF_MOD_ ## mod ##)) \
        {      \
            return FALSE;   \
        }      \
    } \
    else       \
    { \
        if ((m1 ^ m0) & TF_MOD_RL ## mod ##)    \
        {      \
            return FALSE;   \
        }      \
    } \

BOOL CheckModifiers(UINT modCurrent, UINT mod)
{
    mod &= ~TF_MOD_ON_KEYUP;

    if (mod & TF_MOD_IGNORE_ALL_MODIFIER)
    {
        return TRUE;
    }

    if (modCurrent == mod)
    {
        return TRUE;
    }

    if (modCurrent && !mod)
    {
        return FALSE;
    }

    CheckMod(modCurrent, mod, ALT);
    CheckMod(modCurrent, mod, SHIFT);
    CheckMod(modCurrent, mod, CONTROL);

    return TRUE;
}

//---------------------------------------------------------------------
// defined full width characters for Double/Single byte conversion
//---------------------------------------------------------------------
extern const WCHAR FullWidthCharTable[] = {
    //         !       "       #       $       %       &       '       (    )       *       +       ,       -       .       /
    0x3000, 0xFF01, 0xFF02, 0xFF03, 0xFF04, 0xFF05, 0xFF06, 0xFF07, 0xFF08, 0xFF09, 0xFF0A, 0xFF0B, 0xFF0C, 0xFF0D, 0xFF0E, 0xFF0F,
    // 0       1       2       3       4       5       6       7       8       9       :       ;       <       =       >       ?
    0xFF10, 0xFF11, 0xFF12, 0xFF13, 0xFF14, 0xFF15, 0xFF16, 0xFF17, 0xFF18, 0xFF19, 0xFF1A, 0xFF1B, 0xFF1C, 0xFF1D, 0xFF1E, 0xFF1F,
    // @       A       B       C       D       E       F       G       H       I       J       K       L       M       N       0
    0xFF20, 0xFF21, 0xFF22, 0xFF23, 0xFF24, 0xFF25, 0xFF26, 0xFF27, 0xFF28, 0xFF29, 0xFF2A, 0xFF2B, 0xFF2C, 0xFF2D, 0xFF2E, 0xFF2F,
    // P       Q       R       S       T       U       V       W       X       Y       Z       [       \       ]       ^       _
    0xFF30, 0xFF31, 0xFF32, 0xFF33, 0xFF34, 0xFF35, 0xFF36, 0xFF37, 0xFF38, 0xFF39, 0xFF3A, 0xFF3B, 0xFF3C, 0xFF3D, 0xFF3E, 0xFF3F,
    // '       a       b       c       d       e       f       g       h       i       j       k       l       m       n       o       
    0xFF40, 0xFF41, 0xFF42, 0xFF43, 0xFF44, 0xFF45, 0xFF46, 0xFF47, 0xFF48, 0xFF49, 0xFF4A, 0xFF4B, 0xFF4C, 0xFF4D, 0xFF4E, 0xFF4F,
    // p       q       r       s       t       u       v       w       x       y       z       {       |       }       ~
    0xFF50, 0xFF51, 0xFF52, 0xFF53, 0xFF54, 0xFF55, 0xFF56, 0xFF57, 0xFF58, 0xFF59, 0xFF5A, 0xFF5B, 0xFF5C, 0xFF5D, 0xFF5E
};

}
}
