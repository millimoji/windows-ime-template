#pragma once

#include <wil/Tracelogging.h>

// https://learn.microsoft.com/en-us/windows/win32/api/traceloggingprovider/nf-traceloggingprovider-tracelogging_define_provider#provider-name-and-id

DECLARE_TRACELOGGING_CLASS(
    SampleIMETelemetryProvider,
    "millimo.SampleIME",
    // {0c03c94a-383c-5b3c-9388-92b99612c61c}
    (0x0c03c94a,0x383c,0x5b3c,0x93,0x88,0x92,0xb9,0x96,0x12,0xc6,0x1c));

class SampleIMETelemetry : public TelemetryBase
{
    IMPLEMENT_TELEMETRY_CLASS(SampleIMETelemetry, SampleIMETelemetryProvider);
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
