/**
 * Demo window for PanicBellWidget.
 */
#include "mainwindow.h"
#include "panicbell.h"

#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace panicbell {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("Panic Bell demo"));
    setWindowIcon(QIcon(QStringLiteral(":/QFunnyWidgets/panicbell/logo.png")));
    resize(560, 420);
    PanicBellWidget::Options o;
    o.count = 3;
    o.countChanged = [this](int count) { onCount(count); };
    m_bell = new PanicBellWidget(o);
    m_info = new QLabel(tr("Add notifications and watch the stress build. Click the bell to clear."));
    m_info->setAlignment(Qt::AlignCenter);
    QHBoxLayout *row = new QHBoxLayout();
    const struct { const char *label; int amount; } adds[] = {{"+1", 1}, {"+10", 10}, {"+100", 100}};
    for (const auto &add : adds) {
        QPushButton *b = new QPushButton(QString::fromLatin1(add.label));
        const int a = add.amount;
        connect(b, &QPushButton::clicked, this, [this, a]() { m_bell->setCount(m_bell->getCount() + a); });
        row->addWidget(b);
    }
    QPushButton *clear = new QPushButton(tr("Clear"));
    connect(clear, &QPushButton::clicked, m_bell, &PanicBellWidget::clear);
    row->addWidget(clear);
    QWidget *wdg = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(wdg);
    layout->addWidget(m_bell, 1);
    layout->addWidget(m_info);
    layout->addLayout(row);
    setCentralWidget(wdg);
}

void MainWindow::onCount(int count)
{
    m_info->setText(tr("%1 unread notification(s)").arg(count));
}

} // namespace panicbell
