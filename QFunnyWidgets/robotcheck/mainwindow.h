/**
 * Demo window for RobotCheckWidget.
 */
#ifndef QFUNNYWIDGETS_ROBOTCHECK_MAINWINDOW_H
#define QFUNNYWIDGETS_ROBOTCHECK_MAINWINDOW_H

#include <QMainWindow>

class QPushButton;
class RobotCheckWidget;

namespace robotcheck {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

public slots:
    void onVerified();
    void onFailed();

private:
    RobotCheckWidget *m_check = nullptr;
    QPushButton *m_submit = nullptr;
};

} // namespace robotcheck

#endif // QFUNNYWIDGETS_ROBOTCHECK_MAINWINDOW_H
