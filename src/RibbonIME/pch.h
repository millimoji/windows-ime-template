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
#include <shellapi.h>
#include <strsafe.h>

#include <cassert>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <memory>
#include <stdexcept>
#include <tuple>
#include <vector>

#include <wil/com.h>
#include <wil/resource.h>

#pragma warning(disable: 26800)
#pragma warning(disable: 28020)
#include <nlohmann/json.hpp>

#pragma warning(pop)

#pragma warning(disable: 4463)
