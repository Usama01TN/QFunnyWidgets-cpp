/**
 * Entry point for the SleepyCatSectionWidget demo.
 */
#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    catsection::MainWindow w;
    w.show();
    return a.exec();
}
