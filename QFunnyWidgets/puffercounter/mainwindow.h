/**
 * Demo window for PufferCounterWidget.
 */
#ifndef QFUNNYWIDGETS_PUFFERCOUNTER_MAINWINDOW_H
#define QFUNNYWIDGETS_PUFFERCOUNTER_MAINWINDOW_H

#include <QMainWindow>

class QPushButton;
class PufferCounterWidget;

namespace puffercounter {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

public slots:
    void onLimit(bool over);

private:
    PufferCounterWidget *m_counter = nullptr;
    QPushButton *m_post = nullptr;
};

} // namespace puffercounter

#endif // QFUNNYWIDGETS_PUFFERCOUNTER_MAINWINDOW_H
