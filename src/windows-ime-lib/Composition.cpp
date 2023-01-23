// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "Globals.h"
#include "WindowsIME.h"
#include "CompositionBuffer.h"

//+---------------------------------------------------------------------------
//
// ITfCompositionSink::OnCompositionTerminated
//
// Callback for ITfCompositionSink.  The system calls this method whenever
// someone other than this service ends a composition.
//----------------------------------------------------------------------------

STDAPI CWindowsIME::OnCompositionTerminated(TfEditCookie ecWrite, _In_ ITfComposition *pComposition)
{
    auto activity = WindowsImeLibTelemetry::ITfCompositionSink_OnCompositionTerminated();

    // Clear dummy composition
    m_compositionBuffer->_RemoveDummyCompositionForComposing(ecWrite, pComposition);

    // Clear display attribute and end composition, _EndComposition will release composition for us
    // ITfContext* pContext = _pContext;
    auto pContext = m_compositionBuffer->GetContext();
//    if (pContext)
//    {
//        pContext->AddRef();
//    }

    _pCompositionProcessorEngine->EndComposition(pContext.get());

    _pCompositionProcessorEngine->_DeleteCandidateList(FALSE, pContext.get());

//    if (pContext)
//    {
//        pContext->Release();
//        pContext = nullptr;
//    }

    activity.Stop();
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _AddComposingAndChar
//
//----------------------------------------------------------------------------

HRESULT CompositionBuffer::_AddComposingAndChar(TfEditCookie ec, _In_ ITfContext *pContext, const shared_wstring& pstrAddString)
{
    HRESULT hr = S_OK;

    ULONG fetched = 0;
    TF_SELECTION tfSelection = {};
    RETURN_IF_FAILED(pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &fetched));

    wil::com_ptr<ITfRange> selectionRange;
    selectionRange.attach(tfSelection.range);

    RETURN_HR_IF(S_FALSE, fetched != 1);

    //
    // make range start to selection
    //
    wil::com_ptr<ITfRange> pAheadSelection;
    RETURN_IF_FAILED(pContext->GetStart(ec, &pAheadSelection));
    RETURN_IF_FAILED(pAheadSelection->ShiftEndToRange(ec, tfSelection.range, TF_ANCHOR_START));

    wil::com_ptr<ITfRange> pRange;
    BOOL exist_composing = _FindComposingRange(ec, pContext, pAheadSelection.get(), &pRange);

    _SetInputString(ec, pContext, pRange.get(), pstrAddString, exist_composing);

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _AddCharAndFinalize
//
//----------------------------------------------------------------------------

HRESULT CompositionBuffer::_AddCharAndFinalize(TfEditCookie ec, _In_ ITfContext *pContext, const shared_wstring& pstrAddString)
{
    ULONG fetched = 0;
    TF_SELECTION tfSelection;
    RETURN_IF_FAILED(pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &fetched));

    wil::com_ptr<ITfRange> selectionRange;
    selectionRange.attach(tfSelection.range);

    RETURN_HR_IF(E_FAIL, fetched != 1);

    // we use SetText here instead of InsertTextAtSelection because we've already started a composition
    // we don't want to the app to adjust the insertion point inside our composition
    RETURN_IF_FAILED(selectionRange->SetText(ec, 0, pstrAddString->c_str(), (LONG)pstrAddString->length()));

    // update the selection, we'll make it an insertion point just past
    // the inserted text.
    RETURN_IF_FAILED(selectionRange->Collapse(ec, TF_ANCHOR_END));
    RETURN_IF_FAILED(pContext->SetSelection(ec, 1, &tfSelection));

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _FindComposingRange
//
//----------------------------------------------------------------------------

BOOL CompositionBuffer::_FindComposingRange(TfEditCookie ec, _In_ ITfContext *pContext, _In_ ITfRange *pSelection, _Outptr_result_maybenull_ ITfRange **ppRange)
{
    if (ppRange == nullptr)
    {
        return FALSE;
    }

    *ppRange = nullptr;

    // find GUID_PROP_COMPOSING
    ITfProperty* pPropComp = nullptr;
    IEnumTfRanges* enumComp = nullptr;

    HRESULT hr = pContext->GetProperty(GUID_PROP_COMPOSING, &pPropComp);
    if (FAILED(hr) || pPropComp == nullptr)
    {
        return FALSE;
    }

    hr = pPropComp->EnumRanges(ec, &enumComp, pSelection);
    if (FAILED(hr) || enumComp == nullptr)
    {
        pPropComp->Release();
        return FALSE;
    }

    BOOL isCompExist = FALSE;
    VARIANT var;
    ULONG  fetched = 0;

    while (enumComp->Next(1, ppRange, &fetched) == S_OK && fetched == 1)
    {
        hr = pPropComp->GetValue(ec, *ppRange, &var);
        if (hr == S_OK)
        {
            if (var.vt == VT_I4 && var.lVal != 0)
            {
                isCompExist = TRUE;
                break;
            }
        }
        (*ppRange)->Release();
        *ppRange = nullptr;
    }

    pPropComp->Release();
    enumComp->Release();

    return isCompExist;
}

//+---------------------------------------------------------------------------
//
// _SetInputString
//
//----------------------------------------------------------------------------

HRESULT CompositionBuffer::_SetInputString(TfEditCookie ec, _In_ ITfContext *pContext, _Out_opt_ ITfRange *pRange, const shared_wstring& pstrAddString, BOOL exist_composing)
{
    ITfRange* pRangeInsert = nullptr;
    if (!exist_composing)
    {
        _InsertAtSelection(ec, pContext, pstrAddString, &pRangeInsert);
        if (pRangeInsert == nullptr)
        {
            return S_OK;
        }
        pRange = pRangeInsert;
    }
    if (pRange != nullptr)
    {
        pRange->SetText(ec, 0, pstrAddString->c_str(), (LONG)pstrAddString->length());
    }

    _SetCompositionLanguage(ec, pContext);

    _SetCompositionDisplayAttributes(ec, pContext, _gaDisplayAttributeInput);

    // update the selection, we'll make it an insertion point just past
    // the inserted text.
    ITfRange* pSelection = nullptr;
    TF_SELECTION sel = {};

    if ((pRange != nullptr) && (pRange->Clone(&pSelection) == S_OK))
    {
        pSelection->Collapse(ec, TF_ANCHOR_END);

        sel.range = pSelection;
        sel.style.ase = TF_AE_NONE;
        sel.style.fInterimChar = FALSE;
        pContext->SetSelection(ec, 1, &sel);
        pSelection->Release();
    }

    if (pRangeInsert)
    {
        pRangeInsert->Release();
    }


    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _InsertAtSelection
//
//----------------------------------------------------------------------------

HRESULT CompositionBuffer::_InsertAtSelection(TfEditCookie ec, _In_ ITfContext *pContext, const shared_wstring& pstrAddString, _Outptr_ ITfRange **ppCompRange)
{
    RETURN_HR_IF(E_INVALIDARG, ppCompRange == nullptr);
    *ppCompRange = nullptr;

    wil::com_ptr<ITfInsertAtSelection> pias;
    RETURN_IF_FAILED(pContext->QueryInterface(IID_PPV_ARGS(&pias)));
    RETURN_IF_FAILED(pias->InsertTextAtSelection(ec, TF_IAS_QUERYONLY, pstrAddString->c_str(), (LONG)pstrAddString->length(), ppCompRange));

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _RemoveDummyCompositionForComposing
//
//----------------------------------------------------------------------------

HRESULT CompositionBuffer::_RemoveDummyCompositionForComposing(TfEditCookie ec, _In_ ITfComposition *pComposition)
{
    wil::com_ptr<ITfRange> range;

    if (pComposition)
    {
        RETURN_IF_FAILED(pComposition->GetRange(&range));
        RETURN_IF_FAILED(range->SetText(ec, 0, nullptr, 0));
    }

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _SetCompositionLanguage
//
//----------------------------------------------------------------------------

BOOL CompositionBuffer::_SetCompositionLanguage(TfEditCookie ec, _In_ ITfContext *pContext)
{
    HRESULT hr = S_OK;
    BOOL ret = TRUE;

    LANGID langidProfile = WindowsImeLib::g_processorFactory->GetConstantProvider()->GetLangID();

    ITfRange* pRangeComposition = nullptr;
    ITfProperty* pLanguageProperty = nullptr;

    // we need a range and the context it lives in
    hr = _pComposition->GetRange(&pRangeComposition);
    if (FAILED(hr))
    {
        ret = FALSE;
        goto Exit;
    }

    // get our the language property
    hr = pContext->GetProperty(GUID_PROP_LANGID, &pLanguageProperty);
    if (FAILED(hr))
    {
        ret = FALSE;
        goto Exit;
    }

    {
        VARIANT var = {};
        var.vt = VT_I4;   // we're going to set DWORD
        var.lVal = langidProfile;

        hr = pLanguageProperty->SetValue(ec, pRangeComposition, &var);
        if (FAILED(hr))
        {
            ret = FALSE;
            goto Exit;
        }
    }

    pLanguageProperty->Release();
    pRangeComposition->Release();

Exit:
    return ret;
}
