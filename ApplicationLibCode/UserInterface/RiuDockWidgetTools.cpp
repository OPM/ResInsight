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
ads::CDockWidget* RiuDockWidgetTools::createDockWidget( QString title, QString dockName, QWidget* parent )
{
    ads::CDockWidget* dockWidget = new ads::CDockWidget( title, parent );
    dockWidget->setObjectName( dockName );
    dockWidget->setToggleViewActionMode( ads::CDockWidget::ActionModeShow );
    dockWidget->setIcon( RiuDockWidgetTools::dockIcon( dockName ) );

    return dockWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::mainPlotWindowName()
{
    return "dockPlotWindow_mainPlotWindow";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::main3DWindowName()
{
    return "dock3DWindow_mainWindow";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::mainWindowPropertyEditorName()
{
    return "dockPropertyEditor_mainWindow";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::mainWindowResultInfoName()
{
    return "dockResultInfo_mainWindow";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::mainWindowProcessMonitorName()
{
    return "dockProcessMonitor_mainWindow";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::mainWindowResultPlotName()
{
    return "dockResultPlot_mainWindow";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::mainWindowDepthPlotName()
{
    return "dockDepthPlot_mainWindow";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::mainWindowRelPermPlotName()
{
    return "dockRelPermPlot_mainWindow";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::mainWindowPvtPlotName()
{
    return "dockPvtPlot_mainWindow";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::mainWindowSeismicHistogramName()
{
    return "dockSeisHist_mainWindow";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::mainWindowMohrsCirclePlotName()
{
    return "dockMohrsCirclePlot_mainWindow";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::mainWindowUndoStackName()
{
    return "dockUndoStack_mainWindow";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::plotMainWindowPlotManagerName()
{
    return "dockSummaryPlotManager";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::mainWindowProjectTreeName()
{
    return "dockProjectTree_mainWindow";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::mainWindowDataSourceTreeName()
{
    return "dockDataSourceTree_mainWindow";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::mainWindowScriptsTreeName()
{
    return "dockScriptsTree_mainWindow";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::plotMainWindowDataSourceTreeName()
{
    return "dockDataSourceTree_plotMainWindow";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::plotMainWindowPlotsTreeName()
{
    return "dockPlotsTree_plotMainWindow";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::plotMainWindowTemplateTreeName()
{
    return "dockTemplatesTree_plotMainWindow";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::plotMainWindowScriptsTreeName()
{
    return "dockScriptsTree_plotMainWindow";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::plotMainWindowPropertyEditorName()
{
    return "dockPropertyEditor_plotMainWindow";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::plotMainWindowMessagesName()
{
    return "dockMessages_plotMainWindow";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::plotMainWindowUndoStackName()
{
    return "dockUndoStack_plotMainWindow";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::mainWindowMessagesName()
{
    return "dockMessages_mainWindow";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::dockState3DEclipseName()
{
    return "Default (Eclipse data)";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::dockState3DGeoMechName()
{
    return "Default (GeoMech data)";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::dockStatePlotWindowName()
{
    return "Default (Plot Window)";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::dockStateHideAllPlotWindowName()
{
    return "Hide All (Plot Window)";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::dockStateHideAll3DWindowName()
{
    return "Hide All";
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QIcon RiuDockWidgetTools::dockIcon( const QString dockWidgetName )
{
    if ( dockWidgetName == plotMainWindowPlotsTreeName() )
        return QIcon( ":/plots.svg" );
    else if ( dockWidgetName == plotMainWindowDataSourceTreeName() )
        return QIcon( ":/data-sources.svg" );
    else if ( dockWidgetName == plotMainWindowTemplateTreeName() )
        return QIcon( ":/plot-template-standard.svg" );
    else if ( dockWidgetName == plotMainWindowScriptsTreeName() )
        return QIcon( ":/scripts.svg" );
    else if ( dockWidgetName == plotMainWindowPropertyEditorName() )
        return QIcon( ":/property-editor.svg" );
    else if ( dockWidgetName == plotMainWindowMessagesName() )
        return QIcon( ":/messages.svg" );
    else if ( dockWidgetName == plotMainWindowUndoStackName() )
        return QIcon( ":/undo-stack.svg" );
    else if ( dockWidgetName == plotMainWindowPlotManagerName() )
        return QIcon( ":/plot-manager.svg" );
    else if ( dockWidgetName == mainWindowPropertyEditorName() )
        return QIcon( ":/property-editor.svg" );
    else if ( dockWidgetName == mainWindowResultInfoName() )
        return QIcon( ":/info.png" );
    else if ( dockWidgetName == mainWindowProcessMonitorName() )
        return QIcon( ":/view.svg" );
    else if ( dockWidgetName == mainWindowResultPlotName() )
        return QIcon( ":/graph.svg" );
    else if ( dockWidgetName == mainWindowDepthPlotName() )
        return QIcon( ":/graph.svg" );
    else if ( dockWidgetName == mainWindowRelPermPlotName() )
        return QIcon( ":/graph.svg" );
    else if ( dockWidgetName == mainWindowPvtPlotName() )
        return QIcon( ":/graph.svg" );
    else if ( dockWidgetName == mainWindowMessagesName() )
        return QIcon( ":/messages.svg" );
    else if ( dockWidgetName == mainWindowMohrsCirclePlotName() )
        return QIcon( ":/graph.svg" );
    else if ( dockWidgetName == mainWindowUndoStackName() )
        return QIcon( ":/undo-stack.svg" );
    else if ( dockWidgetName == mainWindowProjectTreeName() )
        return QIcon( ":/standard.svg" );
    else if ( dockWidgetName == mainWindowDataSourceTreeName() )
        return QIcon( ":/Calculator.svg" );
    else if ( dockWidgetName == mainWindowScriptsTreeName() )
        return QIcon( ":/scripts.svg" );
    else if ( dockWidgetName == mainPlotWindowName() )
        return QIcon( ":/window-management.svg" );
    else if ( dockWidgetName == main3DWindowName() )
        return QIcon( ":/window-management.svg" );
    else if ( dockWidgetName == mainWindowSeismicHistogramName() )
        return QIcon( ":/graph.svg" );

    return QIcon( ":/view.svg" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QByteArray RiuDockWidgetTools::defaultDockState( const QString& layoutName )
{
    if ( layoutName == dockState3DEclipseName() )
        return defaultEclipseDockState();
    else if ( layoutName == dockState3DGeoMechName() )
        return defaultGeoMechDockState();
    else if ( layoutName == dockStatePlotWindowName() )
        return defaultPlotDockState();
    else if ( layoutName == dockStateHideAll3DWindowName() )
        return hideAllDocking3DState();
    else if ( layoutName == dockStateHideAllPlotWindowName() )
        return hideAllDockingPlotState();

    // unknown dock state name found
    CAF_ASSERT( false );
    return QByteArray();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QByteArray RiuDockWidgetTools::defaultEclipseDockState()
{
    // In Debug builds:
    // Set up the dock widgets the way you want it using the GUI inside ResInsight - then use "Export Layout to
    // Clipboard" from the Windows menu and paste the exported text into this file to update the default states.

    // start paste

    static const char stateData[] =
        { '\x00', '\x00', '\x06', '\x46', '\x78', '\xda', '\x95', '\x55', '\xcb', '\x4e', '\xc3', '\x30', '\x10', '\xbc', '\xf3', '\x15',
          '\x56', '\xee', '\xd0', '\xe6', '\x01', '\xf4', '\x90', '\x16', '\xa1', '\x14', '\x04', '\x87', '\x42', '\x69', '\x5a', '\x7a',
          '\x44', '\x26', '\x59', '\x8a', '\xc1', '\xb1', '\xab', '\xf5', '\xb6', '\x3c', '\xc4', '\xc7', '\xe3', '\x3e', '\x14', '\xa9',
          '\xc5', '\x69', '\xc8', '\x21', '\x8a', '\xbd', '\x9e', '\xf1', '\xce', '\xac', '\xd7', '\x49', '\x7c', '\xf1', '\x59', '\x48',
          '\xb6', '\x04', '\x34', '\x42', '\xab', '\xae', '\xe7', '\x9f', '\xb4', '\x3d', '\x06', '\x2a', '\xd3', '\xb9', '\x50', '\xb3',
          '\xae', '\x37', '\x19', '\x5f', '\x1f', '\x77', '\xbc', '\x8b', '\x5e', '\xfc', '\x40', '\x97', '\xf9', '\x92', '\xab', '\x0c',
          '\xf2', '\xbe', '\xce', '\xde', '\xed', '\x5a', '\xfa', '\x65', '\x08', '\x0a', '\xf6', '\x58', '\x12', '\x3d', '\x36', '\x31',
          '\x80', '\xe5', '\x3c', '\xf4', '\x58', '\xa2', '\x15', '\x71', '\xa1', '\x6c', '\x64', '\xbd', '\x9c', '\x80', '\x22', '\xe4',
          '\x72', '\x2a', '\xf2', '\x19', '\x50', '\xd7', '\xcb', '\xed', '\x3e', '\x61', '\x7f', '\x2a', '\x54', '\xae', '\x3f', '\x9e',
          '\x0a', '\x8b', '\xdb', '\x0c', '\xbd', '\x5e', '\x5c', '\xf2', '\xd8', '\xb5', '\xd4', '\x9c', '\xd6', '\x42', '\xda', '\x36',
          '\x9e', '\xce', '\xa5', '\x20', '\xb2', '\xe1', '\x7b', '\x14', '\x76', '\x2f', '\xbb', '\xb2', '\x4a', '\xf4', '\xb3', '\x4a',
          '\xb4', '\x50', '\x76', '\xc7', '\xa0', '\x12', '\x73', '\xbc', '\x83', '\xb9', '\x44', '\xe0', '\x6c', '\xcc', '\x9f', '\xcd',
          '\x46', '\xe5', '\x02', '\x11', '\xd4', '\x56', '\xd0', '\x10', '\xf5', '\x1b', '\x64', '\x34', '\x46', '\x80', '\x5d', '\x4d',
          '\x1b', '\xd5', '\xec', '\x8e', '\x17', '\x70', '\x10', '\xc9', '\x12', '\xa9', '\x0d', '\xe4', '\x2b', '\xc1', '\x2d', '\x07',
          '\xab', '\xcf', '\x89', '\xa7', '\x7a', '\x81', '\x19', '\x34', '\x24', '\xa6', '\x19', '\x8a', '\x39', '\x99', '\xc3', '\xac',
          '\xd6', '\xca', '\xd9', '\x8e', '\x3f', '\xff', '\xaf', '\xbf', '\x39', '\x20', '\x7d', '\x5d', '\xe5', '\x82', '\x34', '\xd6',
          '\x5b', '\xac', '\x00', '\x3b', '\xd3', '\xa6', '\xe2', '\x1b', '\x4c', '\xef', '\x3c', '\x0a', '\xd9', '\x69', '\xfb', '\x9c',
          '\xc5', '\xad', '\xcd', '\xdc', '\xbe', '\xb7', '\x47', '\xd2', '\xf8', '\x70', '\xf6', '\xc5', '\xbb', '\xba', '\xc5', '\xb6',
          '\x08', '\x9f', '\xad', '\xb1', '\x0e', '\x03', '\x4e', '\x82', '\x53', '\x7a', '\x4d', '\x67', '\x85', '\x87', '\x85', '\x8d',
          '\xc0', '\x2c', '\x24', '\xdd', '\xaa', '\x17', '\x5d', '\x53', '\x51', '\x37', '\xb0', '\xee', '\x10', '\xcf', '\x9c', '\xe9',
          '\x86', '\x52', '\xd3', '\xbf', '\xd2', '\xed', '\x03', '\xeb', '\x5a', '\x14', '\xe6', '\xf4', '\xda', '\x90', '\x33', '\xd0',
          '\xaf', '\x68', '\x12', '\x81', '\x99', '\x84', '\x4a', '\xa6', '\xef', '\x64', '\x8e', '\x40', '\x0e', '\x01', '\x8b', '\x86',
          '\xf9', '\x86', '\xcb', '\xa6', '\xae', '\x52', '\x10', '\xe6', '\x46', '\x18', '\x6a', '\x52', '\xf7', '\x60', '\xaf', '\xee',
          '\x03', '\x30', '\x86', '\xcf', '\xc0', '\xd4', '\x54', '\xdd', '\x05', '\xab', '\xf3', '\x83', '\x3a', '\xb3', '\xac', '\x81',
          '\x56', '\x0d', '\x6e', '\x5a', '\xd4', '\x69', '\x33', '\xdf', '\x8f', '\x02', '\x16', '\x75', '\x2a', '\xae', '\xdb', '\x3a',
          '\xd2', '\x09', '\x4e', '\x59', '\x64', '\x9f', '\x6a', '\x44', '\x14', '\x86', '\x2c', '\xf0', '\x03', '\xdf', '\x09', '\x69',
          '\x95', '\x5f', '\x63', '\x3b', '\xae', '\xf8', '\x0b', '\xf4', '\x8e', '\x7e', '\x01', '\xe9', '\x95', '\x27', '\x97' };

    // end paste

    QByteArray retVal( stateData, sizeof( stateData ) );

    return retVal;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QByteArray RiuDockWidgetTools::defaultGeoMechDockState()
{
    // start paste

    static const char stateData[] =
        { '\x00', '\x00', '\x06', '\x81', '\x78', '\xda', '\x95', '\x55', '\xc1', '\x52', '\xc2', '\x30', '\x10', '\xbd', '\xfb', '\x15',
          '\x99', '\xde', '\x95', '\x96', '\x16', '\xe1', '\x50', '\x70', '\x1c', '\xd0', '\xd1', '\x03', '\x5a', '\x29', '\xca', '\xd1',
          '\x89', '\xed', '\x8a', '\xd1', '\x34', '\x61', '\x92', '\x2d', '\x8a', '\xe3', '\xc7', '\x9b', '\x52', '\xa7', '\x33', '\x60',
          '\x68', '\xe9', '\xa9', '\xc9', '\xe6', '\xbd', '\xbc', '\xb7', '\x9b', '\x4d', '\x1a', '\x5e', '\x7c', '\x65', '\x9c', '\xac',
          '\x41', '\x69', '\x26', '\xc5', '\xd0', '\xf1', '\xce', '\x5c', '\x87', '\x80', '\x48', '\x64', '\xca', '\xc4', '\x72', '\xe8',
          '\x3c', '\xce', '\xaf', '\x4f', '\x07', '\xce', '\xc5', '\x28', '\x7c', '\xc0', '\xcb', '\x74', '\x4d', '\x45', '\x02', '\xe9',
          '\x44', '\x26', '\x1f', '\x66', '\x2d', '\xde', '\x68', '\x84', '\x8c', '\x3c', '\x55', '\x44', '\x87', '\x3c', '\x6a', '\x50',
          '\xd5', '\xdc', '\x77', '\xc8', '\x58', '\x0a', '\xa4', '\x4c', '\x98', '\xc8', '\x76', '\x79', '\x0c', '\x02', '\x15', '\xe5',
          '\x0b', '\x96', '\x2e', '\x01', '\x87', '\x4e', '\x6a', '\xf6', '\xf1', '\x27', '\x0b', '\x26', '\x52', '\xf9', '\xf9', '\x9c',
          '\x19', '\x5c', '\x39', '\x74', '\x46', '\x61', '\xc5', '\x23', '\xd7', '\x5c', '\x52', '\xdc', '\x1a', '\x71', '\x4d', '\x3c',
          '\x5e', '\x71', '\x86', '\x68', '\xc2', '\xf7', '\x8a', '\x99', '\xbd', '\xcc', '\x4a', '\x21', '\xf4', '\x53', '\x08', '\xe5',
          '\x02', '\x0b', '\xc9', '\x43', '\x98', '\xd3', '\x0a', '\xd3', '\x35', '\x98', '\x4b', '\x05', '\x94', '\xcc', '\xe9', '\x8b',
          '\x2e', '\x5d', '\xe6', '\x4a', '\x81', '\xf8', '\x33', '\x14', '\x29', '\xf9', '\x0e', '\x09', '\xce', '\x15', '\xc0', '\xae',
          '\xa7', '\xd2', '\x35', '\xb9', '\xa3', '\x19', '\xd4', '\x22', '\xc9', '\x98', '\x4b', '\x0d', '\x69', '\x61', '\xb8', '\x63',
          '\x61', '\x4d', '\x28', '\xd2', '\x58', '\xe6', '\x2a', '\x81', '\x96', '\xc4', '\x38', '\x51', '\x6c', '\x85', '\xba', '\x9e',
          '\xd5', '\x29', '\x32', '\xdb', '\xc9', '\xcf', '\xfb', '\x9f', '\xdf', '\x0a', '\x14', '\x6e', '\xae', '\x52', '\x86', '\x52',
          '\x35', '\xa7', '\x78', '\x00', '\x6c', '\x95', '\x8d', '\xd9', '\x37', '\xe8', '\x51', '\x3f', '\xf0', '\x49', '\xcf', '\xed',
          '\x93', '\xb0', '\x53', '\xce', '\xcd', '\xf7', '\xef', '\x48', '\x5a', '\x1f', '\xce', '\xbe', '\x79', '\x5b', '\xb7', '\x98',
          '\x16', '\xa1', '\xcb', '\x2d', '\xd6', '\x92', '\x80', '\x95', '\x60', '\xb5', '\x7e', '\x44', '\x67', '\xd5', '\x18', '\x9b',
          '\x81', '\xce', '\x39', '\xde', '\x8a', '\x57', '\xd9', '\x50', '\x51', '\x3b', '\xb0', '\xe9', '\x10', '\x7b', '\x56', '\xb9',
          '\x88', '\x4b', '\x3c', '\x4a', '\x6e', '\x1f', '\xd8', '\xd0', '\x69', '\x53', '\xf9', '\xa6', '\xf4', '\x98', '\xa9', '\x84',
          '\x43', '\x4b', '\x66', '\x0c', '\x4c', '\xdf', '\x30', '\xdd', '\x86', '\x32', '\x03', '\x1e', '\x81', '\xca', '\x0e', '\x0a',
          '\x79', '\x56', '\x56', '\xb4', '\xc6', '\x7a', '\xc6', '\xff', '\x1a', '\x76', '\xf7', '\x6a', '\x38', '\x05', '\xad', '\xe9',
          '\x12', '\x74', '\x43', '\x05', '\x6d', '\xb0', '\x86', '\x94', '\xcc', '\xad', '\x49', '\x0c', '\x6b', '\x2a', '\x45', '\x8b',
          '\x5b', '\xd3', '\xeb', '\x7a', '\xc4', '\xf3', '\x7a', '\x2e', '\x09', '\xfc', '\x81', '\xfd', '\xea', '\x6c', '\x23', '\x83',
          '\x20', '\x20', '\x81', '\x7b', '\x6e', '\x45', '\xd4', '\xf4', '\xe7', '\x04', '\x56', '\xf8', '\x76', '\x44', '\xbf', '\x58',
          '\x71', '\xd6', '\xca', '\x96', '\xf2', '\x81', '\xef', '\x93', '\xae', '\x67', '\xac', '\xbb', '\x56', '\x47', '\x9d', '\xea',
          '\x21', '\x37', '\xe3', '\x03', '\x3f', '\x90', '\xd1', '\xc9', '\x2f', '\xd6', '\xef', '\x3b', '\x86' };

    // end paste

    QByteArray retVal( stateData, sizeof( stateData ) );

    return retVal;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QByteArray RiuDockWidgetTools::defaultPlotDockState()
{
    // start paste

    static const char stateData[] =
        { '\x00', '\x00', '\x04', '\xbd', '\x78', '\xda', '\x9d', '\x54', '\x51', '\x6f', '\x82', '\x30', '\x10', '\x7e', '\xdf', '\xaf',
          '\x68', '\xfa', '\xee', '\x50', '\xc1', '\x89', '\x09', '\x62', '\x8c', '\xce', '\x37', '\x37', '\x17', '\x70', '\x3e', '\x9a',
          '\x8e', '\x5e', '\x48', '\x33', '\x68', '\x49', '\x5b', '\xdd', '\x5c', '\xf6', '\xe3', '\x57', '\x60', '\x61', '\x9b', '\x01',
          '\xc4', '\x3d', '\xd1', '\xbb', '\xfb', '\xbe', '\xde', '\x77', '\xf9', '\xae', '\x78', '\xb3', '\xf7', '\x34', '\x41', '\x47',
          '\x90', '\x8a', '\x09', '\x3e', '\xc5', '\x83', '\xdb', '\x3e', '\x46', '\xc0', '\x23', '\x41', '\x19', '\x8f', '\xa7', '\x78',
          '\x1b', '\xae', '\x7a', '\x2e', '\x9e', '\xf9', '\xde', '\x93', '\x9e', '\xd3', '\x23', '\xe1', '\x11', '\xd0', '\xa5', '\x88',
          '\x5e', '\x4d', '\x2d', '\x38', '\x29', '\x0d', '\x29', '\x7a', '\xae', '\x88', '\x18', '\x6d', '\x15', '\xc8', '\x2a', '\xb6',
          '\x31', '\x5a', '\x08', '\xae', '\x09', '\xe3', '\x26', '\x53', '\x94', '\x17', '\xc0', '\xb5', '\x24', '\xc9', '\x8e', '\xd1',
          '\x18', '\xf4', '\x14', '\x53', '\x73', '\xcf', '\x26', '\x11', '\x7a', '\xc7', '\x38', '\x15', '\x6f', '\xfb', '\xd4', '\x20',
          '\x7f', '\x42', '\xec', '\x7b', '\x15', '\x1b', '\xad', '\x12', '\x41', '\x74', '\x21', '\xa7', '\x6f', '\xf2', '\x41', '\x96',
          '\x30', '\xad', '\x4d', '\xfa', '\x51', '\x32', '\x73', '\xa3', '\xa9', '\xe4', '\xed', '\x3e', '\xf3', '\x76', '\x07', '\xae',
          '\xf3', '\xc6', '\x4d', '\x98', '\x5e', '\x85', '\x19', '\x1a', '\xcc', '\x5c', '\x02', '\x41', '\x21', '\x79', '\x51', '\xa5',
          '\xd6', '\x83', '\x94', '\xc0', '\x7f', '\xc9', '\x52', '\xa1', '\x04', '\xd8', '\x67', '\xe6', '\xb4', '\x36', '\x2a', '\x2a',
          '\x55', '\xa5', '\x7a', '\xf4', '\x40', '\x52', '\xb8', '\x80', '\x45', '\x8b', '\x44', '\x28', '\xa0', '\xb9', '\x68', '\xab',
          '\x86', '\x17', '\x42', '\x9a', '\x25', '\x44', '\xc3', '\x7f', '\xb8', '\x41', '\x24', '\x59', '\xd6', '\xa5', '\xab', '\x95',
          '\x0f', '\xf9', '\x67', '\xd4', '\xc1', '\xf9', '\xa8', '\x52', '\x64', '\x20', '\xf5', '\xe9', '\x9e', '\x32', '\x2d', '\x64',
          '\x97', '\x79', '\x5b', '\x09', '\xb5', '\xed', '\x03', '\xf6', '\x01', '\xca', '\x1f', '\x4f', '\x5c', '\xe4', '\x8c', '\x27',
          '\xc8', '\xb3', '\xca', '\xd8', '\x7c', '\xbf', '\x5d', '\x6a', '\x17', '\xd8', '\xb8', '\x22', '\x66', '\x2f', '\x48', '\x5c',
          '\x30', '\x1a', '\x6c', '\x69', '\xa0', '\xd5', '\x4a', '\xbc', '\x6e', '\x61', '\x86', '\x67', '\x22', '\x97', '\x44', '\x93',
          '\x40', '\x1c', '\x64', '\x04', '\x1d', '\xb7', '\xa6', '\x9d', '\x70', '\xc9', '\xfe', '\x43', '\x9a', '\x12', '\x79', '\xda',
          '\x14', '\x24', '\x4e', '\x62', '\x90', '\x57', '\xdb', '\xbe', '\x06', '\xa5', '\x0c', '\x51', '\x75', '\x90', '\xda', '\x04',
          '\x6d', '\xb1', '\xda', '\x9d', '\xd8', '\xc8', '\x76', '\x9d', '\x5a', '\xab', '\xcb', '\x8c', '\x3d', '\x72', '\xd0', '\x60',
          '\x3c', '\xba', '\x43', '\x8e', '\xed', '\xd6', '\xc2', '\xac', '\xea', '\xf9', '\x9b', '\x73', '\xc3', '\xcf', '\xc7', '\xbf',
          '\xf9', '\x02', '\x51', '\x97', '\xa4', '\x18' };

    // end paste

    QByteArray retVal( stateData, sizeof( stateData ) );

    return retVal;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QByteArray RiuDockWidgetTools::hideAllDocking3DState()
{
    // start paste

    static const char stateData[] =
        { '\x00', '\x00', '\x06', '\x51', '\x78', '\xda', '\x95', '\x95', '\x4d', '\x53', '\xc2', '\x30', '\x10', '\x86', '\xef', '\xfe',
          '\x8a', '\x4c', '\xef', '\x48', '\x01', '\x65', '\x3c', '\xf0', '\x31', '\x4c', '\x91', '\x19', '\x0f', '\x28', '\x5a', '\x90',
          '\xa3', '\x13', '\xdb', '\x15', '\xa2', '\x69', '\xc2', '\x24', '\x0b', '\x8a', '\xe3', '\x8f', '\x37', '\x6d', '\xb1', '\x23',
          '\x25', '\x6d', '\xe9', '\xa9', '\x69', '\xf2', '\xbe', '\xd9', '\x67', '\xb3', '\x9b', '\xb6', '\x37', '\xfc', '\x8a', '\x38',
          '\xd9', '\x81', '\xd2', '\x4c', '\x8a', '\xbe', '\xd3', '\xba', '\x74', '\x1d', '\x02', '\x22', '\x90', '\x21', '\x13', '\xab',
          '\xbe', '\xb3', '\x98', '\x4f', '\x1a', '\x37', '\xce', '\x70', '\xd0', '\x7b', '\xc4', '\x51', '\xb8', '\xa3', '\x22', '\x80',
          '\x70', '\x2c', '\x83', '\x0f', '\xb3', '\xe6', '\xef', '\x35', '\x42', '\x44', '\x9e', '\x33', '\xa3', '\x43', '\x16', '\x1a',
          '\x54', '\xf6', '\xde', '\x71', '\x88', '\x27', '\x05', '\x52', '\x26', '\xcc', '\x4c', '\xb2', '\xec', '\x81', '\x40', '\x45',
          '\xf9', '\x92', '\x85', '\x2b', '\xc0', '\xbe', '\x13', '\x9a', '\x7d', '\x3a', '\xe3', '\x25', '\x13', '\xa1', '\xfc', '\x7c',
          '\x89', '\x8c', '\x2e', '\x1d', '\x3a', '\x83', '\x5e', '\xe6', '\x23', '\x13', '\x2e', '\x29', '\x26', '\x20', '\xae', '\x99',
          '\xf7', '\x37', '\x9c', '\x21', '\x9a', '\xe9', '\x07', '\xc5', '\xcc', '\x5e', '\x66', '\x25', '\x0e', '\xf4', '\x13', '\x07',
          '\xda', '\x0a', '\x8c', '\x43', '\x16', '\x69', '\x1a', '\x99', '\xa6', '\x6d', '\x34', '\x23', '\x05', '\x94', '\xcc', '\xe9',
          '\xab', '\x4e', '\x29', '\xb7', '\x4a', '\x81', '\x38', '\x00', '\xf9', '\x81', '\x62', '\x1b', '\xd4', '\x73', '\x05', '\x70',
          '\xcc', '\x94', '\x52', '\x93', '\x7b', '\x1a', '\x41', '\xaa', '\x9c', '\x29', '\xf9', '\x0e', '\x01', '\xe6', '\x95', '\xc4',
          '\xe3', '\x52', '\x43', '\x18', '\x27', '\xdc', '\xb4', '\xb8', '\xc6', '\x14', '\xa9', '\x2f', '\xb7', '\x2a', '\x80', '\x9a',
          '\xc6', '\x02', '\xb0', '\x63', '\x57', '\x33', '\xce', '\xec', '\x28', '\xbf', '\x56', '\x2e', '\x3f', '\x43', '\xbd', '\x01',
          '\x85', '\xfb', '\xdb', '\x90', '\xa1', '\x54', '\xd5', '\x29', '\x16', '\x88', '\xad', '\x61', '\x7d', '\xf6', '\x0d', '\x7a',
          '\xe0', '\x92', '\x56', '\xa7', '\xd5', '\x25', '\xbd', '\x66', '\xfa', '\x6a', '\x9e', '\x87', '\x8a', '\xd4', '\xae', '\x4d',
          '\x9e', '\xdd', '\xd6', '\x2c', '\xa6', '\x43', '\xe8', '\x2a', '\xd1', '\x5a', '\xf8', '\xad', '\x86', '\x3f', '\x72', '\xf7',
          '\x3f', '\xf9', '\x19', '\x8d', '\x55', '\x02', '\xf6', '\x04', '\x7a', '\xcb', '\xf1', '\x4e', '\xbc', '\xc9', '\x8a', '\x03',
          '\xb5', '\x0b', '\xab', '\x6a', '\x78', '\x95', '\x0b', '\x37', '\x95', '\x6b', '\xa5', '\x3d', '\xa6', '\x02', '\x0e', '\x33',
          '\x2e', '\xf1', '\xac', '\x98', '\x79', '\x61', '\x45', '\xb7', '\x95', '\x84', '\xa8', '\x70', '\x3e', '\x01', '\x9f', '\x81',
          '\x8a', '\x6a', '\xba', '\x66', '\xbb', '\x0a', '\xc2', '\xd3', '\x53', '\x69', '\x9f', '\x76', '\x76', '\x00', '\x5a', '\x4f',
          '\xa5', '\x38', '\xa3', '\xb3', '\xa7', '\x46', '\x49', '\x57', '\xa0', '\xeb', '\x20', '\x16', '\xee', '\x5f', '\x7a', '\x19',
          '\x5c', '\xd2', '\xbe', '\xee', '\xba', '\xf6', '\xeb', '\x90', '\xcc', '\x24', '\xb7', '\xc5', '\x2e', '\x28', '\x69', '\xb9',
          '\x31', '\x6c', '\x70', '\x7d', '\x46', '\xf5', '\xad', '\xba', '\x52', '\xe0', '\x04', '\xd7', '\xce', '\xd3', '\xcc', '\x3e',
          '\xcc', '\x66', '\x5c', '\xf0', '\x43', '\x18', '\x5c', '\xfc', '\x02', '\x10', '\x78', '\x2c', '\xb5' };

    // end paste

    QByteArray retVal( stateData, sizeof( stateData ) );

    return retVal;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QByteArray RiuDockWidgetTools::hideAllDockingPlotState()
{
    // start paste

    static const char stateData[] =
        { '\x00', '\x00', '\x04', '\xae', '\x78', '\xda', '\x9d', '\x54', '\xc1', '\x6e', '\x82', '\x40', '\x10', '\xbd', '\xf7', '\x2b',
          '\x36', '\x7b', '\xb7', '\x20', '\xa6', '\x8d', '\x07', '\xc4', '\x18', '\xad', '\x37', '\x5b', '\x1b', '\xb0', '\x1e', '\xcd',
          '\x96', '\x9d', '\x90', '\x4d', '\x61', '\x97', '\xec', '\x2e', '\xb6', '\x36', '\xfd', '\xf8', '\x0e', '\xa8', '\x58', '\x2d',
          '\x18', '\xf5', '\x04', '\x33', '\xf3', '\xde', '\xce', '\x1b', '\xde', '\xb0', '\xfe', '\xf0', '\x2b', '\x4b', '\xc9', '\x1a',
          '\xb4', '\x11', '\x4a', '\x0e', '\x68', '\xf7', '\xde', '\xa5', '\x04', '\x64', '\xac', '\xb8', '\x90', '\xc9', '\x80', '\x2e',
          '\xa2', '\x69', '\xa7', '\x4f', '\x87', '\x81', '\xff', '\x6a', '\x47', '\x7c', '\xcd', '\x64', '\x0c', '\x7c', '\xa2', '\xe2',
          '\x0f', '\xac', '\x85', '\x1b', '\x63', '\x21', '\x23', '\x6f', '\x35', '\x91', '\x92', '\x85', '\x01', '\x5d', '\xc7', '\x3d',
          '\x4a', '\xc6', '\x4a', '\x5a', '\x26', '\x24', '\x66', '\xaa', '\xf2', '\x18', '\xa4', '\xd5', '\x2c', '\x5d', '\x0a', '\x9e',
          '\x80', '\x1d', '\x50', '\x8e', '\xe7', '\xcc', '\x53', '\x65', '\x97', '\x42', '\x72', '\xf5', '\xb9', '\xca', '\x10', '\x79',
          '\x08', '\x69', '\xe0', '\xd7', '\x6c', '\x32', '\x4d', '\x15', '\xb3', '\x95', '\x1c', '\x17', '\xf3', '\x61', '\x9e', '\x0a',
          '\x6b', '\x31', '\xfd', '\xa2', '\x05', '\x9e', '\x88', '\x95', '\xb2', '\xdd', '\x4f', '\xd9', '\xae', '\x90', '\xb6', '\x6c',
          '\xdc', '\x86', '\xe9', '\xd4', '\x18', '\x0f', '\x31', '\x23', '\x0d', '\x8c', '\x44', '\xec', '\xdd', '\x6c', '\xb5', '\x16',
          '\x5a', '\x83', '\xdc', '\xc9', '\x0a', '\x63', '\x2d', '\x72', '\x6b', '\x22', '\x0d', '\xb0', '\xca', '\x51', '\xd3', '\x0c',
          '\x75', '\xd4', '\xba', '\xb6', '\xfa', '\xc9', '\x33', '\xcb', '\xe0', '\x30', '\x44', '\x23', '\x96', '\x8c', '\x53', '\x65',
          '\x80', '\x97', '\xc3', '\x3b', '\x0d', '\xbc', '\x08', '\xb2', '\x3c', '\x65', '\x16', '\x6e', '\xe1', '\x9e', '\x51', '\x78',
          '\xcc', '\x74', '\xca', '\x31', '\x8f', '\x86', '\xed', '\x9e', '\x0c', '\x3b', '\xd7', '\x2a', '\x07', '\x6d', '\x37', '\x4f',
          '\x5c', '\x58', '\xa5', '\x2f', '\x99', '\xf7', '\x2c', '\xa1', '\xb1', '\x7d', '\x28', '\xbe', '\xc1', '\x04', '\x2e', '\xe9',
          '\x7a', '\xfd', '\x1e', '\xf1', '\x9d', '\x6d', '\x88', '\xcf', '\x9d', '\x4d', '\xe7', '\xf5', '\xb5', '\xee', '\x08', '\x2e',
          '\x06', '\x4b', '\x2a', '\x46', '\x8b', '\x2b', '\x2d', '\xb4', '\xbd', '\x42', '\xf7', '\xaf', '\xc2', '\xeb', '\x36', '\xc6',
          '\x3b', '\xdd', '\x98', '\x22', '\xcb', '\x98', '\xde', '\xcc', '\xab', '\x6f', '\x21', '\x59', '\x02', '\xba', '\x49', '\xd3',
          '\x84', '\x59', '\x16', '\xaa', '\x42', '\xc7', '\x70', '\x8b', '\xe5', '\xff', '\x5b', '\x5c', '\x6b', '\xf5', '\x0c', '\x8c',
          '\x41', '\xa2', '\xb9', '\xc0', '\xe4', '\x36', '\xe8', '\x6d', '\xf6', '\xee', '\x01', '\xde', '\xc3', '\xa3', '\x4b', '\xdc',
          '\x46', '\x88', '\x53', '\xff', '\xef', '\xf8', '\xde', '\x72', '\xdb', '\x04', '\x77', '\xbf', '\xe8', '\x23', '\x9f', '\x33' };

    // end paste

    QByteArray retVal( stateData, sizeof( stateData ) );

    return retVal;
}
