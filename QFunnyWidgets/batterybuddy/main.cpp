/**
 * Entry point for the BatteryBuddyWidget demo.
 */
#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    batterybuddy::MainWindow w;
    w.show();
    return a.exec();
}
