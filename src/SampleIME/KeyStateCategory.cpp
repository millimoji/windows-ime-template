// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "pch.h"
//#include "Private.h"
#include "KeyStateCategory.h"

CKeyStateCategoryFactory* CKeyStateCategoryFactory::_instance;

CKeyStateCategoryFactory::CKeyStateCategoryFactory()
{
    _instance = nullptr;
}

CKeyStateCategoryFactory* CKeyStateCategoryFactory::Instance()
{
    if (nullptr == _instance)
    {
        _instance = new (std::nothrow) CKeyStateCategoryFactory();
    }

    return _instance;
}

CKeyStateCategory* CKeyStateCategoryFactory::MakeKeyStateCategory(
    KEYSTROKE_CATEGORY keyCategory,
    _In_ WindowsImeLib::IWindowsIMECompositionBuffer* pTextService,
    const std::shared_ptr<WindowsImeLib::IWindowsIMECandidateListView>& pCandidateListUIPresenter,
    const std::shared_ptr<CompositionProcessorEngine>& pCompositionProcessorEngine)
{
    CKeyStateCategory* pKeyState = nullptr;

    switch (keyCategory)
    {
    case CATEGORY_NONE:
        pKeyState = new (std::nothrow) CKeyStateNull(pTextService, pCandidateListUIPresenter, pCompositionProcessorEngine);
        break;

    case CATEGORY_COMPOSING:
        pKeyState = new (std::nothrow) CKeyStateComposing(pTextService, pCandidateListUIPresenter, pCompositionProcessorEngine);
        break;

    case CATEGORY_CANDIDATE:
        pKeyState = new (std::nothrow) CKeyStateCandidate(pTextService, pCandidateListUIPresenter, pCompositionProcessorEngine);
        break;

    case CATEGORY_PHRASE:
        pKeyState = new (std::nothrow) CKeyStatePhrase(pTextService, pCandidateListUIPresenter, pCompositionProcessorEngine);
        break;

    default:
        pKeyState = new (std::nothrow) CKeyStateNull(pTextService, pCandidateListUIPresenter, pCompositionProcessorEngine);
        break;
    }
    return pKeyState;
}

void CKeyStateCategoryFactory::Release()
{
    if (_instance)
    {
        delete _instance;
        _instance = nullptr;
    }
}

/*
class CKeyStateCategory
*/
CKeyStateCategory::CKeyStateCategory(
        _In_ WindowsImeLib::IWindowsIMECompositionBuffer* pTextService,
        const std::shared_ptr<WindowsImeLib::IWindowsIMECandidateListView>& pCandidateListUIPresenter,
        const std::shared_ptr<CompositionProcessorEngine>& pCompositionProcessorEngine) :
        _pTextService(pTextService),
        _pCandidateListUIPresenter(pCandidateListUIPresenter),
        _pCompositionProcessorEngine(pCompositionProcessorEngine)
{
}

CKeyStateCategory::~CKeyStateCategory(void)
{
}

