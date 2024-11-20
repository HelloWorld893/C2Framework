// TCPクライアント
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <windows.h>
#include <thread>
#pragma comment(lib, "ws2_32.lib")

#define DEFAULT_BUFLEN 4096

// メッセージを受信
void receiveMessage(SOCKET clientSocket) {
    char recvBuffer[DEFAULT_BUFLEN];
    while (true) {
        memset(recvBuffer, 0, sizeof(recvBuffer));
        int bytesReceived = recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);
        if (bytesReceived > 0) {
            std::cout << std::string(recvBuffer, bytesReceived);
        }
        else if (bytesReceived == 0) {
            std::cerr << "サーバーが接続を閉じました。" << std::endl;
            break;
        }
        else {
            std::cerr << "受信に失敗しました。エラー: " << WSAGetLastError() << std::endl;
            break;
        }
    }
}

// メッセージを送信
void sendMessage(SOCKET clientSocket) {
    std::string command;
    while (true) {
        std::getline(std::cin, command);

        if (command == "exit") {
            break;
        }

        std::string msg = command + "\n";
        int totalSent = 0;
        int msgLength = static_cast<int>(msg.size());

        while (totalSent < msgLength) {
            int bytesSent = send(clientSocket, msg.c_str() + totalSent, msgLength - totalSent, 0);
            if (bytesSent == SOCKET_ERROR) {
                std::cerr << "送信に失敗しました。エラー: " << WSAGetLastError() << std::endl;
                break;
            }
            totalSent += bytesSent;
        }
    }
}

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

    SOCKET clientSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

    // サーバーに接続
    int success = connect(clientSocket, result->ai_addr, (int)result->ai_addrlen);
    if (success == SOCKET_ERROR) {
        std::cerr << "接続に失敗しました。エラー: " << WSAGetLastError() << std::endl;
        return 1;
    }

    freeaddrinfo(result);

    std::cout << "[+] サーバーに接続しました。" << std::endl;

    // サーバーからのメッセージを受信するスレッド
    std::thread receiver(receiveMessage, clientSocket);

    // サーバーにメッセージを送信するスレッド
    std::thread sender(sendMessage, clientSocket);

    sender.join();
    receiver.join();

    closesocket(clientSocket);
    WSACleanup();
    return 0;
}
