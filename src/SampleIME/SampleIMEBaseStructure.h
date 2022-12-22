// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include "../WindowsImeLib.h"
#include <vector>
#include "assert.h"
#include <iostream>

// using std::cout;
// using std::endl;
// 
// //---------------------------------------------------------------------
// // defined keyword
// //---------------------------------------------------------------------
// template<class VALUE>
// struct _DEFINED_KEYWORD
// {
//     LPCWSTR _pwszKeyword;
//     VALUE _value;
// };
// 
// BOOL CLSIDToString(REFGUID refGUID, _Out_writes_ (39) WCHAR *pCLSIDString);

//---------------------------------------------------------------------
// structure
//---------------------------------------------------------------------
struct _PUNCTUATION
{
    WCHAR _Code;
    WCHAR _Punctuation;
};


HRESULT SkipWhiteSpace(LCID locale, _In_ LPCWSTR pwszBuffer, DWORD_PTR dwBufLen, _Out_ DWORD_PTR *pdwIndex);
HRESULT FindChar(WCHAR wch, _In_ LPCWSTR pwszBuffer, DWORD_PTR dwBufLen, _Out_ DWORD_PTR *pdwIndex);

BOOL IsSpace(LCID locale, WCHAR wch);

class CPunctuationPair
{
public:
    CPunctuationPair();
    CPunctuationPair(WCHAR code, WCHAR punctuation, WCHAR pair);

    struct _PUNCTUATION _punctuation;
    WCHAR _pairPunctuation;
    BOOL _isPairToggle;
};

class CPunctuationNestPair
{
public:
    CPunctuationNestPair();
    CPunctuationNestPair(WCHAR wchCode_begin, WCHAR wch_begin, WCHAR wchPair_begin,
        WCHAR wchCode_end,   WCHAR wch_end,   WCHAR wchPair_end);

    struct _PUNCTUATION _punctuation_begin;
    WCHAR _pairPunctuation_begin;

    struct _PUNCTUATION _punctuation_end;
    WCHAR _pairPunctuation_end;

    int _nestCount;
};
