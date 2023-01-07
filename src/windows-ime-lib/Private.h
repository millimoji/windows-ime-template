// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#pragma warning(push)
#pragma warning(disable: 4324)
#pragma warning(disable: 6001)
#pragma warning(disable: 6387)
#pragma warning(disable: 26439)
#pragma warning(disable: 26451)
#pragma warning(disable: 26495)

#include <windows.h>
#include <initguid.h>
#include <combaseapi.h>
#include <ctffunc.h>
#include <intsafe.h>
#include <msctf.h>
#include <olectl.h>
#include <sal.h>
#include <strsafe.h>

#define _SILENCE_CXX20_CISO646_REMOVED_WARNING

#include <functional>
#include <memory>
#include <new>
#include <string>
#include <vector>
#include <cassert>

#include <wrl/module.h>
#include <wrl/implements.h>

#include <wil/com.h>
#include <wil/resource.h>

#pragma warning(disable: 28020)
#include <nlohmann/json.hpp>

#include "WindowsImeLibTelemetry.h"

#pragma warning(pop)
