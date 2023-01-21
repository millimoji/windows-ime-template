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

static inline CANDIDATELIST_FUNCTION KeyStrokeFunctionToCandidateListFunction(KEYSTROKE_FUNCTION keyStrokeFunction)
{
    CANDIDATELIST_FUNCTION candidateListFuntion = CANDIDATELIST_FUNCTION_NONE;
    switch (keyStrokeFunction)
    {
    case FUNCTION_NONE:             candidateListFuntion = CANDIDATELIST_FUNCTION_NONE; break;
    case FUNCTION_MOVE_UP:          candidateListFuntion = CANDIDATELIST_FUNCTION_MOVE_UP; break;
    case FUNCTION_MOVE_DOWN:        candidateListFuntion = CANDIDATELIST_FUNCTION_MOVE_DOWN; break;
    case FUNCTION_MOVE_PAGE_UP:     candidateListFuntion = CANDIDATELIST_FUNCTION_MOVE_PAGE_UP; break;
    case FUNCTION_MOVE_PAGE_DOWN:   candidateListFuntion = CANDIDATELIST_FUNCTION_MOVE_PAGE_DOWN; break;
    case FUNCTION_MOVE_PAGE_TOP:    candidateListFuntion = CANDIDATELIST_FUNCTION_MOVE_PAGE_TOP; break;
    case FUNCTION_MOVE_PAGE_BOTTOM: candidateListFuntion = CANDIDATELIST_FUNCTION_MOVE_PAGE_BOTTOM; break;
    }
    return candidateListFuntion;
}

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
