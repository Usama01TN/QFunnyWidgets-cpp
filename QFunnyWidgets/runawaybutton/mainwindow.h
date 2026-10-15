/**
 * Demo window for RunawayButtonWidget.
 */
#ifndef QFUNNYWIDGETS_RUNAWAYBUTTON_MAINWINDOW_H
#define QFUNNYWIDGETS_RUNAWAYBUTTON_MAINWINDOW_H

#include <QMainWindow>

class QLabel;
class RunawayButtonWidget;

namespace runawaybutton {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

public slots:
    void onEscaped(int count);
    void onCaught(int escapes);

private:
    RunawayButtonWidget *m_button;
    QLabel *m_info;
};

} // namespace runawaybutton

#endif // QFUNNYWIDGETS_RUNAWAYBUTTON_MAINWINDOW_H
