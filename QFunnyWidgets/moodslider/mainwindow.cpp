/**
 * Demo window for MoodSliderWidget.
 */
#include "mainwindow.h"
#include "moodslider.h"

#include <QIcon>
#include <QLabel>
#include <QTextStream>
#include <QVBoxLayout>

namespace moodslider {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("Mood Slider demo"));
    setWindowIcon(QIcon(QStringLiteral(":/QFunnyWidgets/moodslider/logo.png")));
    resize(600, 320);
    QLabel *title = new QLabel(tr("How do you feel about Mondays?"));
    title->setAlignment(Qt::AlignCenter);
    QFont f = title->font();
    f.setPointSize(14);
    f.setBold(true);
    title->setFont(f);
    MoodSliderWidget::Options o;
    o.value = 50;
    o.valueChanged = [this](int value) { onValue(value); };
    o.moodChanged = [this](const QString &mood) { onMood(mood); };
    m_slider = new MoodSliderWidget(o);
    m_slider->setMinimumHeight(120);
    m_info = new QLabel(tr("Drag, scroll or use the arrow keys."));
    m_info->setAlignment(Qt::AlignCenter);
    QWidget *wdg = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(wdg);
    layout->setContentsMargins(30, 20, 30, 20);
    layout->addStretch(1);
    layout->addWidget(title);
    layout->addWidget(m_slider);
    layout->addWidget(m_info);
    layout->addStretch(1);
    setCentralWidget(wdg);
}

void MainWindow::onValue(int value)
{
    m_info->setText(tr("Value: %1 - %2").arg(value).arg(m_slider->getMood()));
}

void MainWindow::onMood(const QString &mood)
{
    QTextStream(stdout) << QStringLiteral("Mood changed: %1").arg(mood) << '\n';
}

} // namespace moodslider
