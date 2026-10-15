/**
 * Demo window for SnailStepperWidget.
 */
#include "mainwindow.h"
#include "snailstepper.h"

#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace snailstepper {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("Snail Stepper demo"));
    setWindowIcon(QIcon(QStringLiteral(":/QFunnyWidgets/snailstepper/logo.png")));
    resize(640, 300);
    SnailStepperWidget::Options o;
    o.arrived = [this](int index) { onArrived(index); };
    m_stepper = new SnailStepperWidget(o);
    m_info = new QLabel(tr("Click Next a few times quickly. The snail will not be rushed."));
    m_info->setAlignment(Qt::AlignCenter);
    QPushButton *back = new QPushButton(tr("Back"));
    connect(back, &QPushButton::clicked, m_stepper, &SnailStepperWidget::previous);
    QPushButton *nxt = new QPushButton(tr("Next"));
    connect(nxt, &QPushButton::clicked, m_stepper, &SnailStepperWidget::next);
    QHBoxLayout *row = new QHBoxLayout();
    row->addWidget(back);
    row->addStretch(1);
    row->addWidget(nxt);
    QWidget *wdg = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(wdg);
    layout->addWidget(m_stepper, 1);
    layout->addWidget(m_info);
    layout->addLayout(row);
    setCentralWidget(wdg);
}

void MainWindow::onArrived(int index)
{
    m_info->setText(tr("The snail reached step %1.").arg(index + 1));
}

} // namespace snailstepper
