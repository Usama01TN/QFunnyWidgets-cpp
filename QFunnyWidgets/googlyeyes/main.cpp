/**
 * Entry point for the GooglyEyesWidget demo.
 */
#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    googlyeyes::MainWindow w;
    w.show();
    return a.exec();
}
