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

class CEndCompositionEditSession : public CEditSessionBase
{
public:
    CEndCompositionEditSession(_In_ CWindowsIME *pTextService, _In_ ITfContext *pContext) : CEditSessionBase(pTextService, pContext)
    {
    }

    // ITfEditSession
    STDMETHODIMP DoEditSession(TfEditCookie ec)
    {
        _pTextService->GetCompositionBuffer()->_TerminateComposition(ec, _pContext, TRUE);
        return S_OK;
    }

};

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

    if (m_pComposition != nullptr)
    {
        // remove the display attribute from the composition range.
        _ClearCompositionDisplayAttributes(ec, pContext);

        if (FAILED(m_pComposition->EndComposition(ec)))
        {
            // if we fail to EndComposition, then we need to close the reverse reading window.
            m_textService->_DeleteCandidateList(TRUE, pContext);
        }

        m_pComposition->Release();
        m_pComposition = nullptr;

        if (m_pContext)
        {
            m_pContext->Release();
            m_pContext = nullptr;
        }
    }
}

//+---------------------------------------------------------------------------
//
// _EndComposition
//
//----------------------------------------------------------------------------

void CWindowsIME::_EndComposition(_In_opt_ ITfContext *pContext)
{
    CEndCompositionEditSession *pEditSession = new (std::nothrow) CEndCompositionEditSession(this, pContext);
    HRESULT hr = S_OK;

    if (nullptr != pEditSession)
    {
        pContext->RequestEditSession(_tfClientId, pEditSession, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, &hr);
        pEditSession->Release();
    }
}

