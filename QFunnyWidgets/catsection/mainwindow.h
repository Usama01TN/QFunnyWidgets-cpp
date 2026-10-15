/**
 * Demo window for SleepyCatSectionWidget.
 */
#ifndef QFUNNYWIDGETS_CATSECTION_MAINWINDOW_H
#define QFUNNYWIDGETS_CATSECTION_MAINWINDOW_H

#include <QMainWindow>

namespace catsection {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
};

} // namespace catsection

#endif // QFUNNYWIDGETS_CATSECTION_MAINWINDOW_H
