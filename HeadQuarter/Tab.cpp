#include "mainwindow.h"
#include <QApplication>

void MainWindow::CreateTab() {
    // タブウィジェットを作成
    tabWidget = new QTabWidget(this);

    // ログウィンドウをタブに追加
    serverLogWindow = new QTextEdit(this);
    serverLogWindow->setReadOnly(true);

    // フォント設定
    QFont font("Consolas", 13);
    serverLogWindow->setFont(font);


    // システムのカラースキームを取得
    QPalette palette = QApplication::palette();
    bool isDarkMode = palette.color(QPalette::Window).lightness() < 128;

    if (isDarkMode) {
        // ダークモードの場合
        serverLogWindow->setStyleSheet(
            "background-color: black;"
            "color: white;"
            "selection-background-color: #1E90FF;"
            "selection-color: #F8F8F2;"
            );
    } else {
        // ライトモードの場合
        serverLogWindow->setStyleSheet(
            "background-color: white;"
            "color: black;"
            "selection-background-color: #1E90FF;"
            "selection-color: #F8F8F2;"
            );
    }

    // ログウィンドウをタブに追加
    tabWidget->addTab(serverLogWindow, "サーバーログ");

    // タブウィジェットをメインウィンドウにセット
    setCentralWidget(tabWidget);
}



// サーバーログウィンドウにログを表示
void MainWindow::logMessage(const std::string& message) {
    serverLogWindow->append(QString::fromStdString(message));
}
