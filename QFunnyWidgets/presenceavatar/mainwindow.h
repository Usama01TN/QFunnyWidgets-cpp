/**
 * Demo window for PresenceAvatarWidget.
 */
#ifndef QFUNNYWIDGETS_PRESENCEAVATAR_MAINWINDOW_H
#define QFUNNYWIDGETS_PRESENCEAVATAR_MAINWINDOW_H

#include <QMainWindow>

class QLabel;
class PresenceAvatarWidget;

namespace presenceavatar {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    void onClicked(const QString &name);

private:
    PresenceAvatarWidget *m_me = nullptr;
    QLabel *m_info = nullptr;
};

} // namespace presenceavatar

#endif // QFUNNYWIDGETS_PRESENCEAVATAR_MAINWINDOW_H
