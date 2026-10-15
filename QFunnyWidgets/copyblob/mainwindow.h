/**
 * Demo window for CopyBlobButtonWidget.
 */
#ifndef QFUNNYWIDGETS_COPYBLOB_MAINWINDOW_H
#define QFUNNYWIDGETS_COPYBLOB_MAINWINDOW_H

#include <QMainWindow>

class QLineEdit;

namespace copyblob {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

public slots:
    void onCopied(const QString &text);

private:
    QLineEdit *m_paste = nullptr;
};

} // namespace copyblob

#endif // QFUNNYWIDGETS_COPYBLOB_MAINWINDOW_H
