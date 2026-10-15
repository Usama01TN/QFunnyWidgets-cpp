/**
 * Demo window for TypingIndicatorWidget.
 */
#include "mainwindow.h"
#include "typingindicator.h"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QIcon>
#include <QMetaEnum>
#include <QPushButton>
#include <QTextStream>
#include <QVBoxLayout>

namespace typingindicator {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("Typing Indicator demo"));
    setWindowIcon(QIcon(QStringLiteral(":/QFunnyWidgets/typingindicator/logo.png")));
    resize(520, 300);
    TypingIndicatorWidget::Options o;
    o.name = QStringLiteral("Alex");
    o.autoCycle = true;
    o.stateChanged = [this](int state) { onState(state); };
    m_typing = new TypingIndicatorWidget(o);
    m_typing->setMinimumHeight(110);
    QHBoxLayout *row = new QHBoxLayout();
    const struct { const char *label; int state; } buttons[] = {
        {QT_TR_NOOP("Typing"), TypingIndicatorWidget::Typing},
        {QT_TR_NOOP("Thinking"), TypingIndicatorWidget::Thinking},
        {QT_TR_NOOP("Deleting"), TypingIndicatorWidget::Deleting},
        {QT_TR_NOOP("Sleeping"), TypingIndicatorWidget::Sleeping}};
    for (const auto &b : buttons) {
        QPushButton *btn = new QPushButton(tr(b.label));
        const int s = b.state;
        connect(btn, &QPushButton::clicked, this, [this, s]() {
            m_auto->setChecked(false);
            m_typing->setState(s);
        });
        row->addWidget(btn);
    }
    m_auto = new QCheckBox(tr("Auto (realistic human)"));
    m_auto->setChecked(true);
    connect(m_auto, &QCheckBox::toggled, m_typing, &TypingIndicatorWidget::setAutoCycle);
    QWidget *wdg = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(wdg);
    layout->addWidget(m_typing, 1);
    layout->addLayout(row);
    layout->addWidget(m_auto);
    setCentralWidget(wdg);
}

void MainWindow::onState(int state)
{
    const QMetaEnum me = QMetaEnum::fromType<TypingIndicatorWidget::States>();
    QTextStream(stdout) << QStringLiteral("State: %1").arg(QString::fromLatin1(me.valueToKey(state))) << '\n';
}

} // namespace typingindicator
