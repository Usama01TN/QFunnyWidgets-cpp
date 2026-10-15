/**
 * Entry point for the DropZoneMonsterWidget demo.
 */
#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    dropzone::MainWindow w;
    w.show();
    return a.exec();
}
