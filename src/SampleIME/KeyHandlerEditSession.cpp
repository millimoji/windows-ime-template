// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "pch.h"
//#include "Private.h"
#include "CompositionProcessorEngine.h"
#include "KeyHandlerEditSession.h"
//#include "EditSession.h"
//#include "WindowsIME.h"
#include "KeyStateCategory.h"

//////////////////////////////////////////////////////////////////////
//
//    ITfEditSession
//        CEditSessionBase
// CKeyHandlerEditSession class
//
//////////////////////////////////////////////////////////////////////

//+---------------------------------------------------------------------------
//
// CKeyHandlerEditSession::DoEditSession
//
//----------------------------------------------------------------------------

HRESULT CompositionProcessorEngine::KeyHandlerEditSession_DoEditSession(TfEditCookie ec, _KEYSTROKE_STATE _KeyState, _In_ ITfContext* pContext, UINT _uCode, WCHAR _wch,
    _In_ WindowsImeLib::ICompositionProcessorEngineOwner* textService)
{
    HRESULT hResult = S_OK;

    CKeyStateCategoryFactory* pKeyStateCategoryFactory = CKeyStateCategoryFactory::Instance();
    CKeyStateCategory* pKeyStateCategory = pKeyStateCategoryFactory->MakeKeyStateCategory(_KeyState.Category, textService);

    if (pKeyStateCategory)
    {
        KeyHandlerEditSessionDTO keyHandlerEditSessioDTO(ec, pContext, _uCode, _wch, _KeyState.Function);
        hResult = pKeyStateCategory->KeyStateHandler(_KeyState.Function, keyHandlerEditSessioDTO);

        pKeyStateCategory->Release();
        pKeyStateCategoryFactory->Release();
    }

    return hResult;
}
