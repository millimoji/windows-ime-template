#pragma once

#include <wil/Tracelogging.h>

// https://learn.microsoft.com/en-us/windows/win32/api/traceloggingprovider/nf-traceloggingprovider-tracelogging_define_provider#provider-name-and-id

DECLARE_TRACELOGGING_CLASS(
    WindowsImeLibTelemetryProvider,
    "millimo.WindowsImeLib", // Human-readable name for the provider
    // {28ccdaf8-3182-58d7-5f69-3254a86b3ebe}
    (0x28ccdaf8,0x3182,0x58d7,0x5f,0x69,0x32,0x54,0xa8,0x6b,0x3e,0xbe));

class WindowsImeLibTelemetry : public TelemetryBase
{
    IMPLEMENT_TELEMETRY_CLASS(WindowsImeLibTelemetry, WindowsImeLibTelemetryProvider);
public:

    // Easy debug purpose
    template <typename ... Args>
    static void TraceLog(char const* const keyText, wchar_t const * const format, Args const & ... args) noexcept
    {
        wchar_t buf[2048];
        swprintf_s(buf, format, args...);
        TraceLoggingWrite(TraceLoggingType::Provider(), "TraceLog", // TODO: investigate forwrd keyText
            TraceLoggingValue(keyText),
            TraceLoggingValue(buf, "args"),
            TraceLoggingLevel(WINEVENT_LEVEL_INFO),
            TelemetryPrivacyDataTag(PDT_ProductAndServicePerformance));
    }

    // Easy debug purpose
    static void TraceLog(char const* const keyText) noexcept
    {
        TraceLoggingWrite(TraceLoggingType::Provider(), "TraceLog", // TODO: investigate forwrd keyText
            TraceLoggingValue(keyText),
            TraceLoggingLevel(WINEVENT_LEVEL_INFO),
            TelemetryPrivacyDataTag(PDT_ProductAndServicePerformance));
    }

};
