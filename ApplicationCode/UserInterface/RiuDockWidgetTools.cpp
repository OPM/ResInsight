/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 Statoil ASA
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
RiuDockWidgetTools::RiuDockWidgetTools()
{
    loadDockWidgetsState();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuDockWidgetTools* RiuDockWidgetTools::instance()
{
    static RiuDockWidgetTools staticInstance;

    return &staticInstance;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::projectTreeName() const
{
    return "dockProjectTree";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::propertyEditorName() const
{
    return "dockpropertyEditor";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::resultInfoName() const
{
    return "dockResultInfo";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::processMonitorName() const
{
    return "dockProcessMonitor";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::resultPlotName() const
{
    return "dockResultPlot";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::relPermPlotName() const
{
    return "dockRelPermPlot";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::pvtPlotName() const
{
    return "dockPvtPlot";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::mohrsCirclePlotName() const
{
    return "dockMohrsCirclePlot";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::messagesName() const
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

    RiuDockWidgetTools::instance()->trySetDockWidgetVisibility(mainWindow, RiuDockWidgetTools::relPermPlotName(), true);
    RiuDockWidgetTools::instance()->trySetDockWidgetVisibility(mainWindow, RiuDockWidgetTools::pvtPlotName(), true);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuDockWidgetTools::setVisibleDockingWindowsForGeoMech()
{
    RiuMainWindow* mainWindow = RiuMainWindow::instance();

    RiuDockWidgetTools::instance()->trySetDockWidgetVisibility(mainWindow, RiuDockWidgetTools::mohrsCirclePlotName(), false);

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
        bool unifiedIsVisible = isVisible;

        auto state = visibilityForWidget(dockWidgetName);
        if (state != RiuDockWidgetTools::USER_DEFINED_UNKNOWN)
        {
            if (state == RiuDockWidgetTools::USER_DEFINED_ON)
            {
                unifiedIsVisible = true;
            }
            else if (state == RiuDockWidgetTools::USER_DEFINED_OFF)
            {
                unifiedIsVisible = false;
            }
        }

        dockWidget->setVisible(unifiedIsVisible);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuDockWidgetTools::UserDefinedVisibility RiuDockWidgetTools::visibilityForWidget(const QString& objectName)
{
    RiuDockWidgetTools::UserDefinedVisibility visibility = USER_DEFINED_UNKNOWN;

    auto windowStateIt = m_userDefinedDockWidgetVisibility.find(objectName);
    if (windowStateIt != m_userDefinedDockWidgetVisibility.end())
    {
        bool isVisible = windowStateIt.value().toBool();
        if (isVisible)
        {
            visibility = USER_DEFINED_ON;
        }
        else
        {
            visibility = USER_DEFINED_OFF;
        }
    }

    return visibility;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuDockWidgetTools::setDockWidgetVisibility(const QString& dockingWindowName, bool isVisible)
{
    m_userDefinedDockWidgetVisibility[dockingWindowName] = isVisible;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuDockWidgetTools::loadDockWidgetsState()
{
    // Company and appname set through QCoreApplication
    QSettings settings;

    m_userDefinedDockWidgetVisibility = settings.value("dockWindowStates").toMap();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuDockWidgetTools::saveDockWidgetsState()
{
    // Company and appname set through QCoreApplication
    QSettings settings;

    QVariant v(m_userDefinedDockWidgetVisibility);
    settings.setValue("dockWindowStates", v);
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
