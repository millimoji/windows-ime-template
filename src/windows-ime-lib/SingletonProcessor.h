#pragma once

#pragma warning(push)
#pragma warning(disable: 28251)
// Do not inlcude directly to suppress warning
#include "TextInputProcessor_h.h"
#pragma warning(pop)

extern wil::com_ptr<ITextInputProcessor> CreateSingletonProcessorBridge();

extern HRESULT CreateSingletonProcessorHost(_In_ IUnknown *pUnkOuter, REFIID riid, _Outptr_ void **ppvObj);
