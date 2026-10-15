/**
 * Demo window for SnailStepperWidget.
 */
#ifndef QFUNNYWIDGETS_SNAILSTEPPER_MAINWINDOW_H
#define QFUNNYWIDGETS_SNAILSTEPPER_MAINWINDOW_H

#include <QMainWindow>

class QLabel;
class SnailStepperWidget;

namespace snailstepper {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

public slots:
    void onArrived(int index);

private:
    SnailStepperWidget *m_stepper = nullptr;
    QLabel *m_info = nullptr;
};

} // namespace snailstepper

#endif // QFUNNYWIDGETS_SNAILSTEPPER_MAINWINDOW_H
