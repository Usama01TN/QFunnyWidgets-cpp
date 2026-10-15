/**
 * Demo window for RocketBackToTop.
 */
#ifndef QFUNNYWIDGETS_ROCKETTOP_MAINWINDOW_H
#define QFUNNYWIDGETS_ROCKETTOP_MAINWINDOW_H

#include <QMainWindow>

class RocketBackToTop;

namespace rockettop {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    RocketBackToTop *m_rocket = nullptr;
};

} // namespace rockettop

#endif // QFUNNYWIDGETS_ROCKETTOP_MAINWINDOW_H
