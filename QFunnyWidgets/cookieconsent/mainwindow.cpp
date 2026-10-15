/**
 * Demo window for CookieConsentWidget.
 */
#include "mainwindow.h"
#include "cookieconsent.h"

#include <QIcon>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace cookieconsent {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("Cookie Consent demo"));
    setWindowIcon(QIcon(QStringLiteral(":/QFunnyWidgets/cookieconsent/logo.png")));
    resize(640, 360);
    CookieConsentWidget::Options o;
    o.accepted = [this]() { onAccepted(); };
    o.declined = [this]() { onDeclined(); };
    m_banner = new CookieConsentWidget(o);
    m_banner->setMinimumHeight(170);
    m_info = new QLabel(tr("Hover the buttons and watch the cookie."));
    m_info->setAlignment(Qt::AlignCenter);
    QPushButton *again = new QPushButton(tr("Ask again"));
    connect(again, &QPushButton::clicked, m_banner, &CookieConsentWidget::reset);
    QWidget *wdg = new QWidget();
    wdg->setStyleSheet(QStringLiteral("background: #ECEEF3;"));
    QVBoxLayout *layout = new QVBoxLayout(wdg);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->addStretch(1);
    layout->addWidget(m_banner);
    layout->addWidget(m_info);
    layout->addWidget(again, 0, Qt::AlignCenter);
    setCentralWidget(wdg);
}

void MainWindow::onAccepted()
{
    m_info->setText(tr("Cookies accepted. The cookie was delicious."));
}

void MainWindow::onDeclined()
{
    m_info->setText(tr("Cookies declined. Somewhere, a cookie weeps."));
}

} // namespace cookieconsent
