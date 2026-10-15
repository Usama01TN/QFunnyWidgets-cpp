/**
 * Entry point for the SnailStepperWidget demo.
 */
#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    snailstepper::MainWindow w;
    w.show();
    return a.exec();
}