HRESULT CKeyStateCategory::KeyStateHandler(KEYSTROKE_FUNCTION function, KeyHandlerEditSessionDTO dto)
{
    switch(function)
    {
    case FUNCTION_INPUT:
        return HandleKeyInput(dto);

    case FUNCTION_FINALIZE_TEXTSTORE_AND_INPUT:
        return HandleKeyFinalizeTextStoreAndInput(dto);

    case FUNCTION_FINALIZE_TEXTSTORE:
        return HandleKeyFinalizeTextStore(dto);

    case FUNCTION_FINALIZE_CANDIDATELIST_AND_INPUT:
        return HandleKeyFinalizeCandidatelistAndInput(dto);

    case FUNCTION_FINALIZE_CANDIDATELIST:
        return HandleKeyFinalizeCandidatelist(dto);

    case FUNCTION_CONVERT:
        return HandleKeyConvert(dto);

    case FUNCTION_CONVERT_WILDCARD:
        return HandleKeyConvertWildCard(dto);

    case FUNCTION_CANCEL:
        return HandleKeyCancel(dto);

    case FUNCTION_BACKSPACE:
        return HandleKeyBackspace(dto);

    case FUNCTION_MOVE_LEFT:
    case FUNCTION_MOVE_RIGHT:
            return HandleKeyArrow(dto);

    case FUNCTION_MOVE_UP:
    case FUNCTION_MOVE_DOWN:
    case FUNCTION_MOVE_PAGE_UP:
    case FUNCTION_MOVE_PAGE_DOWN:
    case FUNCTION_MOVE_PAGE_TOP:
    case FUNCTION_MOVE_PAGE_BOTTOM:
        return HandleKeyArrow(dto);

    case FUNCTION_DOUBLE_SINGLE_BYTE:
        return HandleKeyDoubleSingleByte(dto);

    case FUNCTION_PUNCTUATION:
        return HandleKeyPunctuation(dto);

    case FUNCTION_SELECT_BY_NUMBER:
        return HandleKeySelectByNumber(dto);

    }
    return E_INVALIDARG;
}

void CKeyStateCategory::Release()
{
    delete this;
}

// _HandleCompositionInput
HRESULT CKeyStateCategory::HandleKeyInput(KeyHandlerEditSessionDTO dto)
{
    dto;
    return E_NOTIMPL;
}

// HandleKeyFinalizeTextStore
HRESULT CKeyStateCategory::HandleKeyFinalizeTextStore(KeyHandlerEditSessionDTO dto)
{
    dto;
    return E_NOTIMPL;
}
// HandleKeyCompositionFinalizeTextStoreAndInput
HRESULT CKeyStateCategory::HandleKeyFinalizeTextStoreAndInput(KeyHandlerEditSessionDTO dto)
{
    dto;
    return E_NOTIMPL;
}

// HandleKeyCompositionFinalizeCandidatelistAndInput
HRESULT CKeyStateCategory::HandleKeyFinalizeCandidatelistAndInput(KeyHandlerEditSessionDTO dto)
{
    dto;
    return E_NOTIMPL;
}

// HandleKeyCompositionFinalizeCandidatelist
HRESULT CKeyStateCategory::HandleKeyFinalizeCandidatelist(KeyHandlerEditSessionDTO dto)
{
    dto;
    return E_NOTIMPL;
}

// HandleKeyConvert
HRESULT CKeyStateCategory::HandleKeyConvert(KeyHandlerEditSessionDTO dto)
{
    dto;
    return E_NOTIMPL;
}

// HandleKeyConvertWildCard
HRESULT CKeyStateCategory::HandleKeyConvertWildCard(KeyHandlerEditSessionDTO dto)
{
    dto;
    return E_NOTIMPL;
}

//_HandleCancel
HRESULT CKeyStateCategory::HandleKeyCancel(KeyHandlerEditSessionDTO dto)
{
    dto;
    return E_NOTIMPL;
}

//_HandleCompositionBackspace
HRESULT CKeyStateCategory::HandleKeyBackspace(KeyHandlerEditSessionDTO dto)
{
    dto;
    return E_NOTIMPL;
}

//_HandleCompositionArrowKey
HRESULT CKeyStateCategory::HandleKeyArrow(KeyHandlerEditSessionDTO dto)
{
    dto;
    return E_NOTIMPL;
}

//_HandleCompositionDoubleSingleByte
HRESULT CKeyStateCategory::HandleKeyDoubleSingleByte(KeyHandlerEditSessionDTO dto)
{
    dto;
    return E_NOTIMPL;
}

//_HandleCompositionPunctuation
HRESULT CKeyStateCategory::HandleKeyPunctuation(KeyHandlerEditSessionDTO dto)
{
    dto;
    return E_NOTIMPL;
}

