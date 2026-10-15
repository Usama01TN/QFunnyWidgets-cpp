/**
 * Demo window for TypingIndicatorWidget.
 */
#ifndef QFUNNYWIDGETS_TYPINGINDICATOR_MAINWINDOW_H
#define QFUNNYWIDGETS_TYPINGINDICATOR_MAINWINDOW_H

#include <QMainWindow>

class QCheckBox;
class TypingIndicatorWidget;

namespace typingindicator {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

public slots:
    void onState(int state);

private:
    TypingIndicatorWidget *m_typing = nullptr;
    QCheckBox *m_auto = nullptr;
};

} // namespace typingindicator

#endif // QFUNNYWIDGETS_TYPINGINDICATOR_MAINWINDOW_H
