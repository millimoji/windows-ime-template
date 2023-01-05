#pragma once

constexpr int DISPID_TEST_METHOD = 1;

class SingletonEngineHost :
    public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>,
                                        IDispatch,
                                        Microsoft::WRL::FtmBase>
{
public:
    static HRESULT CreateInstance(_In_ IUnknown *pUnkOuter, REFIID riid, _Outptr_ void **ppvObj);
    SingletonEngineHost();
    virtual ~SingletonEngineHost();

private:
    IFACEMETHODIMP GetTypeInfoCount(UINT*) noexcept override
    {
        return E_NOTIMPL;
    }

    IFACEMETHODIMP GetTypeInfo(UINT, LCID, ITypeInfo**) noexcept override
    {
        return E_NOTIMPL;
    }

    IFACEMETHODIMP GetIDsOfNames(REFIID, LPOLESTR*, UINT, LCID, DISPID*) noexcept override
    {
        return E_NOTIMPL;
    }

    IFACEMETHODIMP Invoke(DISPID dispIdMember, REFIID, LCID, WORD, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO*, UINT*) noexcept override;

private:
};

static inline std::shared_ptr<const std::wstring> InvokeIDispatch_String_String(IDispatch* dispatch, DISPID dispId, const std::wstring_view param)
{
    const auto paramBstr = wil::unique_bstr(SysAllocStringLen(param.data(), static_cast<UINT>(param.length())));
    VARIANT arg1 = {}; arg1.vt = VT_BSTR; arg1.bstrVal = paramBstr.get();
    DISPPARAMS dispParams = {};
    dispParams.cArgs = 1;
    dispParams.rgvarg = &arg1;
    VARIANT result;
    THROW_IF_FAILED(dispatch->Invoke(dispId, GUID_NULL, 0, 0, &dispParams, &result, nullptr, nullptr));
    THROW_HR_IF(E_UNEXPECTED, result.vt != VT_BSTR);
    const auto resultValue = std::make_shared<const std::wstring>(result.bstrVal, SysStringLen(result.bstrVal));
    VariantClear(&result);
    return resultValue;
}
