// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "Define.h"
#include "Globals.h"

static const WCHAR RegInfo_Prefix_CLSID[] = L"CLSID\\";
static const WCHAR RegInfo_Key_InProSvr32[] = L"InProcServer32";
static const WCHAR RegInfo_Key_ThreadModel[] = L"ThreadingModel";

static const WCHAR TEXTSERVICE_DESC[] = L"Sample IME";

static const GUID SupportCategories[] = {
    GUID_TFCAT_TIP_KEYBOARD,
    GUID_TFCAT_DISPLAYATTRIBUTEPROVIDER,
    GUID_TFCAT_TIPCAP_UIELEMENTENABLED, 
    GUID_TFCAT_TIPCAP_SECUREMODE,
    GUID_TFCAT_TIPCAP_COMLESS,
    GUID_TFCAT_TIPCAP_INPUTMODECOMPARTMENT,
    GUID_TFCAT_TIPCAP_IMMERSIVESUPPORT, 
    GUID_TFCAT_TIPCAP_SYSTRAYSUPPORT,
};

//+---------------------------------------------------------------------------
//
//  RegisterProfiles
//
//----------------------------------------------------------------------------

BOOL RegisterProfiles(LANGID langId, int textServiceIconIndex)
{
    HRESULT hr = S_FALSE;

    ITfInputProcessorProfileMgr *pITfInputProcessorProfileMgr = nullptr;
    hr = CoCreateInstance(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER,
        IID_ITfInputProcessorProfileMgr, (void**)&pITfInputProcessorProfileMgr);
    if (FAILED(hr))
    {
        return FALSE;
    }

    WCHAR achIconFile[MAX_PATH] = {'\0'};
    DWORD cchA = 0;
    cchA = GetModuleFileName(Global::dllInstanceHandle, achIconFile, MAX_PATH);
    cchA = cchA >= MAX_PATH ? (MAX_PATH - 1) : cchA;
    achIconFile[cchA] = '\0';

    size_t lenOfDesc = 0;
    hr = StringCchLength(TEXTSERVICE_DESC, STRSAFE_MAX_CCH, &lenOfDesc);
    if (hr != S_OK)
    {
        goto Exit;
    }
    hr = pITfInputProcessorProfileMgr->RegisterProfile(
        WindowsImeLib::g_processorFactory->GetConstantProvider()->IMECLSID(),
        langId,
        WindowsImeLib::g_processorFactory->GetConstantProvider()->IMEProfileGuid(),
        TEXTSERVICE_DESC,
        static_cast<ULONG>(lenOfDesc),
        achIconFile,
        cchA,
        (UINT)textServiceIconIndex, NULL, 0, TRUE, 0);

    if (FAILED(hr))
    {
        goto Exit;
    }

Exit:
    if (pITfInputProcessorProfileMgr)
    {
        pITfInputProcessorProfileMgr->Release();
    }

    return (hr == S_OK);
}

//+---------------------------------------------------------------------------
//
//  UnregisterProfiles
//
//----------------------------------------------------------------------------

void UnregisterProfiles(LANGID langId)
{
    HRESULT hr = S_OK;

    ITfInputProcessorProfileMgr *pITfInputProcessorProfileMgr = nullptr;
    hr = CoCreateInstance(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER,
        IID_ITfInputProcessorProfileMgr, (void**)&pITfInputProcessorProfileMgr);
    if (FAILED(hr))
    {
        goto Exit;
    }

    hr = pITfInputProcessorProfileMgr->UnregisterProfile(
        WindowsImeLib::g_processorFactory->GetConstantProvider()->IMECLSID(),
        langId,
        WindowsImeLib::g_processorFactory->GetConstantProvider()->IMEProfileGuid(),
        0);
    if (FAILED(hr))
    {
        goto Exit;
    }

Exit:
    if (pITfInputProcessorProfileMgr)
    {
        pITfInputProcessorProfileMgr->Release();
    }

    return;
}

//+---------------------------------------------------------------------------
//
//  RegisterCategories
//
//----------------------------------------------------------------------------

