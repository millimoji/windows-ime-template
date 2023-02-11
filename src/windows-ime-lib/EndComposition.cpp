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

HRESULT CompositionBuffer::_TerminateCompositionInternal()
{
    if (m_currentCompositionContext)
    {
        RETURN_IF_FAILED(m_framework->_SubmitEditSessionTask(m_currentCompositionContext.get(), [this](TfEditCookie ec) -> HRESULT
        {
            if (m_currentComposition)
            {
                // remove the display attribute from the composition range.
                LOG_IF_FAILED(_ClearCompositionDisplayAttributes(ec, m_currentCompositionContext.get()));

                if (FAILED_LOG(m_currentComposition->EndComposition(ec)))
                {
                    // if we fail to EndComposition, then we need to close the reverse reading window.
                    // m_framework->GetCompositionProcessorEngine()->CancelCompositioon();
                    m_framework->GetTextInputProcessor()->CancelCompositioon();
                }
            }
            LOG_IF_FAILED(m_framework->_EndLayoutTracking());

            _SaveCompositionAndContext(nullptr, nullptr);
            return S_OK;
        },
        TF_ES_ASYNCDONTCARE | TF_ES_READWRITE));
    }
    m_isComposing = false;
    return S_OK;
}

HRESULT CompositionBuffer::_TerminateComposition()
{
    wil::com_ptr<CEditSessionTask> task;
    RETURN_IF_FAILED(Microsoft::WRL::MakeAndInitialize<CEditSessionTask>(&task,
        [this](TfEditCookie ec) -> HRESULT
        {
            if (m_currentComposition)
            {
                // remove the display attribute from the composition range.
                LOG_IF_FAILED(_ClearCompositionDisplayAttributes(ec, m_workingContext.get()));

                if (FAILED_LOG(m_currentComposition->EndComposition(ec)))
                {
                    // if we fail to EndComposition, then we need to close the reverse reading window.
                    // m_framework->GetCompositionProcessorEngine()->CancelCompositioon();
                    m_framework->GetTextInputProcessor()->CancelCompositioon();
                }
            }
            _SaveCompositionAndContext(nullptr, nullptr);
            m_framework->_EndLayoutTracking();
            return S_OK;
        }));
    m_listTasks.emplace_back(task);
    m_isComposing = false;
    return S_OK;
}
