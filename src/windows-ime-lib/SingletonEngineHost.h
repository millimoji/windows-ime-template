#pragma once


class SingletonEngineHost :
    public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>,
                                        IDispatch,
                                        Microsoft::WRL::FtmBase>
{
public:
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
