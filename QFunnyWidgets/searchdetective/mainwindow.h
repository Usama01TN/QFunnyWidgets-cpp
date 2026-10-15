/**
 * Demo window for SearchDetectiveWidget.
 */
#ifndef QFUNNYWIDGETS_SEARCHDETECTIVE_MAINWINDOW_H
#define QFUNNYWIDGETS_SEARCHDETECTIVE_MAINWINDOW_H

#include <QMainWindow>

class QListWidget;
class SearchDetectiveWidget;

namespace searchdetective {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    int find(const QString &text);

private:
    QListWidget *m_results = nullptr;
    SearchDetectiveWidget *m_search = nullptr;
};

} // namespace searchdetective

#endif // QFUNNYWIDGETS_SEARCHDETECTIVE_MAINWINDOW_H
