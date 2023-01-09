#pragma once

#pragma warning(push)
#pragma warning(disable: 6001)
#pragma warning(disable: 6387)
#pragma warning(disable: 26439)
#pragma warning(disable: 26451)
#pragma warning(disable: 26495)

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <combaseapi.h>
#include <ctffunc.h>
#include <initguid.h>
#include <intsafe.h>
#include <msctf.h>
#include <olectl.h>
#include <sal.h>
#include <strsafe.h>

#include <memory>
#include <vector>
#include <cassert>

#include <wil/com.h>
#include <wil/resource.h>

#pragma warning(pop)

#pragma warning(disable: 4463)
