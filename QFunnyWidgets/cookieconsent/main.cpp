/**
 * Entry point for the CookieConsentWidget demo.
 */
#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    cookieconsent::MainWindow w;
    w.show();
    return a.exec();
}
