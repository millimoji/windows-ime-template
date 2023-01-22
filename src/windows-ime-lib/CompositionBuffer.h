// Copyright (c) millimoji@gmail.com
#pragma once

#include "Private.h"
#include "Globals.h"
#include "../WindowsImeLib.h"
#include "EditSession.h"

struct ICompositionBufferOwner
{
    virtual ~ICompositionBufferOwner() {}

    virtual ITfCompositionSink* GetCompositionSink() = 0;
    virtual std::shared_ptr<WindowsImeLib::ICompositionProcessorEngine> GetCompositionProcessorEngine() = 0;
    virtual HRESULT _SubmitEditSessionTask(_In_ ITfContext* context, const std::function<HRESULT(TfEditCookie ec)>& editSesisonTask, DWORD tfEsFlags) = 0;
};

struct ICompositionBufferInternal
{
    virtual ~ICompositionBufferInternal() {}

    virtual std::shared_ptr<WindowsImeLib::IWindowsIMECompositionBuffer> GetClientInterface() = 0;
    virtual wil::com_ptr<ITfComposition> GetComposition() = 0;
    virtual wil::com_ptr<ITfContext> GetContext() = 0;
    virtual bool _IsComposing() = 0;
    virtual HRESULT _RemoveDummyCompositionForComposing(TfEditCookie ec, _In_ ITfComposition *pComposition) = 0;
};

