// 仮想マシンを検知する関数を定義
#include "AntiVM.h"

void CheckBIOS() {
    int cpuInfo[4];
    char manufacturer[13];

    // CPUIDを使用してBIOSベンダー情報を取得
    __cpuid(cpuInfo, 0);
    memcpy(manufacturer, &cpuInfo[1], 4);
    memcpy(manufacturer + 4, &cpuInfo[3], 4);
    memcpy(manufacturer + 8, &cpuInfo[2], 4);
    manufacturer[12] = '\0';

    // 仮想環境の確認
    if (strcmp(manufacturer, "VMwareVMware") == 0) {
        //std::cout << "VMware detected" << std::endl;
        exit(1);
    }
    else if (strcmp(manufacturer, "Microsoft Hv") == 0) {
        //std::cout << "Hyper-V detected" << std::endl;
        exit(1);

    }
    else if (strcmp(manufacturer, "KVMKVMKVMKVM") == 0) {
        //std::cout << "KVM detected" << std::endl;
        exit(1);
    }
    else if (strcmp(manufacturer, "VBoxVBoxVBox") == 0) {
        //std::cout << "VirtualBox detected" << std::endl;
        exit(1);
    }
    else {
        //std::cout << "検知されませんでした" << std::endl;
    }
}

// プロセスリストによる検出
void CheckProcesses() {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnapshot, &pe32)) {
        do {
            // VMware関連プロセスの検出
            if (wcscmp(pe32.szExeFile, L"vmtoolsd.exe") == 0 ||
                wcscmp(pe32.szExeFile, L"vmwaretray.exe") == 0 ||
                wcscmp(pe32.szExeFile, L"vmwareuser.exe") == 0) {
                //wprintf(L"VMware process detected: %s\n", pe32.szExeFile);
                exit(1);
            }
            // VirtualBox関連プロセスの検出
            else if (wcscmp(pe32.szExeFile, L"VBoxService.exe") == 0 ||
                wcscmp(pe32.szExeFile, L"VBoxTray.exe") == 0) {
                //wprintf(L"VirtualBox process detected: %s\n", pe32.szExeFile);
                exit(1);
            }
        } while (Process32Next(hSnapshot, &pe32));
    }

    //std::cout << "検出されませんでした" << std::endl;

    CloseHandle(hSnapshot);
}

// デバイスドライバによる検出
void CheckDrivers() {
    SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE);
    if (hSCM == NULL) {
        return;
    }

    DWORD bytesNeeded = 0;
    DWORD servicesReturned = 0;
    DWORD resumeHandle = 0;


    EnumServicesStatusEx(hSCM, SC_ENUM_PROCESS_INFO, SERVICE_DRIVER,
        SERVICE_STATE_ALL, NULL, 0, &bytesNeeded, &servicesReturned,
        &resumeHandle, NULL);

    if (bytesNeeded > 0) {
        LPBYTE lpServices = (LPBYTE)malloc(bytesNeeded);
        if (lpServices != NULL) {
            if (EnumServicesStatusEx(hSCM, SC_ENUM_PROCESS_INFO, SERVICE_DRIVER,
                SERVICE_STATE_ALL, lpServices, bytesNeeded, &bytesNeeded,
                &servicesReturned, &resumeHandle, NULL)) {

                LPENUM_SERVICE_STATUS_PROCESS services = (LPENUM_SERVICE_STATUS_PROCESS)lpServices;
                for (DWORD i = 0; i < servicesReturned; i++) {
                    // VMware関連のドライバを検出
                    if (wcscmp(services[i].lpDisplayName, L"VMware Tools") == 0 ||
                        wcscmp(services[i].lpDisplayName, L"VMware") == 0) {
                        //wprintf(L"VM driver detected: %s\n", services[i].lpDisplayName);
                        exit(1);
                    }
                    // VirtualBox関連のドライバを検出
                    else if (wcscmp(services[i].lpDisplayName, L"VirtualBox") == 0 ||
                        wcscmp(services[i].lpDisplayName, L"VBox") == 0) {
                        //wprintf(L"VBox driver detected: %s\n", services[i].lpDisplayName);
                        exit(1);
                    }
                }
            }
            free(lpServices);
        }
    }

    CloseServiceHandle(hSCM);
}

// レジストリによる検出
void CheckRegistry() {
    HKEY hKey;

    // VMware Tools のレジストリキーをチェック
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\VMware, Inc.\\VMware Tools", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        //printf("VMware Tools registry key detected\n");
        RegCloseKey(hKey);
        exit(1);
    }

    // VirtualBox Guest Additions のレジストリキーをチェック
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Oracle\\VirtualBox Guest Additions", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        //printf("VirtualBox Guest Additions registry key detected\n");
        RegCloseKey(hKey);
        exit(1);
    }
}

