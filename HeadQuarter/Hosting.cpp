#include "mainwindow.h"

// ダイアログの入力内容でスレッドを起動
void MainWindow::CreateHostingServer() {
    InputDialog dialog("Hostingサーバー", "127.0.0.1", "5555", this);

    if (dialog.exec() == QDialog::Accepted) {
        QString ipAddress = dialog.getIpAddress();
        QString portString = dialog.getPort();

        std::string ipAddressStd = ipAddress.toStdString();
        int port = portString.toInt();

        // サーバースレッドを作成
        QThread* serverThread = QThread::create([this, ipAddressStd, port]() {
            StartHostingServer(ipAddressStd, port);
        });
        serverThread->start();

        connect(serverThread, &QThread::finished, serverThread, &QObject::deleteLater);
    } else {
        exit(0);
    }
}

// ファイルホスティングサーバー起動
void MainWindow::StartHostingServer(const std::string& ipAddress, int port) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        logMessage("[-] Winsock初期化失敗 エラーコード: " + std::to_string(WSAGetLastError()));
        return;
    }

    HostingServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (HostingServerSocket == INVALID_SOCKET) {
        logMessage("[-] ソケット作成失敗 エラーコード: " + std::to_string(WSAGetLastError()));
        WSACleanup();
        return;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);

    if (inet_pton(AF_INET, ipAddress.c_str(), &serverAddr.sin_addr) <= 0) {
        logMessage("[-] 無効なIPアドレス: " + ipAddress);
        closesocket(HostingServerSocket);
        WSACleanup();
        return;
    }

    if (bind(HostingServerSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        logMessage("[-] バインド失敗 エラーコード: " + std::to_string(WSAGetLastError()));
        closesocket(HostingServerSocket);
        WSACleanup();
        return;
    }

    if (listen(HostingServerSocket, SOMAXCONN) == SOCKET_ERROR) {
        logMessage("[-] リッスン失敗 エラーコード: " + std::to_string(WSAGetLastError()));
        closesocket(HostingServerSocket);
        WSACleanup();
        return;
    }


    logMessage("Hostingサーバー起動 " + ipAddress + ":" + std::to_string(port));


    while (true) {
        SOCKET clientSocket = accept(HostingServerSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) {
            logMessage("[-] クライアント接続の受け入れに失敗 エラーコード: " + std::to_string(WSAGetLastError()));
            continue;
        }

        // クライアント処理を別スレッドで実行
        QThread* thread = QThread::create([this, clientSocket]() {
            HandleFileHosting(clientSocket);
        });
        thread->start();
    }
}

// Hostingフォルダの下に配置されたファイルをホスト
void MainWindow::HandleFileHosting(SOCKET clientSocket) {
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

    std::string request(buffer);
    logMessage(request);
    \
        std::string fileName;
    if (request.find("GET /") != std::string::npos) {
        size_t start = request.find("/") + 1;
        size_t end = request.find(" ", start);
        fileName = request.substr(start, end - start);
    }

    if (fileName.empty()) {
        fileName = "index.html";
    }

    // ホスティングするディレクトリのパス
    const std::string hosting = "Hosting\\";

    std::string fullPath = hosting + fileName;
    sendFile(clientSocket, fullPath);
    closesocket(clientSocket);
}


// 指定されたファイルをクライアントに送信
void MainWindow::sendFile(SOCKET clientSocket, const std::string &filePath) {
    std::ifstream file(filePath, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        std::string response = "HTTP/1.1 404 Not Found\r\n\r\n";
        send(clientSocket, response.c_str(), response.size(), 0);
        return;
    }

    std::string responseHeader = "HTTP/1.1 200 OK\r\n";
    responseHeader += "Content-Type: application/octet-stream\r\n";
    responseHeader += "Connection: close\r\n";
    responseHeader += "\r\n";
    send(clientSocket, responseHeader.c_str(), responseHeader.size(), 0);

    char buffer[1024];
    while (file.read(buffer, sizeof(buffer))) {
        send(clientSocket, buffer, file.gcount(), 0);
    }

    if (file.gcount() > 0) {
        send(clientSocket, buffer, file.gcount(), 0);
    }

    file.close();
}




