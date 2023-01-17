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

//+---------------------------------------------------------------------------
//
// CEndCompositionEditSession
//
//----------------------------------------------------------------------------

// class CEndCompositionEditSession : public CEditSessionBase
// {
// public:
//     CEndCompositionEditSession(_In_ CWindowsIME *pTextService, _In_ ITfContext *pContext) : CEditSessionBase(pTextService, pContext)
//     {
//     }
// 
//     // ITfEditSession
//     STDMETHODIMP DoEditSession(TfEditCookie ec)
//     {
//         _pTextService->GetCompositionBuffer()->_TerminateComposition(ec, _pContext, TRUE);
//         return S_OK;
//     }
// 
// };

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
    isCalledFromDeactivate;

    if (_pComposition != nullptr)
    {
        if (!pContext)
        {
            pContext = _pContext.get();
        }

        // remove the display attribute from the composition range.
        _ClearCompositionDisplayAttributes(ec, pContext);

        if (FAILED(_pComposition->EndComposition(ec)))
        {
            // if we fail to EndComposition, then we need to close the reverse reading window.
            m_framework->GetCompositionProcessorEngine()->_DeleteCandidateList(TRUE, pContext);
        }

        _pComposition->Release();
        _pComposition = nullptr;

//        if (_pContext)
//        {
//            _pContext->Release();
//            _pContext = nullptr;
//        }
        _pContext.reset();
    }
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
