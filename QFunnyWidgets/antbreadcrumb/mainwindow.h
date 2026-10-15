/**
 * Demo window for AntBreadcrumbWidget.
 */
#ifndef QFUNNYWIDGETS_ANTBREADCRUMB_MAINWINDOW_H
#define QFUNNYWIDGETS_ANTBREADCRUMB_MAINWINDOW_H

#include <QMainWindow>

class QListWidget;
class AntBreadcrumbWidget;

namespace antbreadcrumb {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

public slots:
    void onPath(const QStringList &path);

private:
    AntBreadcrumbWidget *m_crumbs = nullptr;
    QListWidget *m_list = nullptr;
};

} // namespace antbreadcrumb

#endif // QFUNNYWIDGETS_ANTBREADCRUMB_MAINWINDOW_H
