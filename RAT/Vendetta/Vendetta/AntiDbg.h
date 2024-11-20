#pragma once

#include "Vendetta.h"
#include "NtStruct.h"
#include <ImageHlp.h>

#pragma comment(lib, "ImageHlp")
#pragma comment(lib,"ntdll.lib")

#pragma comment (linker, "/INCLUDE:_tls_used")
#pragma comment (linker, "/INCLUDE:_tls_callback")
#pragma const_seg (".CRT$XLB")


using namespace std;

extern "C" NTSTATUS NTAPI NtQueryInformationProcess(HANDLE hProcess, ULONG InfoClass, PVOID Buffer, ULONG Length, PULONG ReturnLength);





constexpr auto FLG_HEAP_ENABLE_TAIL_CHECK = 0x10;
constexpr auto FLG_HEAP_ENABLE_FREE_CHECK = 0x20;
constexpr auto FLG_HEAP_VALIDATE_PARAMETERS = 0x40;
#define NT_GLOBAL_FLAG_DEBUGGED (FLG_HEAP_ENABLE_TAIL_CHECK | FLG_HEAP_ENABLE_FREE_CHECK | FLG_HEAP_VALIDATE_PARAMETERS)