class CompositionBuffer :
    public WindowsImeLib::IWindowsIMECompositionBuffer,
    public ICompositionBufferInternal,
    public std::enable_shared_from_this<CompositionBuffer>
{
public:
    CompositionBuffer(
        ICompositionBufferOwner* framework,
        const std::shared_ptr<WindowsImeLib::IWindowsIMECandidateListView>& candidateListView,
        const TfClientId& tfClientId,
        const TfGuidAtom& gaDisplayAttributeInput
    ) :
        m_framework(framework),
        _pCandidateListUIPresenter(candidateListView),
        _tfClientId(tfClientId),
        _gaDisplayAttributeInput(gaDisplayAttributeInput)
    {}
    virtual ~CompositionBuffer() {}

private:
    // functions for the composition object.
    void _TerminateComposition(TfEditCookie ec, _In_ ITfContext *pContext, BOOL isCalledFromDeactivate = FALSE) override;

    // key event handlers for composition/candidate/phrase common objects.
//     HRESULT _HandleComplete(TfEditCookie ec, _In_ ITfContext *pContext);
//    HRESULT _HandleCancel(TfEditCookie ec, _In_ ITfContext* pContext);
    // key event handlers for composition object.
//     HRESULT _HandleCompositionInput(TfEditCookie ec, _In_ ITfContext* pContext, WCHAR wch) override;
//     HRESULT _HandleCompositionFinalize(TfEditCookie ec, _In_ ITfContext* pContext, BOOL fCandidateList) override;

//     HRESULT _HandleCompositionConvert(TfEditCookie ec, _In_ ITfContext* pContext, BOOL isWildcardSearch) override;
//     HRESULT _HandleCompositionBackspace(TfEditCookie ec, _In_ ITfContext* pContext) override;
//     HRESULT _HandleCompositionArrowKey(TfEditCookie ec, _In_ ITfContext* pContext, KEYSTROKE_FUNCTION keyFunction) override;
//     HRESULT _HandleCompositionPunctuation(TfEditCookie ec, _In_ ITfContext* pContext, WCHAR wch) override;
//    HRESULT _HandleCompositionDoubleSingleByte(TfEditCookie ec, _In_ ITfContext* pContext, WCHAR wch) override;
    // key event handlers for candidate object.
//     HRESULT _HandleCandidateFinalize(TfEditCookie ec, _In_ ITfContext* pContext) override;
//     HRESULT _HandleCandidateConvert(TfEditCookie ec, _In_ ITfContext* pContext) override;
//     HRESULT _HandleCandidateArrowKey(TfEditCookie ec, _In_ ITfContext* pContext, _In_ KEYSTROKE_FUNCTION keyFunction) override;
//     HRESULT _HandleCandidateSelectByNumber(TfEditCookie ec, _In_ ITfContext* pContext, _In_ UINT uCode) override;
    // key event handlers for phrase object.
//     HRESULT _HandlePhraseFinalize(TfEditCookie ec, _In_ ITfContext* pContext) override;
//     HRESULT _HandlePhraseArrowKey(TfEditCookie ec, _In_ ITfContext* pContext, _In_ KEYSTROKE_FUNCTION keyFunction) override;
//     HRESULT _HandlePhraseSelectByNumber(TfEditCookie ec, _In_ ITfContext* pContext, _In_ UINT uCode) override;

    // functions for the composition object.
//     HRESULT _HandleCompositionInputWorker(_In_ WindowsImeLib::ICompositionProcessorEngine *pCompositionProcessorEngine, TfEditCookie ec, _In_ ITfContext *pContext) override;
//     HRESULT _CreateAndStartCandidate(_In_ WindowsImeLib::ICompositionProcessorEngine *pCompositionProcessorEngine, TfEditCookie ec, _In_ ITfContext *pContext) override;
//     HRESULT _HandleCandidateWorker(TfEditCookie ec, _In_ ITfContext *pContext) override;

    HRESULT _StartComposition(TfEditCookie ec, _In_ ITfContext *_pContext) override;

    HRESULT _AddComposingAndChar(TfEditCookie ec, _In_ ITfContext *pContext, _In_ CStringRange *pstrAddString) override;
    HRESULT _AddCharAndFinalize(TfEditCookie ec, _In_ ITfContext *pContext, _In_ CStringRange *pstrAddString) override;

    HRESULT _InsertAtSelection(TfEditCookie ec, _In_ ITfContext *pContext, _In_ CStringRange *pstrAddString, _Outptr_ ITfRange **ppCompRange) override;

    BOOL _FindComposingRange(TfEditCookie ec, _In_ ITfContext *pContext, _In_ ITfRange *pSelection, _Outptr_result_maybenull_ ITfRange **ppRange);
    HRESULT _SetInputString(TfEditCookie ec, _In_ ITfContext *pContext, _Out_opt_ ITfRange *pRange, _In_ CStringRange *pstrAddString, BOOL exist_composing);
    // function for the language property
    BOOL _SetCompositionLanguage(TfEditCookie ec, _In_ ITfContext *pContext);

    // function for the display attribute
    void _ClearCompositionDisplayAttributes(TfEditCookie ec, _In_ ITfContext *pContext);
    BOOL _SetCompositionDisplayAttributes(TfEditCookie ec, _In_ ITfContext *pContext, TfGuidAtom gaDisplayAttribute);

//    BOOL _IsRangeCovered(TfEditCookie ec, _In_ ITfRange *pRangeTest, _In_ ITfRange *pRangeCover) override;
//    VOID _DeleteCandidateList(BOOL isForce, _In_opt_ ITfContext *pContext) override;

//    TfClientId GetClientId() override { return _tfClientId; }

private: // ICompositionBufferInternal
    std::shared_ptr<WindowsImeLib::IWindowsIMECompositionBuffer> GetClientInterface() override { return std::static_pointer_cast<WindowsImeLib::IWindowsIMECompositionBuffer>(shared_from_this()); }
    wil::com_ptr<ITfComposition> GetComposition() override { return _pComposition; }
    wil::com_ptr<ITfContext> GetContext() override { return _pContext; }
    bool _IsComposing() override { return !!_pComposition; }
    HRESULT _RemoveDummyCompositionForComposing(TfEditCookie ec, _In_ ITfComposition *pComposition) override;

private:
    HRESULT _SubmitEditSessionTask(_In_ ITfContext* context, const std::function<HRESULT(TfEditCookie ec)>& editSesisonTask, DWORD tfEsFlags) override
    {
        return m_framework->_SubmitEditSessionTask(context, editSesisonTask, tfEsFlags);
    }
private:
    void _SetComposition(_In_ ITfComposition *pComposition) { _pComposition = pComposition; }
    void _SaveCompositionContext(_In_ ITfContext *pContext);

private:
    ICompositionBufferOwner* m_framework = nullptr;
    std::shared_ptr<WindowsImeLib::IWindowsIMECandidateListView> _pCandidateListUIPresenter;

    TfClientId _tfClientId = TF_CLIENTID_NULL;
    TfGuidAtom _gaDisplayAttributeInput = {};

    wil::com_ptr<ITfContext> _pContext;
    wil::com_ptr<ITfComposition> _pComposition;
};
