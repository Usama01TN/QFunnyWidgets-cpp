/**
 * Demo window for DachshundTabsWidget.
 */
#ifndef QFUNNYWIDGETS_DACHSHUNDTABS_MAINWINDOW_H
#define QFUNNYWIDGETS_DACHSHUNDTABS_MAINWINDOW_H

#include <QMainWindow>

class QStackedWidget;
class DachshundTabsWidget;

namespace dachshundtabs {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    QStackedWidget *m_pages = nullptr;
    DachshundTabsWidget *m_tabs = nullptr;
};

} // namespace dachshundtabs

#endif // QFUNNYWIDGETS_DACHSHUNDTABS_MAINWINDOW_H
