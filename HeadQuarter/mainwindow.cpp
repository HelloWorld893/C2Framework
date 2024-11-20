#include "mainwindow.h"












MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    CreateTab();

    CreateMenuBar();





    QTimer::singleShot(0, this, &MainWindow::CreateUploadServer);
    QTimer::singleShot(0, this, &MainWindow::CreateHostingServer);



}

