/**
 * Demo window for RunawayButtonWidget.
 */
#include "mainwindow.h"
#include "runawaybutton.h"

#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

namespace runawaybutton {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("Runaway Button demo"));
    setWindowIcon(QIcon(QStringLiteral(":/QFunnyWidgets/runawaybutton/logo.png")));
    resize(720, 460);
    RunawayButtonWidget::Options o;
    o.text = QStringLiteral("Click me!");
    o.maxEscapes = 7;
    o.color = QColor(QStringLiteral("#FF4D6D"));
    o.caught = [this](int escapes) { onCaught(escapes); };
    o.escaped = [this](int count) { onEscaped(count); };
    m_button = new RunawayButtonWidget(o);
    m_info = new QLabel(tr("Try to click the button. Good luck."));
    m_info->setAlignment(Qt::AlignCenter);
    QSpinBox *escapes = new QSpinBox();
    escapes->setRange(0, 50);
    escapes->setValue(7);
    escapes->setSpecialValueText(tr("Never gives up"));
    connect(escapes, QOverload<int>::of(&QSpinBox::valueChanged), m_button, &RunawayButtonWidget::setMaxEscapes);
    QPushButton *reset = new QPushButton(tr("Reset"));
    connect(reset, &QPushButton::clicked, m_button, &RunawayButtonWidget::reset);
    QHBoxLayout *controls = new QHBoxLayout();
    controls->addWidget(new QLabel(tr("Escapes before giving up")));
    controls->addWidget(escapes);
    controls->addStretch(1);
    controls->addWidget(reset);
    QWidget *wdg = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(wdg);
    layout->addWidget(m_button, 1);
    layout->addWidget(m_info);
    layout->addLayout(controls);
    setCentralWidget(wdg);
}

void MainWindow::onEscaped(int count)
{
    m_info->setText(tr("It escaped %1 time(s)...").arg(count));
}

void MainWindow::onCaught(int escapes)
{
    m_info->setText(tr("Caught it after %1 escapes!").arg(escapes));
}

} // namespace runawaybutton
