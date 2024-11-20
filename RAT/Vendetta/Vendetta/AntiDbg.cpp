// デバッガを検知する関数を定義
#include "AntiDbg.h"


// IsDebuggerPresent関数を使った検知
void IsDebuggerPresentByAPI() {
    if (IsDebuggerPresent()) {
        // 検知
        exit(1);
    }
    return;
}


// NtGlobalFlagを使った検知
void NtGlobalFlagByPEB() {
    PPEB ppeb = (PPEB)__readgsqword(0x60);
    DWORD NtGlobalFlag = ppeb->NtGlobalFlag;
    if (NtGlobalFlag & 0x70) {
        //検知
        exit(1);
    }
}



// tlsコールバックを使った検知
#define NtCurrentProcess() (HANDLE)-1

void NTAPI __stdcall TLS_CALLBACK(PVOID DllHandle, DWORD dwReason, PVOID Reserved){
    HANDLE DebugPort = NULL;
    if (!NtQueryInformationProcess(NtCurrentProcess(), 7, &DebugPort, sizeof(HANDLE), NULL))
    {
        if (DebugPort)
        {
            // 検知
            exit(1);
        }
    }
}


EXTERN_C const PIMAGE_TLS_CALLBACK _tls_callback[] = { TLS_CALLBACK, 0 };






// CheckRemoteDebuggerPresent関数を使った検知
void CheckRemoteDebugger() {

    BOOL isDebug = FALSE;
    if (CheckRemoteDebuggerPresent(GetCurrentProcess(), &isDebug)) {
        if (isDebug) {
            // 検知
            exit(1);
        }
    }
}



// ウィンドウ名での検知
void WindowNameBlock() {

    LPCWSTR windowName = L"x64dbg";
    if (FindWindow(NULL, windowName)) {
        // 検知
        exit(1);
    }
}

// プロセス名での検知
void IsProcessRunning() {
    bool result = false;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32);
        if (Process32First(hSnapshot, &pe32)) {
            do {
                if (wcscmp(pe32.szExeFile, L"x64dbg.exe") == 0) {
                    result = true;
                    break;
                }
            } while (Process32Next(hSnapshot, &pe32));
        }
        CloseHandle(hSnapshot);
        
        if (result) {
            // 検知
            exit(1);
        }
    }
}

void AntiDbg() {
    IsDebuggerPresentByAPI();
    CheckRemoteDebugger();
    NtGlobalFlagByPEB();
    WindowNameBlock();
    IsProcessRunning();
}