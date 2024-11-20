#pragma once

#include <Windows.h>
#include "NtStruct.h"

constexpr auto SHUTDOWN_PRIVILEGE = 19;
constexpr auto OPTION_SHUTDOWN = 6;

// その他
using PUSER_THREAD_START_ROUTINE = NTSTATUS(NTAPI*)(PVOID ThreadParameter);
using PIO_APC_ROUTINE = VOID(NTAPI*)(PVOID ApcContext, PIO_STATUS_BLOCK IoStatusBlock, ULONG Reserved);

// シャットダウンアクション
typedef enum _SHUTDOWN_ACTION {
    ShutdownNoReboot,
    ShutdownReboot,
    ShutdownPowerOff,
    ShutdownCrash,
    ShutdownPowerOffCrash,
    ShutdownPowerOffReboot,
    ShutdownMax
} SHUTDOWN_ACTION;

typedef enum _KEY_VALUE_INFORMATION_CLASS {
    KeyValueBasicInformation,          // 基本的な情報（名前、型、データのサイズ）
    KeyValueFullInformation,           // フル情報（基本情報に加えて、データの内容）
    KeyValuePartialInformation,        // 部分的な情報
    KeyValueLayeredInformation,        // レイヤー情報（レイヤードキーベースの情報）
    KeyValueCachedInformation,          // キャッシュされた情報
    KeyValueVirtualInformation,        // 仮想情報
    KeyValueFlagsInformation,          // フラグ情報
    KeyValueEndOfInformation            // 情報の終わり
} KEY_VALUE_INFORMATION_CLASS;

typedef enum _OBJECT_INFORMATION_CLASS {
    ObjectBasicInformation,          // オブジェクトの基本情報
    ObjectNameInformation,           // オブジェクトの名前情報
    ObjectTypeInformation,           // オブジェクトのタイプ情報
    ObjectAllInformation,            // オブジェクトのすべての情報
    ObjectHandleInformation,         // オブジェクトハンドルに関する情報
    ObjectSessionInformation,        // オブジェクトのセッション情報
    ObjectQuotaInformation,          // オブジェクトのクォータ情報
    ObjectSecurityInformation,       // オブジェクトのセキュリティ情報
    ObjectDataInformation,           // オブジェクトのデータ情報
    ObjectTypeNameInformation,       // オブジェクトのタイプ名情報
    ObjectSystemHandleInformation,   // システムハンドル情報
    ObjectKernelHandleInformation,   // カーネルハンドル情報
    ObjectImageInformation,          // イメージ情報
    ObjectExtendedBasicInformation,  // 拡張基本情報
    ObjectMaximumInformation
} OBJECT_INFORMATION_CLASS;

typedef struct _OBJECT_BASIC_INFORMATION {
    ULONG Attributes;
    ULONG GrantedAccess;
    ULONG HandleCount;
    ULONG PointerCount;
    ULONG PagedPoolCharge;
    ULONG NonPagedPoolCharge;
    ULONG Reserved[3];
    ULONG CreateTimeLow;
    ULONG CreateTimeHigh;
    ULONG Reserved2[3];
    ULONG Reserved3;
} OBJECT_BASIC_INFORMATION;



// Win32API関数プロトタイプ
using pOpenProcess = HANDLE(WINAPI*)(DWORD dwDesiredAccess, BOOL bInheritHandle, DWORD dwProcessId);
using pVirtualAllocEx = LPVOID(WINAPI*)(HANDLE hProcess, LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect);
using pWriteProcessMemory = BOOL(WINAPI*)(HANDLE hProcess, LPVOID lpBaseAddress, LPCVOID lpBuffer, SIZE_T nSize, SIZE_T* lpNumberOfBytesWritten);
using pCreateRemoteThread = HANDLE(WINAPI*)(HANDLE hProcess, LPSECURITY_ATTRIBUTES lpThreadAttributes, SIZE_T dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId);
using pTerminateProcess = BOOL(WINAPI*)(HANDLE hProcess, UINT uExitCode);

