/**
 * Demo window for JellySwitchWidget.
 */
#ifndef QFUNNYWIDGETS_JELLYSWITCH_MAINWINDOW_H
#define QFUNNYWIDGETS_JELLYSWITCH_MAINWINDOW_H

#include <QList>
#include <QMainWindow>

class QLabel;
class JellySwitchWidget;

namespace jellyswitch {

/**
 * MainWindow class.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

public slots:
    void onWobbliness(int value);
    void onToggled(bool checked);

private:
    QList<JellySwitchWidget *> m_switches;
    QLabel *m_info;
};

} // namespace jellyswitch

#endif // QFUNNYWIDGETS_JELLYSWITCH_MAINWINDOW_H
