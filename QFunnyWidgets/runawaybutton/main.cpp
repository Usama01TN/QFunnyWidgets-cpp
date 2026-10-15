/**
 * Entry point for the RunawayButtonWidget demo.
 */
#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    runawaybutton::MainWindow w;
    w.show();
    return a.exec();
}
