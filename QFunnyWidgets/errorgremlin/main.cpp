/**
 * Entry point for the ErrorGremlinWidget demo.
 */
#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    errorgremlin::MainWindow w;
    w.show();
    return a.exec();
}
