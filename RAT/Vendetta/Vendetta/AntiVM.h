#pragma once

#include "Vendetta.h"

#include <intrin.h>
#include <wbemidl.h>
#include <comdef.h>
#include <Wbemcli.h>

#pragma comment(lib, "wbemuuid.lib")

constexpr auto CPUID_VENDOR_INFO = 0x40000000;

typedef struct __CPUID_INFORMATION {
    int EAX;
    int EBX;
    int ECX;
    int EDX;
} CPUID_INFO;