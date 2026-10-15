/**
 * Entry point for the RobotCheckWidget demo.
 */
#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    robotcheck::MainWindow w;
    w.show();
    return a.exec();
}
