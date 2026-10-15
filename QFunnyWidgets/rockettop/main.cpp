/**
 * Entry point for the RocketBackToTop demo.
 */
#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    rockettop::MainWindow w;
    w.show();
    return a.exec();
}
