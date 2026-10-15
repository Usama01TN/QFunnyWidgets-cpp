/**
 * Demo window for EmptyStateWidget.
 */
#include "mainwindow.h"
#include "emptystate.h"

#include <QIcon>
#include <QListWidget>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>

namespace emptystate {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("Empty State demo"));
    setWindowIcon(QIcon(QStringLiteral(":/QFunnyWidgets/emptystate/logo.png")));
    resize(560, 420);
    EmptyStateWidget::Options o;
    o.title = QStringLiteral("No tasks yet");
    o.actionText = QStringLiteral("Add a task");
    o.actionClicked = [this]() { onAdd(); };
    m_empty = new EmptyStateWidget(o);
    m_list = new QListWidget();
    m_stack = new QStackedWidget();
    m_stack->addWidget(m_empty);
    m_stack->addWidget(m_list);
    QPushButton *clear = new QPushButton(tr("Clear list"));
    connect(clear, &QPushButton::clicked, this, &MainWindow::onClear);
    QWidget *wdg = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(wdg);
    layout->addWidget(m_stack, 1);
    layout->addWidget(clear);
    setCentralWidget(wdg);
}

void MainWindow::onAdd()
{
    m_list->addItem(tr("Task #%1: water the cactus").arg(m_list->count() + 1));
    m_stack->setCurrentWidget(m_list);
}

void MainWindow::onClear()
{
    m_list->clear();
    m_stack->setCurrentWidget(m_empty);
}

} // namespace emptystate
