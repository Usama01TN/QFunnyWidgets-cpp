/**
 * Demo window for MoodSliderWidget.
 */
#ifndef QFUNNYWIDGETS_MOODSLIDER_MAINWINDOW_H
#define QFUNNYWIDGETS_MOODSLIDER_MAINWINDOW_H

#include <QMainWindow>

class QLabel;
class MoodSliderWidget;

namespace moodslider {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

public slots:
    void onValue(int value);
    void onMood(const QString &mood);

private:
    MoodSliderWidget *m_slider = nullptr;
    QLabel *m_info = nullptr;
};

} // namespace moodslider

#endif // QFUNNYWIDGETS_MOODSLIDER_MAINWINDOW_H