BOOL RegisterCategories()
{
    ITfCategoryMgr* pCategoryMgr = nullptr;
    HRESULT hr = S_OK;

    hr = CoCreateInstance(CLSID_TF_CategoryMgr, NULL, CLSCTX_INPROC_SERVER, IID_ITfCategoryMgr, (void**)&pCategoryMgr);
    if (FAILED(hr))
    {
        return FALSE;
    }

    for (const GUID& guid: SupportCategories)
    {
        hr = pCategoryMgr->RegisterCategory(
                WindowsImeLib::g_processorFactory->GetConstantProvider()->IMECLSID(),
                guid,
                WindowsImeLib::g_processorFactory->GetConstantProvider()->IMECLSID());
    }

    pCategoryMgr->Release();

    return (hr == S_OK);
}

//+---------------------------------------------------------------------------
//
//  UnregisterCategories
//
//----------------------------------------------------------------------------

void UnregisterCategories()
{
    ITfCategoryMgr* pCategoryMgr = {};
    HRESULT hr = S_OK;

    hr = CoCreateInstance(CLSID_TF_CategoryMgr, NULL, CLSCTX_INPROC_SERVER, IID_ITfCategoryMgr, (void**)&pCategoryMgr);
    if (FAILED(hr))
    {
        return;
    }

    for (const GUID& guid: SupportCategories)
    {
        pCategoryMgr->UnregisterCategory(
            WindowsImeLib::g_processorFactory->GetConstantProvider()->IMECLSID(),
            guid,
            WindowsImeLib::g_processorFactory->GetConstantProvider()->IMECLSID());
    }
  
    pCategoryMgr->Release();

    return;
}

//+---------------------------------------------------------------------------
//
// RecurseDeleteKey
//
// RecurseDeleteKey is necessary because on NT RegDeleteKey doesn't work if the
// specified key has subkeys
//----------------------------------------------------------------------------

LONG RecurseDeleteKey(_In_ HKEY hParentKey, _In_ LPCTSTR lpszKey)
{
    HKEY regKeyHandle = nullptr;
    LONG res = 0;
    FILETIME time;
    WCHAR stringBuffer[256] = {'\0'};
    DWORD size = ARRAYSIZE(stringBuffer);

    if (RegOpenKey(hParentKey, lpszKey, &regKeyHandle) != ERROR_SUCCESS)
    {
        return ERROR_SUCCESS;
    }

    res = ERROR_SUCCESS;
    while (RegEnumKeyEx(regKeyHandle, 0, stringBuffer, &size, NULL, NULL, NULL, &time) == ERROR_SUCCESS)
    {
        stringBuffer[ARRAYSIZE(stringBuffer)-1] = '\0';
        res = RecurseDeleteKey(regKeyHandle, stringBuffer);
        if (res != ERROR_SUCCESS)
        {
            break;
        }
        size = ARRAYSIZE(stringBuffer);
    }
    RegCloseKey(regKeyHandle);

    return res == ERROR_SUCCESS ? RegDeleteKey(hParentKey, lpszKey) : res;
}

//+---------------------------------------------------------------------------
//
//  RegisterServer
//
//----------------------------------------------------------------------------

