/**
 * Demo window for SearchDetectiveWidget.
 */
#include "mainwindow.h"
#include "searchdetective.h"

#include <QIcon>
#include <QListWidget>
#include <QStringList>
#include <QVBoxLayout>

namespace searchdetective {

static const QStringList &suspects()
{
    static const QStringList SUSPECTS = {
        QStringLiteral("Professor Moriarty"), QStringLiteral("Irene Adler"), QStringLiteral("Colonel Moran"),
        QStringLiteral("Mrs Hudson"), QStringLiteral("Inspector Lestrade"), QStringLiteral("Dr Watson"),
        QStringLiteral("Mycroft Holmes"), QStringLiteral("The Hound of the Baskervilles"),
        QStringLiteral("Charles Augustus Milverton")};
    return SUSPECTS;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("Search Detective demo"));
    setWindowIcon(QIcon(QStringLiteral(":/QFunnyWidgets/searchdetective/logo.png")));
    resize(560, 440);
    m_results = new QListWidget();
    SearchDetectiveWidget::Options o;
    o.placeholder = tr("Search suspects (try \"mo\" or \"xyz\")");
    o.searchFunction = [this](const QString &text) { return find(text); };
    m_search = new SearchDetectiveWidget(o);
    m_search->setMinimumHeight(170);
    QWidget *wdg = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(wdg);
    layout->addWidget(m_search);
    layout->addWidget(m_results, 1);
    setCentralWidget(wdg);
}

int MainWindow::find(const QString &text)
{
    QStringList hits;
    for (const QString &s : suspects()) {
        if (s.toLower().contains(text.toLower()))
            hits.append(s);
    }
    m_results->clear();
    m_results->addItems(hits);
    return int(hits.size());
}

} // namespace searchdetective
