/**
 * Qt Designer plugin for QFunnyWidgets (the counterpart of plugins/funnywidgetsplugin.py).
 */
#include "funnywidgetsplugin.h"

#include <QIcon>
#include <QtPlugin>


FunnyWidgetPlugin::FunnyWidgetPlugin(const WidgetInfo &info, QObject *parent)
    : QObject(parent),
      m_info(info)
{
}

void FunnyWidgetPlugin::initialize(QDesignerFormEditorInterface *)
{
    m_initialized = true;
}

bool FunnyWidgetPlugin::isInitialized() const
{
    return m_initialized;
}

QWidget *FunnyWidgetPlugin::createWidget(QWidget *parent)
{
    return createFunnyWidget(m_info.className, parent);
}

QString FunnyWidgetPlugin::name() const
{
    return m_info.className;
}

QString FunnyWidgetPlugin::group() const
{
    return m_info.group;
}

QIcon FunnyWidgetPlugin::icon() const
{
    return QIcon(m_info.logoPath());
}

QString FunnyWidgetPlugin::toolTip() const
{
    return m_info.toolTip;
}

QString FunnyWidgetPlugin::whatsThis() const
{
    return QStringLiteral("%1: %2").arg(m_info.title, m_info.toolTip);
}

bool FunnyWidgetPlugin::isContainer() const
{
    return false;
}

QString FunnyWidgetPlugin::domXml() const
{
    return m_info.domXml();
}

QString FunnyWidgetPlugin::includeFile() const
{
    return m_info.includeFile();
}

FunnyWidgetsCollection::FunnyWidgetsCollection(QObject *parent)
    : QObject(parent)
{
    for (const WidgetInfo &info : funnyWidgetInfos())
        m_widgets.append(new FunnyWidgetPlugin(info, this));
}

QList<QDesignerCustomWidgetInterface *> FunnyWidgetsCollection::customWidgets() const
{
    return m_widgets;
}
