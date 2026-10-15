/**
 * Entry point for the VolumeKnobWidget demo.
 */
#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    volumeknob::MainWindow w;
    w.show();
    return a.exec();
}
