/**
 * Demo window for PufferCounterWidget.
 */
#include "mainwindow.h"
#include "puffercounter.h"

#include <QIcon>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace puffercounter {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("Puffer Counter demo"));
    setWindowIcon(QIcon(QStringLiteral(":/QFunnyWidgets/puffercounter/logo.png")));
    resize(520, 360);
    PufferCounterWidget::Options o;
    o.maxLength = 80;
    o.limitExceeded = [this](bool over) { onLimit(over); };
    m_counter = new PufferCounterWidget(o);
    m_post = new QPushButton(tr("Post"));
    QWidget *wdg = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(wdg);
    layout->addWidget(new QLabel(tr("Compose a very short post (80 characters):")));
    layout->addWidget(m_counter, 1);
    layout->addWidget(m_post);
    setCentralWidget(wdg);
}

void MainWindow::onLimit(bool over)
{
    m_post->setEnabled(!over);
}

} // namespace puffercounter
