// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "pch.h"
#include "CompositionProcessorEngine.h"
#include "SampleIMEGlobals.h"

// 0xF003, 0xF004 are the keys that the touch keyboard sends for next/previous
#define THIRDPARTY_NEXTPAGE  static_cast<WORD>(0xF003)
#define THIRDPARTY_PREVPAGE  static_cast<WORD>(0xF004)

// // Because the code mostly works with VKeys, here map a WCHAR back to a VKKey for certain
// // vkeys that the IME handles specially
// __inline UINT VKeyFromVKPacketAndWchar(UINT vk, WCHAR wch)
// {
//     UINT vkRet = vk;
//     if (LOWORD(vk) == VK_PACKET)
//     {
//         if (wch == L' ')
//         {
//             vkRet = VK_SPACE;
//         }
//         else if ((wch >= L'0') && (wch <= L'9'))
//         {
//             vkRet = static_cast<UINT>(wch);
//         }
//         else if ((wch >= L'a') && (wch <= L'z'))
//         {
//             vkRet = (UINT)(L'A') + ((UINT)(L'z') - static_cast<UINT>(wch));
//         }
//         else if ((wch >= L'A') && (wch <= L'Z'))
//         {
//             vkRet = static_cast<UINT>(wch);
//         }
//         else if (wch == THIRDPARTY_NEXTPAGE)
//         {
//             vkRet = VK_NEXT;
//         }
//         else if (wch == THIRDPARTY_PREVPAGE)
//         {
//             vkRet = VK_PRIOR;
//         }
//     }
//     return vkRet;
// }

//+---------------------------------------------------------------------------
//
// _IsKeyEaten
//
//----------------------------------------------------------------------------

BOOL CompositionProcessorEngine::_IsKeyEaten(_In_ UINT /*codeIn*/, wchar_t wch, UINT vkPackSource, bool isKbdDisabled, _Out_opt_ _KEYSTROKE_STATE* pKeyState)
{
    BOOL isOpen = m_compartmentIsOpen;
    BOOL isDoubleSingleByte = m_compartmentIsDoubleSingleByte;
    BOOL isPunctuation = m_compartmentIsPunctuation;

    if (pKeyState)
    {
        pKeyState->Category = CATEGORY_NONE;
        pKeyState->Function = FUNCTION_NONE;
    }

    // if the keyboard is disabled, we don't eat keys.
    if (isKbdDisabled)
    {
        return FALSE;
    }

    //
    // Map virtual key to character code
    //
    BOOL isTouchKeyboardSpecialKeys = FALSE;
//    WCHAR wch = m_owner->ConvertVKey(codeIn);
//    *pCodeOut = m_owner->VKeyFromVKPacketAndWchar(codeIn, wch);
    if ((wch == THIRDPARTY_NEXTPAGE) || (wch == THIRDPARTY_PREVPAGE))
    {
        // We always eat the above softkeyboard special keys
        isTouchKeyboardSpecialKeys = TRUE;
//        if (pwch)
//        {
//            *pwch = wch;
//        }
    }

    // if the keyboard is closed, we don't eat keys, with the exception of the touch keyboard specials keys
    if (!isOpen && !isDoubleSingleByte && !isPunctuation)
    {
        return isTouchKeyboardSpecialKeys;
    }

//    if (pwch)
//    {
//        *pwch = wch;
//    }

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
        const auto isCandidateWithWildcard = _isCandidateWithWildcard;
        const auto isComposing = m_compositionBuffer->_IsComposing();

        if (IsVirtualKeyNeed(vkPackSource, wch, isComposing, isCandidateWithWildcard, pKeyState))
        {
            return TRUE;
        }
    }

    //
    // Punctuation
    //
    if (IsPunctuation(wch))
    {
        if ((_candidateMode == CANDIDATE_NONE) && isPunctuation)
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
    if (isDoubleSingleByte && IsDoubleSingleByte(wch))
    {
        if (_candidateMode == CANDIDATE_NONE)
        {
            if (pKeyState)
            {
                pKeyState->Category = CATEGORY_COMPOSING;
                pKeyState->Function = FUNCTION_DOUBLE_SINGLE_BYTE;
            }
            return TRUE;
        }
    }

    return isTouchKeyboardSpecialKeys;
}

