/**
 * Demo window for DachshundTabsWidget.
 */
#include "mainwindow.h"
#include "dachshundtabs.h"

#include <QIcon>
#include <QLabel>
#include <QStackedWidget>
#include <QVBoxLayout>

namespace dachshundtabs {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("Dachshund Tabs demo"));
    setWindowIcon(QIcon(QStringLiteral(":/QFunnyWidgets/dachshundtabs/logo.png")));
    resize(620, 320);
    const QStringList names = {QStringLiteral("Overview"), QStringLiteral("Activity"), QStringLiteral("Settings"),
                               QStringLiteral("Billing"), QStringLiteral("Help")};
    m_pages = new QStackedWidget();
    for (const QString &n : names) {
        QLabel *lbl = new QLabel(tr("%1 page").arg(n));
        lbl->setAlignment(Qt::AlignCenter);
        m_pages->addWidget(lbl);
    }
    DachshundTabsWidget::Options o;
    o.tabs = names;
    o.currentChanged = [this](int index) { m_pages->setCurrentIndex(index); };
    m_tabs = new DachshundTabsWidget(o);
    QLabel *hint = new QLabel(tr("Jump from the first tab to the last one. Watch the dog stretch."));
    hint->setAlignment(Qt::AlignCenter);
    QWidget *wdg = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(wdg);
    layout->addWidget(m_tabs);
    layout->addWidget(m_pages, 1);
    layout->addWidget(hint);
    setCentralWidget(wdg);
}

} // namespace dachshundtabs
