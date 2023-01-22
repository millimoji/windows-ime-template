// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "WindowsIME.h"
#include "Globals.h"
#include "TfTextLayoutSink.h"
#include "GetTextExtentEditSession.h"


HRESULT CWindowsIME::_StartLayout(_In_ ITfContext *pContextDocument, TfEditCookie ec, _In_ ITfRange *pRangeComposition)
{
    m_textLayoutSink._pContextDocument = pContextDocument;
    m_textLayoutSink._pRangeComposition = pRangeComposition;
    m_textLayoutSink._tfEditCookie = ec;

    wil::com_ptr<ITfSource> source;
    RETURN_IF_FAILED(pContextDocument->QueryInterface(IID_PPV_ARGS(&source)));
    RETURN_IF_FAILED(source->AdviseSink(IID_ITfTextLayoutSink, (ITfTextLayoutSink *)this, &m_textLayoutSink._dwCookieTextLayoutSink));

    return S_OK;
}

VOID CWindowsIME::_EndLayout()
{
    m_textLayoutSink._pContextDocument.reset();
    m_textLayoutSink._pRangeComposition.reset();
    m_textLayoutSink._tfEditCookie = TF_INVALID_EDIT_COOKIE;;
}

//+---------------------------------------------------------------------------
//
// ITfTextLayoutSink::OnLayoutChange
//
//----------------------------------------------------------------------------

IFACEMETHODIMP CWindowsIME::OnLayoutChange(_In_ ITfContext *pContext, TfLayoutCode lcode, _In_ ITfContextView *pContextView)
{
    // we're interested in only document context.
    if (pContext != m_textLayoutSink._pContextDocument.get())
    {
        return S_OK;
    }

    switch (lcode)
    {
    case TF_LC_CHANGE:
        _SubmitEditSessionTask(pContext, [&](TfEditCookie ec) -> HRESULT {
                RECT rc = {};
                BOOL isClipped = {};
                if (SUCCEEDED_LOG(pContextView->GetTextExt(ec, m_textLayoutSink._pRangeComposition.get(), &rc, &isClipped)))
                {
                    m_candidateListView->_LayoutChangeNotification(&rc);
                }
                return S_OK;
            }, TF_ES_SYNC | TF_ES_READ);
        break;

    case TF_LC_DESTROY:
        m_candidateListView->_LayoutDestroyNotification();
        break;

    }
    return S_OK;
}

HRESULT CWindowsIME::_GetTextExt(TfEditCookie ec, _Out_ RECT *lpRect)
{
    if (!m_textLayoutSink._pContextDocument || !m_textLayoutSink._pRangeComposition)
    {
        return E_FAIL;
    }

    BOOL isClipped = TRUE;
    wil::com_ptr<ITfContextView> pContextView;
    RETURN_IF_FAILED(m_textLayoutSink._pContextDocument->GetActiveView(&pContextView));
    RETURN_IF_FAILED(pContextView->GetTextExt(ec, m_textLayoutSink._pRangeComposition.get(), lpRect, &isClipped));
    return S_OK;
}

