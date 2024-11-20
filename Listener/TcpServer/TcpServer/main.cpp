// リバースTCP
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <thread>
#include <atomic>
#include <windows.h>
#include <sstream>
#include <math.h>
#include <chrono>

#pragma comment(lib, "ws2_32.lib")

#define DEFAULT_BUFLEN 4096

// ソケットとアドレス情報
SOCKET listenSocket = INVALID_SOCKET;
SOCKET clientSocket = INVALID_SOCKET;
std::atomic<bool> running(true);


// メッセージを送信
void sendMessage() {
    std::string command;
    while (running) {
        std::getline(std::cin, command);

        if (command == "exit") {
            running = false;
            exit(0);
        }

        std::string msg = command + "\n";
        int totalSent = 0;
        int msgLength = static_cast<int>(msg.size());

        while (totalSent < msgLength) {
            int bytesSent = send(clientSocket, msg.c_str() + totalSent, msgLength - totalSent, 0);
            if (bytesSent == SOCKET_ERROR) {
                std::cerr << "送信に失敗しました。エラー: " << WSAGetLastError() << std::endl;
                running = false;
                break;
            }
            totalSent += bytesSent;
        }
    }
}

// メッセージを受信
void receiveMessage() {
    char recvBuffer[DEFAULT_BUFLEN];
    while (running) {
        memset(recvBuffer, 0, sizeof(recvBuffer));
        int bytesReceived = recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);
        if (bytesReceived > 0) {
            std::cout << std::string(recvBuffer, bytesReceived);
        }
        else if (bytesReceived == 0) {
            std::cerr << "クライアントが接続を閉じました。" << std::endl;
            running = false; // ループを終了
            break;
        }
        else {
            std::cerr << "受信に失敗しました。エラー: " << WSAGetLastError() << std::endl;
            running = false; // ループを終了
            break;
        }
    }
}

// 待機メッセージを表示
void ShowListeningInfo(const std::string& ipAddress, const std::string& port) {
    std::cout << "[+] TCPリスナー" << std::endl;
    std::cout << "[+] IPアドレス: " << ipAddress << std::endl;
    std::cout << "[+] ポート番号: " << port << std::endl;
    std::cout << "[+] クライアントの接続を待っています..." << std::endl;
}

void ShowAcceptInfo(sockaddr_in& clientAddr) {
    char ipStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &clientAddr.sin_addr, ipStr, INET_ADDRSTRLEN);
    std::cout << "---------------------------------------------------------------------------------------------------------------" << std::endl;
    std::cout << "[+] クライアントが接続されました。" << std::endl;
    std::cout << "[+] クライアントのIPアドレス: " << ipStr << std::endl;
    std::cout << "[+] クライアントのポート番号: " << ntohs(clientAddr.sin_port) << std::endl;
}

// メイン関数
int main(int argc, char* argv[]) {

    if (argc != 3) {
        std::cout << "使用方法: " << argv[0] << " <IPアドレス> <ポート番号>" << std::endl;
        return 1;
    }

    std::string ipAddress = argv[1];
    std::string port = argv[2];

    WSADATA wsaData;
    struct addrinfo* result = NULL, hints;

    WSAStartup(MAKEWORD(2, 2), &wsaData);

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    getaddrinfo(ipAddress.c_str(), port.c_str(), &hints, &result);

    listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

    // ソケットにバインド
    int success = bind(listenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (success == SOCKET_ERROR) {
        std::cerr << "[-] バインドに失敗しました。エラー: " << WSAGetLastError() << std::endl;
        return 1;
    }

    freeaddrinfo(result);

    ShowListeningInfo(ipAddress, port);

    // 接続をリッスン
    listen(listenSocket, SOMAXCONN);

    sockaddr_in clientAddr;
    int clientAddrSize = sizeof(clientAddr);

    clientSocket = accept(listenSocket, (struct sockaddr*)&clientAddr, &clientAddrSize);

    ShowAcceptInfo(clientAddr);

    std::thread receiver(receiveMessage);
    std::thread sender(sendMessage);

    sender.join();
    receiver.join();

    closesocket(clientSocket);
    closesocket(listenSocket);
    WSACleanup();
    return 0;
}
