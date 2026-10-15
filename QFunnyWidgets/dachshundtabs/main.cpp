/**
 * Entry point for the DachshundTabsWidget demo.
 */
#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    dachshundtabs::MainWindow w;
    w.show();
    return a.exec();
}
