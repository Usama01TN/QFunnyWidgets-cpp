/**
 * Demo window for AntBreadcrumbWidget.
 */
#include "mainwindow.h"
#include "antbreadcrumb.h"

#include <QHash>
#include <QIcon>
#include <QLabel>
#include <QListWidget>
#include <QVBoxLayout>

namespace antbreadcrumb {

static QStringList treeChildren(const QString &name)
{
    static const QHash<QString, QStringList> TREE = {
        {QStringLiteral("Home"), {QStringLiteral("Projects"), QStringLiteral("Photos"), QStringLiteral("Recipes")}},
        {QStringLiteral("Projects"), {QStringLiteral("QFunnyWidgets"), QStringLiteral("Secret Plan")}},
        {QStringLiteral("QFunnyWidgets"), {QStringLiteral("src"), QStringLiteral("docs")}},
        {QStringLiteral("Photos"), {QStringLiteral("Cats"), QStringLiteral("More cats")}},
        {QStringLiteral("Recipes"), {QStringLiteral("Pancakes")}}};
    return TREE.value(name);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("Ant Breadcrumb demo"));
    setWindowIcon(QIcon(QStringLiteral(":/QFunnyWidgets/antbreadcrumb/logo.png")));
    resize(560, 360);
    m_list = new QListWidget();
    AntBreadcrumbWidget::Options o;
    o.path = QStringList{QStringLiteral("Home")};
    o.pathChanged = [this](const QStringList &path) { onPath(path); };
    m_crumbs = new AntBreadcrumbWidget(o);
    connect(m_list, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *it) { m_crumbs->push(it->text()); });
    QWidget *wdg = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(wdg);
    layout->addWidget(m_crumbs);
    layout->addWidget(new QLabel(tr("Double-click a folder to go deeper. Click a crumb to go back.")));
    layout->addWidget(m_list, 1);
    setCentralWidget(wdg);
    onPath(m_crumbs->getPath());
}

void MainWindow::onPath(const QStringList &path)
{
    m_list->clear();
    m_list->addItems(treeChildren(path.last()));
}

} // namespace antbreadcrumb
