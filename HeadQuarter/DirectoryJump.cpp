// explorer.exeを起動し、指定されたディレクトリを開く
#include "mainwindow.h"



void MainWindow::onGeneratedJmp() {
    QString path = "Generated";


    QProcess::startDetached("explorer.exe", QStringList() << path);
}

void MainWindow::onHostingJmp() {
    QString path = "Hosting";

    QProcess::startDetached("explorer.exe", QStringList() << path);
}


void MainWindow::onUploadedJmp() {
    QString path = "Uploaded";

    QProcess::startDetached("explorer.exe", QStringList() << path);
}
