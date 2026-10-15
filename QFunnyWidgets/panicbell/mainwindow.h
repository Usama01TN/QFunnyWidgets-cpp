/**
 * Demo window for PanicBellWidget.
 */
#ifndef QFUNNYWIDGETS_PANICBELL_MAINWINDOW_H
#define QFUNNYWIDGETS_PANICBELL_MAINWINDOW_H

#include <QMainWindow>

class QLabel;
class PanicBellWidget;

namespace panicbell {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

public slots:
    void onCount(int count);

private:
    PanicBellWidget *m_bell = nullptr;
    QLabel *m_info = nullptr;
};

} // namespace panicbell

#endif // QFUNNYWIDGETS_PANICBELL_MAINWINDOW_H
