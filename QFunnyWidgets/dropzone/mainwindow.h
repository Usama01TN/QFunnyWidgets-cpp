/**
 * Demo window for DropZoneMonsterWidget.
 */
#ifndef QFUNNYWIDGETS_DROPZONE_MAINWINDOW_H
#define QFUNNYWIDGETS_DROPZONE_MAINWINDOW_H

#include <QMainWindow>

class QLabel;
class QListWidget;
class DropZoneMonsterWidget;

namespace dropzone {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

public slots:
    void onDropped(const QStringList &paths);
    void onRejected(const QStringList &paths);

private:
    DropZoneMonsterWidget *m_zone = nullptr;
    QLabel *m_info = nullptr;
    QListWidget *m_list = nullptr;
};

} // namespace dropzone

#endif // QFUNNYWIDGETS_DROPZONE_MAINWINDOW_H
