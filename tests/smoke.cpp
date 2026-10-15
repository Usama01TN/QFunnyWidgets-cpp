/**
 * Headless smoke test (the counterpart of tests/smoke.py): opens, resizes and closes every widget demo, then checks
 * the Qt Designer plugin metadata (factory, icon, domXml, include file) for every Designer widget.
 *
 * Any Qt critical/fatal message, or a warning that is not known offscreen-platform noise, counts as a failure.
 */
#include "funnywidgetsinfo.h"

#include <QApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QIcon>
#include <QPair>
#include <QStringList>
#include <QTextStream>
#include <QWidget>
#include <QXmlStreamReader>
#include <functional>

#include "antbreadcrumb/mainwindow.h"
#include "autosave/mainwindow.h"
#include "batterybuddy/mainwindow.h"
#include "catsection/mainwindow.h"
#include "chameleonpicker/mainwindow.h"
#include "cookieconsent/mainwindow.h"
#include "copyblob/mainwindow.h"
#include "dachshundtabs/mainwindow.h"
#include "deadline/mainwindow.h"
#include "dropzone/mainwindow.h"
#include "ducklingpager/mainwindow.h"
#include "emptystate/mainwindow.h"
#include "errorgremlin/mainwindow.h"
#include "googlyeyes/mainwindow.h"
#include "hamsterloader/mainwindow.h"
#include "heartlike/mainwindow.h"
#include "helpowl/mainwindow.h"
#include "holdbutton/mainwindow.h"
#include "jellyswitch/mainwindow.h"
#include "moodslider/mainwindow.h"
#include "pancakestepper/mainwindow.h"
#include "panicbell/mainwindow.h"
#include "passwordjudge/mainwindow.h"
#include "pigeontoast/mainwindow.h"
#include "presenceavatar/mainwindow.h"
#include "puffercounter/mainwindow.h"
#include "ratingstars/mainwindow.h"
#include "robotcheck/mainwindow.h"
#include "rockettop/mainwindow.h"
#include "rubberduck/mainwindow.h"
#include "runawaybutton/mainwindow.h"
#include "searchdetective/mainwindow.h"
#include "snailstepper/mainwindow.h"
#include "typingindicator/mainwindow.h"
#include "volumeknob/mainwindow.h"

