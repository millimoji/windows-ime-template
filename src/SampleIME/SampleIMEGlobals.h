// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include "SampleIMEDefine.h"
#include "SampleIMEBaseStructure.h"

namespace Global
{

extern const CLSID SampleIMEGuidImeModePreserveKey;
extern const CLSID SampleIMEGuidDoubleSingleBytePreserveKey;
extern const CLSID SampleIMEGuidPunctuationPreserveKey;

BOOL CheckModifiers(UINT uModCurrent, UINT uMod);

extern const struct _PUNCTUATION PunctuationTable[14];

extern const GUID SampleIMEGuidLangBarDoubleSingleByte;
extern const GUID SampleIMEGuidLangBarPunctuation;

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
}
