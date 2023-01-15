// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "Globals.h"
#include "WindowsIME.h"
#include "../WindowsImeLib.h"

BOOL CWindowsIME::VerifyIMECLSID(_In_ REFCLSID clsid)
{
    if (IsEqualCLSID(clsid, WindowsImeLib::g_processorFactory->GetConstantProvider()->IMECLSID()))
    {
        return TRUE;
    }
    return FALSE;
}

//+---------------------------------------------------------------------------
//
// ITfActiveLanguageProfileNotifySink::OnActivated
//
// Sink called by the framework when changes activate language profile.
//----------------------------------------------------------------------------

STDAPI CWindowsIME::OnActivated(_In_ REFCLSID clsid, _In_ REFGUID, _In_ BOOL isActivated)
{
    auto activity = WindowsImeLibTelemetry::ITfActiveLanguageProfileNotifySink_OnActivated();

    if (FALSE == VerifyIMECLSID(clsid))
    {
        activity.Stop();
        return S_OK;
    }

    if (isActivated)
    {
        _AddTextProcessorEngine();
    }

    if (!_pCompositionProcessorEngine)
    {
        activity.Stop();
        return S_OK;
    }

    if (isActivated)
    {
        if (m_inprocClient)
        {
            m_inprocClient->SetLanguageBarStatus(TF_LBI_STATUS_HIDDEN, FALSE); // ShowAll
            m_inprocClient->ConversionModeCompartmentUpdated();
            //_pCompositionProcessorEngine->ShowAllLanguageBarIcons();
            // _pCompositionProcessorEngine->ConversionModeCompartmentUpdated(_pThreadMgr);
        }
    }
    else
    {
        m_compositionBuffer->_DeleteCandidateList(FALSE, nullptr);

        if (m_inprocClient)
        {
            m_inprocClient->SetLanguageBarStatus(TF_LBI_STATUS_HIDDEN, TRUE); // HideAll
            // _pCompositionProcessorEngine->HideAllLanguageBarIcons();
        }
    }

    activity.Stop();
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _InitActiveLanguageProfileNotifySink
//
// Advise a active language profile notify sink.
//----------------------------------------------------------------------------

BOOL CWindowsIME::_InitActiveLanguageProfileNotifySink()
{
    ITfSource* pSource = nullptr;
    BOOL ret = FALSE;

    if (_pThreadMgr->QueryInterface(IID_ITfSource, (void **)&pSource) != S_OK)
    {
        return ret;
    }

    if (pSource->AdviseSink(IID_ITfActiveLanguageProfileNotifySink, (ITfActiveLanguageProfileNotifySink *)this, &_activeLanguageProfileNotifySinkCookie) != S_OK)
    {
        _activeLanguageProfileNotifySinkCookie = TF_INVALID_COOKIE;
        goto Exit;
    }

    ret = TRUE;

Exit:
    pSource->Release();
    return ret;
}

//+---------------------------------------------------------------------------
//
// _UninitActiveLanguageProfileNotifySink
//
// Unadvise a active language profile notify sink.  Assumes we have advised one already.
//----------------------------------------------------------------------------

void CWindowsIME::_UninitActiveLanguageProfileNotifySink()
{
    ITfSource* pSource = nullptr;

    if (_activeLanguageProfileNotifySinkCookie == TF_INVALID_COOKIE)
    {
        return; // never Advised
    }

    if (_pThreadMgr->QueryInterface(IID_ITfSource, (void **)&pSource) == S_OK)
    {
        pSource->UnadviseSink(_activeLanguageProfileNotifySinkCookie);
        pSource->Release();
    }

    _activeLanguageProfileNotifySinkCookie = TF_INVALID_COOKIE;
}
