/**
 * Entry point for the AntBreadcrumbWidget demo.
 */
#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    antbreadcrumb::MainWindow w;
    w.show();
    return a.exec();
}
