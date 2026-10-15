/**
 * Demo window for RobotCheckWidget.
 */
#include "mainwindow.h"
#include "robotcheck.h"

#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QVBoxLayout>

namespace robotcheck {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("Robot Check demo"));
    setWindowIcon(QIcon(QStringLiteral(":/QFunnyWidgets/robotcheck/logo.png")));
    resize(480, 360);
    RobotCheckWidget::Options o;
    o.verified = [this]() { onVerified(); };
    o.failed = [this]() { onFailed(); };
    m_check = new RobotCheckWidget(o);
    m_submit = new QPushButton(tr("Create account"));
    m_submit->setEnabled(false);
    QPushButton *reset = new QPushButton(tr("Reset"));
    connect(reset, &QPushButton::clicked, m_check, &RobotCheckWidget::reset);
    connect(reset, &QPushButton::clicked, this, [this]() { m_submit->setEnabled(false); });
    QSlider *fail = new QSlider(Qt::Horizontal);
    fail->setRange(0, 100);
    connect(fail, &QSlider::valueChanged, this, [this](int v) { m_check->setFailChance(v / 100.0); });
    QHBoxLayout *row = new QHBoxLayout();
    row->addWidget(new QLabel(tr("Suspicion level")));
    row->addWidget(fail, 1);
    row->addWidget(reset);
    QWidget *wdg = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(wdg);
    layout->addWidget(m_check, 1);
    layout->addWidget(m_submit);
    layout->addLayout(row);
    setCentralWidget(wdg);
}

void MainWindow::onVerified()
{
    m_submit->setEnabled(true);
}

void MainWindow::onFailed()
{
    m_submit->setEnabled(false);
}

} // namespace robotcheck
