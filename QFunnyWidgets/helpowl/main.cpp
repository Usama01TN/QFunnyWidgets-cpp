/**
 * Entry point for the HelpOwlWidget demo.
 */
#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    helpowl::MainWindow w;
    w.show();
    return a.exec();
}
