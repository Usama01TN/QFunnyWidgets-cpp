/**
 * Entry point for the HeartLikeButtonWidget demo.
 */
#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    heartlike::MainWindow w;
    w.show();
    return a.exec();
}
