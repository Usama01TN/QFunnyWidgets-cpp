/**
 * Demo window for ErrorGremlinWidget.
 */
#ifndef QFUNNYWIDGETS_ERRORGREMLIN_MAINWINDOW_H
#define QFUNNYWIDGETS_ERRORGREMLIN_MAINWINDOW_H

#include <QList>
#include <QMainWindow>

class ErrorGremlinWidget;

namespace errorgremlin {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    QList<ErrorGremlinWidget *> m_banners;
};

} // namespace errorgremlin

#endif // QFUNNYWIDGETS_ERRORGREMLIN_MAINWINDOW_H
