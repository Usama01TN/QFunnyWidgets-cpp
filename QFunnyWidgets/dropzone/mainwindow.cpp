/**
 * Demo window for DropZoneMonsterWidget.
 */
#include "mainwindow.h"
#include "dropzone.h"

#include <QFileInfo>
#include <QIcon>
#include <QLabel>
#include <QListWidget>
#include <QVBoxLayout>

namespace dropzone {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("Drop Zone Monster demo"));
    setWindowIcon(QIcon(QStringLiteral(":/QFunnyWidgets/dropzone/logo.png")));
    resize(520, 560);
    DropZoneMonsterWidget::Options o;
    o.acceptedExtensions = QStringList{QStringLiteral("png"), QStringLiteral("jpg"), QStringLiteral("jpeg"),
                                       QStringLiteral("pdf"), QStringLiteral("txt")};
    o.filesDropped = [this](const QStringList &paths) { onDropped(paths); };
    o.filesRejected = [this](const QStringList &paths) { onRejected(paths); };
    m_zone = new DropZoneMonsterWidget(o);
    m_info = new QLabel(tr("Drag images, PDFs or text files from your file manager onto the monster."));
    m_info->setAlignment(Qt::AlignCenter);
    m_info->setWordWrap(true);
    m_list = new QListWidget();
    m_list->setMaximumHeight(120);
    QWidget *wdg = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(wdg);
    layout->addWidget(m_zone, 1);
    layout->addWidget(m_info);
    layout->addWidget(m_list);
    setCentralWidget(wdg);
}

void MainWindow::onDropped(const QStringList &paths)
{
    for (const QString &pth : paths)
        m_list->addItem(tr("Eaten: %1").arg(QFileInfo(pth).fileName()));
}

void MainWindow::onRejected(const QStringList &paths)
{
    QStringList names;
    for (const QString &pth : paths)
        names.append(QFileInfo(pth).fileName());
    m_info->setText(tr("Rejected: %1").arg(names.join(QStringLiteral(", "))));
}

} // namespace dropzone
