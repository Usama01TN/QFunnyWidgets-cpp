/**
 * Demo window for CopyBlobButtonWidget.
 */
#include "mainwindow.h"
#include "copyblob.h"

#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QStatusBar>
#include <QVBoxLayout>

namespace copyblob {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("Copy Blob Button demo"));
    setWindowIcon(QIcon(QStringLiteral(":/QFunnyWidgets/copyblob/logo.png")));
    resize(480, 400);
    QWidget *wdg = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(wdg);
    layout->addWidget(new QLabel(tr("Install:")));
    CopyBlobButtonWidget::Options o1;
    o1.text = QStringLiteral("pip install funnywidgets");
    o1.copied = [this](const QString &text) { onCopied(text); };
    layout->addWidget(new CopyBlobButtonWidget(o1));
    layout->addWidget(new QLabel(tr("Your API key (masked):")));
    CopyBlobButtonWidget::Options o2;
    o2.text = QStringLiteral("sk-live-8f3a9c2e7b1d4f60");
    o2.label = QStringLiteral("sk-live-****************");
    o2.prompt = QString();
    o2.blobColor = QColor(QStringLiteral("#FF9F1C"));
    o2.copied = [this](const QString &text) { onCopied(text); };
    layout->addWidget(new CopyBlobButtonWidget(o2));
    layout->addWidget(new QLabel(tr("Paste here to check:")));
    m_paste = new QLineEdit();
    layout->addWidget(m_paste);
    setCentralWidget(wdg);
}

void MainWindow::onCopied(const QString &text)
{
    statusBar()->showMessage(tr("Copied %1 characters").arg(text.toUcs4().size()), 2000);
}

} // namespace copyblob
