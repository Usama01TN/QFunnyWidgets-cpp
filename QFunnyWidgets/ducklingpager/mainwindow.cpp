/**
 * Demo window for DucklingPagerWidget.
 */
#include "mainwindow.h"
#include "ducklingpager.h"

#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>

namespace ducklingpager {

static const char *const SLIDES[] = {QT_TRANSLATE_NOOP("ducklingpager::MainWindow", "Welcome aboard"),
                                     QT_TRANSLATE_NOOP("ducklingpager::MainWindow", "Meet your dashboard"),
                                     QT_TRANSLATE_NOOP("ducklingpager::MainWindow", "Invite your team"),
                                     QT_TRANSLATE_NOOP("ducklingpager::MainWindow", "Set your goals"),
                                     QT_TRANSLATE_NOOP("ducklingpager::MainWindow", "You are ready!")};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("Duckling Pager demo"));
    setWindowIcon(QIcon(QStringLiteral(":/QFunnyWidgets/ducklingpager/logo.png")));
    resize(560, 360);
    m_stack = new QStackedWidget();
    for (const char *title : SLIDES) {
        QLabel *lbl = new QLabel(tr(title));
        lbl->setAlignment(Qt::AlignCenter);
        QFont f = lbl->font();
        f.setPointSize(18);
        f.setBold(true);
        lbl->setFont(f);
        m_stack->addWidget(lbl);
    }
    DucklingPagerWidget::Options o;
    o.count = int(sizeof(SLIDES) / sizeof(SLIDES[0]));
    o.ducklings = 3;
    o.currentChanged = [this](int index) { m_stack->setCurrentIndex(index); };
    m_pager = new DucklingPagerWidget(o);
    QPushButton *prev = new QPushButton(tr("Back"));
    connect(prev, &QPushButton::clicked, m_pager, &DucklingPagerWidget::previous);
    QPushButton *nxt = new QPushButton(tr("Next"));
    connect(nxt, &QPushButton::clicked, m_pager, &DucklingPagerWidget::next);
    QHBoxLayout *row = new QHBoxLayout();
    row->addWidget(prev);
    row->addWidget(m_pager, 1);
    row->addWidget(nxt);
    QWidget *wdg = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(wdg);
    layout->addWidget(m_stack, 1);
    layout->addLayout(row);
    setCentralWidget(wdg);
}

} // namespace ducklingpager
