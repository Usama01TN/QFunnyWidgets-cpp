/**
 * Entry point for the ChameleonColorPickerWidget demo.
 */
#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    chameleonpicker::MainWindow w;
    w.show();
    return a.exec();
}
