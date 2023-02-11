// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


#pragma once

namespace WindowsImeLib
{
    struct IWindowsIMECompositionBuffer;
}

class CEditSessionTask :
    public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>,
                                        ITfEditSession,
                                        Microsoft::WRL::FtmBase>
{
public:
    CEditSessionTask() {}
    virtual ~CEditSessionTask() {}

    HRESULT RuntimeClassInitialize(const std::function<HRESULT (TfEditCookie ec)>& editSesisonTask)
    {
        m_editSesisonTask = editSesisonTask;
        return S_OK;
    }

    // ITfEditSession
    IFACEMETHODIMP DoEditSession(TfEditCookie ec) override
    {
        return m_editSesisonTask(ec);
    }

private:
    std::function<HRESULT (TfEditCookie ec)> m_editSesisonTask;
};
