#include "Private.h"
#include "SingletonEngineHost.h"

SingletonEngineHost::SingletonEngineHost()
{
}

SingletonEngineHost::~SingletonEngineHost()
{
}

IFACEMETHODIMP SingletonEngineHost::Invoke(DISPID dispIdMember, REFIID, LCID, WORD, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO*, UINT*) noexcept
{
    (void)dispIdMember;
    (void)pDispParams;
    (void)pVarResult;
    
    return S_OK;
}