// 非公開API関数プロトタイプ
using pNtTestAlert = NTSTATUS(NTAPI*)();
using pNtDeleteFile = NTSTATUS(NTAPI*)(POBJECT_ATTRIBUTES ObjectAttributes);
using pNtTerminateProcess = NTSTATUS(NTAPI*)(HANDLE ProcessHandle, NTSTATUS ExitStatus);
using pNtCreateProcessEx = NTSTATUS(NTAPI*)(PHANDLE ProcessHandle, ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes, HANDLE ParentProcess, ULONG Flags, HANDLE SectionHandle, HANDLE DebugPort, HANDLE ExceptionPort, BOOLEAN InJob);
using pNtCreateThreadEx = NTSTATUS(NTAPI*)(PHANDLE ThreadHandle, ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes, HANDLE ProcessHandle, PVOID StartRoutine, PVOID Argument, ULONG CreateFlags, ULONG_PTR ZeroBits, SIZE_T StackSize, SIZE_T MaximumStackSize, PVOID AttributeList);
using ptQueryInformationProcess = NTSTATUS(NTAPI*)(HANDLE ProcessHandle, DWORD ProcessInformationClass, PVOID ProcessInformation, ULONG ProcessInformationLength, PULONG ReturnLength);
using pNtCreateSection = NTSTATUS(NTAPI*)(PHANDLE SectionHandle, ULONG DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes, PLARGE_INTEGER MaximumSize, ULONG PageAttributes, ULONG SectionAttributes, HANDLE FileHandle);
using pNtMapViewOfSection = NTSTATUS(NTAPI*)(HANDLE SectionHandle, HANDLE ProcessHandle, PVOID* BaseAddress, ULONG_PTR ZeroBits, SIZE_T CommitSize, PLARGE_INTEGER SectionOffset, PSIZE_T ViewSize, DWORD InheritDisposition, ULONG AllocationType, ULONG Win32Protect);
using pRtlCreateUserThread = NTSTATUS(NTAPI*)(HANDLE ProcessHandle, PSECURITY_DESCRIPTOR SecurityDescriptor, BOOLEAN CreateSuspended, ULONG StackZeroBits, PULONG StackReserved, PULONG StackCommit, PVOID StartAddress, PVOID StartParameter, PHANDLE ThreadHandle, PCLIENT_ID ClientID);
using pNtOpenProcess = NTSTATUS(NTAPI*)(PHANDLE ProcessHandle, ACCESS_MASK AccessMask, POBJECT_ATTRIBUTES ObjectAttributes, PCLIENT_ID ClientID);
using pNtAllocateVirtualMemory = NTSTATUS(NTAPI*)(HANDLE ProcessHandle, PVOID* BaseAddress, ULONG ZeroBits, PULONG RegionSize, ULONG AllocationType, ULONG Protect);
using pNtWriteVirtualMemory = NTSTATUS(NTAPI*)(HANDLE ProcessHandle, PVOID BaseAddress, PVOID Buffer, ULONG NumberOfBytesToWrite, PULONG NumberOfBytesWritten);
using pRtlInitUnicodeString = NTSTATUS(NTAPI*)(PUNICODE_STRING DestinationString, PCWSTR SourceString);
using pLdrLoadDll = NTSTATUS(NTAPI*)(PWCHAR PathToFile, ULONG Flags, PUNICODE_STRING ModuleFileName, PHANDLE ModuleHandle);
using pNtQueueApcThread = NTSTATUS(NTAPI*)(HANDLE ThreadHandle, PIO_APC_ROUTINE ApcRoutine, PVOID ApcRoutineContext, PIO_STATUS_BLOCK ApcStatusBlock, ULONG ApcReserved);
using pNtOpenThread = NTSTATUS(NTAPI*)(PHANDLE ThreadHandle, ACCESS_MASK AccessMask, POBJECT_ATTRIBUTES ObjectAttributes, PCLIENT_ID ClientId);
using pNtShutdownSystem = NTSTATUS(NTAPI*)(SHUTDOWN_ACTION ShutdownAction);
using pNtCreateFile = NTSTATUS(NTAPI*)(PHANDLE FileHandle, ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes, PIO_STATUS_BLOCK IoStatusBlock, PLARGE_INTEGER AllocationSize, ULONG FileAttributes, ULONG ShareAccess, ULONG CreateDisposition, ULONG CreateOptions, PVOID Ea, ULONG EaLength);
using pNtOpenKey = NTSTATUS(NTAPI*)(PHANDLE keyHandle, ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes);
using pNtSetValueKey = NTSTATUS(NTAPI*)(HANDLE KeyHandle, PUNICODE_STRING ValueName, ULONG TitleIndex, ULONG Type, PVOID Data, ULONG DataSize);
using pNtQueryValueKey = NTSTATUS(NTAPI*)(HANDLE KeyHandle, PUNICODE_STRING ValueName, KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass, PVOID KeyValueInformation, ULONG KeyValueInformationLength, PULONG ResultLength);
using pNtDeleteValueKey = NTSTATUS(NTAPI*)(HANDLE KeyHandle, PUNICODE_STRING ValueName);
using pNtDuplicateObject = NTSTATUS(NTAPI*)(HANDLE SourceHandle, HANDLE TargetProcessHandle, PHANDLE TargetHandle, ULONG DesiredAccess, ULONG Attributes, ACCESS_MASK Options);
using pNtSetInformationObject = NTSTATUS(NTAPI*)(HANDLE ObjectHandle, OBJECT_INFORMATION_CLASS ObjectInformationClass, PVOID ObjectInformation, ULONG ObjectInformationLength);
using pNtQueryObject = NTSTATUS(NTAPI*)(HANDLE ObjectHandle, OBJECT_INFORMATION_CLASS ObjectInformationClass, PVOID ObjectInformation, ULONG ObjectInformationLength, PULONG ReturnLength);
using pNtSuspendProcess = NTSTATUS(NTAPI*)(HANDLE ProcessHandle);
using pNtResumeProcess = NTSTATUS(NTAPI*)(HANDLE ProcessHandle);
using pRtlAdjustPrivilege = NTSTATUS(NTAPI*)(ULONG Privilege, BOOLEAN Enable, BOOLEAN CurrentThread, PBOOLEAN Enabled);
using pNtRaiseHardError =NTSTATUS(NTAPI*)(NTSTATUS ErrorStatus, ULONG NumberOfParameters,ULONG UnicodeStringParameterMask, PULONG_PTR* Parameters, ULONG ValidResponseOption, PULONG Response);


