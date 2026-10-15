/**
 * Entry point for the NervousHoldButtonWidget demo.
 */
#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    holdbutton::MainWindow w;
    w.show();
    return a.exec();
}
