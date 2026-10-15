/**
 * Demo window for SleepyCatSectionWidget.
 */
#include "mainwindow.h"
#include "catsection.h"

#include <QCheckBox>
#include <QFormLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QVBoxLayout>

namespace catsection {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("Sleepy Cat Section demo"));
    setWindowIcon(QIcon(QStringLiteral(":/QFunnyWidgets/catsection/logo.png")));
    resize(480, 520);
    QWidget *form = new QWidget();
    QFormLayout *fl = new QFormLayout(form);
    fl->addRow(tr("Server"), new QLineEdit(QStringLiteral("api.example.com")));
    fl->addRow(tr("Port"), new QSpinBox());
    fl->addRow(QString(), new QCheckBox(tr("Enable turbo mode")));
    QLabel *notes = new QLabel(tr("Nothing to see here. The cat is just resting its eyes."));
    notes->setWordWrap(true);
    QWidget *wdg = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(wdg);
    SleepyCatSectionWidget::Options a;
    a.title = tr("Advanced settings");
    a.content = form;
    layout->addWidget(new SleepyCatSectionWidget(a));
    SleepyCatSectionWidget::Options b;
    b.title = tr("Release notes");
    b.content = notes;
    b.expanded = true;
    b.catColor = QColor(QStringLiteral("#8D99AE"));
    layout->addWidget(new SleepyCatSectionWidget(b));
    layout->addStretch(1);
    setCentralWidget(wdg);
}

} // namespace catsection
