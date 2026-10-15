/**
 * Demo window for NervousHoldButtonWidget.
 */
#ifndef QFUNNYWIDGETS_HOLDBUTTON_MAINWINDOW_H
#define QFUNNYWIDGETS_HOLDBUTTON_MAINWINDOW_H

#include <QMainWindow>

class QLabel;
class NervousHoldButtonWidget;

namespace holdbutton {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

public slots:
    void onConfirmed();
    void onCancelled(qreal progress);

private:
    NervousHoldButtonWidget *m_button = nullptr;
    QLabel *m_info = nullptr;
};

} // namespace holdbutton

#endif // QFUNNYWIDGETS_HOLDBUTTON_MAINWINDOW_H
