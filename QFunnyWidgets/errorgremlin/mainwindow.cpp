/**
 * Demo window for ErrorGremlinWidget.
 */
#include "mainwindow.h"
#include "errorgremlin.h"

#include <QIcon>
#include <QPushButton>
#include <QVBoxLayout>

namespace errorgremlin {

static const char *const ALERTS[4][2] = {
    {"Heads up", "Scheduled maintenance tonight at 2 am. The gremlin is resting up for it."},
    {"Disk almost full", "Only 3% space left. Something is hoarding cat pictures."},
    {"Connection lost", "We could not reach the server. Someone appears to be chewing the cable."},
    {"Production is on fire", "Literally. Please do not panic. Panic quietly."}};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("Error Gremlin demo"));
    setWindowIcon(QIcon(QStringLiteral(":/QFunnyWidgets/errorgremlin/logo.png")));
    resize(640, 520);
    QWidget *wdg = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(wdg);
    for (int sev = 0; sev < 4; ++sev) {
        ErrorGremlinWidget::Options o;
        o.severity = sev;
        o.title = QString::fromLatin1(ALERTS[sev][0]);
        o.message = QString::fromLatin1(ALERTS[sev][1]);
        ErrorGremlinWidget *b = new ErrorGremlinWidget(o);
        b->setMinimumHeight(100);
        m_banners.append(b);
        layout->addWidget(b);
    }
    QPushButton *again = new QPushButton(tr("Show all again"));
    connect(again, &QPushButton::clicked, this, [this]() {
        for (ErrorGremlinWidget *b : std::as_const(m_banners))
            b->showAlert();
    });
    layout->addStretch(1);
    layout->addWidget(again);
    setCentralWidget(wdg);
}

} // namespace errorgremlin