HRESULT CKeyStateCategory::HandleKeySelectByNumber(KeyHandlerEditSessionDTO dto)
{
    dto;
    return E_NOTIMPL;
}

/*
class CKeyStateComposing
*/
CKeyStateComposing::CKeyStateComposing(
    _In_ WindowsImeLib::IWindowsIMECompositionBuffer* pTextService,
    const std::shared_ptr<WindowsImeLib::IWindowsIMECandidateListView>& pCandidateListUIPresenter,
    const std::shared_ptr<CompositionProcessorEngine>& pCompositionProcessorEngine) :
    CKeyStateCategory(pTextService, pCandidateListUIPresenter, pCompositionProcessorEngine)
{
}

HRESULT CKeyStateComposing::HandleKeyInput(KeyHandlerEditSessionDTO dto)
{
    return dto.owner->_SubmitEditSessionTask(dto.pContext, [this, dto](TfEditCookie ec, WindowsImeLib::IWindowsIMECompositionBuffer*) -> HRESULT
    {
        return _HandleCompositionInput(ec, dto.pContext, dto.wch);
    }, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE);
}

HRESULT CKeyStateComposing::HandleKeyFinalizeTextStoreAndInput(KeyHandlerEditSessionDTO dto)
{
    return dto.owner->_SubmitEditSessionTask(dto.pContext, [this, dto](TfEditCookie ec, WindowsImeLib::IWindowsIMECompositionBuffer*) -> HRESULT
    {
        _HandleCompositionFinalize(ec, dto.pContext, FALSE);
        return _HandleCompositionInput(ec, dto.pContext, dto.wch);
    }, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE);
}

HRESULT CKeyStateComposing::HandleKeyFinalizeTextStore(KeyHandlerEditSessionDTO dto)
{
    return dto.owner->_SubmitEditSessionTask(dto.pContext, [this, dto](TfEditCookie ec, WindowsImeLib::IWindowsIMECompositionBuffer*) -> HRESULT
    {
        return _HandleCompositionFinalize(ec, dto.pContext, FALSE);
    }, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE);
}

HRESULT CKeyStateComposing::HandleKeyFinalizeCandidatelistAndInput(KeyHandlerEditSessionDTO dto)
{
    return dto.owner->_SubmitEditSessionTask(dto.pContext, [this, dto](TfEditCookie ec, WindowsImeLib::IWindowsIMECompositionBuffer*) -> HRESULT
    {
        _HandleCompositionFinalize(ec, dto.pContext, TRUE);
        return _HandleCompositionInput(ec, dto.pContext, dto.wch);
    }, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE);
}

HRESULT CKeyStateComposing::HandleKeyFinalizeCandidatelist(KeyHandlerEditSessionDTO dto)
{
    return dto.owner->_SubmitEditSessionTask(dto.pContext, [this, dto](TfEditCookie ec, WindowsImeLib::IWindowsIMECompositionBuffer*) -> HRESULT
    {
        return _HandleCompositionFinalize(ec, dto.pContext, TRUE);
    }, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE);
}

HRESULT CKeyStateComposing::HandleKeyConvert(KeyHandlerEditSessionDTO dto)
{
    return dto.owner->_SubmitEditSessionTask(dto.pContext, [this, dto](TfEditCookie ec, WindowsImeLib::IWindowsIMECompositionBuffer*) -> HRESULT
    {
        return _HandleCompositionConvert(ec, dto.pContext, FALSE);
    }, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE);
}

HRESULT CKeyStateComposing::HandleKeyConvertWildCard(KeyHandlerEditSessionDTO dto)
{
    return dto.owner->_SubmitEditSessionTask(dto.pContext, [this, dto](TfEditCookie ec, WindowsImeLib::IWindowsIMECompositionBuffer*) -> HRESULT
    {
        return _HandleCompositionConvert(ec, dto.pContext, TRUE);
    }, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE);
}

