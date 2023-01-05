#include "Private.h"
#include "SingletonEngineHost.h"

namespace wrl
{
    using namespace Microsoft::WRL;
}

SingletonEngineHost::SingletonEngineHost()
{
}

SingletonEngineHost::~SingletonEngineHost()
{
}

std::wstring VARIANTtoString(const VARIANT& variant)
{
    THROW_HR_IF(E_UNEXPECTED, variant.vt != VT_BSTR);
    return std::wstring(variant.bstrVal, SysStringLen(variant.bstrVal));
}

void StringToVARIANT(std::wstring_view text, VARIANT& variant)
{
    variant.vt = VT_BSTR;
    variant.bstrVal = SysAllocStringLen(text.data(), static_cast<UINT>(text.length()));
}

IFACEMETHODIMP SingletonEngineHost::Invoke(DISPID dispIdMember, REFIID, LCID, WORD, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO*, UINT*) noexcept try
{
    switch (dispIdMember)
    {
    case DISPID_TEST_METHOD:
        {
            THROW_HR_IF(E_UNEXPECTED, pDispParams->cArgs != 1);
            const auto arg = VARIANTtoString(pDispParams->rgvarg[0]);
            const auto returnStr = arg + L"-suffix";
            StringToVARIANT(returnStr, *pVarResult);
        }
        return S_OK;
    }
    return S_OK;
}
CATCH_RETURN()


/* static */
HRESULT SingletonEngineHost::CreateInstance(_In_ IUnknown *pUnkOuter, REFIID riid, _Outptr_ void **ppvObj) try
{
    if (pUnkOuter)
    {
        return CLASS_E_NOAGGREGATION;
    }

    wil::com_ptr<SingletonEngineHost> engineHost;

    RETURN_IF_FAILED(wrl::MakeAndInitialize<SingletonEngineHost>(&engineHost));

    RETURN_IF_FAILED(engineHost->QueryInterface(riid, ppvObj));

    return S_OK;
}
CATCH_RETURN()
