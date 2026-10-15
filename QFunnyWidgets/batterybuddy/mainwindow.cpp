/**
 * Demo window for BatteryBuddyWidget.
 */
#include "mainwindow.h"
#include "batterybuddy.h"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QSlider>
#include <QVBoxLayout>

namespace batterybuddy {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("Battery Buddy demo"));
    setWindowIcon(QIcon(QStringLiteral(":/QFunnyWidgets/batterybuddy/logo.png")));
    resize(720, 360);
    QHBoxLayout *row = new QHBoxLayout();
    const struct { int lv; bool chg; } samples[] = {{95, false}, {60, false}, {30, false}, {12, false}, {3, false}, {45, true}};
    for (const auto &smp : samples) {
        BatteryBuddyWidget::Options so;
        so.level = smp.lv;
        so.charging = smp.chg;
        row->addWidget(new BatteryBuddyWidget(so));
    }
    BatteryBuddyWidget::Options o;
    o.level = 75;
    o.lowBattery = [this]() { onLow(); };
    m_battery = new BatteryBuddyWidget(o);
    m_battery->setMinimumHeight(170);
    QSlider *slider = new QSlider(Qt::Horizontal);
    slider->setRange(0, 100);
    slider->setValue(75);
    connect(slider, &QSlider::valueChanged, m_battery, &BatteryBuddyWidget::setLevel);
    QCheckBox *chg = new QCheckBox(tr("Charging"));
    connect(chg, &QCheckBox::toggled, m_battery, &BatteryBuddyWidget::setCharging);
    m_info = new QLabel();
    QHBoxLayout *ctl = new QHBoxLayout();
    ctl->addWidget(new QLabel(tr("Level")));
    ctl->addWidget(slider, 1);
    ctl->addWidget(chg);
    ctl->addWidget(m_info);
    QWidget *wdg = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(wdg);
    layout->addLayout(row, 1);
    layout->addWidget(m_battery, 1);
    layout->addLayout(ctl);
    setCentralWidget(wdg);
}

void MainWindow::onLow()
{
    m_info->setText(tr("Low battery!"));
}

} // namespace batterybuddy
