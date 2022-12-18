#pragma once
#include <windows.h>

// legacy code, hard to make all be in specific namspace.

extern BOOL WindowsImeLib_DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID pvReserved);

extern HRESULT WindowsImeLib_DllGetClassObject(_In_ REFCLSID rclsid, _In_ REFIID riid, _Outptr_ void** ppv);
extern HRESULT WindowsImeLib_DllCanUnloadNow(void);
extern HRESULT WindowsImeLib_DllUnregisterServer(void);
extern HRESULT WindowsImeLib_DllRegisterServer(void);

