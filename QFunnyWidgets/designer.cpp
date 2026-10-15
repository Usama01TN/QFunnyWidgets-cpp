/**
 * Launches Qt Designer with the QFunnyWidgets plugin available (the counterpart of designer.py).
 *
 *   funnywidgets-designer [--plugins-path] [--print-env] [designer arguments...]
 *
 * Designer loads custom widget plugins from "<dir>/designer" for every <dir> in QT_PLUGIN_PATH, so the build
 * directory containing designer/<funnywidgetsplugin library> is prepended to QT_PLUGIN_PATH.
 */
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QLibraryInfo>
#include <QProcess>
#include <QProcessEnvironment>
#include <QStandardPaths>
#include <QStringList>
#include <QTextStream>

namespace {

/** Directory whose "designer" subdirectory holds the plugin, or an empty string. */
QString pluginsPath()
{
    QStringList candidates;
    const QString appDir = QCoreApplication::applicationDirPath();
    candidates << appDir << QDir(appDir).filePath(QStringLiteral("../lib/qt/plugins"))
               << QDir(appDir).filePath(QStringLiteral("../plugins"));
#ifdef QFW_PLUGIN_INSTALL_RELDIR
    // Installed layout: <prefix>/bin/funnywidgets-designer and <prefix>/<libdir>/qt<N>/plugins/designer/.
    candidates << QDir(appDir).filePath(QStringLiteral("../" QFW_PLUGIN_INSTALL_RELDIR));
#endif
#ifdef QFW_PLUGIN_BUILD_DIR
    candidates << QFileInfo(QStringLiteral(QFW_PLUGIN_BUILD_DIR)).absolutePath();
#endif
    for (const QString &candidate : std::as_const(candidates)) {
        const QDir dir(QDir(candidate).filePath(QStringLiteral("designer")));
        if (!dir.exists())
            continue;
        if (!dir.entryList({QStringLiteral("*funnywidgetsplugin*")}, QDir::Files).isEmpty())
            return QDir(candidate).absolutePath();
    }
    return QString();
}

QProcessEnvironment designerEnvironment(const QString &plugins)
{
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QStringList parts = env.value(QStringLiteral("QT_PLUGIN_PATH")).split(QDir::listSeparator(), Qt::SkipEmptyParts);
    if (!parts.contains(plugins))
        parts.prepend(plugins);
    env.insert(QStringLiteral("QT_PLUGIN_PATH"), parts.join(QDir::listSeparator()));
    return env;
}

/** Designer executable matching the Qt major version this tool was built with. */
QString findDesigner()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    const QString binDir = QLibraryInfo::path(QLibraryInfo::BinariesPath);
    const QString libexecDir = QLibraryInfo::path(QLibraryInfo::LibraryExecutablesPath);
    const QStringList names = {QStringLiteral("designer6"), QStringLiteral("designer-qt6"), QStringLiteral("designer")};
#else
    const QString binDir = QLibraryInfo::location(QLibraryInfo::BinariesPath);
    const QString libexecDir = QLibraryInfo::location(QLibraryInfo::LibraryExecutablesPath);
    const QStringList names = {QStringLiteral("designer-qt5"), QStringLiteral("designer")};
#endif
    // Prefer the Designer that belongs to this Qt (plugins must match its Qt version), then the PATH.
    for (const QString &name : names) {
        for (const QString &dir : {binDir, libexecDir}) {
            const QString found = QStandardPaths::findExecutable(name, {dir});
            if (!found.isEmpty())
                return found;
        }
    }
    for (const QString &name : names) {
        const QString found = QStandardPaths::findExecutable(name);
        if (!found.isEmpty())
            return found;
    }
    return QString();
}

} // namespace

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QTextStream out(stdout);
    QTextStream err(stderr);
    QStringList args = app.arguments().mid(1);
    const QString plugins = pluginsPath();
    if (args.contains(QStringLiteral("--plugins-path"))) {
        out << plugins << Qt::endl;
        return plugins.isEmpty() ? 1 : 0;
    }
    if (plugins.isEmpty()) {
        err << "QFunnyWidgets Designer plugin not found (build the funnywidgetsplugin target)." << Qt::endl;
        return 1;
    }
    const QProcessEnvironment env = designerEnvironment(plugins);
    if (args.contains(QStringLiteral("--print-env"))) {
        out << "QT_PLUGIN_PATH=" << env.value(QStringLiteral("QT_PLUGIN_PATH")) << Qt::endl;
        return 0;
    }
    const QString designer = findDesigner();
    if (designer.isEmpty()) {
        err << "No Qt Designer found. Install Qt Designer for Qt " << QT_VERSION_MAJOR
            << " (e.g. qttools5-dev-tools / qt6-tools-dev-tools, or the Qt online installer), then run again.\n"
            << "Plugins: " << plugins << Qt::endl;
        return 1;
    }
    QProcess process;
    process.setProcessEnvironment(env);
    process.setProcessChannelMode(QProcess::ForwardedChannels);
    process.start(designer, args);
    if (!process.waitForStarted()) {
        err << "Could not start " << designer << Qt::endl;
        return 1;
    }
    process.waitForFinished(-1);
    return process.exitCode();
}
