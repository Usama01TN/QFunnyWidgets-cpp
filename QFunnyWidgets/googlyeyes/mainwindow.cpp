/**
 * Demo window for GooglyEyesWidget.
 */
#include "mainwindow.h"
#include "googlyeyes.h"

#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QVBoxLayout>

namespace googlyeyes {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("Googly Eyes demo"));
    setWindowIcon(QIcon(QStringLiteral(":/QFunnyWidgets/googlyeyes/logo.png")));
    resize(640, 420);
    GooglyEyesWidget::Options o;
    o.eyeCount = 2;
    o.wobble = 0.6;
    o.gravity = 900;
    o.blinkInterval = 3500;
    o.poked = [this](int count) { onPoked(count); };
    o.gotAngry = [this]() { onAngry(); };
    m_eyes = new GooglyEyesWidget(o);
    m_info = new QLabel(tr("Move the mouse around. Click an eye to poke it (3 quick pokes = angry)."));
    m_info->setAlignment(Qt::AlignCenter);
    QSpinBox *count = new QSpinBox();
    count->setRange(1, 6);
    count->setValue(2);
    connect(count, QOverload<int>::of(&QSpinBox::valueChanged), m_eyes, &GooglyEyesWidget::setEyeCount);
    QSlider *wobble = new QSlider(Qt::Horizontal);
    wobble->setRange(0, 100);
    wobble->setValue(60);
    connect(wobble, &QSlider::valueChanged, this, [this](int v) { m_eyes->setWobble(v / 100.0); });
    QSlider *gravity = new QSlider(Qt::Horizontal);
    gravity->setRange(0, 3000);
    gravity->setValue(900);
    connect(gravity, &QSlider::valueChanged, this, [this](int v) { m_eyes->setGravity(v); });
    QPushButton *blink = new QPushButton(tr("Blink"));
    connect(blink, &QPushButton::clicked, m_eyes, &GooglyEyesWidget::blink);
    QPushButton *dizzy = new QPushButton(tr("Dizzy!"));
    connect(dizzy, &QPushButton::clicked, m_eyes, &GooglyEyesWidget::makeDizzy);
    QHBoxLayout *controls = new QHBoxLayout();
    for (QWidget *w : {static_cast<QWidget *>(new QLabel(tr("Eyes"))), static_cast<QWidget *>(count),
                       static_cast<QWidget *>(new QLabel(tr("Wobble"))), static_cast<QWidget *>(wobble),
                       static_cast<QWidget *>(new QLabel(tr("Gravity"))), static_cast<QWidget *>(gravity),
                       static_cast<QWidget *>(blink), static_cast<QWidget *>(dizzy)})
        controls->addWidget(w);
    QWidget *wdg = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(wdg);
    layout->addWidget(m_eyes, 1);
    layout->addWidget(m_info);
    layout->addLayout(controls);
    setCentralWidget(wdg);
}

void MainWindow::onPoked(int count)
{
    m_info->setText(tr("Ouch! Poked %1 time(s).").arg(count));
}

void MainWindow::onAngry()
{
    m_info->setText(tr("HEY! Stop poking me!"));
}

} // namespace googlyeyes
