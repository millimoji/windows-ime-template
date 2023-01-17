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

HRESULT CompositionProcessorEngine::KeyHandlerEditSession_DoEditSession(_KEYSTROKE_STATE _KeyState, _In_ ITfContext* pContext, UINT _uCode, WCHAR _wch)
{
    HRESULT hResult = S_OK;

    CKeyStateCategoryFactory* pKeyStateCategoryFactory = CKeyStateCategoryFactory::Instance();
    CKeyStateCategory* pKeyStateCategory = pKeyStateCategoryFactory->MakeKeyStateCategory(_KeyState.Category,
        m_compositionBuffer.get(),
        m_candidateListView,
        shared_from_this());

    if (pKeyStateCategory)
    {
        KeyHandlerEditSessionDTO keyHandlerEditSessioDTO(pContext, _uCode, _wch, _KeyState.Function);
        pKeyStateCategory->KeyStateHandler(_KeyState.Function, keyHandlerEditSessioDTO);

//        return m_owner->_SubmitEditSessionTask(pContext, [=](TfEditCookie ec, _In_ WindowsImeLib::IWindowsIMECompositionBuffer*) -> HRESULT
//        {
//            return pKeyStateCategory->KeyStateHandler(_KeyState.Function, keyHandlerEditSessioDTO);
//        }, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE);

        pKeyStateCategory->Release();
        pKeyStateCategoryFactory->Release();
    }

    return hResult;
}
