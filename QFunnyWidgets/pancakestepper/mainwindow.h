/**
 * Demo window for PancakeStepperWidget.
 */
#ifndef QFUNNYWIDGETS_PANCAKESTEPPER_MAINWINDOW_H
#define QFUNNYWIDGETS_PANCAKESTEPPER_MAINWINDOW_H

#include <QMainWindow>

class QLabel;
class PancakeStepperWidget;

namespace pancakestepper {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

public slots:
    void onValue(int value);

private:
    PancakeStepperWidget *m_stepper = nullptr;
    QLabel *m_total = nullptr;
};

} // namespace pancakestepper

#endif // QFUNNYWIDGETS_PANCAKESTEPPER_MAINWINDOW_H