BOOL RegisterServer()
{
    DWORD copiedStringLen = 0;
    HKEY regKeyHandle = nullptr;
    HKEY regSubkeyHandle = nullptr;
    BOOL ret = FALSE;
    WCHAR achIMEKey[ARRAYSIZE(RegInfo_Prefix_CLSID) + CLSID_STRLEN] = {'\0'};
    WCHAR achFileName[MAX_PATH] = {'\0'};

    if (!CLSIDToString(WindowsImeLib::g_processorFactory->GetConstantProvider()->IMECLSID(), achIMEKey + ARRAYSIZE(RegInfo_Prefix_CLSID) - 1))
    {
        return FALSE;
    }

    memcpy(achIMEKey, RegInfo_Prefix_CLSID, sizeof(RegInfo_Prefix_CLSID) - sizeof(WCHAR));

    if (RegCreateKeyEx(HKEY_CLASSES_ROOT, achIMEKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &regKeyHandle, &copiedStringLen) == ERROR_SUCCESS)
    {
        if (RegSetValueEx(regKeyHandle, NULL, 0, REG_SZ, (const BYTE *)TEXTSERVICE_DESC, (_countof(TEXTSERVICE_DESC))*sizeof(WCHAR)) != ERROR_SUCCESS)
        {
            goto Exit;
        }

        if (RegCreateKeyEx(regKeyHandle, RegInfo_Key_InProSvr32, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &regSubkeyHandle, &copiedStringLen) == ERROR_SUCCESS)
        {
            copiedStringLen = GetModuleFileNameW(Global::dllInstanceHandle, achFileName, ARRAYSIZE(achFileName));
            copiedStringLen = (copiedStringLen >= (MAX_PATH - 1)) ? MAX_PATH : (++copiedStringLen);
            if (RegSetValueEx(regSubkeyHandle, NULL, 0, REG_SZ, (const BYTE *)achFileName, (copiedStringLen)*sizeof(WCHAR)) != ERROR_SUCCESS)
            {
                goto Exit;
            }
            if (RegSetValueEx(regSubkeyHandle, RegInfo_Key_ThreadModel, 0, REG_SZ, (const BYTE *)TEXTSERVICE_MODEL, (_countof(TEXTSERVICE_MODEL)) * sizeof(WCHAR)) != ERROR_SUCCESS)
            {
                goto Exit;
            }

            ret = TRUE;
        }
    }

Exit:
    if (regSubkeyHandle)
    {
        RegCloseKey(regSubkeyHandle);
        regSubkeyHandle = nullptr;
    }
    if (regKeyHandle)
    {
        RegCloseKey(regKeyHandle);
        regKeyHandle = nullptr;
    }

    return ret;
}

//+---------------------------------------------------------------------------
//
//  UnregisterServer
//
//----------------------------------------------------------------------------

void UnregisterServer()
{
    WCHAR achIMEKey[ARRAYSIZE(RegInfo_Prefix_CLSID) + CLSID_STRLEN] = {'\0'};

    if (!CLSIDToString(WindowsImeLib::g_processorFactory->GetConstantProvider()->IMECLSID(), achIMEKey + ARRAYSIZE(RegInfo_Prefix_CLSID) - 1))
    {
        return;
    }

    memcpy(achIMEKey, RegInfo_Prefix_CLSID, sizeof(RegInfo_Prefix_CLSID) - sizeof(WCHAR));

    RecurseDeleteKey(HKEY_CLASSES_ROOT, achIMEKey);
}

