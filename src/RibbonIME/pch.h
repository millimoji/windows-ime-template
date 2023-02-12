// Copyright (c) millimoji@gmail.com
#pragma once

#pragma warning(push)
#pragma warning(disable: 6001)
#pragma warning(disable: 6387)
#pragma warning(disable: 26439)
#pragma warning(disable: 26451)
#pragma warning(disable: 26495)

#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <combaseapi.h>
#include <ctffunc.h>
#include <initguid.h>
#include <intsafe.h>
#include <msctf.h>
#include <olectl.h>
#include <sal.h>
#include <strsafe.h>

#include <functional>
#include <memory>
#include <vector>
#include <cassert>

#include <wil/com.h>
#include <wil/resource.h>

#pragma warning(disable: 26800)
#pragma warning(disable: 28020)
#include <nlohmann/json.hpp>

#pragma warning(pop)

#pragma warning(disable: 4463)

inline HMODULE GetCurrentModuleHandle() {
    static HMODULE currentModule = ([]() {
        HMODULE moduleHandle = {};
        GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                          reinterpret_cast<LPCWSTR>(GetCurrentModuleHandle), &moduleHandle);
        return moduleHandle;
    })();
    return currentModule;
}

inline std::wstring GetStringFromResource(int resId) {
    const wchar_t* textInResource = {};
    auto textLength = LoadString(GetCurrentModuleHandle(), resId, reinterpret_cast<LPWSTR>(&textInResource), 0);
    return std::wstring(textInResource, textLength);
}
