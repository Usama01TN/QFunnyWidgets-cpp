/**
 * Demo window for DeadlineCountdownWidget.
 */
#ifndef QFUNNYWIDGETS_DEADLINE_MAINWINDOW_H
#define QFUNNYWIDGETS_DEADLINE_MAINWINDOW_H

#include <QMainWindow>

class QLabel;
class DeadlineCountdownWidget;

namespace deadline {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

public slots:
    void onExpired();

private:
    DeadlineCountdownWidget *m_deadline = nullptr;
    QLabel *m_info = nullptr;
};

} // namespace deadline

#endif // QFUNNYWIDGETS_DEADLINE_MAINWINDOW_H
