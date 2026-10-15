/**
 * Entry point for the TypingIndicatorWidget demo.
 */
#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    typingindicator::MainWindow w;
    w.show();
    return a.exec();
}
