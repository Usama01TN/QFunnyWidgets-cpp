/**
 * Demo window for HelpOwlWidget.
 */
#include "mainwindow.h"
#include "helpowl.h"

#include <QGridLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>

namespace helpowl {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("Help Owl demo"));
    setWindowIcon(QIcon(QStringLiteral(":/QFunnyWidgets/helpowl/logo.png")));
    resize(480, 260);
    QWidget *wdg = new QWidget();
    QGridLayout *grid = new QGridLayout(wdg);
    const struct { QString label; QWidget *field; QString help; } rows[] = {
        {tr("Workspace name"), new QLineEdit(QStringLiteral("acme-corp")),
         tr("The name everyone in your team will see. You can change it later (we think).")},
        {tr("Retention (days)"), new QSpinBox(), tr("How long we keep deleted items. After that the owl eats them.")},
        {tr("Webhook URL"), new QLineEdit(), tr("We POST a JSON payload here whenever something happens. Hoo!")}};
    int r = 0;
    for (const auto &row : rows) {
        grid->addWidget(new QLabel(row.label), r, 0);
        grid->addWidget(row.field, r, 1);
        HelpOwlWidget::Options o;
        o.helpText = row.help;
        o.title = row.label;
        grid->addWidget(new HelpOwlWidget(o), r, 2);
        ++r;
    }
    grid->setRowStretch(3, 1);
    setCentralWidget(wdg);
}

} // namespace helpowl
