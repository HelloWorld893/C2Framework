#include "mainwindow.h"

// ランダム文字列を生成
std::string MainWindow::generateRandomFileName() {
    const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::string result;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, sizeof(charset) - 2);

    for (int i = 0; i < 16; ++i) {
        result += charset[dis(gen)];
    }
    return result;
}

// exeファイルを作成して保存
bool MainWindow::SaveToFile(const QByteArray& data, QWidget* parent, const QString& defaultPath) {

    QString newFilePath = QFileDialog::getSaveFileName(parent, "保存先を選択", defaultPath, "Executable Files (*.exe)");
    if (newFilePath.isEmpty()) {
        return false;
    }

    QFile newFile(newFilePath);
    if (!newFile.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(parent, "エラー", "新しいファイルの作成に失敗しました。");
        return false;
    }

    newFile.write(data);
    newFile.close();

    return true;
}
