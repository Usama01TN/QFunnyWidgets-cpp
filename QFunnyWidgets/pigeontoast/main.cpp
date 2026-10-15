/**
 * Entry point for the PigeonToast demo.
 */
#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    pigeontoast::MainWindow w;
    w.show();
    return a.exec();
}
