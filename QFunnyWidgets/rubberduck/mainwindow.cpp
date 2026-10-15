/**
 * Demo window for RubberDuckWidget.
 */
#include "mainwindow.h"
#include "rubberduck.h"

#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

namespace rubberduck {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("Rubber Duck demo"));
    setWindowIcon(QIcon(QStringLiteral(":/QFunnyWidgets/rubberduck/logo.png")));
    resize(560, 460);
    RubberDuckWidget::Options o;
    o.squeaked = [this](int count) { onSqueak(count); };
    m_duck = new RubberDuckWidget(o);
    m_info = new QLabel(tr("Click the duck, or describe your bug below."));
    m_info->setAlignment(Qt::AlignCenter);
    m_question = new QLineEdit();
    m_question->setPlaceholderText(tr("My code works but I do not know why..."));
    QPushButton *ask = new QPushButton(tr("Explain to duck"));
    connect(ask, &QPushButton::clicked, this, &MainWindow::onAsk);
    connect(m_question, &QLineEdit::returnPressed, this, &MainWindow::onAsk);
    QHBoxLayout *row = new QHBoxLayout();
    row->addWidget(m_question, 1);
    row->addWidget(ask);
    QWidget *wdg = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(wdg);
    layout->addWidget(m_duck, 1);
    layout->addWidget(m_info);
    layout->addLayout(row);
    setCentralWidget(wdg);
}

void MainWindow::onAsk()
{
    m_duck->listen(m_question->text());
    m_question->clear();
}

void MainWindow::onSqueak(int count)
{
    m_info->setText(tr("Squeaks: %1").arg(count));
}

} // namespace rubberduck
