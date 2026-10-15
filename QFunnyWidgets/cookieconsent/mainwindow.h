/**
 * Demo window for CookieConsentWidget.
 */
#ifndef QFUNNYWIDGETS_COOKIECONSENT_MAINWINDOW_H
#define QFUNNYWIDGETS_COOKIECONSENT_MAINWINDOW_H

#include <QMainWindow>

class QLabel;
class CookieConsentWidget;

namespace cookieconsent {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

public slots:
    void onAccepted();
    void onDeclined();

private:
    CookieConsentWidget *m_banner = nullptr;
    QLabel *m_info = nullptr;
};

} // namespace cookieconsent

#endif // QFUNNYWIDGETS_COOKIECONSENT_MAINWINDOW_H
