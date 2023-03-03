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
        return QIcon( ":/SummaryTemplate16x16.png" );
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
        { '\x00', '\x00', '\x06', '\x92', '\x78', '\xda', '\xb5', '\x55', '\x5d', '\x4f', '\xc2', '\x30', '\x14', '\x7d', '\xf7', '\x57',
          '\x34', '\x7b', '\x47', '\x18', '\x83', '\x30', '\x13', '\xc0', '\x18', '\x90', '\xc4', '\x07', '\x14', '\x1d', '\xca', '\xa3',
          '\xa9', '\xdb', '\x15', '\xab', '\x5d', '\x4b', '\x6e', '\x2f', '\x28', '\xc6', '\x1f', '\x6f', '\xc7', '\xcc', '\x12', '\xb0',
          '\x50', '\x79', '\xf0', '\x69', '\xed', '\xed', '\x39', '\x3d', '\xe7', '\x7e', '\x6c', '\xeb', '\x9e', '\x7f', '\xe4', '\x92',
          '\xad', '\x00', '\x8d', '\xd0', '\xaa', '\x17', '\x84', '\xa7', '\x8d', '\x80', '\x81', '\x4a', '\x75', '\x26', '\xd4', '\xbc',
          '\x17', '\xdc', '\x4f', '\x47', '\xb5', '\x38', '\x38', '\xef', '\x77', '\x6f', '\xe9', '\x22', '\x5b', '\x71', '\x95', '\x42',
          '\x36', '\xd4', '\xe9', '\x9b', '\x3d', '\x4b', '\xd6', '\x86', '\x20', '\x67', '\x0f', '\x15', '\x31', '\x60', '\xf7', '\x06',
          '\xb0', '\xda', '\x47', '\x01', '\x1b', '\x68', '\x45', '\x5c', '\x28', '\x1b', '\xd9', '\x1c', '\x0f', '\x40', '\x11', '\x72',
          '\x39', '\x13', '\xd9', '\x1c', '\xa8', '\x17', '\x64', '\xf6', '\x9e', '\x68', '\x38', '\x13', '\x2a', '\xd3', '\xef', '\x8f',
          '\xb9', '\xc5', '\x95', '\xcb', '\xa0', '\xdf', '\xad', '\x78', '\x6c', '\x24', '\x35', '\xa7', '\x8d', '\x91', '\x86', '\x8d',
          '\x27', '\x0b', '\x29', '\x88', '\x6c', '\xf8', '\x06', '\x85', '\xbd', '\xcb', '\x9e', '\x14', '\x42', '\x5f', '\x85', '\xd0',
          '\x52', '\x51', '\x21', '\xb9', '\x0f', '\x53', '\xab', '\x30', '\x4d', '\x8b', '\xb9', '\x40', '\xe0', '\x6c', '\xca', '\x9f',
          '\x4c', '\xe9', '\x72', '\x89', '\x08', '\xea', '\xc7', '\xd0', '\x04', '\xf5', '\x2b', '\xa4', '\x34', '\x45', '\x80', '\x6d',
          '\x4f', '\xa5', '\x6b', '\x76', '\xcd', '\x73', '\x38', '\x88', '\x64', '\x03', '\xa9', '\x0d', '\x64', '\x85', '\xe1', '\xba',
          '\x83', '\x35', '\xe4', '\xc4', '\x13', '\xbd', '\xc4', '\x14', '\x8e', '\x24', '\x26', '\x29', '\x8a', '\x05', '\x99', '\xc3',
          '\xac', '\x7a', '\x91', '\xd9', '\x56', '\x7e', '\xe1', '\xef', '\xfc', '\x16', '\x80', '\xb4', '\xbe', '\xcc', '\x04', '\x69',
          '\xf4', '\xa7', '\xb8', '\x07', '\xec', '\x94', '\x4d', '\xc4', '\x27', '\x98', '\x7e', '\xa7', '\x73', '\xc6', '\xda', '\x51',
          '\xc8', '\xba', '\xf5', '\x72', '\x6f', '\x9f', '\x3f', '\x2d', '\x39', '\xba', '\x39', '\xbb', '\xe6', '\x5d', '\xd3', '\x62',
          '\x47', '\x84', '\xcf', '\x37', '\x58', '\x47', '\x02', '\x4e', '\x82', '\xd3', '\xba', '\x67', '\xb2', '\x3c', '\xc6', '\xee',
          '\xc0', '\x2c', '\x25', '\x5d', '\xa9', '\x67', '\xed', '\xa9', '\xa8', '\x1b', '\xe8', '\x6b', '\x62', '\xcb', '\x29', '\x37',
          '\x91', '\x9a', '\xfe', '\x24', '\xb7', '\x0b', '\xf4', '\x4c', '\xda', '\x58', '\xbf', '\xa0', '\x19', '\x08', '\x4c', '\x25',
          '\xec', '\x65', '\x86', '\x4e', '\xe6', '\x1d', '\xc8', '\x09', '\x60', '\x7e', '\xa4', '\xde', '\x64', '\xe5', '\x71', '\xb8',
          '\x35', '\x5e', '\xad', '\xf0', '\x8c', '\x85', '\xcd', '\x38', '\x72', '\xcf', '\xd7', '\x26', '\x12', '\xc7', '\x6d', '\xd6',
          '\x6a', '\xb6', '\xff', '\x63', '\x02', '\x87', '\xb0', '\xa0', '\x97', '\x3f', '\x14', '\xde', '\x89', '\xf3', '\xb5', '\xb9',
          '\xb9', '\x23', '\x36', '\x06', '\x63', '\xf8', '\x1c', '\x8c', '\x47', '\xcb', '\x05', '\xf3', '\x95', '\x1c', '\x75', '\x6a',
          '\x59', '\x63', '\xad', '\x8e', '\x78', '\xb1', '\x0f', '\x96', '\xb5', '\xec', '\x4d', '\x14', '\xb1', '\xb0', '\xd3', '\x88',
          '\x59', '\xab', '\xd1', '\x71', '\xc2', '\xea', '\xd5', '\x97', '\xdd', '\xae', '\xf7', '\xfc', '\x51', '\xfa', '\x27', '\xdf',
          '\x7c', '\xb5', '\x40', '\x74' };

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
        { '\x00', '\x00', '\x06', '\x4e', '\x78', '\xda', '\x95', '\x55', '\x5d', '\x4f', '\xc2', '\x30', '\x14', '\x7d', '\xf7', '\x57',
          '\x34', '\x7b', '\x47', '\x36', '\x36', '\x44', '\x12', '\x3e', '\x42', '\x40', '\x12', '\x1f', '\x50', '\x64', '\x20', '\x8f',
          '\xa6', '\x6e', '\x57', '\xa8', '\x76', '\x2d', '\x69', '\x2f', '\x28', '\xc6', '\x1f', '\x6f', '\xc7', '\xc8', '\x12', '\xa0',
          '\x6c', '\xf0', '\xb4', '\xee', '\xf6', '\x9c', '\x9e', '\x73', '\x3f', '\xba', '\xb5', '\xba', '\x3f', '\x09', '\x27', '\x1b',
          '\x50', '\x9a', '\x49', '\xd1', '\x76', '\xbc', '\x5b', '\xd7', '\x21', '\x20', '\x22', '\x19', '\x33', '\xb1', '\x68', '\x3b',
          '\xb3', '\xe9', '\xb0', '\x72', '\xef', '\x74', '\x3b', '\xad', '\x17', '\xec', '\xc5', '\x1b', '\x2a', '\x22', '\x88', '\x07',
          '\x32', '\xfa', '\x32', '\x7b', '\xe1', '\x56', '\x23', '\x24', '\xe4', '\x35', '\x27', '\x3a', '\x64', '\xa6', '\x41', '\xe5',
          '\xef', '\xbe', '\x43', '\xfa', '\x52', '\x20', '\x65', '\xc2', '\x44', '\x76', '\xdb', '\x7d', '\x10', '\xa8', '\x28', '\x9f',
          '\xb3', '\x78', '\x01', '\xd8', '\x76', '\x62', '\x73', '\x8e', '\x3f', '\x98', '\x33', '\x11', '\xcb', '\xef', '\xb7', '\xc4',
          '\xe0', '\xb2', '\xa5', '\xd3', '\x69', '\xe5', '\x3c', '\x32', '\xe4', '\x92', '\xe2', '\xce', '\x88', '\x6b', '\xe2', '\xe1',
          '\x8a', '\x33', '\x44', '\x13', '\x7e', '\x56', '\xcc', '\x9c', '\x65', '\x76', '\x52', '\xa1', '\xbf', '\x54', '\x68', '\x2d',
          '\x30', '\x95', '\x3c', '\x87', '\xa9', '\xe4', '\x98', '\x9a', '\xc1', '\xf4', '\x14', '\x50', '\x32', '\xa5', '\xef', '\x3a',
          '\x73', '\xb9', '\x56', '\x0a', '\xc4', '\xde', '\xd0', '\x58', '\xc9', '\x4f', '\x88', '\x70', '\xaa', '\x00', '\x0e', '\x3d',
          '\x65', '\xae', '\xc9', '\x13', '\x4d', '\xa0', '\x10', '\x49', '\xfa', '\x5c', '\x6a', '\x88', '\x53', '\xc3', '\x55', '\x0b',
          '\x6b', '\x40', '\x91', '\x86', '\x72', '\xad', '\x22', '\xb8', '\x92', '\x18', '\x46', '\x8a', '\xad', '\x50', '\x17', '\xb3',
          '\xaa', '\x69', '\x66', '\x07', '\xf9', '\x79', '\xa7', '\xf9', '\xad', '\x40', '\xe1', '\xf6', '\x21', '\x66', '\x28', '\x55',
          '\x79', '\x8a', '\x67', '\xc0', '\x56', '\xd9', '\x90', '\xfd', '\x82', '\xee', '\x34', '\x1a', '\x4d', '\x52', '\xf7', '\x3d',
          '\xd2', '\xaa', '\x66', '\xef', '\xe6', '\xb9', '\x6f', '\xc9', '\xd5', '\xcd', '\x39', '\x36', '\x6f', '\x9b', '\x16', '\x33',
          '\x22', '\x74', '\xb1', '\xc3', '\x5a', '\x12', '\xb0', '\x12', '\xac', '\xd6', '\x2f', '\x98', '\xac', '\x02', '\x63', '\x13',
          '\xd0', '\x6b', '\x8e', '\x8f', '\xe2', '\x43', '\x96', '\x54', '\xd4', '\x0e', '\x2c', '\x6b', '\x62', '\x60', '\x95', '\x1b',
          '\x73', '\x89', '\x17', '\xc9', '\x1d', '\x03', '\x4b', '\x26', '\x6d', '\x24', '\x97', '\x4a', '\xf7', '\x99', '\x8a', '\x38',
          '\x5c', '\xc9', '\x9c', '\x00', '\x1f', '\x83', '\x4a', '\xce', '\xb2', '\x3c', '\x2b', '\x6b', '\xbc', '\xc1', '\x62', '\xc6',
          '\x69', '\x41', '\x6a', '\x47', '\x05', '\x19', '\x81', '\xd6', '\x74', '\x01', '\xba', '\xa4', '\x1c', '\x36', '\x58', '\x49',
          '\x4a', '\xe6', '\x0a', '\x44', '\x86', '\x35', '\x92', '\xe2', '\x8a', '\x2b', '\x50', '\xaf', '\x79', '\xc4', '\xf3', '\xea',
          '\x2e', '\x09', '\xfc', '\x7b', '\xfb', '\x3d', '\xd8', '\x45', '\x9a', '\x6e', '\x40', '\x02', '\xf7', '\xce', '\x8a', '\x28',
          '\x18', '\xb6', '\x01', '\xac', '\x70', '\x79', '\x41', '\xf3', '\xad', '\x38', '\x6b', '\x65', '\x33', '\xf9', '\xc0', '\xf7',
          '\x49', '\xcd', '\x33', '\xd6', '\x5d', '\xab', '\xa3', '\x6a', '\xfe', '\x55', '\x36', '\xeb', '\x33', '\x7f', '\x83', '\xce',
          '\xcd', '\x3f', '\x00', '\xe7', '\x29', '\xe5' };

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
