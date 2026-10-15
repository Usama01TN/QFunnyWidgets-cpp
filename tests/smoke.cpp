/**
 * Headless smoke test (the counterpart of tests/smoke.py): opens, resizes and closes every widget demo, then checks
 * the Qt Designer plugin metadata (factory, icon, domXml, include file) for every Designer widget.
 *
 * Every Qt warning, critical or fatal message counts as a failure of the demo that produced it, except messages
 * about the test machine itself (fonts, platform plugin, runtime directories): those are environment notes, listed
 * separately so they stay visible. See isEnvironmentMessage(); selfCheck() guards that classification.
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
#include <utility>

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

QStringList ERRORS;       // problems: each one fails the demo that was running
QStringList NOTES;        // environment messages, reported but not failures

/**
 * True for messages about the machine rather than about the widgets: Qt's platform-plugin and font-database
 * logging categories, plus a few well-known uncategorized messages from the same subsystems (Qt 5 logs most of them
 * without a category). Anything else (QPainter misuse, failed connections, layout or font-size warnings, ...) is a
 * real problem.
 */
bool isEnvironmentMessage(const char *category, const QString &msg)
{
    const QLatin1String cat(category ? category : "");
    if (cat.startsWith(QLatin1String("qt.qpa.")) || cat.startsWith(QLatin1String("qt.text.font")))
        return true;
    static const char *const KNOWN[] = {
        "QFontDatabase: Cannot find font directory",  // offscreen platform without bundled fonts (Windows)
        "Note that Qt no longer ships fonts",         // second line of the message above
        "Populating font family aliases",             // a requested family (e.g. "Sans Serif") missing (macOS)
        "propagateSizeHints",                         // offscreen platform: unsupported window hint
        "XDG_RUNTIME_DIR",                            // headless Linux session
        "QStandardPaths:",                            // same, older wording
    };
    for (const char *k : KNOWN) {
        if (msg.contains(QLatin1String(k)))
            return true;
    }
    return false;
}

void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    if (type == QtDebugMsg || type == QtInfoMsg)
        return;
    if (isEnvironmentMessage(context.category, msg)) {
        if (!NOTES.contains(msg))
            NOTES.append(msg);
        return;
    }
    ERRORS.append(msg);
}

/** The classifier must keep accepting the environment messages seen on CI and keep rejecting real problems. */
QStringList selfCheck()
{
    QStringList problems;
    const char *const environment[][2] = {
        {"default", "QFontDatabase: Cannot find font directory D:/a/x/Qt/6.10.3/msvc2022_64/lib/fonts.\n"
                    "Note that Qt no longer ships fonts. Deploy some (from https://dejavu-fonts.github.io/ for "
                    "example) or switch to fontconfig."},
        {"default", "Populating font family aliases took 132 ms. Replace uses of missing font family "
                    "\"Sans Serif\" with one that exists to avoid this cost."},
        {"qt.text.font.db", "Populating font family aliases took 94 ms. Replace uses of missing font family "
                            "\"Monospace\" with one that exists to avoid this cost."},
        {"qt.qpa.fonts", "Unable to enumerate family ' \"Foo\" '"},
        {"default", "QStandardPaths: XDG_RUNTIME_DIR not set, defaulting to '/tmp/runtime-root'"},
    };
    const char *const problemsToCatch[][2] = {
        {"default", "QPainter::begin: Paint device returned engine == 0, type: 1"},
        {"default", "QObject::connect: No such signal JellySwitchWidget::toggled(int)"},
        {"default", "QFont::setPointSizeF: Point size <= 0 (-1.000000), must be greater than 0"},
        {"default", "QLayout: Attempting to add QLayout \"\" to QWidget \"\", which already has a layout"},
        {"qt.widgets.painting", "Something went wrong while painting"},
    };
    for (const auto &m : environment) {
        if (!isEnvironmentMessage(m[0], QString::fromLatin1(m[1])))
            problems.append(QStringLiteral("self-check: environment message treated as a failure: %1")
                                .arg(QString::fromLatin1(m[1])));
    }
    for (const auto &m : problemsToCatch) {
        if (isEnvironmentMessage(m[0], QString::fromLatin1(m[1])))
            problems.append(QStringLiteral("self-check: real problem ignored: %1").arg(QString::fromLatin1(m[1])));
    }
    return problems;
}

/** The offscreen platform has no fonts on Windows unless pointed at some (Qt stopped shipping them). */
void provideFontsForOffscreen()
{
#ifdef Q_OS_WIN
    if (qEnvironmentVariableIsEmpty("QT_QPA_FONTDIR")) {
        const QString fonts = QDir(qEnvironmentVariable("WINDIR", QStringLiteral("C:/Windows")))
                                  .filePath(QStringLiteral("Fonts"));
        if (QDir(fonts).exists())
            qputenv("QT_QPA_FONTDIR", QDir::toNativeSeparators(fonts).toLocal8Bit());
    }
#endif
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
        if (!ok) {
            failed.append(demo.first);
            for (int i = before; i < ERRORS.size(); ++i)
                out() << "      -> " << ERRORS.at(i).trimmed().replace(QLatin1Char('\n'), QLatin1String("\n         ")) << Qt::endl;
        }
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
    if (qgetenv("QT_QPA_PLATFORM").startsWith("offscreen"))
        provideFontsForOffscreen();
    qInstallMessageHandler(messageHandler);
    QApplication app(argc, argv);
    const QStringList selfProblems = selfCheck();
    out() << "Qt version: " << qVersion() << Qt::endl;
    out() << "Widget demos:" << Qt::endl;
    const QStringList failed = testDemos(app);
    out() << "Designer plugins:" << Qt::endl;
    const QStringList problems = testDesignerPlugins();
    if (!NOTES.isEmpty()) {
        out() << "Environment notes (not failures):" << Qt::endl;
        for (const QString &n : std::as_const(NOTES))
            out() << "  - " << n.trimmed().replace(QLatin1Char('\n'), QLatin1String("\n    ")) << Qt::endl;
    }
    for (const QString &e : selfProblems + problems) {
        out() << QString(70, QLatin1Char('-')) << Qt::endl;
        out() << e.trimmed() << Qt::endl;
    }
    const int total = int(demos().size());
    out() << "Result: " << (total - failed.size()) << '/' << total << " demos ok, " << problems.size()
          << " plugin problem(s), " << NOTES.size() << " environment note(s)" << Qt::endl;
    return (!failed.isEmpty() || !problems.isEmpty() || !ERRORS.isEmpty() || !selfProblems.isEmpty()) ? 1 : 0;
}
