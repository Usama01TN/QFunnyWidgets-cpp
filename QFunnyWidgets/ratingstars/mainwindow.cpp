/**
 * Demo window for StarRatingWidget.
 */
#include "mainwindow.h"
#include "ratingstars.h"

#include <QIcon>
#include <QLabel>
#include <QVBoxLayout>

namespace ratingstars {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("Star Rating demo"));
    setWindowIcon(QIcon(QStringLiteral(":/QFunnyWidgets/ratingstars/logo.png")));
    resize(560, 360);
    QLabel *title = new QLabel(tr("How was your experience?"));
    title->setAlignment(Qt::AlignCenter);
    QFont f = title->font();
    f.setPointSize(14);
    f.setBold(true);
    title->setFont(f);
    StarRatingWidget::Options o;
    o.rating = 0;
    o.ratingChanged = [this](int rating) { onRating(rating); };
    m_stars = new StarRatingWidget(o);
    m_stars->setMinimumHeight(130);
    StarRatingWidget::Options so;
    so.rating = 3;
    so.maxRating = 5;
    so.starColor = QColor(QStringLiteral("#FF8FA3"));
    so.showLabel = false;
    m_small = new StarRatingWidget(so);
    m_small->setMaximumHeight(50);
    m_info = new QLabel(tr("Hover to preview, click to rate (click again to clear). Keys 0-5 work too."));
    m_info->setAlignment(Qt::AlignCenter);
    QWidget *wdg = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(wdg);
    layout->addWidget(title);
    layout->addWidget(m_stars, 1);
    layout->addWidget(m_info);
    layout->addWidget(m_small);
    setCentralWidget(wdg);
}

void MainWindow::onRating(int rating)
{
    m_info->setText(rating ? tr("You rated %1 / 5").arg(rating) : tr("Rating cleared."));
}

} // namespace ratingstars