HRESULT CKeyStateComposing::HandleKeyCancel(KeyHandlerEditSessionDTO dto)
{
    return dto.owner->_SubmitEditSessionTask(dto.pContext, [this, dto](TfEditCookie ec, WindowsImeLib::IWindowsIMECompositionBuffer*) -> HRESULT
    {
        return _HandleCancel(ec, dto.pContext);
    }, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE);
}

HRESULT CKeyStateComposing::HandleKeyBackspace(KeyHandlerEditSessionDTO dto)
{
    return dto.owner->_SubmitEditSessionTask(dto.pContext, [this, dto](TfEditCookie ec, WindowsImeLib::IWindowsIMECompositionBuffer*) -> HRESULT
    {
        return _HandleCompositionBackspace(ec, dto.pContext);
    }, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE);
}

HRESULT CKeyStateComposing::HandleKeyArrow(KeyHandlerEditSessionDTO dto)
{
    return dto.owner->_SubmitEditSessionTask(dto.pContext, [this, dto](TfEditCookie ec, WindowsImeLib::IWindowsIMECompositionBuffer*) -> HRESULT
    {
        return _HandleCompositionArrowKey(ec, dto.pContext, dto.arrowKey);
    }, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE);
}

HRESULT CKeyStateComposing::HandleKeyDoubleSingleByte(KeyHandlerEditSessionDTO dto)
{
    return dto.owner->_SubmitEditSessionTask(dto.pContext, [this, dto](TfEditCookie ec, WindowsImeLib::IWindowsIMECompositionBuffer*) -> HRESULT
    {
        return _HandleCompositionDoubleSingleByte(ec, dto.pContext, dto.wch);
    }, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE);
}

HRESULT CKeyStateComposing::HandleKeyPunctuation(KeyHandlerEditSessionDTO dto)
{
    return dto.owner->_SubmitEditSessionTask(dto.pContext, [this, dto](TfEditCookie ec, WindowsImeLib::IWindowsIMECompositionBuffer*) -> HRESULT
    {
        return _HandleCompositionPunctuation(ec, dto.pContext, dto.wch);
    }, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE);
}

/*
class CKeyStateCandidate
*/
CKeyStateCandidate::CKeyStateCandidate(
    _In_ WindowsImeLib::IWindowsIMECompositionBuffer* pTextService,
    const std::shared_ptr<WindowsImeLib::IWindowsIMECandidateListView>& pCandidateListUIPresenter,
    const std::shared_ptr<CompositionProcessorEngine>& pCompositionProcessorEngine) :
    CKeyStateCategory(pTextService, pCandidateListUIPresenter, pCompositionProcessorEngine)
{
}

// _HandleCandidateInput
HRESULT CKeyStateCandidate::HandleKeyFinalizeCandidatelist(KeyHandlerEditSessionDTO dto)
{
    return dto.owner->_SubmitEditSessionTask(dto.pContext, [this, dto](TfEditCookie ec, WindowsImeLib::IWindowsIMECompositionBuffer*) -> HRESULT
    {
        return _HandleCandidateFinalize(ec, dto.pContext);
    }, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE);
}

// HandleKeyFinalizeCandidatelistAndInput
HRESULT CKeyStateCandidate::HandleKeyFinalizeCandidatelistAndInput(KeyHandlerEditSessionDTO dto)
{
    return dto.owner->_SubmitEditSessionTask(dto.pContext, [this, dto](TfEditCookie ec, WindowsImeLib::IWindowsIMECompositionBuffer*) -> HRESULT
    {
        _HandleCandidateFinalize(ec, dto.pContext);
        return _HandleCompositionInput(ec, dto.pContext, dto.wch);
    }, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE);
}

//_HandleCandidateConvert
HRESULT CKeyStateCandidate::HandleKeyConvert(KeyHandlerEditSessionDTO dto)
{
    return dto.owner->_SubmitEditSessionTask(dto.pContext, [this, dto](TfEditCookie ec, WindowsImeLib::IWindowsIMECompositionBuffer*) -> HRESULT
    {
        return _HandleCandidateConvert(ec, dto.pContext);
    }, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE);
}

