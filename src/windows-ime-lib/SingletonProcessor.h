#pragma once

extern std::shared_ptr<WindowsImeLib::ITextInputProcessor> CreateSingletonProcessorBridge();

extern HRESULT CreateSingletonProcessorHost(_In_ IUnknown *pUnkOuter, REFIID riid, _Outptr_ void **ppvObj);

constexpr int DISPID_TEST_METHOD = 1;
constexpr int DISPID_SET_FOCUS =2;


inline std::wstring VARIANTtoWString(const VARIANT& variant)
{
    THROW_HR_IF(E_UNEXPECTED, variant.vt != VT_BSTR);
    return std::wstring(variant.bstrVal, SysStringLen(variant.bstrVal));
}

inline wil::unique_bstr WStringToVARIANT(std::wstring_view text, _Inout_ VARIANT& variant)
{
    auto bstr = wil::unique_bstr(SysAllocStringLen(text.data(), static_cast<UINT>(text.length())));
    variant.vt = VT_BSTR;
    variant.bstrVal = bstr.get();
    return bstr;
}

inline wil::unique_bstr StringToVARIANT(std::string_view text, _Inout_ VARIANT& variant)
{
    // hack, set utf8 into BSTR.
    auto bstr = wil::unique_bstr(SysAllocStringLen(
        reinterpret_cast<const wchar_t*>(text.data()), static_cast<UINT>((text.length() + 1) / sizeof(wchar_t))));
    variant.vt = VT_BSTR;
    variant.bstrVal = bstr.get();
    return bstr;
}

inline std::string VARIANTtoString(const VARIANT& variant)
{
    // hack, set utf8 into BSTR.
    THROW_HR_IF(E_UNEXPECTED, variant.vt != VT_BSTR);
    return std::string(reinterpret_cast<const char*>(variant.bstrVal));
}

inline std::wstring InvokeIDispatch_WString_WString(IDispatch* dispatch, DISPID dispId, const std::wstring_view param)
{
    VARIANT arg = {};
    const auto bstr = WStringToVARIANT(param, arg);
    DISPPARAMS dispParams = {};
    dispParams.cArgs = 1;
    dispParams.rgvarg = &arg;
    VARIANT result;
    THROW_IF_FAILED(dispatch->Invoke(dispId, GUID_NULL, 0, 0, &dispParams, &result, nullptr, nullptr));
    const auto resultValue = VARIANTtoWString(result);
    VariantClear(&result);
    return resultValue;
}

inline void InvokeIDispatch_String_Void(IDispatch* dispatch, DISPID dispId, const std::string_view param)
{
    VARIANT arg = {};
    const auto bstr = StringToVARIANT(param, arg);
    DISPPARAMS dispParams = {};
    dispParams.cArgs = 1;
    dispParams.rgvarg = &arg;
    VARIANT result;
    THROW_IF_FAILED(dispatch->Invoke(dispId, GUID_NULL, 0, 0, &dispParams, &result, nullptr, nullptr));
    VariantClear(&result);
}
