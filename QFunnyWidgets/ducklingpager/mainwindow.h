/**
 * Demo window for DucklingPagerWidget.
 */
#ifndef QFUNNYWIDGETS_DUCKLINGPAGER_MAINWINDOW_H
#define QFUNNYWIDGETS_DUCKLINGPAGER_MAINWINDOW_H

#include <QMainWindow>

class QStackedWidget;
class DucklingPagerWidget;

namespace ducklingpager {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    QStackedWidget *m_stack = nullptr;
    DucklingPagerWidget *m_pager = nullptr;
};

} // namespace ducklingpager

#endif // QFUNNYWIDGETS_DUCKLINGPAGER_MAINWINDOW_H
