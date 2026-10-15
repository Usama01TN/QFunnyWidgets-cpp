/**
 * Demo window for EmptyStateWidget.
 */
#ifndef QFUNNYWIDGETS_EMPTYSTATE_MAINWINDOW_H
#define QFUNNYWIDGETS_EMPTYSTATE_MAINWINDOW_H

#include <QMainWindow>

class QListWidget;
class QStackedWidget;
class EmptyStateWidget;

namespace emptystate {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

public slots:
    void onAdd();
    void onClear();

private:
    EmptyStateWidget *m_empty = nullptr;
    QListWidget *m_list = nullptr;
    QStackedWidget *m_stack = nullptr;
};

} // namespace emptystate

#endif // QFUNNYWIDGETS_EMPTYSTATE_MAINWINDOW_H
