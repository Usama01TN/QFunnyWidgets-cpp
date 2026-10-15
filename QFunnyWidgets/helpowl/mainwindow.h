/**
 * Demo window for HelpOwlWidget.
 */
#ifndef QFUNNYWIDGETS_HELPOWL_MAINWINDOW_H
#define QFUNNYWIDGETS_HELPOWL_MAINWINDOW_H

#include <QMainWindow>

namespace helpowl {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
};

} // namespace helpowl

#endif // QFUNNYWIDGETS_HELPOWL_MAINWINDOW_H
