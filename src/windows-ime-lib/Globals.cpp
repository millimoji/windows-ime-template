g// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "BaseWindow.h"
#include "BaseStructure.h"
#include "Globals.h"
#include "../WindowsImeLib.h"

namespace WindowsImeLibLocal {
namespace Global {

HINSTANCE dllInstanceHandle;

LONG dllRefCount = -1;

CRITICAL_SECTION CS;
HFONT defaultlFontHandle = nullptr; // Global font object we use everywhere

//---------------------------------------------------------------------
// PreserveKey GUID
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// LanguageBars
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// UI element
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// windows class / titile / atom
//---------------------------------------------------------------------
extern const WCHAR CandidateClassName[] = L"SampleIME.CandidateWindow";
ATOM AtomCandidateWindow;

extern const WCHAR ShadowClassName[] = L"SampleIME.ShadowWindow";
ATOM AtomShadowWindow;

extern const WCHAR ScrollBarClassName[] = L"SampleIME.ScrollBarWindow";
ATOM AtomScrollBarWindow;

BOOL RegisterWindowClass()
{
    if (!CBaseWindow::_InitWindowClass(CandidateClassName, &Global::AtomCandidateWindow))
    {
        return FALSE;
    }
    if (!CBaseWindow::_InitWindowClass(ShadowClassName, &AtomShadowWindow))
    {
        return FALSE;
    }
    if (!CBaseWindow::_InitWindowClass(ScrollBarClassName, &AtomScrollBarWindow))
    {
        return FALSE;
    }
    return TRUE;
}

// //---------------------------------------------------------------------
// // defined full width characters for Double/Single byte conversion
// //---------------------------------------------------------------------
// extern const WCHAR FullWidthCharTable[] = {
//     //         !       "       #       $       %       &       '       (    )       *       +       ,       -       .       /
//     0x3000, 0xFF01, 0xFF02, 0xFF03, 0xFF04, 0xFF05, 0xFF06, 0xFF07, 0xFF08, 0xFF09, 0xFF0A, 0xFF0B, 0xFF0C, 0xFF0D, 0xFF0E, 0xFF0F,
//     // 0       1       2       3       4       5       6       7       8       9       :       ;       <       =       >       ?
//     0xFF10, 0xFF11, 0xFF12, 0xFF13, 0xFF14, 0xFF15, 0xFF16, 0xFF17, 0xFF18, 0xFF19, 0xFF1A, 0xFF1B, 0xFF1C, 0xFF1D, 0xFF1E, 0xFF1F,
//     // @       A       B       C       D       E       F       G       H       I       J       K       L       M       N       0
//     0xFF20, 0xFF21, 0xFF22, 0xFF23, 0xFF24, 0xFF25, 0xFF26, 0xFF27, 0xFF28, 0xFF29, 0xFF2A, 0xFF2B, 0xFF2C, 0xFF2D, 0xFF2E, 0xFF2F,
//     // P       Q       R       S       T       U       V       W       X       Y       Z       [       \       ]       ^       _
//     0xFF30, 0xFF31, 0xFF32, 0xFF33, 0xFF34, 0xFF35, 0xFF36, 0xFF37, 0xFF38, 0xFF39, 0xFF3A, 0xFF3B, 0xFF3C, 0xFF3D, 0xFF3E, 0xFF3F,
//     // '       a       b       c       d       e       f       g       h       i       j       k       l       m       n       o       
//     0xFF40, 0xFF41, 0xFF42, 0xFF43, 0xFF44, 0xFF45, 0xFF46, 0xFF47, 0xFF48, 0xFF49, 0xFF4A, 0xFF4B, 0xFF4C, 0xFF4D, 0xFF4E, 0xFF4F,
//     // p       q       r       s       t       u       v       w       x       y       z       {       |       }       ~
//     0xFF50, 0xFF51, 0xFF52, 0xFF53, 0xFF54, 0xFF55, 0xFF56, 0xFF57, 0xFF58, 0xFF59, 0xFF5A, 0xFF5B, 0xFF5C, 0xFF5D, 0xFF5E
// };

//+---------------------------------------------------------------------------
//
// UpdateModifiers
//
//    wParam - virtual-key code
//    lParam - [0-15]  Repeat count
//  [16-23] Scan code
//  [24]    Extended key
//  [25-28] Reserved
//  [29]    Context code
//  [30]    Previous key state
//  [31]    Transition state
//----------------------------------------------------------------------------

DWORD UpdateModifiers()
{
    // high-order bit : key down
    // low-order bit  : toggled
    return  ((GetKeyState(VK_MENU)  & 0x8000) ? TF_MOD_ALT : 0) |
            ((GetKeyState(VK_LMENU) & 0x8000) ? (TF_MOD_LALT | TF_MOD_ALT) : 0) |
            ((GetKeyState(VK_RMENU) & 0x8000) ? (TF_MOD_RALT | TF_MOD_ALT) : 0) |
            ((GetKeyState(VK_CONTROL)  & 0x8000) ? TF_MOD_CONTROL : 0) |
            ((GetKeyState(VK_LCONTROL) & 0x8000) ? (TF_MOD_LCONTROL | TF_MOD_CONTROL) : 0) |
            ((GetKeyState(VK_RCONTROL) & 0x8000) ? (TF_MOD_RCONTROL | TF_MOD_CONTROL) : 0) |
            ((GetKeyState(VK_SHIFT)  & 0x8000) ? TF_MOD_SHIFT : 0) |
            ((GetKeyState(VK_LSHIFT) & 0x8000) ? (TF_MOD_LSHIFT | TF_MOD_SHIFT) : 0) |
            ((GetKeyState(VK_RSHIFT) & 0x8000) ? (TF_MOD_RSHIFT | TF_MOD_SHIFT) : 0);

//    SHORT sksMenu = GetKeyState(VK_MENU);
//    SHORT sksCtrl = GetKeyState(VK_CONTROL);
//    SHORT sksShft = GetKeyState(VK_SHIFT);
//
//    switch (wParam & 0xff)
//    {
//    case VK_MENU:
//        // is VK_MENU down?
//        if (sksMenu & 0x8000)
//        {
//            // is extended key?
//            if (lParam & 0x01000000)
//            {
//                Global::ModifiersValue |= (TF_MOD_RALT | TF_MOD_ALT);
//            }
//            else
//            {
//                Global::ModifiersValue |= (TF_MOD_LALT | TF_MOD_ALT);
//            }
//
//            // is previous key state up?
//            if (!(lParam & 0x40000000))
//            {
//                // is VK_CONTROL and VK_SHIFT up?
//                if (!(sksCtrl & 0x8000) && !(sksShft & 0x8000))
//                {
//                    Global::IsAltKeyDownOnly = TRUE;
//                }
//                else
//                {
//                    Global::IsShiftKeyDownOnly = FALSE;
//                    Global::IsControlKeyDownOnly = FALSE;
//                    Global::IsAltKeyDownOnly = FALSE;
//                }
//            }
//        }
//        break;
//
//    case VK_CONTROL:
//        // is VK_CONTROL down?
//        if (sksCtrl & 0x8000)
//        {
//            // is extended key?
//            if (lParam & 0x01000000)
//            {
//                Global::ModifiersValue |= (TF_MOD_RCONTROL | TF_MOD_CONTROL);
//            }
//            else
//            {
//                Global::ModifiersValue |= (TF_MOD_LCONTROL | TF_MOD_CONTROL);
//            }
//
//            // is previous key state up?
//            if (!(lParam & 0x40000000))
//            {
//                // is VK_SHIFT and VK_MENU up?
//                if (!(sksShft & 0x8000) && !(sksMenu & 0x8000))
//                {
//                    Global::IsControlKeyDownOnly = TRUE;
//                }
//                else
//                {
//                    Global::IsShiftKeyDownOnly = FALSE;
//                    Global::IsControlKeyDownOnly = FALSE;
//                    Global::IsAltKeyDownOnly = FALSE;
//                }
//            }
//        }
//        break;
//
//    case VK_SHIFT:
//        // is VK_SHIFT down?
//        if (sksShft & 0x8000)
//        {
//            // is scan code 0x36(right shift)?
//            if (((lParam >> 16) & 0x00ff) == 0x36)
//            {
//                Global::ModifiersValue |= (TF_MOD_RSHIFT | TF_MOD_SHIFT);
//            }
//            else
//            {
//                Global::ModifiersValue |= (TF_MOD_LSHIFT | TF_MOD_SHIFT);
//            }
//
//            // is previous key state up?
//            if (!(lParam & 0x40000000))
//            {
//                // is VK_MENU and VK_CONTROL up?
//                if (!(sksMenu & 0x8000) && !(sksCtrl & 0x8000))
//                {
//                    Global::IsShiftKeyDownOnly = TRUE;
//                }
//                else
//                {
//                    Global::IsShiftKeyDownOnly = FALSE;
//                    Global::IsControlKeyDownOnly = FALSE;
//                    Global::IsAltKeyDownOnly = FALSE;
//                }
//            }
//        }
//        break;
//
//    default:
//        Global::IsShiftKeyDownOnly = FALSE;
//        Global::IsControlKeyDownOnly = FALSE;
//        Global::IsAltKeyDownOnly = FALSE;
//        break;
//    }
//
//    if (!(sksMenu & 0x8000))
//    {
//        Global::ModifiersValue &= ~TF_MOD_ALLALT;
//    }
//    if (!(sksCtrl & 0x8000))
//    {
//        Global::ModifiersValue &= ~TF_MOD_ALLCONTROL;
//    }
//    if (!(sksShft & 0x8000))
//    {
//        Global::ModifiersValue &= ~TF_MOD_ALLSHIFT;
//    }
//
//    Global::UniqueModifiersValue = (Global::IsShiftKeyDownOnly ? TF_MOD_SHIFT : 0) |
//                                   (Global::IsControlKeyDownOnly ? TF_MOD_CONTROL : 0) |
//                                   (Global::IsAltKeyDownOnly ? TF_MOD_ALT : 0);
//#endif
//    return TRUE;
}

} // Global
} // WindowsImeLibLocal