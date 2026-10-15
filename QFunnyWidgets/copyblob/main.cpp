/**
 * Entry point for the CopyBlobButtonWidget demo.
 */
#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    copyblob::MainWindow w;
    w.show();
    return a.exec();
}
