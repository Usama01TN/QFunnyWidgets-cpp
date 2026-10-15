/**
 * Demo window for AutosaveSquirrelWidget.
 */
#include "mainwindow.h"
#include "autosave.h"

#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPointer>
#include <QRandomGenerator>
#include <QSlider>
#include <QTimer>
#include <QVBoxLayout>

namespace autosave {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("Autosave Squirrel demo"));
    setWindowIcon(QIcon(QStringLiteral(":/QFunnyWidgets/autosave/logo.png")));
    resize(520, 380);
    m_editor = new QPlainTextEdit();
    m_editor->setPlaceholderText(tr("Type something. Stop typing. Watch the squirrel."));
    AutosaveSquirrelWidget::Options o;
    o.autosaveDelay = 1200;
    o.retryDelay = 2500;
    o.saveRequested = [this]() { onSave(); };
    m_squirrel = new AutosaveSquirrelWidget(o);
    connect(m_editor, &QPlainTextEdit::textChanged, m_squirrel, &AutosaveSquirrelWidget::markDirty);
    m_fail = new QSlider(Qt::Horizontal);
    m_fail->setRange(0, 100);
    m_fail->setValue(20);
    QHBoxLayout *row = new QHBoxLayout();
    row->addWidget(new QLabel(tr("Server flakiness")));
    row->addWidget(m_fail, 1);
    QWidget *wdg = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(wdg);
    layout->addWidget(m_editor, 1);
    layout->addWidget(m_squirrel);
    layout->addLayout(row);
    setCentralWidget(wdg);
}

void MainWindow::onSave()
{
    const bool ok = QRandomGenerator::global()->generateDouble() * 100 >= m_fail->value();
    QPointer<AutosaveSquirrelWidget> sq = m_squirrel;
    QTimer::singleShot(1300, this, [sq, ok]() {
        if (sq)
            sq->saveFinished(ok);
    });
}

} // namespace autosave
