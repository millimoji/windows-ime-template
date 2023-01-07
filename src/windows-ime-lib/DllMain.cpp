// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "Globals.h"
#include "../WindowsImeLib.h"

//+---------------------------------------------------------------------------
//
// DllMain
//
//----------------------------------------------------------------------------

BOOL WindowsImeLib::DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID pvReserved)
{
    pvReserved;

    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        wil::SetResultTelemetryFallback(WindowsImeLibTelemetry::FallbackTelemetryCallback);

        Global::dllInstanceHandle = hInstance;

        if (!InitializeCriticalSectionAndSpinCount(&Global::CS, 0))
        {
            return FALSE;
        }

        if (!Global::RegisterWindowClass()) {
            return FALSE;
        }

        break;

    case DLL_PROCESS_DETACH:

        DeleteCriticalSection(&Global::CS);

        break;

    case DLL_THREAD_ATTACH:

        break;

    case DLL_THREAD_DETACH:

        break;
    }

    return TRUE;
}

void WindowsImeLib::TraceLog(const char* format, ...)
{
    va_list arg;
    va_start(arg, format);
    char buf[4096];
    vsprintf_s(buf, format, arg);
    va_end(arg);

    WindowsImeLibTelemetry::TraceLogStr(buf);
}

void WindowsImeLib::TraceLog(const wchar_t* format, ...)
{
    va_list arg;
    va_start(arg, format);
    wchar_t buf[4096];
    vswprintf_s(buf, format, arg);
    va_end(arg);

    WindowsImeLibTelemetry::TraceLogWstr(buf);
}

#include "SingletonProcessor.h"

__declspec(dllexport) void TestFunction()
{
    const auto bridge = CreateSingletonProcessorBridge();
    const auto result = bridge->TestMethod(L"abc");
    WindowsImeLibTelemetry::TraceLog("TEST_METHOD result", L"%s", result.c_str());
}
