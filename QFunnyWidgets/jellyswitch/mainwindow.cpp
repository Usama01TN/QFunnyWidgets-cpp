/**
 * Demo window for JellySwitchWidget.
 */
#include "mainwindow.h"
#include "jellyswitch.h"

#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QSlider>
#include <QVBoxLayout>

namespace jellyswitch {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("Jelly Switch demo"));
    setWindowIcon(QIcon(QStringLiteral(":/QFunnyWidgets/jellyswitch/logo.png")));
    resize(520, 360);
    QHBoxLayout *row = new QHBoxLayout();
    const struct { const char *on; bool checked; } specs[] = {
        {"#34C759", false}, {"#5E60CE", true}, {"#FF9F1C", false}};
    for (const auto &spec : specs) {
        JellySwitchWidget::Options o;
        o.checked = spec.checked;
        o.onColor = QColor(QString::fromLatin1(spec.on));
        o.toggled = [this](bool checked) { onToggled(checked); };
        JellySwitchWidget *sw = new JellySwitchWidget(o);
        sw->setMinimumSize(130, 80);
        m_switches.append(sw);
        row->addWidget(sw);
    }
    m_info = new QLabel(tr("Click a switch (or focus it and press Space)."));
    m_info->setAlignment(Qt::AlignCenter);
    QSlider *wob = new QSlider(Qt::Horizontal);
    wob->setRange(0, 100);
    wob->setValue(70);
    connect(wob, &QSlider::valueChanged, this, &MainWindow::onWobbliness);
    QHBoxLayout *controls = new QHBoxLayout();
    controls->addWidget(new QLabel(tr("Wobbliness")));
    controls->addWidget(wob);
    JellySwitchWidget::Options bo;
    bo.checked = true;
    bo.onColor = QColor(QStringLiteral("#FF4D6D"));
    JellySwitchWidget *big = new JellySwitchWidget(bo);
    big->setMinimumHeight(140);
    m_switches.append(big);
    QWidget *wdg = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(wdg);
    layout->addLayout(row);
    layout->addWidget(big, 1);
    layout->addWidget(m_info);
    layout->addLayout(controls);
    setCentralWidget(wdg);
}

void MainWindow::onWobbliness(int value)
{
    for (JellySwitchWidget *sw : m_switches)
        sw->setWobbliness(value / 100.0);
}

void MainWindow::onToggled(bool checked)
{
    m_info->setText(checked ? tr("Good morning!") : tr("Back to sleep... zzz"));
}

} // namespace jellyswitch
