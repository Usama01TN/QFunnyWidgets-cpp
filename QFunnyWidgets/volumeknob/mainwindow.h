/**
 * Demo window for VolumeKnobWidget.
 */
#ifndef QFUNNYWIDGETS_VOLUMEKNOB_MAINWINDOW_H
#define QFUNNYWIDGETS_VOLUMEKNOB_MAINWINDOW_H

#include <QList>
#include <QMainWindow>

class QLabel;
class VolumeKnobWidget;

namespace volumeknob {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

public slots:
    void onValue(int value);

private:
    QList<VolumeKnobWidget *> m_knobs;
    QLabel *m_info = nullptr;
};

} // namespace volumeknob

#endif // QFUNNYWIDGETS_VOLUMEKNOB_MAINWINDOW_H
