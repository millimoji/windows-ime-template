// Copyright (c) millimoji@gmail.com

#include "pch.h"
#include "resource.h"
#include "../WindowsImeLib.h"
#include "RibbonIMEConstants.h"

class RibbonIMEConstants : public WindowsImeLib::IConstantProvider
{
    // GUIDs
    const CLSID& IMECLSID() noexcept override
    {
        // {FF089024-807B-4131-9CDF-3C3B067F081C}
        static const GUID imeClasId = { 0xff089024, 0x807b, 0x4131, { 0x9c, 0xdf, 0x3c, 0x3b, 0x6, 0x7f, 0x8, 0x1c } };
        return imeClasId;
    }
    const GUID& IMEProfileGuid() noexcept override
    {
        // {0F86F935-F13C-4813-9E26-14888E3F128E}
        static const GUID imeProfileId = { 0xf86f935, 0xf13c, 0x4813, { 0x9e, 0x26, 0x14, 0x88, 0x8e, 0x3f, 0x12, 0x8e } };
        return imeProfileId;
    }
    const GUID& CandUIElement() noexcept override
    {
        // {E1124AF5-A86E-4F39-8081-6FEEE3C90504}
        static const GUID candUiElementId = { 0xe1124af5, 0xa86e, 0x4f39, { 0x80, 0x81, 0x6f, 0xee, 0xe3, 0xc9, 0x5, 0x4 } };
        return candUiElementId;
    }
    const LCID GetLocale() noexcept override
    {
        return MAKELCID(GetLangID(), SORT_DEFAULT);
    }
    const LANGID GetLangID() noexcept override
    {
        return MAKELANGID(LANG_JAPANESE, SUBLANG_JAPANESE_JAPAN);
    }
    const CLSID& ServerCLSID() noexcept override
    {
        // {37D39577-1437-4108-8FEC-6CB1EB730333}
        static const GUID serverCLSID = { 0x37d39577, 0x1437, 0x4108, { 0x8f, 0xec, 0x6c, 0xb1, 0xeb, 0x73, 0x3, 0x33 } };
        return serverCLSID;
    }
    const GUID& ServerAppID() noexcept override
    {
        // {BD659CE0-D6BF-4B15-9EA9-5C936A58FE30}
        static const GUID serverAppID = { 0xbd659ce0, 0xd6bf, 0x4b15, { 0x9e, 0xa9, 0x5c, 0x93, 0x6a, 0x58, 0xfe, 0x30 } };
        return serverAppID;
    }
    const wchar_t* ServerName() noexcept override
    {
        return L"Ribbon IME Singleton Sever";
    }
    void GetPreferredTouchKeyboardLayout(_Out_ TKBLayoutType* layoutType, _Out_ WORD* preferredLayoutId) noexcept override
    {
        *layoutType = TKBLT_OPTIMIZED;
        *preferredLayoutId = TKBL_OPT_JAPANESE_ABC;
    }
    UINT GetCandidateWindowWidth() noexcept override
    {
        return 13; // CAND_WIDTH
    }
    const int GetDefaultCandidateTextFontResourceID()  override
    {
        return IDS_DEFAULT_FONT;
    }
};


std::shared_ptr<WindowsImeLib::IConstantProvider> RibbonIMEConstants_CreateInstance() {
	return std::make_shared<RibbonIMEConstants>();
}