HRESULT RegisterSingletonServer()
{
    wchar_t clsId[CLSID_STRLEN + 1] = { '\0' };
    if (StringFromGUID2(WindowsImeLib::g_processorFactory->GetConstantProvider()->ServerCLSID(), clsId, ARRAYSIZE(clsId)) == 0)
    {
        RETURN_LAST_ERROR();
    }
    wchar_t appId[CLSID_STRLEN + 1] = { '\0' };
    if (StringFromGUID2(WindowsImeLib::g_processorFactory->GetConstantProvider()->ServerAppID(), appId, ARRAYSIZE(appId)) == 0)
    {
        RETURN_LAST_ERROR();
    }
    const DWORD cbAppId = static_cast<DWORD>(sizeof(wchar_t) * (wcslen(appId) + 1));

    const wchar_t* serverName = WindowsImeLib::g_processorFactory->GetConstantProvider()->ServerName();
    const DWORD cbServerName = static_cast<DWORD>(sizeof(wchar_t) * (wcslen(serverName) + 1));

    wchar_t dllFileName[MAX_PATH] = {'\0'};
    GetModuleFileName(Global::dllInstanceHandle, dllFileName, ARRAYSIZE(dllFileName));
    const DWORD cbDllFileName = static_cast<DWORD>(sizeof(wchar_t) * (wcslen(dllFileName) + 1));

    // CLSID\[GUID]
    //      {} REG_SZ "App Name"
    //      AppID REG_SZ GUID
    // CLSID\[GUID]\InprocServer32
    //      {} REG_SZ full path
    //      ThreadingModel REG_SZ Both
    // AppID\[GUID]
    //      {} REG_SZ /d "App Name"
    //      DllSurrogate REG_SZ ""
    //      RunAs REG_SZ "Interactive User"

    wil::unique_hkey clsIdRoot;
    RETURN_IF_WIN32_ERROR(RegCreateKeyEx(HKEY_CLASSES_ROOT, L"CLSID", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &clsIdRoot, nullptr));

    wil::unique_hkey clsIdGuid;
    RETURN_IF_WIN32_ERROR(RegCreateKeyEx(clsIdRoot.get(), clsId, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &clsIdGuid, nullptr));

    wil::unique_hkey inprocServer32;
    RETURN_IF_WIN32_ERROR(RegCreateKeyEx(clsIdGuid.get(), L"InprocServer32", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &inprocServer32, nullptr));

    wil::unique_hkey appIdRoot;
    RETURN_IF_WIN32_ERROR(RegCreateKeyEx(HKEY_CLASSES_ROOT, L"AppID", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &appIdRoot, nullptr));

    wil::unique_hkey appIdGuid;
    RETURN_IF_WIN32_ERROR(RegCreateKeyEx(appIdRoot.get(), appId, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &appIdGuid, nullptr));

    // CLSID/[GUID]
    RETURN_IF_WIN32_ERROR(RegSetValueEx(clsIdGuid.get(), nullptr, 0, REG_SZ, reinterpret_cast<const BYTE*>(serverName), cbServerName));

    RETURN_IF_WIN32_ERROR(RegSetValueEx(clsIdGuid.get(), L"AppID", 0, REG_SZ, reinterpret_cast<const BYTE*>(appId), cbAppId));

    // CLSID/[GUID]/InprocServer32

    RETURN_IF_WIN32_ERROR(RegSetValueEx(inprocServer32.get(), nullptr, 0, REG_SZ, reinterpret_cast<const BYTE*>(dllFileName), cbDllFileName));

    RETURN_IF_WIN32_ERROR(RegSetValueEx(inprocServer32.get(), L"ThreadingModel", 0, REG_SZ, reinterpret_cast<const BYTE*>(L"Both"), 10));

    // AppID/[GUID]

    RETURN_IF_WIN32_ERROR(RegSetValueEx(appIdGuid.get(), nullptr, 0, REG_SZ, reinterpret_cast<const BYTE*>(serverName), cbServerName));

    RETURN_IF_WIN32_ERROR(RegSetValueEx(appIdGuid.get(), L"DllSurrogate", 0, REG_SZ, reinterpret_cast<const BYTE*>(L""), 2));

    const wchar_t* interactiveUser = L"Interactive User";
    const DWORD cbInteractiveUser = static_cast<DWORD>(sizeof(wchar_t) * (wcslen(interactiveUser) + 1));

    RETURN_IF_WIN32_ERROR(RegSetValueEx(appIdGuid.get(), L"RunAs", 0, REG_SZ, reinterpret_cast<const BYTE*>(interactiveUser), cbInteractiveUser));

    return S_OK;
}

void UnregisterSingletonServer()
{
    wchar_t clsId[CLSID_STRLEN + 1] = {'\0'};
    if (StringFromGUID2(WindowsImeLib::g_processorFactory->GetConstantProvider()->ServerCLSID(), clsId, ARRAYSIZE(clsId)) == 0)
    {
        LOG_LAST_ERROR();
        return;
    }
    wchar_t appId[CLSID_STRLEN + 1] = {'\0'};
    if (StringFromGUID2(WindowsImeLib::g_processorFactory->GetConstantProvider()->ServerAppID(), appId, ARRAYSIZE(appId)) == 0)
    {
        LOG_LAST_ERROR();
        return;
    }

    wil::unique_hkey clsIdRoot;
    LOG_IF_WIN32_ERROR(RegCreateKeyEx(HKEY_CLASSES_ROOT, L"CLSID", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &clsIdRoot, nullptr));

    wil::unique_hkey appIdRoot;
    LOG_IF_WIN32_ERROR(RegCreateKeyEx(HKEY_CLASSES_ROOT, L"AppID", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &appIdRoot, nullptr));

    if (clsIdRoot)
    {
        LOG_IF_WIN32_ERROR(RegDeleteTree(clsIdRoot.get(), clsId));
    }

    if (appIdRoot)
    {
        LOG_IF_WIN32_ERROR(RegDeleteTree(appIdRoot.get(), appId));
    }
}
