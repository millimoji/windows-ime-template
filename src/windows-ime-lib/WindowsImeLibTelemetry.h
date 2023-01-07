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

    DEFINE_CALLCONTEXT_ACTIVITY(ITfTextInputProcessorEx_ActivateEx);
    DEFINE_CALLCONTEXT_ACTIVITY(ITfTextInputProcessorEx_Deactivate);
    DEFINE_CALLCONTEXT_ACTIVITY(ITfThreadMgrEventSink_OnInitDocumentMgr);
    DEFINE_CALLCONTEXT_ACTIVITY(ITfThreadMgrEventSink_OnUninitDocumentMgr);
    DEFINE_CALLCONTEXT_ACTIVITY(ITfThreadMgrEventSink_OnSetFocus);
    DEFINE_CALLCONTEXT_ACTIVITY(ITfThreadMgrEventSink_OnPushContext);
    DEFINE_CALLCONTEXT_ACTIVITY(ITfThreadMgrEventSink_OnPopContext);
    DEFINE_CALLCONTEXT_ACTIVITY(ITfTextEditSink_OnEndEdit);
    DEFINE_CALLCONTEXT_ACTIVITY(ITfKeyEventSink_OnSetFocus);
    DEFINE_CALLCONTEXT_ACTIVITY(ITfKeyEventSink_OnTestKeyDown);
    DEFINE_CALLCONTEXT_ACTIVITY(ITfKeyEventSink_OnKeyDown);
    DEFINE_CALLCONTEXT_ACTIVITY(ITfKeyEventSink_OnTestKeyUp);
    DEFINE_CALLCONTEXT_ACTIVITY(ITfKeyEventSink_OnKeyUp);
    DEFINE_CALLCONTEXT_ACTIVITY(ITfKeyEventSink_OnPreservedKey);
    DEFINE_CALLCONTEXT_ACTIVITY(ITfCompositionSink_OnCompositionTerminated);
    DEFINE_CALLCONTEXT_ACTIVITY(ITfDisplayAttributeProvider_EnumDisplayAttributeInfo);
    DEFINE_CALLCONTEXT_ACTIVITY(ITfDisplayAttributeProvider_GetDisplayAttributeInfo);
    DEFINE_CALLCONTEXT_ACTIVITY(ITfActiveLanguageProfileNotifySink_OnActivated);
    DEFINE_CALLCONTEXT_ACTIVITY(ITfThreadFocusSink_OnSetThreadFocus);
    DEFINE_CALLCONTEXT_ACTIVITY(ITfThreadFocusSink_OnKillThreadFocus);
    DEFINE_CALLCONTEXT_ACTIVITY(ITfFunctionProvider_GetType);
    DEFINE_CALLCONTEXT_ACTIVITY(ITfFunctionProvider_GetDescription);
    DEFINE_CALLCONTEXT_ACTIVITY(ITfFunctionProvider_GetFunction);
    DEFINE_CALLCONTEXT_ACTIVITY(ITfFunction_GetDisplayName);
    DEFINE_CALLCONTEXT_ACTIVITY(ITfFnGetPreferredTouchKeyboardLayout_GetLayout);

    static void TraceLogStr(char const* const args) noexcept
    {
        TraceLoggingWrite(TraceLoggingType::Provider(), "TraceLog", // TODO: investigate forwrd keyText
            TraceLoggingValue(args),
            TraceLoggingLevel(WINEVENT_LEVEL_INFO),
            TelemetryPrivacyDataTag(PDT_ProductAndServicePerformance));
    }

    static void TraceLogWstr(wchar_t const* const args) noexcept
    {
        TraceLoggingWrite(TraceLoggingType::Provider(), "TraceLog", // TODO: investigate forwrd keyText
            TraceLoggingValue(args),
            TraceLoggingLevel(WINEVENT_LEVEL_INFO),
            TelemetryPrivacyDataTag(PDT_ProductAndServicePerformance));
    }

    // Easy debug purpose
    template <typename ... Args>
    static void TraceLog(char const* const keyText, wchar_t const * const format, Args const & ... args) noexcept
    {
        wchar_t buf[2048] = { 0 };
        swprintf_s(buf, format, args...);
        TraceLoggingWrite(TraceLoggingType::Provider(), "TraceLog", // TODO: investigate forwrd keyText
            TraceLoggingValue(keyText),
            TraceLoggingValue(buf, "args"),
            TraceLoggingLevel(WINEVENT_LEVEL_INFO),
            TelemetryPrivacyDataTag(PDT_ProductAndServicePerformance));
    }

    // Easy debug purpose
    template <typename ... Args>
    static void TraceLog(char const* const keyText, char const* const format, Args const & ... args) noexcept
    {
        char buf[2048] = { 0 };
        sprintf_s(buf, format, args...);
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
