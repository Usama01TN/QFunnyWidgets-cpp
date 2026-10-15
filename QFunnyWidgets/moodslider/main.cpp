/**
 * Entry point for the MoodSliderWidget demo.
 */
#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    moodslider::MainWindow w;
    w.show();
    return a.exec();
}
