/**
 * Demo window for DeadlineCountdownWidget.
 */
#include "mainwindow.h"
#include "deadline.h"

#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace deadline {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("Deadline Countdown demo"));
    setWindowIcon(QIcon(QStringLiteral(":/QFunnyWidgets/deadline/logo.png")));
    resize(560, 300);
    DeadlineCountdownWidget::Options o;
    o.duration = 30;
    o.title = QStringLiteral("Quarterly report due");
    o.expired = [this]() { onExpired(); };
    m_deadline = new DeadlineCountdownWidget(o);
    m_info = new QLabel(tr("A 30 second deadline. Watch the calendar cope."));
    QHBoxLayout *row = new QHBoxLayout();
    QPushButton *restart = new QPushButton(tr("Restart"));
    connect(restart, &QPushButton::clicked, this, [this]() { m_deadline->start(); });
    row->addWidget(restart);
    QPushButton *pause = new QPushButton(tr("Pause"));
    connect(pause, &QPushButton::clicked, this, [this]() { m_deadline->pause(); });
    row->addWidget(pause);
    QPushButton *resume = new QPushButton(tr("Resume"));
    connect(resume, &QPushButton::clicked, this, [this]() { m_deadline->resume(); });
    row->addWidget(resume);
    QPushButton *ext = new QPushButton(tr("Extend +15 s"));
    connect(ext, &QPushButton::clicked, this, [this]() { m_deadline->extend(15); });
    row->addWidget(ext);
    QPushButton *fast = new QPushButton(tr("Speed x5"));
    fast->setCheckable(true);
    connect(fast, &QPushButton::toggled, this, [this](bool on) { m_deadline->setTimeScale(on ? 5.0 : 1.0); });
    row->addWidget(fast);
    QWidget *wdg = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(wdg);
    layout->addWidget(m_deadline, 1);
    layout->addWidget(m_info);
    layout->addLayout(row);
    setCentralWidget(wdg);
}

void MainWindow::onExpired()
{
    m_info->setText(tr("Deadline missed. The calendar is fine. Everything is fine."));
}

} // namespace deadline
