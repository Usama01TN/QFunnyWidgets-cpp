/**
 * Demo window for PasswordJudgeWidget.
 */
#include "mainwindow.h"
#include "passwordjudge.h"

#include <QIcon>
#include <QLabel>
#include <QTextStream>
#include <QVBoxLayout>

namespace passwordjudge {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("Password Judge demo"));
    setWindowIcon(QIcon(QStringLiteral(":/QFunnyWidgets/passwordjudge/logo.png")));
    resize(560, 340);
    QLabel *title = new QLabel(tr("Create your account"));
    QFont f = title->font();
    f.setPointSize(14);
    f.setBold(true);
    title->setFont(f);
    PasswordJudgeWidget::Options o;
    o.minLength = 8;
    o.strengthChanged = [this](int strength) { onStrength(strength); };
    m_judge = new PasswordJudgeWidget(o);
    m_info = new QLabel(tr("Try \"password\", then something long with symbols. Then press Show."));
    QWidget *wdg = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(wdg);
    layout->setContentsMargins(24, 20, 24, 20);
    layout->addWidget(title);
    layout->addWidget(m_judge, 1);
    layout->addWidget(m_info);
    setCentralWidget(wdg);
}

void MainWindow::onStrength(int strength)
{
    QTextStream(stdout) << tr("Strength: %1").arg(strength) << '\n';
}

} // namespace passwordjudge
