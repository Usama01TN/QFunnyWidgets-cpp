/**
 * Entry point for the HamsterLoaderWidget demo.
 */
#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    hamsterloader::MainWindow w;
    w.show();
    return a.exec();
}
