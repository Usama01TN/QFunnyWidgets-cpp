/**
 * Entry point for the PufferCounterWidget demo.
 */
#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    puffercounter::MainWindow w;
    w.show();
    return a.exec();
}