// CTfTextLayoutSink::CTfTextLayoutSink(_In_ IInternalFrameworkService *pTextService)
// {
//     _pTextService = pTextService;
// //    _pTextService->AddRef();
// 
//     _pRangeComposition = nullptr;
//     _pContextDocument = nullptr;
// //    _tfEditCookie = TF_INVALID_EDIT_COOKIE;
// 
//     _dwCookieTextLayoutSink = TF_INVALID_COOKIE;
// 
//     _refCount = 1;
// 
//     DllAddRef();
// }
// 
// CTfTextLayoutSink::~CTfTextLayoutSink()
// {
// //    if (_pTextService)
// //    {
// //        _pTextService->Release();
// //    }
// 
//     DllRelease();
// }
// 
// STDAPI CTfTextLayoutSink::QueryInterface(REFIID riid, _Outptr_ void **ppvObj)
// {
//     if (ppvObj == nullptr)
//     {
//         return E_INVALIDARG;
//     }
// 
//     *ppvObj = nullptr;
// 
//     if (IsEqualIID(riid, IID_IUnknown) ||
//         IsEqualIID(riid, IID_ITfTextLayoutSink))
//     {
//         *ppvObj = (ITfTextLayoutSink *)this;
//     }
// 
//     if (*ppvObj)
//     {
//         AddRef();
//         return S_OK;
//     }
// 
//     return E_NOINTERFACE;
// }
// 
// STDAPI_(ULONG) CTfTextLayoutSink::AddRef()
// {
//     return ++_refCount;
// }
// 
// STDAPI_(ULONG) CTfTextLayoutSink::Release()
// {
//     LONG cr = --_refCount;
// 
//     assert(_refCount >= 0);
// 
//     if (_refCount == 0)
//     {
//         delete this;
//     }
// 
//     return cr;
// }
// 
// //+---------------------------------------------------------------------------
// //
// // ITfTextLayoutSink::OnLayoutChange
// //
// //----------------------------------------------------------------------------
// 
// STDAPI CTfTextLayoutSink::OnLayoutChange(_In_ ITfContext *pContext, TfLayoutCode lcode, _In_ ITfContextView *pContextView)
// {
//     // we're interested in only document context.
//     if (pContext != _pContextDocument)
//     {
//         return S_OK;
//     }
// 
//     switch (lcode)
//     {
//     case TF_LC_CHANGE:
//         {
// //            CGetTextExtentEditSession* pEditSession = nullptr;
// //            pEditSession = new (std::nothrow) CGetTextExtentEditSession(_pTextService, pContext, pContextView, _pRangeComposition, this);
// //            if (nullptr != (pEditSession))
// //            {
// //                HRESULT hr = S_OK;
// //                pContext->RequestEditSession(_pTextService->_GetClientId(), pEditSession, TF_ES_SYNC | TF_ES_READ, &hr);
// //
// //                pEditSession->Release();
// //            }
// 
//             _pTextService->_SubmitEditSessionTask(pContext, [&](TfEditCookie ec) -> HRESULT {
//                     RECT rc = {};
//                     BOOL isClipped = {};
//                     if (SUCCEEDED_LOG(pContextView->GetTextExt(ec, _pRangeComposition, &rc, &isClipped)))
//                     {
//                         _LayoutChangeNotification(&rc);
//                     }
//                     return S_OK;
//                 }, TF_ES_SYNC | TF_ES_READ);
//         }
//         break;
// 
//     case TF_LC_DESTROY:
//         _LayoutDestroyNotification();
//         break;
// 
//     }
//     return S_OK;
// }
// 
// HRESULT CTfTextLayoutSink::_StartLayout(_In_ ITfContext *pContextDocument, TfEditCookie ec, _In_ ITfRange *pRangeComposition)
// {
//     _pContextDocument = pContextDocument;
//     _pContextDocument->AddRef();
// 
//     _pRangeComposition = pRangeComposition;
//     _pRangeComposition->AddRef();
// 
//     _tfEditCookie = ec;
// 
//     return _AdviseTextLayoutSink();
// }
// 
// VOID CTfTextLayoutSink::_EndLayout()
// {
//     if (_pRangeComposition)
//     {
//         _pRangeComposition->Release();
//         _pRangeComposition = nullptr;
//     }
// 
//     if (_pContextDocument)
//     {
//         _UnadviseTextLayoutSink();
//         _pContextDocument->Release();
//         _pContextDocument = nullptr;
//     }
// }
// 
// HRESULT CTfTextLayoutSink::_AdviseTextLayoutSink()
// {
//     HRESULT hr = S_OK;
//     ITfSource* pSource = nullptr;
// 
//     hr = _pContextDocument->QueryInterface(IID_ITfSource, (void **)&pSource);
//     if (FAILED(hr))
//     {
//         return hr;
//     }
// 
//     hr = pSource->AdviseSink(IID_ITfTextLayoutSink, (ITfTextLayoutSink *)this, &_dwCookieTextLayoutSink);
//     if (FAILED(hr))
//     {
//         pSource->Release();
//         return hr;
//     }
// 
//     pSource->Release();
// 
//     return hr;
// }
// 
// HRESULT CTfTextLayoutSink::_UnadviseTextLayoutSink()
// {
//     HRESULT hr = S_OK;
//     ITfSource* pSource = nullptr;
// 
//     if (nullptr == _pContextDocument)
//     {
//         return E_FAIL;
//     }
// 
//     hr = _pContextDocument->QueryInterface(IID_ITfSource, (void **)&pSource);
//     if (FAILED(hr))
//     {
//         return hr;
//     }
// 
//     hr = pSource->UnadviseSink(_dwCookieTextLayoutSink);
//     if (FAILED(hr))
//     {
//         pSource->Release();
//         return hr;
//     }
// 
//     pSource->Release();
// 
//     return hr;
// }
// 
// HRESULT CTfTextLayoutSink::_GetTextExt(TfEditCookie ec, _Out_ RECT *lpRect)
// {
//     HRESULT hr = S_OK;
//     BOOL isClipped = TRUE;
//     ITfContextView* pContextView = nullptr;
// 
//     hr = _pContextDocument->GetActiveView(&pContextView);
//     if (FAILED(hr))
//     {
//         return hr;
//     }
// 
//     if (FAILED(hr = pContextView->GetTextExt(ec, _pRangeComposition, lpRect, &isClipped)))
//     {
//         return hr;
//     }
// 
//     pContextView->Release();
// 
//     return S_OK;
// }
