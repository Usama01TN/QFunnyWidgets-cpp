/**
 * Entry point for the DeadlineCountdownWidget demo.
 */
#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    deadline::MainWindow w;
    w.show();
    return a.exec();
}
