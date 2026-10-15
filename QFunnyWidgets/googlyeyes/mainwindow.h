/**
 * Demo window for GooglyEyesWidget.
 */
#ifndef QFUNNYWIDGETS_GOOGLYEYES_MAINWINDOW_H
#define QFUNNYWIDGETS_GOOGLYEYES_MAINWINDOW_H

#include <QMainWindow>

class QLabel;
class GooglyEyesWidget;

namespace googlyeyes {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

public slots:
    void onPoked(int count);
    void onAngry();

private:
    GooglyEyesWidget *m_eyes;
    QLabel *m_info;
};

} // namespace googlyeyes

#endif // QFUNNYWIDGETS_GOOGLYEYES_MAINWINDOW_H
