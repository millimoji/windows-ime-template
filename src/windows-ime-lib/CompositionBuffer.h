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

    virtual HRESULT _StartLayoutTracking(_In_ ITfContext *pContextDocument, TfEditCookie ec, _In_ ITfRange *pRangeComposition) = 0;
    virtual HRESULT _EndLayoutTracking() = 0;
};

struct ICompositionBufferInternal
{
    virtual ~ICompositionBufferInternal() {}

    virtual void FlushTasks() = 0;
    virtual std::shared_ptr<WindowsImeLib::IWindowsIMECompositionBuffer> GetClientInterface() = 0;
    virtual wil::com_ptr<ITfComposition> GetComposition() = 0;
    virtual wil::com_ptr<ITfContext> GetCompositionContext() = 0;
    virtual bool _IsComposing() = 0;
    virtual void SaveWorkingContext(_In_ ITfContext *pContext) = 0;
    virtual HRESULT _TerminateCompositionInternal() = 0;
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
    // IWindowsIMECompositionBuffer
    HRESULT _StartComposition() override;
    HRESULT _TerminateComposition() override;
    HRESULT _AddComposingAndChar(const shared_wstring& pstrAddString) override;
    HRESULT _AddCharAndFinalize(const shared_wstring& pstrAddString) override;
    HRESULT _RemoveDummyCompositionForComposing() override;

private: // ICompositionBufferInternal
    void FlushTasks() override;
    std::shared_ptr<WindowsImeLib::IWindowsIMECompositionBuffer> GetClientInterface() override { return std::static_pointer_cast<WindowsImeLib::IWindowsIMECompositionBuffer>(shared_from_this()); }
    wil::com_ptr<ITfComposition> GetComposition() override { return m_currentComposition; }
    wil::com_ptr<ITfContext> GetCompositionContext() override { return m_currentCompositionContext; }
    bool _IsComposing() override { return m_isComposing; }
    void SaveWorkingContext(_In_ ITfContext *pContext) override { m_workingContext = pContext; }
    HRESULT _TerminateCompositionInternal() override;

    HRESULT _InsertAtSelection(TfEditCookie ec, _In_ ITfContext *pContext, const shared_wstring& pstrAddString, _Outptr_ ITfRange **ppCompRange);
    BOOL _FindComposingRange(TfEditCookie ec, _In_ ITfContext *pContext, _In_ ITfRange *pSelection, _Outptr_result_maybenull_ ITfRange **ppRange);
    HRESULT _SetInputString(TfEditCookie ec, _In_ ITfContext *pContext, _Out_opt_ ITfRange *pRange, const shared_wstring& pstrAddString, BOOL exist_composing);
    // function for the language property
    BOOL _SetCompositionLanguage(TfEditCookie ec, _In_ ITfContext *pContext);

    // function for the display attribute
    HRESULT _ClearCompositionDisplayAttributes(TfEditCookie ec, _In_ ITfContext *pContext);
    BOOL _SetCompositionDisplayAttributes(TfEditCookie ec, _In_ ITfContext *pContext, TfGuidAtom gaDisplayAttribute);

private:
    void _SaveCompositionAndContext(_In_ ITfComposition *pComposition, _In_ ITfContext *pContext)
    {
        m_currentComposition = pComposition;
        m_currentCompositionContext = pContext;
    }

private:
    ICompositionBufferOwner* m_framework = nullptr;
    std::shared_ptr<WindowsImeLib::IWindowsIMECandidateListView> _pCandidateListUIPresenter;

    TfClientId _tfClientId = TF_CLIENTID_NULL;
    TfGuidAtom _gaDisplayAttributeInput = {};
    bool m_isComposing = false;

    std::list<wil::com_ptr<CEditSessionTask>> m_listTasks;

    wil::com_ptr<ITfContext> m_currentCompositionContext;
    wil::com_ptr<ITfComposition> m_currentComposition;
    wil::com_ptr<ITfContext> m_workingContext;
};
