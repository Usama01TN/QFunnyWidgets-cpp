/**
 * Demo window for NervousHoldButtonWidget.
 */
#include "mainwindow.h"
#include "holdbutton.h"

#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QSpinBox>
#include <QVBoxLayout>

namespace holdbutton {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("Nervous Hold Button demo"));
    setWindowIcon(QIcon(QStringLiteral(":/QFunnyWidgets/holdbutton/logo.png")));
    resize(560, 360);
    NervousHoldButtonWidget::Options o;
    o.holdDuration = 1800;
    o.confirmed = [this]() { onConfirmed(); };
    o.cancelled = [this](qreal progress) { onCancelled(progress); };
    m_button = new NervousHoldButtonWidget(o);
    m_button->setMinimumHeight(120);
    NervousHoldButtonWidget::Options co;
    co.stages = QStringList{QStringLiteral("Hold to send"), QStringLiteral("Sending..."), QStringLiteral("Almost...")};
    co.doneText = QStringLiteral("Sent!");
    co.reliefText = QStringLiteral("Draft saved");
    co.color = QColor(QStringLiteral("#5E60CE"));
    NervousHoldButtonWidget *calm = new NervousHoldButtonWidget(co);
    m_info = new QLabel(tr("Press and hold (mouse or Space). Let go early to chicken out."));
    m_info->setAlignment(Qt::AlignCenter);
    QSpinBox *dur = new QSpinBox();
    dur->setRange(300, 6000);
    dur->setSingleStep(100);
    dur->setSuffix(QStringLiteral(" ms"));
    dur->setValue(1800);
    connect(dur, QOverload<int>::of(&QSpinBox::valueChanged), m_button, &NervousHoldButtonWidget::setHoldDuration);
    connect(dur, QOverload<int>::of(&QSpinBox::valueChanged), calm, &NervousHoldButtonWidget::setHoldDuration);
    QHBoxLayout *row = new QHBoxLayout();
    row->addStretch(1);
    row->addWidget(new QLabel(tr("Hold duration")));
    row->addWidget(dur);
    row->addStretch(1);
    QWidget *wdg = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(wdg);
    layout->addWidget(m_button, 1);
    layout->addWidget(calm, 1);
    layout->addWidget(m_info);
    layout->addLayout(row);
    setCentralWidget(wdg);
}

void MainWindow::onConfirmed()
{
    m_info->setText(tr("Confirmed. The button has fainted."));
}

void MainWindow::onCancelled(qreal progress)
{
    m_info->setText(tr("Cancelled at %1%. Phew.").arg(int(progress * 100)));
}

} // namespace holdbutton
