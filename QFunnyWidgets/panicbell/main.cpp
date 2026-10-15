/**
 * Entry point for the PanicBellWidget demo.
 */
#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    panicbell::MainWindow w;
    w.show();
    return a.exec();
}
