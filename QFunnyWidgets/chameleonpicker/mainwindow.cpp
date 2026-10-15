/**
 * Demo window for ChameleonColorPickerWidget.
 */
#include "mainwindow.h"
#include "chameleonpicker.h"

#include <QIcon>
#include <QLabel>
#include <QVBoxLayout>

namespace chameleonpicker {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("Chameleon Color Picker demo"));
    setWindowIcon(QIcon(QStringLiteral(":/QFunnyWidgets/chameleonpicker/logo.png")));
    resize(620, 360);
    m_preview = new QLabel(tr("Your theme color"));
    m_preview->setAlignment(Qt::AlignCenter);
    m_preview->setMinimumHeight(50);
    ChameleonColorPickerWidget::Options o;
    o.colorChanged = [this](const QColor &color) { onColor(color); };
    m_picker = new ChameleonColorPickerWidget(o);
    QWidget *wdg = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(wdg);
    layout->addWidget(m_picker, 1);
    layout->addWidget(m_preview);
    setCentralWidget(wdg);
    onColor(m_picker->getColor());
}

void MainWindow::onColor(const QColor &color)
{
    m_preview->setStyleSheet(
        QStringLiteral("background: %1; color: white; border-radius: 10px; font-weight: bold;").arg(color.name()));
    m_preview->setText(tr("Your theme color: %1").arg(color.name().toUpper()));
}

} // namespace chameleonpicker
