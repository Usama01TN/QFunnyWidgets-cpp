/**
 * Entry point for the PresenceAvatarWidget demo.
 */
#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    presenceavatar::MainWindow w;
    w.show();
    return a.exec();
}
