// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

struct IInternalFrameworkService;

class CTfTextLayoutSink : public ITfTextLayoutSink
{
public:
    CTfTextLayoutSink(_In_ IInternalFrameworkService *pTextService);
    virtual ~CTfTextLayoutSink();

    // IUnknown methods
    STDMETHODIMP QueryInterface(REFIID riid, _Outptr_ void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

    // ITfTextLayoutSink
    STDMETHODIMP OnLayoutChange(_In_ ITfContext *pContext, TfLayoutCode lcode, _In_ ITfContextView *pContextView);

    HRESULT _StartLayout(_In_ ITfContext *pContextDocument, TfEditCookie ec, _In_ ITfRange *pRangeComposition);
    VOID _EndLayout();

    HRESULT _GetTextExt(TfEditCookie ec, _Out_ RECT *lpRect);
    ITfContext* _GetContextDocument() { return _pContextDocument; };

    virtual VOID _LayoutChangeNotification(_In_ RECT *lpRect) = 0;
    virtual VOID _LayoutDestroyNotification() = 0;

protected:
    TfEditCookie _tfEditCookie = TF_INVALID_EDIT_COOKIE;

private:
    HRESULT _AdviseTextLayoutSink();
    HRESULT _UnadviseTextLayoutSink();

private:
    ITfRange* _pRangeComposition;
    ITfContext* _pContextDocument;
    IInternalFrameworkService* _pTextService;
    DWORD _dwCookieTextLayoutSink;
    LONG _refCount;
};
