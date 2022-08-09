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

#include "DockManager.h"
#include "DockWidget.h"

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
QString RiuDockWidgetTools::undoStackName()
{
    return "dockUndoStack";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::summaryPlotManagerName()
{
    return "dockSummaryPlotManager";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::mainWindowProjectTreeName()
{
    return "mainWindow_dockProjectTree";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::mainWindowDataSourceTreeName()
{
    return "mainWindow_dockDataSourceTree";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::mainWindowScriptsTreeName()
{
    return "mainWindow_dockScriptsTree";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::plotMainWindowDataSourceTreeName()
{
    return "plotMainWindow_dockDataSourceTree";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::plotMainWindowPlotsTreeName()
{
    return "plotMainWindow_dockPlotsTree";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::plotMainWindowTemplateTreeName()
{
    return "plotMainWindow_dockTemplatesTree";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::plotMainWindowScriptsTreeName()
{
    return "plotMainWindow_dockScriptsTree";
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
QString RiuDockWidgetTools::plotMainWindowMessagesName()
{
    return "plotMainWindow_dockMessages";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::plotMainWindowUndoStackName()
{
    return "plotMainWindow_dockUndoStack";
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
ads::CDockWidget* RiuDockWidgetTools::findDockWidget( const ads::CDockManager* dockManager, const QString& dockWidgetName )
{
    return dockManager->findDockWidget( dockWidgetName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QAction* RiuDockWidgetTools::toggleActionForWidget( const ads::CDockManager* dockManager, const QString& dockWidgetName )
{
    auto w = findDockWidget( dockManager, dockWidgetName );
    if ( w )
    {
        return w->toggleViewAction();
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuDockWidgetTools::showDockWidget( const ads::CDockManager* dockManager, const QString& dockWidgetName )
{
    auto dw = findDockWidget( dockManager, dockWidgetName );
    if ( dw )
    {
        dw->show();
        dw->raise();
    }
}
