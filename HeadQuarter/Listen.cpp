#include "mainwindow.h"


// リスナープロセスを作成
void StartListenerProcess(const char* cmdLine){
    STARTUPINFOA si = {};
    PROCESS_INFORMATION pi = {};

    CreateProcessA(nullptr, (LPSTR)cmdLine, nullptr, nullptr, FALSE, CREATE_NEW_CONSOLE, nullptr, nullptr, &si, &pi);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

}

// コマンドラインを形成
const char* MakeCmdLine(QString program, QString ipAddress, QString portString){
    std::string ipAddressStd = ipAddress.toStdString();
    int port = portString.toInt();
    QStringList arguments;
    arguments << ipAddress << QString::number(port);
    QString argumentsStr = arguments.join(" ");

    QString commandLine = program + " " + argumentsStr;

    QByteArray commandLineUtf8 = commandLine.toUtf8();
    const char* cmdLine = commandLineUtf8.constData();

    return cmdLine;
}

// リバースTCP
void MainWindow::ReverseTcpListen() {
    InputDialog dialog("TCPサーバー", "127.0.0.1", "4444", this);

    if (dialog.exec() == QDialog::Accepted) {
        QString ipAddress = dialog.getIpAddress();
        QString portString = dialog.getPort();

        QString program = "Listener\\ReverseTCPListener.exe";

        const char* cmdLine = MakeCmdLine(program, ipAddress, portString);

        StartListenerProcess(cmdLine);


    }
}

// バインドTCP
void MainWindow::BindTcpListen() {
    InputDialog dialog("TCPクライアント", "127.0.0.1", "4444", this);

    if (dialog.exec() == QDialog::Accepted) {
        QString ipAddress = dialog.getIpAddress();
        QString portString = dialog.getPort();

        QString program = "Listener\\ReverseTcpListener.exe";

        const char* cmdLine = MakeCmdLine(program, ipAddress, portString);

        StartListenerProcess(cmdLine);


    }
}



