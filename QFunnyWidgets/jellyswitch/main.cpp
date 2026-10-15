/**
 * Entry point for the JellySwitchWidget demo.
 */
#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    jellyswitch::MainWindow w;
    w.show();
    return a.exec();
}
