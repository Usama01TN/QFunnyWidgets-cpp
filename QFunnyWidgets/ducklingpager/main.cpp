/**
 * Entry point for the DucklingPagerWidget demo.
 */
#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ducklingpager::MainWindow w;
    w.show();
    return a.exec();
}
