/**
 * Entry point for the SearchDetectiveWidget demo.
 */
#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    searchdetective::MainWindow w;
    w.show();
    return a.exec();
}
