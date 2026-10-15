/**
 * Demo window for AutosaveSquirrelWidget.
 */
#ifndef QFUNNYWIDGETS_AUTOSAVE_MAINWINDOW_H
#define QFUNNYWIDGETS_AUTOSAVE_MAINWINDOW_H

#include <QMainWindow>

class QPlainTextEdit;
class QSlider;
class AutosaveSquirrelWidget;

namespace autosave {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

public slots:
    void onSave();

private:
    QPlainTextEdit *m_editor = nullptr;
    AutosaveSquirrelWidget *m_squirrel = nullptr;
    QSlider *m_fail = nullptr;
};

} // namespace autosave

#endif // QFUNNYWIDGETS_AUTOSAVE_MAINWINDOW_H