// //+---------------------------------------------------------------------------
// //
// // ConvertVKey
// //
// //----------------------------------------------------------------------------
// 
// WCHAR CWindowsIME::ConvertVKey(UINT code)
// {
//     //
//     // Map virtual key to scan code
//     //
//     UINT scanCode = 0;
//     scanCode = MapVirtualKey(code, 0);
// 
//     //
//     // Keyboard state
//     //
//     BYTE abKbdState[256] = {'\0'};
//     if (!GetKeyboardState(abKbdState))
//     {
//         return 0;
//     }
// 
//     //
//     // Map virtual key to character code
//     //
//     WCHAR wch = '\0';
//     if (ToUnicode(code, scanCode, abKbdState, &wch, 1, 0) == 1)
//     {
//         return wch;
//     }
// 
//     return 0;
// }
// 

//+---------------------------------------------------------------------------
//
// ITfKeyEventSink::OnTestKeyDown
//
// Called by the system to query this service wants a potential keystroke.
//----------------------------------------------------------------------------

HRESULT CompositionProcessorEngine::OnTestKeyDown(WPARAM wParam, LPARAM /*lParam*/, BOOL* pIsEaten, wchar_t wch, UINT vkPackSource, bool isKbdDisabled)
{
    _KEYSTROKE_STATE KeystrokeState;
    //WCHAR wch = '\0';
    UINT code = 0;
    *pIsEaten = _IsKeyEaten((UINT)wParam, wch, vkPackSource, isKbdDisabled, &KeystrokeState);

    if (KeystrokeState.Category == CATEGORY_INVOKE_COMPOSITION_EDIT_SESSION)
    {
        //
        // Invoke key handler edit session
        //
        KeystrokeState.Category = CATEGORY_COMPOSING;
        return KeyHandlerEditSession_DoEditSession(KeystrokeState, code, wch);
    }

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfKeyEventSink::OnKeyDown
//
// Called by the system to offer this service a keystroke.  If *pIsEaten == TRUE
// on exit, the application will not handle the keystroke.
//----------------------------------------------------------------------------

HRESULT CompositionProcessorEngine::OnKeyDown(WPARAM wParam, LPARAM /*lParam*/, BOOL* pIsEaten, wchar_t wch, UINT vkPackSource, bool isKbdDisabled)
{
    _KEYSTROKE_STATE KeystrokeState;

    *pIsEaten = _IsKeyEaten((UINT)wParam, wch, vkPackSource, isKbdDisabled, &KeystrokeState);

    if (*pIsEaten)
    {
        bool needInvokeKeyHandler = true;
        //
        // Invoke key handler edit session
        //
        if (vkPackSource == VK_ESCAPE)
        {
            KeystrokeState.Category = CATEGORY_COMPOSING;
        }

        // Always eat THIRDPARTY_NEXTPAGE and THIRDPARTY_PREVPAGE keys, but don't always process them.
        if ((wch == THIRDPARTY_NEXTPAGE) || (wch == THIRDPARTY_PREVPAGE))
        {
            needInvokeKeyHandler = !((KeystrokeState.Category == CATEGORY_NONE) && (KeystrokeState.Function == FUNCTION_NONE));
        }

        if (needInvokeKeyHandler)
        {
            KeyHandlerEditSession_DoEditSession(KeystrokeState, vkPackSource, wch);
        }
    }
    else if (KeystrokeState.Category == CATEGORY_INVOKE_COMPOSITION_EDIT_SESSION)
    {
        // Invoke key handler edit session
        KeystrokeState.Category = CATEGORY_COMPOSING;
        KeyHandlerEditSession_DoEditSession(KeystrokeState, vkPackSource, wch);
    }

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfKeyEventSink::OnTestKeyUp
//
// Called by the system to query this service wants a potential keystroke.
//----------------------------------------------------------------------------

HRESULT CompositionProcessorEngine::OnTestKeyUp(WPARAM wParam, LPARAM /*lParam*/, BOOL* pIsEaten, wchar_t wch, UINT vkPackSource, bool isKbdDisabled)
{
    *pIsEaten = _IsKeyEaten((UINT)wParam, wch, vkPackSource, isKbdDisabled, NULL);
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfKeyEventSink::OnKeyUp
//
// Called by the system to offer this service a keystroke.  If *pIsEaten == TRUE
// on exit, the application will not handle the keystroke.
//----------------------------------------------------------------------------

HRESULT CompositionProcessorEngine::OnKeyUp(WPARAM wParam, LPARAM /*lParam*/, BOOL* pIsEaten, wchar_t wch, UINT vkPackSource, bool isKbdDisabled)
{
    *pIsEaten = _IsKeyEaten((UINT)wParam, wch, vkPackSource, isKbdDisabled, NULL);
    return S_OK;
}

