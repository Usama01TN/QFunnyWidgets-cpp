/**
 * Qt Designer plugin for QFunnyWidgets (the counterpart of plugins/funnywidgetsplugin.py): one
 * QDesignerCustomWidgetInterface per widget, bundled in a collection.
 */
#ifndef QFUNNYWIDGETS_FUNNYWIDGETSPLUGIN_H
#define QFUNNYWIDGETS_FUNNYWIDGETSPLUGIN_H

#include "funnywidgetsinfo.h"

#include <QObject>
#include <QtUiPlugin/QDesignerCustomWidgetCollectionInterface>
#include <QtUiPlugin/QDesignerCustomWidgetInterface>

/**
 * Designer plugin for one widget, driven by its WidgetInfo.
 */
class FunnyWidgetPlugin : public QObject, public QDesignerCustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
    explicit FunnyWidgetPlugin(const WidgetInfo &info, QObject *parent = nullptr);

    void initialize(QDesignerFormEditorInterface *core) override;
    bool isInitialized() const override;
    QWidget *createWidget(QWidget *parent) override;
    QString name() const override;
    QString group() const override;
    QIcon icon() const override;
    QString toolTip() const override;
    QString whatsThis() const override;
    bool isContainer() const override;
    QString domXml() const override;
    QString includeFile() const override;

private:
    WidgetInfo m_info;
    bool m_initialized = false;
};

/**
 * The plugin library entry point: all widget plugins.
 */
class FunnyWidgetsCollection : public QObject, public QDesignerCustomWidgetCollectionInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QDesignerCustomWidgetCollectionInterface")
    Q_INTERFACES(QDesignerCustomWidgetCollectionInterface)

public:
    explicit FunnyWidgetsCollection(QObject *parent = nullptr);

    QList<QDesignerCustomWidgetInterface *> customWidgets() const override;

private:
    QList<QDesignerCustomWidgetInterface *> m_widgets;
};

#endif // QFUNNYWIDGETS_FUNNYWIDGETSPLUGIN_H
