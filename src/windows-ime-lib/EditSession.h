// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


#pragma once

class CWindowsIME;
namespace WindowsImeLib
{
    struct IWindowsIMECompositionBuffer;
}

class CEditSessionBase : public ITfEditSession
{
public:
    CEditSessionBase(_In_ CWindowsIME *pTextService, _In_ ITfContext *pContext);
    virtual ~CEditSessionBase();

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, _Outptr_ void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

    // ITfEditSession
    virtual STDMETHODIMP DoEditSession(TfEditCookie ec) = 0;

protected:
    ITfContext *_pContext;
    CWindowsIME *_pTextService;

private:
    LONG _refCount;     // COM ref count
};

class CEditSessionTask :
    public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>,
                                        ITfEditSession,
                                        Microsoft::WRL::FtmBase>
{
public:
    CEditSessionTask() {}
    virtual ~CEditSessionTask() {}

    HRESULT RuntimeClassInitialize(const std::function<HRESULT (TfEditCookie ec, WindowsImeLib::IWindowsIMECompositionBuffer* textService)>& editSesisonTask, WindowsImeLib::IWindowsIMECompositionBuffer* textService)
    {
        m_editSesisonTask = editSesisonTask;
        m_textService = textService;
        return S_OK;
    }

    // ITfEditSession
    IFACEMETHODIMP DoEditSession(TfEditCookie ec) override
    {
        return m_editSesisonTask(ec, m_textService);
    }

private:
    std::function<HRESULT (TfEditCookie ec, WindowsImeLib::IWindowsIMECompositionBuffer* textService)> m_editSesisonTask;
    WindowsImeLib::IWindowsIMECompositionBuffer* m_textService = nullptr;
};
