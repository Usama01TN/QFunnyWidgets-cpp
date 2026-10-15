/**
 * Demo window for PasswordJudgeWidget.
 */
#ifndef QFUNNYWIDGETS_PASSWORDJUDGE_MAINWINDOW_H
#define QFUNNYWIDGETS_PASSWORDJUDGE_MAINWINDOW_H

#include <QMainWindow>

class QLabel;
class PasswordJudgeWidget;

namespace passwordjudge {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

public slots:
    void onStrength(int strength);

private:
    PasswordJudgeWidget *m_judge = nullptr;
    QLabel *m_info = nullptr;
};

} // namespace passwordjudge

#endif // QFUNNYWIDGETS_PASSWORDJUDGE_MAINWINDOW_H
