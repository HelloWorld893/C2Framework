#include "mainwindow.h"

// ダイアログの内容でサーバースレッド作成
void MainWindow::CreateUploadServer() {
    InputDialog dialog("Uploadedサーバー", "127.0.0.1", "6666", this);

    if (dialog.exec() == QDialog::Accepted) {
        QString ipAddress = dialog.getIpAddress();
        QString portString = dialog.getPort();

        // QStringをstd::stringに変換
        std::string ipAddressStd = ipAddress.toStdString();
        int port = portString.toInt();

        // サーバースレッドを作成
        QThread* serverThread = QThread::create([this, ipAddressStd, port]() {
            StartUploadServer(ipAddressStd, port);
        });
        serverThread->start();

        connect(serverThread, &QThread::finished, serverThread, &QObject::deleteLater);
    } else {
        exit(0);
    }
}

// アップロード受付サーバーを作成
void MainWindow::StartUploadServer(const std::string& ipAddress, int port) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        logMessage("[-] Winsock初期化失敗 エラーコード: " + std::to_string(WSAGetLastError()));
        return;
    }

    uploadServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (uploadServerSocket == INVALID_SOCKET) {
        logMessage("[-] ソケット作成失敗 エラーコード: " + std::to_string(WSAGetLastError()));
        WSACleanup();
        return;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);

    if (inet_pton(AF_INET, ipAddress.c_str(), &serverAddr.sin_addr) <= 0) {
        logMessage("[-] 無効なIPアドレス: " + ipAddress);
        closesocket(uploadServerSocket);
        WSACleanup();
        return;
    }

    if (bind(uploadServerSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        logMessage("[-] バインド失敗 エラーコード: " + std::to_string(WSAGetLastError()));
        closesocket(uploadServerSocket);
        WSACleanup();
        return;
    }

    if (listen(uploadServerSocket, SOMAXCONN) == SOCKET_ERROR) {
        logMessage("[-] リッスン失敗 エラーコード: " + std::to_string(WSAGetLastError()));
        closesocket(uploadServerSocket);
        WSACleanup();
        return;
    }


    logMessage("Uploadedサーバー起動 " + ipAddress + ":" + std::to_string(port));


    while (true) {
        SOCKET clientSocket = accept(uploadServerSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) {
            logMessage("[-] クライアント接続の受け入れに失敗 エラーコード: " + std::to_string(WSAGetLastError()));
            continue;
        }

        // クライアント処理を別スレッドで実行
        QThread* thread = QThread::create([this, clientSocket]() {
            HandleFileUpload(clientSocket);
        });
        thread->start();
    }
}

// 送信されたファイルデータを受信し、指定したディレクトリ内にランダムなファイル名で保存
void MainWindow::HandleFileUpload(SOCKET clientSocket) {
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));

    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived <= 0) {
        logMessage("[-] ファイルの受信エラー");
        closesocket(clientSocket);
        return;
    }

    std::string fileName = generateRandomFileName();

    // 保存先ディレクトリが存在しない場合は作成
    std::string fileDirectory = "Uploaded\\";
    if (CreateDirectoryA(fileDirectory.c_str(), NULL) == 0 && GetLastError() != ERROR_ALREADY_EXISTS) {
        logMessage("[-] ディレクトリ作成失敗: " + fileDirectory);
        closesocket(clientSocket);
        return;
    }

    // ランダムなファイル名を保存するディレクトリのパス
    std::string fullPath = fileDirectory + fileName;

    // ファイルの保存
    std::ofstream outFile(fullPath, std::ios::binary);
    if (!outFile.is_open()) {
        logMessage("[-] ファイルの保存に失敗: " + fullPath);
        closesocket(clientSocket);
        return;
    }

    outFile.write(buffer, bytesReceived);
    while ((bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0) {
        outFile.write(buffer, bytesReceived);
    }

    outFile.close();
    logMessage("[+] ファイル保存完了: " + fullPath);

    closesocket(clientSocket);
}


