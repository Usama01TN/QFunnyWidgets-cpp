/**
 * Entry point for the StarRatingWidget demo.
 */
#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ratingstars::MainWindow w;
    w.show();
    return a.exec();
}
