/**
 * Demo window for PigeonToast.
 */
#include "mainwindow.h"
#include "pigeontoast.h"

#include <QHBoxLayout>
#include <QIcon>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QRandomGenerator>
#include <QStringList>
#include <QTextStream>
#include <QVBoxLayout>

namespace pigeontoast {

namespace {

QString randomSample()
{
    static const QStringList SAMPLES = {
        QStringLiteral("File saved successfully."), QStringLiteral("Your build passed! (Somehow.)"),
        QStringLiteral("3 new comments on your pull request."), QStringLiteral("Meeting moved to 4 pm. Again."),
        QStringLiteral("Backup completed. Your data is safe-ish.")};
    return SAMPLES.at(int(QRandomGenerator::global()->bounded(int(SAMPLES.size()))));
}

} // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("Pigeon Toast demo"));
    setWindowIcon(QIcon(QStringLiteral(":/QFunnyWidgets/pigeontoast/logo.png")));
    resize(720, 460);
    QWidget *wdg = new QWidget();
    QListWidget *content = new QListWidget();
    for (int i = 0; i < 12; ++i)
        content->addItem(tr("Some very important document #%1").arg(i + 1));
    m_edit = new QLineEdit();
    m_edit->setPlaceholderText(tr("Type a message for the pigeon..."));
    QPushButton *send = new QPushButton(tr("Send pigeon"));
    connect(send, &QPushButton::clicked, this, &MainWindow::onSend);
    connect(m_edit, &QLineEdit::returnPressed, this, &MainWindow::onSend);
    QPushButton *flock = new QPushButton(tr("Send 3"));
    connect(flock, &QPushButton::clicked, this, &MainWindow::onFlock);
    QHBoxLayout *row = new QHBoxLayout();
    row->addWidget(m_edit, 1);
    row->addWidget(send);
    row->addWidget(flock);
    QVBoxLayout *layout = new QVBoxLayout(wdg);
    layout->addWidget(content, 1);
    layout->addLayout(row);
    setCentralWidget(wdg);
    // The overlay covers the central widget and follows its size.
    PigeonToast::Options o;
    o.duration = 2600;
    o.messageFinished = [this](const QString &text) { onDelivered(text); };
    m_toast = new PigeonToast(wdg, o);
    m_toast->showMessage(tr("Welcome! This message was delivered by pigeon."));
}

void MainWindow::onSend()
{
    const QString text = m_edit->text();
    m_toast->showMessage(!text.isEmpty() ? text : randomSample());
    m_edit->clear();
}

void MainWindow::onFlock()
{
    for (int i = 0; i < 3; ++i)
        m_toast->showMessage(randomSample(), 1500);
}

void MainWindow::onDelivered(const QString &text)
{
    QTextStream(stdout) << QStringLiteral("Delivered: %1").arg(text) << '\n';
}

} // namespace pigeontoast
