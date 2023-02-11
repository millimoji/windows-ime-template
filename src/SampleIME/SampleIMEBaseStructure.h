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


class CStringRange
{
public:
    CStringRange();
    ~CStringRange();

    const WCHAR *Get() const;
    const DWORD_PTR GetLength() const;
    void Clear();
    void Set(const WCHAR *pwch, DWORD_PTR dwLength);
    void Set(CStringRange &sr);
    CStringRange& operator=(const CStringRange& sr);
    void CharNext(_Inout_ CStringRange* pCharNext);
    static int Compare(LCID locale, _In_ CStringRange* pString1, _In_ CStringRange* pString2);
    static int Compare(LCID locale, const std::wstring& pString1, _In_ CStringRange* pString2);
    static BOOL WildcardCompare(LCID locale, _In_ CStringRange* stringWithWildcard, _In_ CStringRange* targetString);
    shared_wstring ToSharedWstring() { return std::make_shared<const std::wstring>(_pStringBuf, _stringBufLen); }

protected:
    DWORD_PTR _stringBufLen;         // Length is in character count.
    const WCHAR *_pStringBuf;    // Buffer which is not add zero terminate.
};

//---------------------------------------------------------------------
// CCandidateListItem
//  _ItemString - candidate string
//  _FindKeyCode - tailing string
//---------------------------------------------------------------------
struct CCandidateListItem
{
    CStringRange _ItemString;
    CStringRange _FindKeyCode;

    CCandidateListItem& operator =( const CCandidateListItem& rhs)
    {
        _ItemString = rhs._ItemString;
        _FindKeyCode = rhs._FindKeyCode;
        return *this;
    }
};

// struct ITfThreadMgr;
// typedef /* [uuid] */  DECLSPEC_UUID("de403c21-89fd-4f85-8b87-64584d063fbc") DWORD TfClientId;

//---------------------------------------------------------------------
// structure
//---------------------------------------------------------------------
struct _KEYSTROKE_STATE
{
    KEYSTROKE_CATEGORY Category;
    KEYSTROKE_FUNCTION Function;
};

static inline WindowsImeLib::CANDIDATELIST_FUNCTION KeyStrokeFunctionToCandidateListFunction(KEYSTROKE_FUNCTION keyStrokeFunction)
{
    WindowsImeLib::CANDIDATELIST_FUNCTION candidateListFuntion = WindowsImeLib::CANDIDATELIST_FUNCTION::NONE;
    switch (keyStrokeFunction)
    {
    case FUNCTION_NONE:             candidateListFuntion = WindowsImeLib::CANDIDATELIST_FUNCTION::NONE; break;
    case FUNCTION_MOVE_UP:          candidateListFuntion = WindowsImeLib::CANDIDATELIST_FUNCTION::MOVE_UP; break;
    case FUNCTION_MOVE_DOWN:        candidateListFuntion = WindowsImeLib::CANDIDATELIST_FUNCTION::MOVE_DOWN; break;
    case FUNCTION_MOVE_PAGE_UP:     candidateListFuntion = WindowsImeLib::CANDIDATELIST_FUNCTION::MOVE_PAGE_UP; break;
    case FUNCTION_MOVE_PAGE_DOWN:   candidateListFuntion = WindowsImeLib::CANDIDATELIST_FUNCTION::MOVE_PAGE_DOWN; break;
    case FUNCTION_MOVE_PAGE_TOP:    candidateListFuntion = WindowsImeLib::CANDIDATELIST_FUNCTION::MOVE_PAGE_TOP; break;
    case FUNCTION_MOVE_PAGE_BOTTOM: candidateListFuntion = WindowsImeLib::CANDIDATELIST_FUNCTION::MOVE_PAGE_BOTTOM; break;
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
