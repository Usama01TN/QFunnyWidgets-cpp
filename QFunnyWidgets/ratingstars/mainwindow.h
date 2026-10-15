/**
 * Demo window for StarRatingWidget.
 */
#ifndef QFUNNYWIDGETS_RATINGSTARS_MAINWINDOW_H
#define QFUNNYWIDGETS_RATINGSTARS_MAINWINDOW_H

#include <QMainWindow>

class QLabel;
class StarRatingWidget;

namespace ratingstars {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

public slots:
    void onRating(int rating);

private:
    StarRatingWidget *m_stars = nullptr;
    StarRatingWidget *m_small = nullptr;
    QLabel *m_info = nullptr;
};

} // namespace ratingstars

#endif // QFUNNYWIDGETS_RATINGSTARS_MAINWINDOW_H
