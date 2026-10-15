/**
 * Demo window for BatteryBuddyWidget.
 */
#ifndef QFUNNYWIDGETS_BATTERYBUDDY_MAINWINDOW_H
#define QFUNNYWIDGETS_BATTERYBUDDY_MAINWINDOW_H

#include <QMainWindow>

class QLabel;
class BatteryBuddyWidget;

namespace batterybuddy {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

public slots:
    void onLow();

private:
    BatteryBuddyWidget *m_battery = nullptr;
    QLabel *m_info = nullptr;
};

} // namespace batterybuddy

#endif // QFUNNYWIDGETS_BATTERYBUDDY_MAINWINDOW_H
