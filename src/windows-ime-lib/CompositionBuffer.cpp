// Copyright (c) millimoji@gmail.com

#include "Private.h"
#include "Globals.h"
#include "../WindowsImeLib.h"

class CompositionBuffer :
    public WindowsImeLib::IWindowsIMECompositionBuffer,
    public std::enable_shared_from_this<CompositionBuffer>
{
public:
    CompositionBuffer(WindowsImeLib::IWindowsIMECompositionBuffer* windowsIme)
        : m_windowsIme(windowsIme)
    {}
    virtual ~CompositionBuffer() {}

private:
    // key event handlers for composition/candidate/phrase common objects.
    HRESULT _HandleCancel(TfEditCookie ec, _In_ ITfContext* pContext) override {
        return m_windowsIme->_HandleCancel(ec, pContext);
    }
    // key event handlers for composition object.
    HRESULT _HandleCompositionInput(TfEditCookie ec, _In_ ITfContext* pContext, WCHAR wch) override {
        return m_windowsIme->_HandleCompositionInput(ec, pContext, wch);
    }
    HRESULT _HandleCompositionFinalize(TfEditCookie ec, _In_ ITfContext* pContext, BOOL fCandidateList) override {
        return m_windowsIme->_HandleCompositionFinalize(ec, pContext, fCandidateList);
    }
    HRESULT _HandleCompositionConvert(TfEditCookie ec, _In_ ITfContext* pContext, BOOL isWildcardSearch) override {
        return m_windowsIme->_HandleCompositionConvert(ec, pContext, isWildcardSearch);
    }
    HRESULT _HandleCompositionBackspace(TfEditCookie ec, _In_ ITfContext* pContext) override {
        return m_windowsIme->_HandleCompositionBackspace(ec, pContext);
    }
    HRESULT _HandleCompositionArrowKey(TfEditCookie ec, _In_ ITfContext* pContext, KEYSTROKE_FUNCTION keyFunction) override {
        return m_windowsIme->_HandleCompositionArrowKey(ec, pContext, keyFunction);
    }
    HRESULT _HandleCompositionPunctuation(TfEditCookie ec, _In_ ITfContext* pContext, WCHAR wch) override {
        return m_windowsIme->_HandleCompositionPunctuation(ec, pContext, wch);
    }
    HRESULT _HandleCompositionDoubleSingleByte(TfEditCookie ec, _In_ ITfContext* pContext, WCHAR wch) override {
        return m_windowsIme->_HandleCompositionDoubleSingleByte(ec, pContext, wch);
    }
    // key event handlers for candidate object.
    HRESULT _HandleCandidateFinalize(TfEditCookie ec, _In_ ITfContext* pContext) override {
        return m_windowsIme->_HandleCandidateFinalize(ec, pContext);
    }
    HRESULT _HandleCandidateConvert(TfEditCookie ec, _In_ ITfContext* pContext) override {
        return m_windowsIme->_HandleCandidateConvert(ec, pContext);
    }
    HRESULT _HandleCandidateArrowKey(TfEditCookie ec, _In_ ITfContext* pContext, _In_ KEYSTROKE_FUNCTION keyFunction) override {
        return m_windowsIme->_HandleCandidateArrowKey(ec, pContext, keyFunction);
    }
    HRESULT _HandleCandidateSelectByNumber(TfEditCookie ec, _In_ ITfContext* pContext, _In_ UINT uCode) override {
        return m_windowsIme->_HandleCandidateSelectByNumber(ec, pContext, uCode);
    }
    // key event handlers for phrase object.
    HRESULT _HandlePhraseFinalize(TfEditCookie ec, _In_ ITfContext* pContext) override {
        return m_windowsIme->_HandlePhraseFinalize(ec, pContext);
    }
    HRESULT _HandlePhraseArrowKey(TfEditCookie ec, _In_ ITfContext* pContext, _In_ KEYSTROKE_FUNCTION keyFunction) override {
        return m_windowsIme->_HandlePhraseArrowKey(ec, pContext, keyFunction);
    }
    HRESULT _HandlePhraseSelectByNumber(TfEditCookie ec, _In_ ITfContext* pContext, _In_ UINT uCode) override {
        return m_windowsIme->_HandlePhraseSelectByNumber(ec, pContext, uCode);
    }

private:
    WindowsImeLib::IWindowsIMECompositionBuffer* m_windowsIme = nullptr;
};

std::shared_ptr<WindowsImeLib::IWindowsIMECompositionBuffer> CreateCompositionBuffer(WindowsImeLib::IWindowsIMECompositionBuffer* windowsIme)
{
    auto _this = std::make_shared<CompositionBuffer>(windowsIme);
    return std::static_pointer_cast<WindowsImeLib::IWindowsIMECompositionBuffer>(_this);
}
