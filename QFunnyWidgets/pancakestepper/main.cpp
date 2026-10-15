/**
 * Entry point for the PancakeStepperWidget demo.
 */
#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    pancakestepper::MainWindow w;
    w.show();
    return a.exec();
}
