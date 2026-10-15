/**
 * Entry point for the AutosaveSquirrelWidget demo.
 */
#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    autosave::MainWindow w;
    w.show();
    return a.exec();
}
