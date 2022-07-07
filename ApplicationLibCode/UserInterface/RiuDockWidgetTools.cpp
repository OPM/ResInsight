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
QMap<QString, QVariant> RiuDockWidgetTools::widgetVisibilitiesForEclipse()
{
    QMap<QString, QVariant> widgetVisibility;

    widgetVisibility[RiuDockWidgetTools::projectTreeName()]    = true;
    widgetVisibility[RiuDockWidgetTools::propertyEditorName()] = true;
    widgetVisibility[RiuDockWidgetTools::resultInfoName()]     = true;
    widgetVisibility[RiuDockWidgetTools::processMonitorName()] = true;
    widgetVisibility[RiuDockWidgetTools::resultPlotName()]     = true;
    widgetVisibility[RiuDockWidgetTools::relPermPlotName()]    = true;
    widgetVisibility[RiuDockWidgetTools::pvtPlotName()]        = true;
    widgetVisibility[RiuDockWidgetTools::messagesName()]       = true;
    widgetVisibility[RiuDockWidgetTools::undoStackName()]      = false;

    widgetVisibility[RiuDockWidgetTools::mohrsCirclePlotName()] = false;

    return widgetVisibility;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QMap<QString, QVariant> RiuDockWidgetTools::widgetVisibilitiesForGeoMech()
{
    QMap<QString, QVariant> widgetVisibility;

    widgetVisibility[RiuDockWidgetTools::projectTreeName()]    = true;
    widgetVisibility[RiuDockWidgetTools::propertyEditorName()] = true;
    widgetVisibility[RiuDockWidgetTools::resultInfoName()]     = true;
    widgetVisibility[RiuDockWidgetTools::processMonitorName()] = true;
    widgetVisibility[RiuDockWidgetTools::resultPlotName()]     = true;
    widgetVisibility[RiuDockWidgetTools::relPermPlotName()]    = false;
    widgetVisibility[RiuDockWidgetTools::pvtPlotName()]        = false;
    widgetVisibility[RiuDockWidgetTools::messagesName()]       = true;
    widgetVisibility[RiuDockWidgetTools::undoStackName()]      = false;

    widgetVisibility[RiuDockWidgetTools::mohrsCirclePlotName()] = true;

    return widgetVisibility;
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
void RiuDockWidgetTools::setVisibleDockingWindowsForEclipse()
{
    if ( !RiuMainWindow::instance() ) return;

    RiuMainWindow* mainWindow         = RiuMainWindow::instance();
    auto           widgetVisibilities = widgetVisibilitiesForEclipse();

    applyDockWidgetVisibilities( mainWindow->dockManager(), widgetVisibilities );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuDockWidgetTools::setVisibleDockingWindowsForGeoMech()
{
    if ( !RiuMainWindow::instance() ) return;

    RiuMainWindow* mainWindow         = RiuMainWindow::instance();
    auto           widgetVisibilities = widgetVisibilitiesForGeoMech();

    applyDockWidgetVisibilities( mainWindow->dockManager(), widgetVisibilities );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuDockWidgetTools::setDockWidgetVisibility( const ads::CDockManager* dockManager,
                                                  const QString&           dockWidgetName,
                                                  bool                     isVisible )
{
    ads::CDockWidget* dockWidget = findDockWidget( dockManager, dockWidgetName );
    if ( dockWidget )
    {
        dockWidget->setVisible( isVisible );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QVariant RiuDockWidgetTools::dockWidgetsVisibility( const ads::CDockManager* dockManager )
{
    QMap<QString, QVariant> widgetVisibility;

    auto dockWidgets = dockManager->dockWidgetsMap();

    for ( auto dock : dockWidgets )
    {
        if ( dock )
        {
            bool isVisible                       = dock->isVisible();
            widgetVisibility[dock->objectName()] = isVisible;
        }
    }

    return QVariant( widgetVisibility );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QVariant RiuDockWidgetTools::defaultDockWidgetVisibilities()
{
    return QVariant( widgetVisibilitiesForEclipse() );
}

//--------------------------------------------------------------------------------------------------
/// Qwt widgets in non-visible dock widgets (tabbed dock windows) will on some systems enter an
/// eternal update loop. This is seen on both Windows and Linux.
/// The workaround is to hide all dock widgets, and then set visible the docking windows seen to
/// trigger the unwanted behavior
///
/// https://github.com/OPM/ResInsight/issues/6743
/// https://github.com/OPM/ResInsight/issues/6627
//--------------------------------------------------------------------------------------------------
void RiuDockWidgetTools::workaroundForQwtDockWidgets()
{
    if ( !RiuMainWindow::instance() ) return;

    RiuMainWindow* mainWindow = RiuMainWindow::instance();

    auto dockWidgets = mainWindow->dockManager()->dockWidgetsMap();

    for ( auto dock : dockWidgets )
    {
        if ( dock ) dock->setVisible( false );
    }
    QApplication::processEvents();

    {
        auto dock = mainWindow->dockManager()->findDockWidget( relPermPlotName() );
        if ( dock )
        {
            dock->setVisible( true );
        }
    }

    {
        auto dock = mainWindow->dockManager()->findDockWidget( pvtPlotName() );
        if ( dock )
        {
            dock->setVisible( true );
        }
    }

    QApplication::processEvents();

    mainWindow->loadWinGeoAndDockToolBarLayout();
    mainWindow->restoreDockWidgetVisibilities();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuDockWidgetTools::applyDockWidgetVisibilities( const ads::CDockManager*       dockManager,
                                                      const QMap<QString, QVariant>& widgetVisibilities )
{
    auto dockWidgets = dockManager->dockWidgetsMap();

    for ( auto dock : dockWidgets )
    {
        if ( dock )
        {
            bool isVisible = true;

            auto widgetVisibility = widgetVisibilities.find( dock->objectName() );
            if ( widgetVisibility != widgetVisibilities.end() )
            {
                isVisible = widgetVisibility.value().toBool();
            }

            dock->setVisible( isVisible );
        }
    }
}
