/**
 * Entry point for the PasswordJudgeWidget demo.
 */
#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    passwordjudge::MainWindow w;
    w.show();
    return a.exec();
}
