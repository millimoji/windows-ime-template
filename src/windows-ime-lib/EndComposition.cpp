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

void CompositionBuffer::_TerminateComposition()
{
    LOG_IF_FAILED(m_framework->_SubmitEditSessionTask(m_workingContext.get(), [this, xpContext = m_workingContext](TfEditCookie ec) -> HRESULT
    {
        if (_pComposition)
        {
            ITfContext* pContext = (xpContext ? xpContext.get() : _pContext.get());

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
        return S_OK;
    },
    TF_ES_ASYNCDONTCARE | TF_ES_READWRITE));
}
