#include "mainwindow.h"



// メニューバーを作成
void MainWindow::CreateMenuBar() {
    QMenuBar *menuBar = new QMenuBar();

    QMenu* fileMenu = CreateFileMenu();
    QMenu* listenerMenu = CreateListenerMenu();
    QMenu* generatePayloadMenu = CreatePayloadGenerationMenu();
    QMenu* generateStagerInjectorMenu = CreateDropperMenu();
    QMenu* settingsMenu = CreateSettingsMenu();

    // メニューバーにメニューを追加
    menuBar->addMenu(fileMenu);
    menuBar->addMenu(listenerMenu);
    menuBar->addMenu(generatePayloadMenu);
    menuBar->addMenu(generateStagerInjectorMenu);
    menuBar->addMenu(settingsMenu);


    setMenuBar(menuBar);

}
//ファイルメニュー
QMenu* MainWindow::CreateFileMenu() {
    QMenu* file = new QMenu("ファイル");

    QStringList actions = {
        "Generatedを開く (生成したファイルが配置されるデフォルトフォルダ)",
        "Hostingを開く (ホスティングされるファイルのフォルダ)",
        "Uploadを開く （RATのUploadコマンドでアップされたファイルが保存されるフォルダ）",
    };

    QList<QAction*> actionList;

    for (const QString& actionName : actions) {
        QAction* action = new QAction(actionName);
        file->addAction(action);
        actionList.append(action);
    }

    connect(actionList[0], &QAction::triggered, this, &MainWindow::onGeneratedJmp);
    connect(actionList[1], &QAction::triggered, this, &MainWindow::onHostingJmp);
    connect(actionList[2], &QAction::triggered, this, &MainWindow::onUploadedJmp);

    return file;
}

// ドロッパー生成メニュー
QMenu* MainWindow::CreateDropperMenu() {
    QMenu* loader = new QMenu("ドロッパー生成");

    QStringList actions = {
        "プロセスハロウィング （EXEインジェクター）",
        "反射型DLL読み込み （PEローダー）",
        "反射型DLLインジェクション （PEインジェクター）",
        "トランザクションハロウィング （EXEインジェクター）",
        "プロセスゴースティング （EXEインジェクター）",
    };

    QList<QAction*> actionList;

    for (const QString& actionName : actions) {
        QAction* action = new QAction(actionName);
        loader->addAction(action);
        actionList.append(action);
    }

    connect(actionList[0], &QAction::triggered, this, &MainWindow::onProcessHollowingSelected);
    connect(actionList[1], &QAction::triggered, this, &MainWindow::onReflectiveDllLoadingSelected);
    connect(actionList[2], &QAction::triggered, this, &MainWindow::onReflectiveDllInjectionSelected);
    connect(actionList[3], &QAction::triggered, this, &MainWindow::onTransactionHollowingSelected);
    connect(actionList[4], &QAction::triggered, this, &MainWindow::onProcessGhostingSelected);

    return loader;
}

// リスナー起動メニュー
QMenu* MainWindow::CreateListenerMenu() {
    QMenu* listener = new QMenu("リスナー");

    QStringList actions = {
        "バインドTCP",
        "リバースTCP"
    };


    QList<QAction*> actionList;


    for (const QString& actionName : actions) {
        QAction* action = new QAction(actionName);
        action->setIcon(QIcon());
        listener->addAction(action);
        actionList.append(action);
    }

    connect(actionList[0], &QAction::triggered, this, &MainWindow::BindTcpListen);
    connect(actionList[1], &QAction::triggered, this, &MainWindow::ReverseTcpListen);

    return listener;
}

// ペイロード生成メニュー
QMenu* MainWindow::CreatePayloadGenerationMenu() {
    QMenu* generatePayload = new QMenu("ペイロード生成");

    QMenu* cmdMenu = new QMenu("コマンドプロンプト（cmd.exe）");
    QMenu* powershellMenu = new QMenu("Windows PowerShell（powershell.exe）");
    QMenu* vendettaMenu = new QMenu("Vendetta");

    generatePayload->addMenu(cmdMenu);
    generatePayload->addMenu(powershellMenu);
    generatePayload->addMenu(vendettaMenu);

    // cmd操作ペイロード
    QStringList cmdActions = {
        "TCPバインドシェル",
        "TCPリバースシェル"
    };

    QList<QAction*> cmdActionList;
    for (const QString& actionName : cmdActions) {
        QAction* action = new QAction(actionName, this);
        cmdMenu->addAction(action);
        cmdActionList.append(action);
    }

    connect(cmdActionList[0], &QAction::triggered, this, &MainWindow::GenerateBindTcp_Cmd);
    connect(cmdActionList[1], &QAction::triggered, this, &MainWindow::GenerateReverseTcp_Cmd);

    // powershell操作ペイロード
    QStringList powershellActions = {
        "TCPバインドシェル",
        "TCPリバースシェル"
    };

    QList<QAction*> powershellActionList;
    for (const QString& actionName : powershellActions) {
        QAction* action = new QAction(actionName, this);
        powershellMenu->addAction(action);
        powershellActionList.append(action);
    }

    connect(powershellActionList[0], &QAction::triggered, this, &MainWindow::GenerateBindTcp_Powershell);
    connect(powershellActionList[1], &QAction::triggered, this, &MainWindow::GenerateReverseTcp_Powershell);

    // VendettaRAT操作ペイロード
    QStringList vendettaActions = {
        "TCPリバースシェル"
    };

    QList<QAction*> vendettaActionList;
    for (const QString& actionName : vendettaActions) {
        QAction* action = new QAction(actionName, this);
        vendettaMenu->addAction(action);
        vendettaActionList.append(action);
    }

    connect(vendettaActionList[0], &QAction::triggered, this, &MainWindow::GenerateReverseTcp_Vendetta);

    return generatePayload;
}


// 設定メニュー
QMenu* MainWindow::CreateSettingsMenu() {
    QMenu* file = new QMenu("設定");

    QStringList actions = {
        "Qtについて"
    };

    QList<QAction*> actionList;

    for (const QString& actionName : actions) {
        QAction* action = new QAction(actionName);
        file->addAction(action);
        actionList.append(action);
    }

    connect(actionList[0], &QAction::triggered, this, [this]() {
        QMessageBox::aboutQt(this);
    });

    return file;
}



