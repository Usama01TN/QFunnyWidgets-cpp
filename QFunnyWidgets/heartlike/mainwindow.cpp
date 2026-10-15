/**
 * Demo window for HeartLikeButtonWidget.
 */
#include "mainwindow.h"
#include "heartlike.h"

#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QVBoxLayout>

namespace heartlike {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("Heart Like Button demo"));
    setWindowIcon(QIcon(QStringLiteral(":/QFunnyWidgets/heartlike/logo.png")));
    resize(520, 300);
    QLabel *post = new QLabel(tr("Just shipped a new feature on a Friday afternoon. What could go wrong?"));
    post->setWordWrap(true);
    QFont f = post->font();
    f.setPointSize(12);
    post->setFont(f);
    QHBoxLayout *row = new QHBoxLayout();
    HeartLikeButtonWidget::Options o;
    o.count = 41;
    o.likedChanged = [this](bool liked) { onLiked(liked); };
    m_heart = new HeartLikeButtonWidget(o);
    row->addWidget(m_heart);
    HeartLikeButtonWidget::Options o2;
    o2.count = 1203;
    o2.liked = true;
    o2.color = QColor(QStringLiteral("#7B5CFF"));
    row->addWidget(new HeartLikeButtonWidget(o2));
    m_info = new QLabel(tr("Click to like. Click again to break its heart. Spam it to make it dizzy."));
    m_info->setAlignment(Qt::AlignCenter);
    QWidget *wdg = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(wdg);
    layout->addWidget(post);
    layout->addLayout(row, 1);
    layout->addWidget(m_info);
    setCentralWidget(wdg);
}

void MainWindow::onLiked(bool liked)
{
    m_info->setText(liked ? tr("Liked!") : tr("Unliked. It will heal."));
}

} // namespace heartlike
