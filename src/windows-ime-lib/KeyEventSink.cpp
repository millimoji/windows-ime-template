// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "Globals.h"
#include "WindowsIME.h"
#include "CandidateListUIPresenter.h"
//#include "KeyHandlerEditSession.h"
#include "../Compartment.h"

// 0xF003, 0xF004 are the keys that the touch keyboard sends for next/previous
#define THIRDPARTY_NEXTPAGE  static_cast<WORD>(0xF003)
#define THIRDPARTY_PREVPAGE  static_cast<WORD>(0xF004)

// Because the code mostly works with VKeys, here map a WCHAR back to a VKKey for certain
// vkeys that the IME handles specially
UINT CWindowsIME::VKeyFromVKPacketAndWchar(UINT vk, WCHAR wch)
{
    UINT vkRet = vk;
    if (LOWORD(vk) == VK_PACKET)
    {
        if (wch == L' ')
        {
            vkRet = VK_SPACE;
        }
        else if ((wch >= L'0') && (wch <= L'9'))
        {
            vkRet = static_cast<UINT>(wch);
        }
        else if ((wch >= L'a') && (wch <= L'z'))
        {
            vkRet = (UINT)(L'A') + ((UINT)(L'z') - static_cast<UINT>(wch));
        }
        else if ((wch >= L'A') && (wch <= L'Z'))
        {
            vkRet = static_cast<UINT>(wch);
        }
        else if (wch == THIRDPARTY_NEXTPAGE)
        {
            vkRet = VK_NEXT;
        }
        else if (wch == THIRDPARTY_PREVPAGE)
        {
            vkRet = VK_PRIOR;
        }
    }
    return vkRet;
}

//+---------------------------------------------------------------------------
//
// ConvertVKey
//
//----------------------------------------------------------------------------

wchar_t CWindowsIME::ConvertVKey(UINT code)
{
    //
    // Map virtual key to scan code
    //
    UINT scanCode = 0;
    scanCode = MapVirtualKey(code, 0);

    //
    // Keyboard state
    //
    BYTE abKbdState[256] = {'\0'};
    if (!GetKeyboardState(abKbdState))
    {
        return 0;
    }

    //
    // Map virtual key to character code
    //
    WCHAR wch = '\0';
    if (ToUnicode(code, scanCode, abKbdState, &wch, 1, 0) == 1)
    {
        return wch;
    }

    return 0;
}

//+---------------------------------------------------------------------------
//
// _IsKeyboardDisabled
//
//----------------------------------------------------------------------------

bool CWindowsIME::_IsKeyboardDisabled()
{
    ITfDocumentMgr* pDocMgrFocus = nullptr;
    ITfContext* pContext = nullptr;
    BOOL isDisabled = FALSE;

    if ((_pThreadMgr->GetFocus(&pDocMgrFocus) != S_OK) ||
        (pDocMgrFocus == nullptr))
    {
        // if there is no focus document manager object, the keyboard 
        // is disabled.
        isDisabled = TRUE;
    }
    else if ((pDocMgrFocus->GetTop(&pContext) != S_OK) ||
        (pContext == nullptr))
    {
        // if there is no context object, the keyboard is disabled.
        isDisabled = TRUE;
    }
    else
    {
        CCompartment CompartmentKeyboardDisabled(_pThreadMgr.get(), _tfClientId, GUID_COMPARTMENT_KEYBOARD_DISABLED);
        CompartmentKeyboardDisabled._GetCompartmentBOOL(isDisabled);

        CCompartment CompartmentEmptyContext(_pThreadMgr.get(), _tfClientId, GUID_COMPARTMENT_EMPTYCONTEXT);
        CompartmentEmptyContext._GetCompartmentBOOL(isDisabled);
    }

    if (pContext)
    {
        pContext->Release();
    }

    if (pDocMgrFocus)
    {
        pDocMgrFocus->Release();
    }

    return !!isDisabled;
}

//+---------------------------------------------------------------------------
//
// ITfKeyEventSink::OnSetFocus
//
// Called by the system whenever this service gets the keystroke device focus.
//----------------------------------------------------------------------------

STDAPI CWindowsIME::OnSetFocus(BOOL /*fForeground*/) try
{
    // auto activity = WindowsImeLibTelemetry::ITfKeyEventSink_OnSetFocus();
    // activity.Stop();
    return S_OK;
}
CATCH_RETURN()

//+---------------------------------------------------------------------------
//
// ITfKeyEventSink::OnTestKeyDown
//
// Called by the system to query this service wants a potential keystroke.
//----------------------------------------------------------------------------

