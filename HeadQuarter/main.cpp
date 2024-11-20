#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    w.resize(1000, 600);
    w.showMaximized();
    w.setWindowTitle("HeadQuarter");

    w.show();
    return a.exec();
}
