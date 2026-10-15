/**
 * Demo window for PresenceAvatarWidget.
 */
#include "mainwindow.h"
#include "presenceavatar.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QMetaEnum>
#include <QVBoxLayout>

namespace presenceavatar {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("Presence Avatar demo"));
    setWindowIcon(QIcon(QStringLiteral(":/QFunnyWidgets/presenceavatar/logo.png")));
    resize(620, 340);
    QHBoxLayout *row = new QHBoxLayout();
    const struct { const char *name; int status; } team[] = {
        {"Ada Lovelace", PresenceAvatarWidget::Online}, {"Alan Turing", PresenceAvatarWidget::Away},
        {"Grace Hopper", PresenceAvatarWidget::Busy}, {"Charles Babbage", PresenceAvatarWidget::Offline}};
    for (const auto &member : team) {
        const QString name = QString::fromLatin1(member.name);
        PresenceAvatarWidget::Options o;
        o.name = name;
        o.status = member.status;
        o.clicked = [this, name]() { onClicked(name); };
        row->addWidget(new PresenceAvatarWidget(o));
    }
    PresenceAvatarWidget::Options mo;
    mo.name = QStringLiteral("You Yourself");
    mo.status = PresenceAvatarWidget::Online;
    m_me = new PresenceAvatarWidget(mo);
    m_me->setMinimumSize(140, 160);
    QComboBox *combo = new QComboBox();
    const QMetaEnum me = QMetaEnum::fromType<PresenceAvatarWidget::Status>();
    for (int i = 0; i < me.keyCount(); ++i)
        combo->addItem(QString::fromLatin1(me.key(i)));
    connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged), m_me, &PresenceAvatarWidget::setStatus);
    m_info = new QLabel(tr("Hover an avatar. Click to wave."));
    m_info->setAlignment(Qt::AlignCenter);
    QHBoxLayout *bottom = new QHBoxLayout();
    bottom->addWidget(m_me);
    bottom->addWidget(new QLabel(tr("My status:")));
    bottom->addWidget(combo);
    bottom->addStretch(1);
    QWidget *wdg = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(wdg);
    layout->addLayout(row, 1);
    layout->addWidget(m_info);
    layout->addLayout(bottom);
    setCentralWidget(wdg);
}

void MainWindow::onClicked(const QString &name)
{
    m_info->setText(tr("You waved at %1.").arg(name));
}

} // namespace presenceavatar
