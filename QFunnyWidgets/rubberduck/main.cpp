/**
 * Entry point for the RubberDuckWidget demo.
 */
#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    rubberduck::MainWindow w;
    w.show();
    return a.exec();
}
