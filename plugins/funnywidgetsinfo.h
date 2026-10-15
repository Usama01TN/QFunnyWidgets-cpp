/**
 * Shared metadata for the Qt Designer plugin (the counterpart of plugins/funnywidgetsinfo.py): one entry per
 * widget that can be placed in Designer, plus a factory creating a widget from its class name.
 *
 * PigeonToast and RocketBackToTop are runtime overlays (they attach to a parent or a scroll area at construction),
 * so they are created in code rather than placed in Designer: 33 widgets are listed.
 */
#ifndef QFUNNYWIDGETS_FUNNYWIDGETSINFO_H
#define QFUNNYWIDGETS_FUNNYWIDGETSINFO_H

#include <QList>
#include <QString>

class QWidget;

extern const char *const GROUP_INPUTS;
extern const char *const GROUP_BUTTONS;
extern const char *const GROUP_NAVIGATION;
extern const char *const GROUP_FEEDBACK;
extern const char *const GROUP_HELPERS;
extern const char *const GROUP_MASCOTS;

/**
 * Designer metadata of one widget.
 */
struct WidgetInfo
{
    QString className;
    QString package;
    QString title;
    QString group;
    QString toolTip;
    int width = 200;
    int height = 100;

    /** "JellySwitchWidget" -> "jellySwitch". */
    QString objectName() const;
    /** Header to include, relative to the QFunnyWidgets source directory ("jellyswitch/jellyswitch.h"). */
    QString includeFile() const;
    /** Resource path of the package logo. */
    QString logoPath() const;
    /** The <ui> snippet Designer uses when the widget is dropped on a form. */
    QString domXml() const;
};

/** All widgets that can be placed in Designer, in palette order. */
const QList<WidgetInfo> &funnyWidgetInfos();

/** Create a widget by class name (nullptr for unknown names). */
QWidget *createFunnyWidget(const QString &className, QWidget *parent = nullptr);

#endif // QFUNNYWIDGETS_FUNNYWIDGETSINFO_H