STDAPI CWindowsIME::OnTestKeyDown(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pIsEaten) 
{
    auto activity = WindowsImeLibTelemetry::ITfKeyEventSink_OnTestKeyDown();

    m_compositionBuffer->SaveWorkingContext(pContext);
    auto resetWorkingContext = wil::scope_exit([&]() { m_compositionBuffer->SaveWorkingContext(nullptr); });

    Global::UpdateModifiers(wParam, lParam);

    if (_pCompositionProcessorEngine)
    {
        WCHAR wch = ConvertVKey(static_cast<UINT>(wParam));
        UINT vkPackSource = VKeyFromVKPacketAndWchar(static_cast<UINT>(wParam), wch);
        bool isKbdDisabled = _IsKeyboardDisabled();
        _pCompositionProcessorEngine->OnKeyEvent(pContext, wParam, lParam, pIsEaten, wch, vkPackSource, isKbdDisabled, Global::ModifiersValue, Global::UniqueModifiersValue, true /*test*/, true /*down*/);
    }

//    _KEYSTROKE_STATE KeystrokeState;
//    WCHAR wch = '\0';
//    UINT code = 0;
//    *pIsEaten = _IsKeyEaten(pContext, (UINT)wParam, &code, &wch, &KeystrokeState);
//
//    if (KeystrokeState.Category == CATEGORY_INVOKE_COMPOSITION_EDIT_SESSION)
//    {
//        //
//        // Invoke key handler edit session
//        //
//        KeystrokeState.Category = CATEGORY_COMPOSING;
//
//        _InvokeKeyHandler(pContext, code, wch, (DWORD)lParam, KeystrokeState);
//    }
//
    activity.Stop();
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfKeyEventSink::OnKeyDown
//
// Called by the system to offer this service a keystroke.  If *pIsEaten == TRUE
// on exit, the application will not handle the keystroke.
//----------------------------------------------------------------------------

STDAPI CWindowsIME::OnKeyDown(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pIsEaten)
{
    auto activity = WindowsImeLibTelemetry::ITfKeyEventSink_OnKeyDown();

    m_compositionBuffer->SaveWorkingContext(pContext);
    auto resetWorkingContext = wil::scope_exit([&]() { m_compositionBuffer->SaveWorkingContext(nullptr); });

    Global::UpdateModifiers(wParam, lParam);

    if (_pCompositionProcessorEngine)
    {
        WCHAR wch = ConvertVKey(static_cast<UINT>(wParam));
        UINT vkPackSource = VKeyFromVKPacketAndWchar(static_cast<UINT>(wParam), wch);
        bool isKbdDisabled = _IsKeyboardDisabled();
        _pCompositionProcessorEngine->OnKeyEvent(pContext, wParam, lParam, pIsEaten, wch, vkPackSource, isKbdDisabled, Global::ModifiersValue, Global::UniqueModifiersValue, false /*test*/, true /*down*/);
    }

//    _KEYSTROKE_STATE KeystrokeState;
//    WCHAR wch = '\0';
//    UINT code = 0;
//
//    *pIsEaten = _IsKeyEaten(pContext, (UINT)wParam, &code, &wch, &KeystrokeState);
//
//    if (*pIsEaten)
//    {
//        bool needInvokeKeyHandler = true;
//        //
//        // Invoke key handler edit session
//        //
//        if (code == VK_ESCAPE)
//        {
//            KeystrokeState.Category = CATEGORY_COMPOSING;
//        }
//
//        // Always eat THIRDPARTY_NEXTPAGE and THIRDPARTY_PREVPAGE keys, but don't always process them.
//        if ((wch == THIRDPARTY_NEXTPAGE) || (wch == THIRDPARTY_PREVPAGE))
//        {
//            needInvokeKeyHandler = !((KeystrokeState.Category == CATEGORY_NONE) && (KeystrokeState.Function == FUNCTION_NONE));
//        }
//
//        if (needInvokeKeyHandler)
//        {
//            _InvokeKeyHandler(pContext, code, wch, (DWORD)lParam, KeystrokeState);
//        }
//    }
//    else if (KeystrokeState.Category == CATEGORY_INVOKE_COMPOSITION_EDIT_SESSION)
//    {
//        // Invoke key handler edit session
//        KeystrokeState.Category = CATEGORY_COMPOSING;
//        _InvokeKeyHandler(pContext, code, wch, (DWORD)lParam, KeystrokeState);
//    }

    activity.Stop();
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfKeyEventSink::OnTestKeyUp
//
// Called by the system to query this service wants a potential keystroke.
//----------------------------------------------------------------------------

STDAPI CWindowsIME::OnTestKeyUp(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pIsEaten)
{
    auto activity = WindowsImeLibTelemetry::ITfKeyEventSink_OnTestKeyUp();

    m_compositionBuffer->SaveWorkingContext(pContext);
    auto resetWorkingContext = wil::scope_exit([&]() { m_compositionBuffer->SaveWorkingContext(nullptr); });

    Global::UpdateModifiers(wParam, lParam);

    if (_pCompositionProcessorEngine)
    {
        WCHAR wch = ConvertVKey(static_cast<UINT>(wParam));
        UINT vkPackSource = VKeyFromVKPacketAndWchar(static_cast<UINT>(wParam), wch);
        bool isKbdDisabled = _IsKeyboardDisabled();
        _pCompositionProcessorEngine->OnKeyEvent(pContext, wParam, lParam, pIsEaten, wch, vkPackSource, isKbdDisabled, Global::ModifiersValue, Global::UniqueModifiersValue, true /*test*/, false /*down*/);
    }

//    if (pIsEaten == nullptr)
//    {
//        return E_INVALIDARG;
//    }
//
//    Global::UpdateModifiers(wParam, lParam);
//
//    WCHAR wch = '\0';
//    UINT code = 0;
//
//    *pIsEaten = _IsKeyEaten(pContext, (UINT)wParam, &code, &wch, NULL);

    activity.Stop();
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfKeyEventSink::OnKeyUp
//
// Called by the system to offer this service a keystroke.  If *pIsEaten == TRUE
// on exit, the application will not handle the keystroke.
//----------------------------------------------------------------------------

STDAPI CWindowsIME::OnKeyUp(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pIsEaten)
{
    auto activity = WindowsImeLibTelemetry::ITfKeyEventSink_OnKeyUp();

    m_compositionBuffer->SaveWorkingContext(pContext);
    auto resetWorkingContext = wil::scope_exit([&]() { m_compositionBuffer->SaveWorkingContext(nullptr); });

    Global::UpdateModifiers(wParam, lParam);

    if (_pCompositionProcessorEngine)
    {
        WCHAR wch = ConvertVKey(static_cast<UINT>(wParam));
        UINT vkPackSource = VKeyFromVKPacketAndWchar(static_cast<UINT>(wParam), wch);
        bool isKbdDisabled = _IsKeyboardDisabled();
        _pCompositionProcessorEngine->OnKeyEvent(pContext, wParam, lParam, pIsEaten, wch, vkPackSource, isKbdDisabled, Global::ModifiersValue, Global::UniqueModifiersValue, false /*test*/, false /*down*/);
    }

//    WCHAR wch = '\0';
//    UINT code = 0;
//
//    *pIsEaten = _IsKeyEaten(pContext, (UINT)wParam, &code, &wch, NULL);

    activity.Stop();
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfKeyEventSink::OnPreservedKey
//
// Called when a hotkey (registered by us, or by the system) is typed.
//----------------------------------------------------------------------------

STDAPI CWindowsIME::OnPreservedKey(ITfContext* /*pContext*/, REFGUID rguid, BOOL* pIsEaten)
{
    auto activity = WindowsImeLibTelemetry::ITfKeyEventSink_OnPreservedKey();

    if (m_inprocClient)
    {
        m_inprocClient->OnPreservedKey(rguid, pIsEaten, _pThreadMgr.get(), _tfClientId);
    }

    activity.Stop();
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _InitKeyEventSink
//
// Advise a keystroke sink.
//----------------------------------------------------------------------------

BOOL CWindowsIME::_InitKeyEventSink()
{
    ITfKeystrokeMgr* pKeystrokeMgr = nullptr;
    HRESULT hr = S_OK;

    if (FAILED(_pThreadMgr->QueryInterface(IID_ITfKeystrokeMgr, (void **)&pKeystrokeMgr)))
    {
        return FALSE;
    }

    hr = pKeystrokeMgr->AdviseKeyEventSink(_tfClientId, (ITfKeyEventSink *)this, TRUE);

    pKeystrokeMgr->Release();

    return (hr == S_OK);
}

//+---------------------------------------------------------------------------
//
// _UninitKeyEventSink
//
// Unadvise a keystroke sink.  Assumes we have advised one already.
//----------------------------------------------------------------------------

void CWindowsIME::_UninitKeyEventSink()
{
    ITfKeystrokeMgr* pKeystrokeMgr = nullptr;

    if (FAILED(_pThreadMgr->QueryInterface(IID_ITfKeystrokeMgr, (void **)&pKeystrokeMgr)))
    {
        return;
    }

    pKeystrokeMgr->UnadviseKeyEventSink(_tfClientId);

    pKeystrokeMgr->Release();
}
