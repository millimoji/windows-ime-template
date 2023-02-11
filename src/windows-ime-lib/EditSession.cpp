// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "EditSession.h"
#include "WindowsIME.h"

HRESULT CWindowsIME::_SubmitEditSessionTask(_In_ ITfContext* context, const std::function<HRESULT(TfEditCookie ec)>& editSesisonTask, DWORD tfEsFlags)
{
    wil::com_ptr<ITfEditSession> editSession;
    RETURN_IF_FAILED(Microsoft::WRL::MakeAndInitialize<CEditSessionTask>(&editSession, editSesisonTask));
    HRESULT hr;
    RETURN_IF_FAILED(context->RequestEditSession(_tfClientId, editSession.get(), tfEsFlags, &hr));
    return hr;
}
