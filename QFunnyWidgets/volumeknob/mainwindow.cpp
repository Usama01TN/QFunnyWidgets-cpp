/**
 * Demo window for VolumeKnobWidget.
 */
#include "mainwindow.h"
#include "volumeknob.h"

#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QVBoxLayout>

namespace volumeknob {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("Volume Knob demo"));
    setWindowIcon(QIcon(QStringLiteral(":/QFunnyWidgets/volumeknob/logo.png")));
    resize(640, 380);
    QHBoxLayout *row = new QHBoxLayout();
    for (int v : {0, 25, 55, 80, 100}) {
        VolumeKnobWidget::Options o;
        o.value = v;
        o.valueChanged = [this](int value) { onValue(value); };
        VolumeKnobWidget *k = new VolumeKnobWidget(o);
        m_knobs.append(k);
        row->addWidget(k);
    }
    m_info = new QLabel(tr("Drag around a knob, scroll, use arrows. Double-click or M to mute."));
    m_info->setAlignment(Qt::AlignCenter);
    QWidget *wdg = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(wdg);
    layout->addLayout(row, 1);
    layout->addWidget(m_info);
    setCentralWidget(wdg);
}

void MainWindow::onValue(int value)
{
    m_info->setText(tr("Volume: %1%").arg(value));
}

} // namespace volumeknob
