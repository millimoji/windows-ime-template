// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "Globals.h"
#include "EditSession.h"
#include "WindowsIME.h"

//////////////////////////////////////////////////////////////////////
//
//    ITfEditSession
//        CEditSessionBase
// CEndCompositionEditSession class
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//
// CWindowsIME class
//
//////////////////////////////////////////////////////////////////////

//+---------------------------------------------------------------------------
//
// _TerminateComposition
//
//----------------------------------------------------------------------------

void CompositionBuffer::_TerminateComposition(TfEditCookie ec, _In_ ITfContext *pContext, BOOL isCalledFromDeactivate)
{
    (void)isCalledFromDeactivate;

    if (_pComposition)
    {
        if (pContext == nullptr)
        {
            pContext = _pContext.get();
        }

        // remove the display attribute from the composition range.
        _ClearCompositionDisplayAttributes(ec, pContext);

        if (FAILED_LOG(_pComposition->EndComposition(ec)))
        {
            // if we fail to EndComposition, then we need to close the reverse reading window.
            m_framework->GetCompositionProcessorEngine()->_DeleteCandidateList(TRUE, pContext);
        }

        _pComposition.reset();
        _pContext.reset();
    }

    m_framework->_EndLayoutTracking();
}

// //+---------------------------------------------------------------------------
// //
// // _EndComposition
// //
// //----------------------------------------------------------------------------
// 
// void CWindowsIME::_EndComposition(_In_opt_ ITfContext *pContext)
// {
//     if (!pContext)
//     {
//         return;
//     }
// 
//     _SubmitEditSessionTask(pContext, [pContext](TfEditCookie ec, WindowsImeLib::IWindowsIMECompositionBuffer* textService) -> HRESULT
//     {
//         textService->_TerminateComposition(ec, pContext, TRUE);
//         return S_OK;
//     }, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE);
// 
// //    CEndCompositionEditSession *pEditSession = new (std::nothrow) CEndCompositionEditSession(this, pContext);
// //    HRESULT hr = S_OK;
// //
// //    if (nullptr != pEditSession)
// //    {
// //        pContext->RequestEditSession(_tfClientId, pEditSession, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, &hr);
// //        pEditSession->Release();
// //    }
// }
// 