//_HandleCancel
HRESULT CKeyStateCandidate::HandleKeyCancel(KeyHandlerEditSessionDTO dto)    
{
    return dto.owner->_SubmitEditSessionTask(dto.pContext, [this, dto](TfEditCookie ec, WindowsImeLib::IWindowsIMECompositionBuffer*) -> HRESULT
    {
        return _HandleCancel(ec, dto.pContext);
    }, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE);
}

//_HandleCandidateArrowKey
HRESULT CKeyStateCandidate::HandleKeyArrow(KeyHandlerEditSessionDTO dto)
{
    return dto.owner->_SubmitEditSessionTask(dto.pContext, [this, dto](TfEditCookie ec, WindowsImeLib::IWindowsIMECompositionBuffer*) -> HRESULT
    {
        return _HandleCandidateArrowKey(ec, dto.pContext, dto.arrowKey);
    }, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE);
}

//_HandleCandidateSelectByNumber
HRESULT CKeyStateCandidate::HandleKeySelectByNumber(KeyHandlerEditSessionDTO dto)
{
    return dto.owner->_SubmitEditSessionTask(dto.pContext, [this, dto](TfEditCookie ec, WindowsImeLib::IWindowsIMECompositionBuffer*) -> HRESULT
    {
        return _HandleCandidateSelectByNumber(ec, dto.pContext, dto.code);
    }, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE);
}

/*
class CKeyStatePhrase
*/

CKeyStatePhrase::CKeyStatePhrase(
    _In_ WindowsImeLib::IWindowsIMECompositionBuffer* pTextService,
    const std::shared_ptr<WindowsImeLib::IWindowsIMECandidateListView>& pCandidateListUIPresenter,
    const std::shared_ptr<CompositionProcessorEngine>& pCompositionProcessorEngine) :
    CKeyStateCategory(pTextService, pCandidateListUIPresenter, pCompositionProcessorEngine)
{
}

//HandleKeyFinalizeCandidatelist
HRESULT CKeyStatePhrase::HandleKeyFinalizeCandidatelist(KeyHandlerEditSessionDTO dto)
{
    return dto.owner->_SubmitEditSessionTask(dto.pContext, [this, dto](TfEditCookie ec, WindowsImeLib::IWindowsIMECompositionBuffer*) -> HRESULT
    {
        return _HandlePhraseFinalize(ec, dto.pContext);
    }, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE);
}

//HandleKeyCancel
HRESULT CKeyStatePhrase::HandleKeyCancel(KeyHandlerEditSessionDTO dto)
{
    return dto.owner->_SubmitEditSessionTask(dto.pContext, [this, dto](TfEditCookie ec, WindowsImeLib::IWindowsIMECompositionBuffer*) -> HRESULT
    {
        return _HandleCancel(ec, dto.pContext);
    }, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE);
}

//HandleKeyArrow
HRESULT CKeyStatePhrase::HandleKeyArrow(KeyHandlerEditSessionDTO dto)
{
    return dto.owner->_SubmitEditSessionTask(dto.pContext, [this, dto](TfEditCookie ec, WindowsImeLib::IWindowsIMECompositionBuffer*) -> HRESULT
    {
        return _HandlePhraseArrowKey(ec, dto.pContext, dto.arrowKey);
    }, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE);
}

//HandleKeySelectByNumber
HRESULT CKeyStatePhrase::HandleKeySelectByNumber(KeyHandlerEditSessionDTO dto)
{
    return dto.owner->_SubmitEditSessionTask(dto.pContext, [this, dto](TfEditCookie ec, WindowsImeLib::IWindowsIMECompositionBuffer*) -> HRESULT
    {
        return _HandlePhraseSelectByNumber(ec, dto.pContext, dto.code);
    }, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE);
}

