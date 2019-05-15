/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "RiuDockWidgetTools.h"

#include "RimEclipseView.h"
#include "RimGeoMechView.h"

#include "RiuMainWindow.h"

#include "cvfAssert.h"

#include <QDockWidget>
#include <QSettings>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::projectTreeName()
{
    return "dockProjectTree";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::propertyEditorName()
{
    return "dockpropertyEditor";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::resultInfoName()
{
    return "dockResultInfo";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::processMonitorName()
{
    return "dockProcessMonitor";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::resultPlotName()
{
    return "dockResultPlot";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::relPermPlotName()
{
    return "dockRelPermPlot";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::pvtPlotName()
{
    return "dockPvtPlot";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::mohrsCirclePlotName()
{
    return "dockMohrsCirclePlot";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::plotMainWindowProjectTreeName() 
{
    return "plotMainWindow_dockProjectTree";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::plotMainWindowPropertyEditorName() 
{
    return "plotMainWindow_dockPropertyEditor";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::messagesName()
{
    return "dockMessages";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QDockWidget* RiuDockWidgetTools::findDockWidget(const QObject* parent, const QString& dockWidgetName)
{
    QList<QDockWidget*> dockWidgets = parent->findChildren<QDockWidget*>();

    for (QDockWidget* dock : dockWidgets)
    {
        if (dock->objectName() == dockWidgetName)
        {
            return dock;
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QAction* RiuDockWidgetTools::toggleActionForWidget(const QObject* parent, const QString& dockWidgetName)
{
    auto w = RiuDockWidgetTools::findDockWidget(parent, dockWidgetName);
    if (w)
    {
        return w->toggleViewAction();
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuDockWidgetTools::setVisibleDockingWindowsForEclipse()
{
    RiuMainWindow* mainWindow = RiuMainWindow::instance();

    {
        QDockWidget* dockWidget = findDockWidget(mainWindow, RiuDockWidgetTools::mohrsCirclePlotName());
        if (dockWidget)
        {
            dockWidget->hide();
        }
    }

    RiuDockWidgetTools::trySetDockWidgetVisibility(mainWindow, RiuDockWidgetTools::relPermPlotName(), true);
    RiuDockWidgetTools::trySetDockWidgetVisibility(mainWindow, RiuDockWidgetTools::pvtPlotName(), true);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuDockWidgetTools::setVisibleDockingWindowsForGeoMech()
{
    RiuMainWindow* mainWindow = RiuMainWindow::instance();

    RiuDockWidgetTools::trySetDockWidgetVisibility(mainWindow, RiuDockWidgetTools::mohrsCirclePlotName(), false);

    {
        QDockWidget* dockWidget = findDockWidget(mainWindow, RiuDockWidgetTools::relPermPlotName());
        if (dockWidget)
        {
            dockWidget->hide();
        }
    }

    {
        QDockWidget* dockWidget = findDockWidget(mainWindow, RiuDockWidgetTools::pvtPlotName());
        if (dockWidget)
        {
            dockWidget->hide();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuDockWidgetTools::trySetDockWidgetVisibility(const QObject* parent, const QString& dockWidgetName, bool isVisible)
{
    QDockWidget* dockWidget = findDockWidget(parent, dockWidgetName);
    if (dockWidget)
    {
        dockWidget->setVisible(isVisible);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QVariant RiuDockWidgetTools::storeDockWidgetsVisibility(const QObject* parent)
{
    QMap<QString, QVariant> widgetVisibility;

    QList<QDockWidget*> dockWidgets = parent->findChildren<QDockWidget*>();

    for (QDockWidget* dock : dockWidgets)
    {
        if (dock)
        {
            bool isVisible = dock->isVisible();
            widgetVisibility[dock->objectName()] = isVisible;

            // qDebug() << "Store " << dock->objectName() << " : " << (isVisible ? "visible" : "not visible");
        }
    }

    return QVariant(widgetVisibility);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuDockWidgetTools::restoreDockWidgetsVisibility(const QObject* parent, QVariant widgetVisibilities)
{
    QMap<QString, QVariant> widgetVisibilityMap = widgetVisibilities.toMap();

    QList<QDockWidget*> dockWidgets = parent->findChildren<QDockWidget*>();

    for (QDockWidget* dock : dockWidgets)
    {
        if (dock)
        {
            auto widgetVisibility = widgetVisibilityMap.find(dock->objectName());

            if (widgetVisibility != widgetVisibilityMap.end())
            {
                bool isVisible = widgetVisibility.value().toBool();
                dock->setVisible(isVisible);

                // qDebug() << "Restore " << dock->objectName() << " : " << (isVisible ? "visible" : "not visible");
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuDockWidgetTools::changeDockWidgetVisibilityBasedOnView(Rim3dView* view)
{
    if (dynamic_cast<RimEclipseView*>(view))
    {
        setVisibleDockingWindowsForEclipse();
    }
    else if (dynamic_cast<RimGeoMechView*>(view))
    {
        setVisibleDockingWindowsForGeoMech();
    }
}
