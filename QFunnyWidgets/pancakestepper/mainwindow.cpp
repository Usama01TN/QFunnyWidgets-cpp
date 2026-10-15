/**
 * Demo window for PancakeStepperWidget.
 */
#include "mainwindow.h"
#include "pancakestepper.h"

#include <QIcon>
#include <QLabel>
#include <QVBoxLayout>

namespace pancakestepper {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("Pancake Stepper demo"));
    setWindowIcon(QIcon(QStringLiteral(":/QFunnyWidgets/pancakestepper/logo.png")));
    resize(360, 440);
    m_total = new QLabel();
    m_total->setAlignment(Qt::AlignCenter);
    PancakeStepperWidget::Options o;
    o.value = 2;
    o.maximum = 8;
    o.valueChanged = [this](int value) { onValue(value); };
    m_stepper = new PancakeStepperWidget(o);
    QWidget *wdg = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(wdg);
    layout->addWidget(new QLabel(tr("Breakfast order (+/-, wheel or arrow keys):")));
    layout->addWidget(m_stepper, 1);
    layout->addWidget(m_total);
    setCentralWidget(wdg);
    onValue(m_stepper->getValue());
}

void MainWindow::onValue(int value)
{
    m_total->setText(tr("Total: $%1").arg(value * 2.5, 0, 'f', 2));
}

} // namespace pancakestepper
