/**
 * Demo window for RocketBackToTop.
 */
#include "mainwindow.h"
#include "rockettop.h"

#include <QIcon>
#include <QLabel>
#include <QScrollArea>
#include <QStringList>

namespace rockettop {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("Rocket Back To Top demo"));
    setWindowIcon(QIcon(QStringLiteral(":/QFunnyWidgets/rockettop/logo.png")));
    resize(520, 520);
    QStringList chapters;
    for (int i = 0; i < 40; ++i)
        chapters.append(tr("Chapter %1: In which the reader scrolls a little further, "
                           "wondering whether the rocket will appear. It will.").arg(i + 1));
    QLabel *body = new QLabel(chapters.join(QStringLiteral("\n\n")));
    body->setWordWrap(true);
    body->setAlignment(Qt::AlignTop);
    body->setContentsMargins(16, 16, 90, 16);
    QScrollArea *area = new QScrollArea();
    area->setWidgetResizable(true);
    area->setWidget(body);
    setCentralWidget(area);
    RocketBackToTop::Options o;
    o.threshold = 250;
    m_rocket = new RocketBackToTop(area, o);
}

} // namespace rockettop
