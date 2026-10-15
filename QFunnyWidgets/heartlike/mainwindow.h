/**
 * Demo window for HeartLikeButtonWidget.
 */
#ifndef QFUNNYWIDGETS_HEARTLIKE_MAINWINDOW_H
#define QFUNNYWIDGETS_HEARTLIKE_MAINWINDOW_H

#include <QMainWindow>

class QLabel;
class HeartLikeButtonWidget;

namespace heartlike {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

public slots:
    void onLiked(bool liked);

private:
    HeartLikeButtonWidget *m_heart = nullptr;
    QLabel *m_info = nullptr;
};

} // namespace heartlike

#endif // QFUNNYWIDGETS_HEARTLIKE_MAINWINDOW_H
