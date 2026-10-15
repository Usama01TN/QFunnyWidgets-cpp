/**
 * Demo window for RubberDuckWidget.
 */
#ifndef QFUNNYWIDGETS_RUBBERDUCK_MAINWINDOW_H
#define QFUNNYWIDGETS_RUBBERDUCK_MAINWINDOW_H

#include <QMainWindow>

class QLabel;
class QLineEdit;
class RubberDuckWidget;

namespace rubberduck {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

public slots:
    void onAsk();
    void onSqueak(int count);

private:
    RubberDuckWidget *m_duck = nullptr;
    QLabel *m_info = nullptr;
    QLineEdit *m_question = nullptr;
};

} // namespace rubberduck

#endif // QFUNNYWIDGETS_RUBBERDUCK_MAINWINDOW_H
