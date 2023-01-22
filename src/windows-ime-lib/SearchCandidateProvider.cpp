// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "SearchCandidateProvider.h"
#include "WindowsIME.h"
#include "TipCandidateList.h"
#include "TipCandidateString.h"

/*------------------------------------------------------------------------------

create instance of CSearchCandidateProvider

------------------------------------------------------------------------------*/
HRESULT CSearchCandidateProvider::CreateInstance(_Outptr_ ITfFnSearchCandidateProvider **ppobj, _In_ ITfTextInputProcessorEx *ptip, _In_ ISearchCandidateProviderOwner *owner)
{
    RETURN_HR_IF(E_INVALIDARG, ppobj == nullptr);
    *ppobj = nullptr;

    wil::com_ptr<ITfFnSearchCandidateProvider> provider;
    RETURN_IF_FAILED(Microsoft::WRL::MakeAndInitialize<CSearchCandidateProvider>(&provider, ptip, owner));
    RETURN_IF_FAILED(provider->QueryInterface(IID_PPV_ARGS(ppobj)));

    return S_OK;
}

// /*------------------------------------------------------------------------------
// 
// create instance of CSearchCandidateProvider
// 
// ------------------------------------------------------------------------------*/
// HRESULT CSearchCandidateProvider::CreateInstance(REFIID riid, _Outptr_ void **ppvObj, _In_ ITfTextInputProcessorEx *ptip, _In_ ISearchCandidateProviderOwner *owner)
// { 
//     RETURN_HR_IF(E_INVALIDARG, ppobj == nullptr);
//     *ppobj = nullptr;
// 
//     wil::com_ptr<ITfFnSearchCandidateProvider> provider;
//     RETURN_IF_FAILED(Microsoft::WRL::MakeAndInitialize<CSearchCandidateProvider>(&provider, ptip));
//     RETURN_IF_FAILED(provider->QueryInterface(riid, ppvObj));
// 
//     return S_OK;
// }

/*------------------------------------------------------------------------------

constructor of CSearchCandidateProvider

------------------------------------------------------------------------------*/
HRESULT CSearchCandidateProvider::RuntimeClassInitialize(_In_ ITfTextInputProcessorEx *ptip, _In_ ISearchCandidateProviderOwner *owner)
{
    RETURN_HR_IF(E_INVALIDARG, ptip == nullptr);

    _pTip = ptip;
    m_owner = owner;

    return S_OK;
}

/*------------------------------------------------------------------------------

destructor of CSearchCandidateProvider

------------------------------------------------------------------------------*/
CSearchCandidateProvider::~CSearchCandidateProvider(void)
{  
}

STDMETHODIMP CSearchCandidateProvider::GetDisplayName(_Out_ BSTR *pbstrName)
{
    if (pbstrName == nullptr)
    {
        return E_INVALIDARG;
    }

    *pbstrName = SysAllocString(L"SearchCandidateProvider");
    return  S_OK;
}

STDMETHODIMP CSearchCandidateProvider::GetSearchCandidates(BSTR bstrQuery, BSTR bstrApplicationID, _Outptr_result_maybenull_ ITfCandidateList **pplist)
{
    bstrApplicationID;bstrQuery;
    HRESULT hr = E_FAIL;
    *pplist = nullptr;

    if (nullptr == _pTip)
    {
        return hr;
    }

    auto pCompositionProcessorEngine = m_owner->GetCompositionProcessorEngine();
    if (!pCompositionProcessorEngine)
    {
        return hr;
    }

    std::vector<CCandidateListItem> candidateList;
    pCompositionProcessorEngine->GetCandidateList(&candidateList, TRUE, FALSE);

    int cCand = std::min(static_cast<int>(candidateList.size()), FAKECANDIDATENUMBER);
    if (0 < cCand)
    {
        hr = CTipCandidateList::CreateInstance(pplist, cCand);
        if (FAILED(hr))
        {
            return hr;
        }
        for (int iCand = 0; iCand < cCand; iCand++)
        {
            ITfCandidateString* pCandStr = nullptr;
            CTipCandidateString::CreateInstance(IID_ITfCandidateString, (void**)&pCandStr);

            ((CTipCandidateString*)pCandStr)->SetIndex(iCand);
            ((CTipCandidateString*)pCandStr)->SetString(candidateList.at(iCand)._ItemString.Get(), candidateList.at(iCand)._ItemString.GetLength());

            ((CTipCandidateList*)(*pplist))->SetCandidate(&pCandStr);
        }
    }
    hr = S_OK;

    return hr;
}

/*------------------------------------------------------------------------------

set result
(ITfFnSearchCandidateProvider method)

------------------------------------------------------------------------------*/
STDMETHODIMP CSearchCandidateProvider::SetResult(BSTR bstrQuery, BSTR bstrApplicationID, BSTR bstrResult)
{
    bstrQuery;bstrApplicationID;bstrResult;

    return E_NOTIMPL;
}

