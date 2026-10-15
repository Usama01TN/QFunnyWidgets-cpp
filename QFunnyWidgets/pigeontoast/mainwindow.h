/**
 * Demo window for PigeonToast.
 */
#ifndef QFUNNYWIDGETS_PIGEONTOAST_MAINWINDOW_H
#define QFUNNYWIDGETS_PIGEONTOAST_MAINWINDOW_H

#include <QMainWindow>

class QLineEdit;
class PigeonToast;

namespace pigeontoast {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

public slots:
    void onSend();
    void onFlock();
    void onDelivered(const QString &text);

private:
    QLineEdit *m_edit = nullptr;
    PigeonToast *m_toast = nullptr;
};

} // namespace pigeontoast

#endif // QFUNNYWIDGETS_PIGEONTOAST_MAINWINDOW_H
