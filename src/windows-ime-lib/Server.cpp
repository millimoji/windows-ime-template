// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "Globals.h"
#include "SampleIME.h"
#include "WindowsImeLib.h"

// from Register.cpp
BOOL RegisterProfiles();
void UnregisterProfiles();
BOOL RegisterCategories();
void UnregisterCategories();
BOOL RegisterServer();
void UnregisterServer();

void FreeGlobalObjects(void);

class CClassFactory;
static CClassFactory* classFactoryObjects[1] = { nullptr };

//+---------------------------------------------------------------------------
//
//  DllAddRef
//
//----------------------------------------------------------------------------

void DllAddRef(void)
{
    InterlockedIncrement(&Global::dllRefCount);
}

//+---------------------------------------------------------------------------
//
//  DllRelease
//
//----------------------------------------------------------------------------

void DllRelease(void)
{
    if (InterlockedDecrement(&Global::dllRefCount) < 0)
    {
        EnterCriticalSection(&Global::CS);

        if (nullptr != classFactoryObjects[0])
        {
            FreeGlobalObjects();
        }
        __pragma(warning(push)) __pragma(warning(disable:28112))
        assert(Global::dllRefCount == -1);
        __pragma(warning(pop))

        LeaveCriticalSection(&Global::CS);
    }
}

//+---------------------------------------------------------------------------
//
//  CClassFactory declaration with IClassFactory Interface
//
//----------------------------------------------------------------------------

class CClassFactory : public IClassFactory
{
public:
    // IUnknown methods
    STDMETHODIMP QueryInterface(REFIID riid, _Outptr_ void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

    // IClassFactory methods
    STDMETHODIMP CreateInstance(_In_opt_ IUnknown *pUnkOuter, _In_ REFIID riid, _COM_Outptr_ void **ppvObj);
    STDMETHODIMP LockServer(BOOL fLock);

    // Constructor
    CClassFactory(REFCLSID rclsid, HRESULT (*pfnCreateInstance)(IUnknown *pUnkOuter, REFIID riid, void **ppvObj))
        : _rclsid(rclsid)
    {
        _pfnCreateInstance = pfnCreateInstance;
    }

public:
    REFCLSID _rclsid;
    HRESULT (*_pfnCreateInstance)(IUnknown *pUnkOuter, REFIID riid, _COM_Outptr_ void **ppvObj);
private:
	CClassFactory& operator=(const CClassFactory& rhn) {rhn;};
};

//+---------------------------------------------------------------------------
//
//  CClassFactory::QueryInterface
//
//----------------------------------------------------------------------------

STDAPI CClassFactory::QueryInterface(REFIID riid, _Outptr_ void **ppvObj)
{
    if (IsEqualIID(riid, IID_IClassFactory) || IsEqualIID(riid, IID_IUnknown))
    {
        *ppvObj = this;
        DllAddRef();
        return NOERROR;
    }
    *ppvObj = nullptr;

    return E_NOINTERFACE;
}

//+---------------------------------------------------------------------------
//
//  CClassFactory::AddRef
//
//----------------------------------------------------------------------------

STDAPI_(ULONG) CClassFactory::AddRef()
{
    DllAddRef();
    return (Global::dllRefCount + 1);
}

//+---------------------------------------------------------------------------
//
//  CClassFactory::Release
//
//----------------------------------------------------------------------------

STDAPI_(ULONG) CClassFactory::Release()
{
    DllRelease();
    return (Global::dllRefCount + 1);
}

//+---------------------------------------------------------------------------
//
//  CClassFactory::CreateInstance
//
//----------------------------------------------------------------------------

__pragma(warning(push)) __pragma(warning(disable:6388))
STDAPI CClassFactory::CreateInstance(_In_opt_ IUnknown *pUnkOuter, _In_ REFIID riid, _COM_Outptr_ void **ppvObj)
{
    return _pfnCreateInstance(pUnkOuter, riid, ppvObj);
}
__pragma(warning(pop))

//+---------------------------------------------------------------------------
//
//  CClassFactory::LockServer
//
//----------------------------------------------------------------------------

STDAPI CClassFactory::LockServer(BOOL fLock)
{
    if (fLock)
    {
        DllAddRef();
    }
    else
    {
        DllRelease();
    }

    return S_OK;
}

//+---------------------------------------------------------------------------
//
//  BuildGlobalObjects
//
//----------------------------------------------------------------------------

void BuildGlobalObjects(void)
{
    classFactoryObjects[0] = new (std::nothrow) CClassFactory(Global::SampleIMECLSID, CSampleIME::CreateInstance);
}

//+---------------------------------------------------------------------------
//
//  FreeGlobalObjects
//
//----------------------------------------------------------------------------

void FreeGlobalObjects(void)
{
    for (int i = 0; i < ARRAYSIZE(classFactoryObjects); i++)
    {
        if (nullptr != classFactoryObjects[i])
        {
            delete classFactoryObjects[i];
            classFactoryObjects[i] = nullptr;
        }
    }

    DeleteObject(Global::defaultlFontHandle);
}

//+---------------------------------------------------------------------------
//
//  DllGetClassObject
//
//----------------------------------------------------------------------------
HRESULT WindowsImeLib_DllGetClassObject(_In_ REFCLSID rclsid, _In_ REFIID riid, _Outptr_ void** ppv)
{
    if (classFactoryObjects[0] == nullptr)
    {
        EnterCriticalSection(&Global::CS);

        // need to check ref again after grabbing mutex
        if (classFactoryObjects[0] == nullptr)
        {
            BuildGlobalObjects();
        }

        LeaveCriticalSection(&Global::CS);
    }

    if (IsEqualIID(riid, IID_IClassFactory) ||
        IsEqualIID(riid, IID_IUnknown))
    {
        for (int i = 0; i < ARRAYSIZE(classFactoryObjects); i++)
        {
            if (nullptr != classFactoryObjects[i] &&
                IsEqualGUID(rclsid, classFactoryObjects[i]->_rclsid))
            {
                *ppv = (void *)classFactoryObjects[i];
                DllAddRef();    // class factory holds DLL ref count
                return NOERROR;
            }
        }
    }

    *ppv = nullptr;

    return CLASS_E_CLASSNOTAVAILABLE;
}

//+---------------------------------------------------------------------------
//
//  DllCanUnloadNow
//
//----------------------------------------------------------------------------

HRESULT WindowsImeLib_DllCanUnloadNow(void)
{
    if (Global::dllRefCount >= 0)
    {
        return S_FALSE;
    }

    return S_OK;
}

//+---------------------------------------------------------------------------
//
//  DllUnregisterServer
//
//----------------------------------------------------------------------------

HRESULT WindowsImeLib_DllUnregisterServer(void)
{
    UnregisterProfiles();
    UnregisterCategories();
    UnregisterServer();

    return S_OK;
}

//+---------------------------------------------------------------------------
//
//  DllRegisterServer
//
//----------------------------------------------------------------------------

HRESULT WindowsImeLib_DllRegisterServer(void)
{
    if ((!RegisterServer()) || (!RegisterProfiles()) || (!RegisterCategories()))
    {
        DllUnregisterServer();
        return E_FAIL;
    }
    return S_OK;
}