namespace {

QStringList ERRORS;

void messageHandler(QtMsgType type, const QMessageLogContext &, const QString &msg)
{
    // Platform noise of the offscreen plugin, not problems of the widgets.
    static const char *const NOISE[] = {"propagateSizeHints", "XDG_RUNTIME_DIR", "QStandardPaths"};
    if (type == QtDebugMsg || type == QtInfoMsg)
        return;
    for (const char *n : NOISE) {
        if (msg.contains(QLatin1String(n)))
            return;
    }
    ERRORS.append(msg);
}

void spin(QApplication &app, int ms)
{
    QElapsedTimer clock;
    clock.start();
    while (clock.elapsed() < ms)
        app.processEvents();
}

QTextStream &out()
{
    static QTextStream stream(stdout);
    return stream;
}

QList<QPair<QString, std::function<QWidget *()>>> demos()
{
    return {
        {"antbreadcrumb", []() -> QWidget * { return new antbreadcrumb::MainWindow(); }},
        {"autosave", []() -> QWidget * { return new autosave::MainWindow(); }},
        {"batterybuddy", []() -> QWidget * { return new batterybuddy::MainWindow(); }},
        {"catsection", []() -> QWidget * { return new catsection::MainWindow(); }},
        {"chameleonpicker", []() -> QWidget * { return new chameleonpicker::MainWindow(); }},
        {"cookieconsent", []() -> QWidget * { return new cookieconsent::MainWindow(); }},
        {"copyblob", []() -> QWidget * { return new copyblob::MainWindow(); }},
        {"dachshundtabs", []() -> QWidget * { return new dachshundtabs::MainWindow(); }},
        {"deadline", []() -> QWidget * { return new deadline::MainWindow(); }},
        {"dropzone", []() -> QWidget * { return new dropzone::MainWindow(); }},
        {"ducklingpager", []() -> QWidget * { return new ducklingpager::MainWindow(); }},
        {"emptystate", []() -> QWidget * { return new emptystate::MainWindow(); }},
        {"errorgremlin", []() -> QWidget * { return new errorgremlin::MainWindow(); }},
        {"googlyeyes", []() -> QWidget * { return new googlyeyes::MainWindow(); }},
        {"hamsterloader", []() -> QWidget * { return new hamsterloader::MainWindow(); }},
        {"heartlike", []() -> QWidget * { return new heartlike::MainWindow(); }},
        {"helpowl", []() -> QWidget * { return new helpowl::MainWindow(); }},
        {"holdbutton", []() -> QWidget * { return new holdbutton::MainWindow(); }},
        {"jellyswitch", []() -> QWidget * { return new jellyswitch::MainWindow(); }},
        {"moodslider", []() -> QWidget * { return new moodslider::MainWindow(); }},
        {"pancakestepper", []() -> QWidget * { return new pancakestepper::MainWindow(); }},
        {"panicbell", []() -> QWidget * { return new panicbell::MainWindow(); }},
        {"passwordjudge", []() -> QWidget * { return new passwordjudge::MainWindow(); }},
        {"pigeontoast", []() -> QWidget * { return new pigeontoast::MainWindow(); }},
        {"presenceavatar", []() -> QWidget * { return new presenceavatar::MainWindow(); }},
        {"puffercounter", []() -> QWidget * { return new puffercounter::MainWindow(); }},
        {"ratingstars", []() -> QWidget * { return new ratingstars::MainWindow(); }},
        {"robotcheck", []() -> QWidget * { return new robotcheck::MainWindow(); }},
        {"rockettop", []() -> QWidget * { return new rockettop::MainWindow(); }},
        {"rubberduck", []() -> QWidget * { return new rubberduck::MainWindow(); }},
        {"runawaybutton", []() -> QWidget * { return new runawaybutton::MainWindow(); }},
        {"searchdetective", []() -> QWidget * { return new searchdetective::MainWindow(); }},
        {"snailstepper", []() -> QWidget * { return new snailstepper::MainWindow(); }},
        {"typingindicator", []() -> QWidget * { return new typingindicator::MainWindow(); }},
        {"volumeknob", []() -> QWidget * { return new volumeknob::MainWindow(); }},
    };
}

QStringList testDemos(QApplication &app)
{
    QStringList failed;
    for (const auto &demo : demos()) {
        const int before = int(ERRORS.size());
        QWidget *window = demo.second();
        window->show();
        spin(app, 300);
        window->resize(window->width() - 60, window->height() - 40);
        spin(app, 200);
        window->close();
        window->deleteLater();
        spin(app, 20);
        const bool ok = ERRORS.size() == before;
        out() << "  " << demo.first.leftJustified(18) << ' ' << (ok ? "ok" : "FAILED") << Qt::endl;
        if (!ok)
            failed.append(demo.first);
    }
    return failed;
}

QStringList testDesignerPlugins()
{
    QStringList problems;
    QWidget form;
    const QList<WidgetInfo> &infos = funnyWidgetInfos();
    for (const WidgetInfo &info : infos) {
        QWidget *w = createFunnyWidget(info.className, &form);
        if (w == nullptr) {
            problems.append(QStringLiteral("%1: createWidget returned nothing").arg(info.className));
            continue;
        }
        if (QString::fromLatin1(w->metaObject()->className()) != info.className)
            problems.append(QStringLiteral("%1: createWidget returned %2").arg(info.className,
                                                                             QString::fromLatin1(w->metaObject()->className())));
        if (QIcon(info.logoPath()).isNull())
            problems.append(QStringLiteral("%1: missing icon").arg(info.className));
        QXmlStreamReader xml(info.domXml());
        while (!xml.atEnd())
            xml.readNext();
        if (xml.hasError())
            problems.append(QStringLiteral("%1: invalid domXml: %2").arg(info.className, xml.errorString()));
        const QString include = QDir(QStringLiteral(QFW_SOURCE_DIR)).filePath(info.includeFile());
        QFile header(include);
        if (!header.open(QIODevice::ReadOnly)
            || !QString::fromUtf8(header.readAll()).contains(QStringLiteral("class %1 ").arg(info.className)))
            problems.append(QStringLiteral("%1: includeFile %2 does not declare it").arg(info.className, info.includeFile()));
    }
    if (infos.size() != 33)
        problems.append(QStringLiteral("expected 33 Designer widgets, found %1").arg(infos.size()));
    out() << "  " << infos.size() << " Designer widgets checked" << Qt::endl;
    return problems;
}

} // namespace

int main(int argc, char *argv[])
{
    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM"))
        qputenv("QT_QPA_PLATFORM", "offscreen");
    qInstallMessageHandler(messageHandler);
    QApplication app(argc, argv);
    out() << "Qt version: " << qVersion() << Qt::endl;
    out() << "Widget demos:" << Qt::endl;
    const QStringList failed = testDemos(app);
    out() << "Designer plugins:" << Qt::endl;
    const QStringList problems = testDesignerPlugins();
    for (const QString &e : ERRORS + problems) {
        out() << QString(70, QLatin1Char('-')) << Qt::endl;
        out() << e.trimmed() << Qt::endl;
    }
    const int total = int(demos().size());
    out() << "Result: " << (total - failed.size()) << '/' << total << " demos ok, " << problems.size()
          << " plugin problem(s)" << Qt::endl;
    return (!failed.isEmpty() || !problems.isEmpty() || !ERRORS.isEmpty()) ? 1 : 0;
}
