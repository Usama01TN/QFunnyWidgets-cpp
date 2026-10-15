/**
 * Entry point for the QFunnyWidgets gallery.
 */
#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    // HiDPI switches exist (and matter) on Qt 5 only; Qt 6 always scales.
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);
#endif
    QApplication a(argc, argv);
    a.setStyle(QStringLiteral("Fusion"));
    MainWindow w;
    w.show();
    return a.exec();
}
