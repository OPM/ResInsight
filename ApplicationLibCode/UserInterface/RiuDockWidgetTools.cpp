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
ads::CDockWidget* RiuDockWidgetTools::findDockWidget( const QObject* parent, const QString& dockWidgetName )
{
    QList<ads::CDockWidget*> dockWidgets = parent->findChildren<ads::CDockWidget*>();

    for ( auto dock : dockWidgets )
    {
        if ( dock->objectName() == dockWidgetName )
        {
            return dock;
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QAction* RiuDockWidgetTools::toggleActionForWidget( const QObject* parent, const QString& dockWidgetName )
{
    auto w = RiuDockWidgetTools::findDockWidget( parent, dockWidgetName );
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

    applyDockWidgetVisibilities( mainWindow, widgetVisibilities );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuDockWidgetTools::setVisibleDockingWindowsForGeoMech()
{
    if ( !RiuMainWindow::instance() ) return;

    RiuMainWindow* mainWindow         = RiuMainWindow::instance();
    auto           widgetVisibilities = widgetVisibilitiesForGeoMech();

    applyDockWidgetVisibilities( mainWindow, widgetVisibilities );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuDockWidgetTools::setDockWidgetVisibility( const QObject* parent, const QString& dockWidgetName, bool isVisible )
{
    ads::CDockWidget* dockWidget = findDockWidget( parent, dockWidgetName );
    if ( dockWidget )
    {
        dockWidget->setVisible( isVisible );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QVariant RiuDockWidgetTools::dockWidgetsVisibility( const QObject* parent )
{
    QMap<QString, QVariant> widgetVisibility;

    QList<ads::CDockWidget*> dockWidgets = parent->findChildren<ads::CDockWidget*>();

    for ( auto dock : dockWidgets )
    {
        if ( dock )
        {
            bool isVisible                       = dock->isVisible();
            widgetVisibility[dock->objectName()] = isVisible;

            // qDebug() << "Store " << dock->objectName() << " : " << (isVisible ? "visible" : "not visible");
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

    QList<ads::CDockWidget*> dockWidgets = mainWindow->findChildren<ads::CDockWidget*>();
    dockWidgets.removeAll( nullptr );

    for ( auto dock : dockWidgets )
    {
        dock->setVisible( false );
    }
    QApplication::processEvents();

    {
        auto dock = findDockWidget( mainWindow, relPermPlotName() );
        if ( dock )
        {
            dock->setVisible( true );
        }
    }

    {
        auto dock = findDockWidget( mainWindow, pvtPlotName() );
        if ( dock )
        {
            dock->setVisible( true );
        }
    }

    QApplication::processEvents();

    mainWindow->restoreDockWidgetVisibilities();
    mainWindow->loadWinGeoAndDockToolBarLayout();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuDockWidgetTools::applyDockWidgetVisibilities( const QObject*                 parent,
                                                      const QMap<QString, QVariant>& widgetVisibilities )
{
    QList<ads::CDockWidget*> dockWidgets = parent->findChildren<ads::CDockWidget*>();

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

            // qDebug() << "Restore " << dock->objectName() << " : " << (isVisible ? "visible" : "not visible");
        }
    }
}
