/**
 * Demo window for HamsterLoaderWidget.
 */
#include "mainwindow.h"
#include "hamsterloader.h"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QTextStream>
#include <QTimer>
#include <QVBoxLayout>

namespace hamsterloader {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("Hamster Loader demo"));
    setWindowIcon(QIcon(QStringLiteral(":/QFunnyWidgets/hamsterloader/logo.png")));
    resize(520, 480);
    HamsterLoaderWidget::Options o;
    o.progress = 0.0;
    o.speed = 1.0;
    o.finished = [this]() { onFinished(); };
    m_loader = new HamsterLoaderWidget(o);
    m_slider = new QSlider(Qt::Horizontal);
    m_slider->setRange(0, 100);
    m_slider->setMinimumWidth(140);
    connect(m_slider, &QSlider::valueChanged, this, [this](int v) { m_loader->setProgress(v / 100.0); });
    m_indet = new QCheckBox(tr("Indeterminate"));
    connect(m_indet, &QCheckBox::toggled, this, &MainWindow::onIndeterminate);
    QPushButton *sim = new QPushButton(tr("Simulate download"));
    connect(sim, &QPushButton::clicked, this, &MainWindow::simulate);
    m_sim = new QTimer(this);
    connect(m_sim, &QTimer::timeout, this, &MainWindow::step);
    QHBoxLayout *controls = new QHBoxLayout();
    controls->addWidget(new QLabel(tr("Progress")));
    controls->addWidget(m_slider, 1);
    controls->addWidget(m_indet);
    controls->addWidget(sim);
    QWidget *wdg = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(wdg);
    layout->addWidget(m_loader, 1);
    layout->addLayout(controls);
    setCentralWidget(wdg);
    simulate();
}

void MainWindow::simulate()
{
    m_indet->setChecked(false);
    m_slider->setValue(0);
    m_loader->reset();
    m_sim->start(90);
}

void MainWindow::step()
{
    const int v = m_slider->value() + 1;
    m_slider->setValue(v);
    if (v >= 100)
        m_sim->stop();
}

void MainWindow::onIndeterminate(bool on)
{
    if (on) {
        m_sim->stop();
        m_loader->setProgress(-1.0);
    } else {
        m_loader->setProgress(m_slider->value() / 100.0);
    }
}

void MainWindow::onFinished()
{
    QTextStream(stdout) << tr("Loading finished!") << '\n';
}

} // namespace hamsterloader
