/**
 * Demo window for ChameleonColorPickerWidget.
 */
#ifndef QFUNNYWIDGETS_CHAMELEONPICKER_MAINWINDOW_H
#define QFUNNYWIDGETS_CHAMELEONPICKER_MAINWINDOW_H

#include <QMainWindow>

class QLabel;
class ChameleonColorPickerWidget;

namespace chameleonpicker {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    void onColor(const QColor &color);

private:
    QLabel *m_preview = nullptr;
    ChameleonColorPickerWidget *m_picker = nullptr;
};

} // namespace chameleonpicker

#endif // QFUNNYWIDGETS_CHAMELEONPICKER_MAINWINDOW_H
