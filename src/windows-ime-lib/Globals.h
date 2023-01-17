// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include "Define.h"
#include "BaseStructure.h"

void DllAddRef();
void DllRelease();

namespace WindowsImeLibLocal
{
namespace Global
{

inline USHORT ModifiersValue = 0;
inline USHORT UniqueModifiersValue = 0;
inline BOOL   IsShiftKeyDownOnly = FALSE;
inline BOOL   IsControlKeyDownOnly = FALSE;
inline BOOL   IsAltKeyDownOnly = FALSE;

//+---------------------------------------------------------------------------
//
// IsTooSimilar
//
//  Return TRUE if the colors cr1 and cr2 are so similar that they
//  are hard to distinguish. Used for deciding to use reverse video
//  selection instead of system selection colors.
//
//----------------------------------------------------------------------------

inline BOOL IsTooSimilar(COLORREF cr1, COLORREF cr2)
{
    if ((cr1 | cr2) & 0xFF000000)        // One color and/or the other isn't RGB, so algorithm doesn't apply
    {
        return FALSE;
    }

    LONG DeltaR = abs(GetRValue(cr1) - GetRValue(cr2));
    LONG DeltaG = abs(GetGValue(cr1) - GetGValue(cr2));
    LONG DeltaB = abs(GetBValue(cr1) - GetBValue(cr2));

    return DeltaR + DeltaG + DeltaB < 80;
}

//+---------------------------------------------------------------------------
//
// CheckModifiers
//
//----------------------------------------------------------------------------

#define TF_MOD_ALLALT     (TF_MOD_RALT | TF_MOD_LALT | TF_MOD_ALT)
#define TF_MOD_ALLCONTROL (TF_MOD_RCONTROL | TF_MOD_LCONTROL | TF_MOD_CONTROL)
#define TF_MOD_ALLSHIFT   (TF_MOD_RSHIFT | TF_MOD_LSHIFT | TF_MOD_SHIFT)

//---------------------------------------------------------------------
// extern
//---------------------------------------------------------------------
extern HINSTANCE dllInstanceHandle;
extern HFONT defaultlFontHandle;                // Global font object we use everywhere

// extern ATOM AtomCandidateWindow;
extern ATOM AtomShadowWindow;
extern ATOM AtomScrollBarWindow;

BOOL RegisterWindowClass();

extern LONG dllRefCount;

extern CRITICAL_SECTION CS;

BOOL UpdateModifiers(WPARAM wParam, LPARAM lParam);

// extern const WCHAR FullWidthCharTable[];

}
} // namespace WindowsImeLibLocal
using namespace WindowsImeLibLocal;
