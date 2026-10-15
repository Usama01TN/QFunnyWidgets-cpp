/**
 * Demo window for HamsterLoaderWidget.
 */
#ifndef QFUNNYWIDGETS_HAMSTERLOADER_MAINWINDOW_H
#define QFUNNYWIDGETS_HAMSTERLOADER_MAINWINDOW_H

#include <QMainWindow>

class QCheckBox;
class QSlider;
class QTimer;
class HamsterLoaderWidget;

namespace hamsterloader {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

public slots:
    void simulate();
    void onIndeterminate(bool on);
    void onFinished();

private slots:
    void step();

private:
    HamsterLoaderWidget *m_loader;
    QSlider *m_slider;
    QCheckBox *m_indet;
    QTimer *m_sim;
};

} // namespace hamsterloader

#endif // QFUNNYWIDGETS_HAMSTERLOADER_MAINWINDOW_H
