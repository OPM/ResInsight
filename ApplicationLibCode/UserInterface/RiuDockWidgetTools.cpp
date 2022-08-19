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
QString RiuDockWidgetTools::propertyEditorName()
{
    return "dockPropertyEditor_mainWindow";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::resultInfoName()
{
    return "dockResultInfo_mainWindow";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::processMonitorName()
{
    return "dockProcessMonitor_mainWindow";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::resultPlotName()
{
    return "dockResultPlot_mainWindow";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::relPermPlotName()
{
    return "dockRelPermPlot_mainWindow";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::pvtPlotName()
{
    return "dockPvtPlot_mainWindow";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::mohrsCirclePlotName()
{
    return "dockMohrsCirclePlot_mainWindow";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::undoStackName()
{
    return "dockUndoStack_mainWindow";
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
QString RiuDockWidgetTools::messagesName()
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
    else if ( dockWidgetName == summaryPlotManagerName() )
        return QIcon( ":/plot-manager.svg" );
    else if ( dockWidgetName == propertyEditorName() )
        return QIcon( ":/property-editor.svg" );
    else if ( dockWidgetName == resultInfoName() )
        return QIcon( ":/info.png" );
    else if ( dockWidgetName == processMonitorName() )
        return QIcon( ":/view.svg" );
    else if ( dockWidgetName == resultPlotName() )
        return QIcon( ":/graph.svg" );
    else if ( dockWidgetName == relPermPlotName() )
        return QIcon( ":/graph.svg" );
    else if ( dockWidgetName == pvtPlotName() )
        return QIcon( ":/graph.svg" );
    else if ( dockWidgetName == messagesName() )
        return QIcon( ":/messages.svg" );
    else if ( dockWidgetName == mohrsCirclePlotName() )
        return QIcon( ":/graph.svg" );
    else if ( dockWidgetName == undoStackName() )
        return QIcon( ":/undo-stack.svg" );
    else if ( dockWidgetName == mainWindowProjectTreeName() )
        return QIcon( ":/standard.svg" );
    else if ( dockWidgetName == mainWindowDataSourceTreeName() )
        return QIcon( ":/data-sources.svg" );
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

    static const char stateData[] =
        { '\x00', '\x00', '\x06', '\x13', '\x78', '\xda', '\x95', '\x54', '\xdf', '\x4f', '\xc2', '\x30', '\x10',
          '\x7e', '\xf7', '\xaf', '\x68', '\xf6', '\xae', '\x30', '\xc6', '\x94', '\x25', '\x80', '\x31', '\xa0',
          '\x89', '\x0f', '\x28', '\x3a', '\xd0', '\x47', '\x53', '\xbb', '\x13', '\xab', '\x5d', '\x4b', '\xae',
          '\x07', '\xfe', '\x88', '\x7f', '\xbc', '\x1d', '\x18', '\x12', '\xa0', '\xdb', '\xe4', '\x69', '\xdd',
          '\xdd', '\xf7', '\xf5', '\xfb', '\xae', '\xbd', '\x6b', '\xf7', '\xfc', '\x33', '\x57', '\x6c', '\x09',
          '\x68', '\xa5', '\xd1', '\xbd', '\x20', '\x3c', '\x69', '\x06', '\x0c', '\xb4', '\x30', '\x99', '\xd4',
          '\xb3', '\x5e', '\x30', '\x9d', '\x5c', '\x1d', '\x77', '\x82', '\xf3', '\x7e', '\xf7', '\x8e', '\x2e',
          '\xb2', '\x25', '\xd7', '\x02', '\xb2', '\xa1', '\x11', '\xef', '\x2e', '\x97', '\x7e', '\x59', '\x82',
          '\x9c', '\x3d', '\x6c', '\x88', '\x01', '\x9b', '\x5a', '\xc0', '\xcd', '\x7f', '\x2b', '\x60', '\x03',
          '\xa3', '\x89', '\x4b', '\xed', '\x22', '\xab', '\xf4', '\x00', '\x34', '\x21', '\x57', '\x8f', '\x32',
          '\x9b', '\x01', '\xf5', '\x82', '\xcc', '\xed', '\x13', '\x0d', '\x1f', '\xa5', '\xce', '\xcc', '\xc7',
          '\x53', '\xee', '\x70', '\xeb', '\x65', '\xd0', '\xef', '\x6e', '\x78', '\xec', '\x4a', '\x19', '\x4e',
          '\x2b', '\x23', '\x4d', '\x17', '\x4f', '\xe7', '\x4a', '\x12', '\xb9', '\xf0', '\x2d', '\x4a', '\xb7',
          '\x97', '\xcb', '\x14', '\x42', '\x3f', '\x85', '\xd0', '\x42', '\x53', '\x21', '\x59', '\x86', '\x39',
          '\xde', '\xc2', '\x5c', '\x20', '\x70', '\x36', '\xe1', '\xcf', '\xce', '\x56', '\xe4', '\x12', '\x0b',
          '\x44', '\xd0', '\x7f', '\x86', '\xc6', '\x68', '\xde', '\x40', '\xd0', '\x04', '\x01', '\xb6', '\x3d',
          '\xad', '\x5d', '\xb3', '\x1b', '\x9e', '\x43', '\x25', '\x92', '\x0d', '\x94', '\xb1', '\x90', '\x15',
          '\x86', '\x1b', '\x1e', '\xd6', '\x90', '\x13', '\x4f', '\xcd', '\x02', '\x05', '\x1c', '\x48', '\x4c',
          '\x05', '\xca', '\x39', '\xd9', '\x6a', '\x56', '\xa3', '\xa8', '\x6c', '\xab', '\xbe', '\x70', '\xbf',
          '\xbe', '\x39', '\x20', '\x7d', '\x5d', '\x66', '\x92', '\x0c', '\xd6', '\x97', '\x58', '\x02', '\xf6',
          '\xca', '\xa6', '\xf2', '\x1b', '\x6c', '\xff', '\xac', '\xd3', '\x62', '\xed', '\xd3', '\x0e', '\xeb',
          '\x36', '\xd6', '\xff', '\xee', '\xfb', '\x77', '\x25', '\x07', '\x5f', '\xce', '\xae', '\x79', '\x5f',
          '\xb7', '\xb8', '\x16', '\xe1', '\xb3', '\x15', '\xd6', '\x53', '\x80', '\x97', '\xe0', '\xb5', '\x5e',
          '\xd3', '\x59', '\x51', '\xb5', '\xb1', '\x7b', '\xb0', '\x0b', '\x45', '\xd7', '\xfa', '\xc5', '\xd4',
          '\x9c', '\xa8', '\x1f', '\x58', '\x77', '\x89', '\x6d', '\xaf', '\xdc', '\x58', '\x19', '\xfa', '\x97',
          '\xdc', '\x2e', '\xb0', '\xa6', '\xd3', '\x46', '\xe6', '\x15', '\xed', '\x40', '\xa2', '\x50', '\x50',
          '\xca', '\x0c', '\xbd', '\xcc', '\x7b', '\x50', '\x63', '\xc0', '\xfc', '\x40', '\xbd', '\xf1', '\xb2',
          '\xc6', '\xe1', '\xfe', '\x81', '\xec', '\x4e', '\xed', '\x08', '\xac', '\xe5', '\x33', '\xb0', '\x35',
          '\xc7', '\xe1', '\x83', '\xd5', '\x99', '\x43', '\x23', '\x1c', '\x6b', '\x64', '\x74', '\xf5', '\x08',
          '\xec', '\x11', '\xa7', '\x0e', '\x94', '\x12', '\x17', '\xef', '\xff', '\x1c', '\x9b', '\x28', '\x89',
          '\x59', '\xd8', '\x4a', '\x9a', '\xac', '\x1d', '\x97', '\xcc', '\xce', '\x2a', '\xd2', '\x49', '\x4e',
          '\x59', '\x14', '\xb7', '\x2b', '\x10', '\x51', '\x92', '\xb0', '\x56', '\x18', '\xc7', '\x5e', '\x48',
          '\x63', '\xf3', '\xb4', '\xba', '\x75', '\xc9', '\x93', '\xde', '\x3f', '\xfa', '\x05', '\x48', '\x0b',
          '\x16', '\x15' };

    QByteArray retVal( stateData, sizeof( stateData ) );

    return retVal;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QByteArray RiuDockWidgetTools::defaultGeoMechDockState()
{
    static const char stateData[] =
        { '\x00', '\x00', '\x06', '\x13', '\x78', '\xda', '\x95', '\x54', '\xdf', '\x4f', '\xc2', '\x30', '\x10',
          '\x7e', '\xf7', '\xaf', '\x68', '\xf6', '\x8e', '\xb0', '\x8d', '\x29', '\x4b', '\xf8', '\x11', '\x03',
          '\x9a', '\xf8', '\x80', '\xa2', '\x03', '\x79', '\x34', '\xb5', '\x3b', '\xb1', '\xda', '\xb5', '\xa4',
          '\x3d', '\xf0', '\x47', '\xfc', '\xe3', '\xed', '\x86', '\x21', '\x01', '\xba', '\x0d', '\x9e', '\xda',
          '\xde', '\x7d', '\x5f', '\xef', '\xbb', '\xeb', '\x5d', '\xbb', '\x83', '\xaf', '\x4c', '\x90', '\x35',
          '\x68', '\xc3', '\x95', '\xec', '\x79', '\xfe', '\x79', '\xcb', '\x23', '\x20', '\x99', '\x4a', '\xb9',
          '\x5c', '\xf4', '\xbc', '\xd9', '\xf4', '\xa6', '\xd1', '\xf1', '\x06', '\xfd', '\xee', '\x03', '\x5e',
          '\xa5', '\x6b', '\x2a', '\x19', '\xa4', '\x23', '\xc5', '\x3e', '\xac', '\x2f', '\xf9', '\x36', '\x08',
          '\x19', '\x79', '\xda', '\x12', '\x3d', '\x32', '\x33', '\xa0', '\xb7', '\xe7', '\xc0', '\x23', '\x43',
          '\x25', '\x91', '\x72', '\x69', '\x2d', '\x85', '\x7b', '\x08', '\x12', '\x35', '\x15', '\x73', '\x9e',
          '\x2e', '\x00', '\x7b', '\x5e', '\x6a', '\xef', '\x09', '\x47', '\x73', '\x2e', '\x53', '\xf5', '\xf9',
          '\x9c', '\x59', '\xdc', '\x66', '\xeb', '\xf5', '\xbb', '\x5b', '\x1e', '\xb9', '\x11', '\x8a', '\x62',
          '\x21', '\xa4', '\x65', '\xed', '\xc9', '\x52', '\x70', '\x44', '\x6b', '\xbe', '\xd7', '\xdc', '\xde',
          '\x65', '\x3d', '\x79', '\xa0', '\xdf', '\x3c', '\xd0', '\x4a', '\x62', '\x1e', '\xb2', '\x0c', '\xd3',
          '\xd8', '\xc1', '\x5c', '\x69', '\xa0', '\x64', '\x4a', '\x5f', '\xac', '\xac', '\xd0', '\x3a', '\x56',
          '\x5a', '\x83', '\xfc', '\x17', '\x34', '\xd1', '\xea', '\x1d', '\x18', '\x4e', '\x35', '\xc0', '\xae',
          '\xa6', '\x8d', '\x6a', '\x72', '\x47', '\x33', '\xa8', '\x44', '\x92', '\xa1', '\x50', '\x06', '\xd2',
          '\x5c', '\x70', '\xd3', '\xc1', '\x1a', '\x51', '\xa4', '\x89', '\x5a', '\x69', '\x06', '\x27', '\x12',
          '\x13', '\xa6', '\xf9', '\x12', '\x4d', '\x35', '\xab', '\x99', '\x67', '\xb6', '\x93', '\x9f', '\x7f',
          '\x98', '\xdf', '\x12', '\x34', '\x7e', '\x5f', '\xa7', '\x1c', '\x95', '\xae', '\x4f', '\xb1', '\x04',
          '\xec', '\x0c', '\x9b', '\xf0', '\x1f', '\x30', '\xfd', '\xcb', '\x4e', '\x40', '\xda', '\x17', '\x1d',
          '\xd2', '\x6d', '\x6e', '\xce', '\x76', '\xfd', '\x7f', '\x92', '\x93', '\x1f', '\x67', '\x5f', '\xbc',
          '\xab', '\x5b', '\x6c', '\x8b', '\xd0', '\x45', '\x81', '\x75', '\x24', '\xe0', '\x24', '\x38', '\xa5',
          '\xd7', '\x74', '\x56', '\x58', '\x2d', '\xec', '\x11', '\xcc', '\x4a', '\xe0', '\xad', '\x7c', '\x55',
          '\x35', '\x15', '\x75', '\x03', '\xeb', '\x1e', '\xb1', '\xed', '\x0c', '\x37', '\x11', '\x0a', '\x8f',
          '\x0a', '\xb7', '\x0f', '\xac', '\xe9', '\xb4', '\xb1', '\x7a', '\xd3', '\x66', '\xc8', '\x35', '\x13',
          '\x70', '\x22', '\xf3', '\x11', '\xc4', '\x04', '\x74', '\x56', '\xca', '\xf2', '\x9d', '\xac', '\xc9',
          '\x1a', '\xab', '\x19', '\x87', '\x05', '\xd9', '\x9f', '\xda', '\x31', '\x18', '\x43', '\x17', '\x60',
          '\x6a', '\xca', '\xe1', '\x82', '\xd5', '\xa4', '\x64', '\x47', '\x80', '\x59', '\xd6', '\x58', '\xc9',
          '\xea', '\x11', '\x38', '\x20', '\xce', '\x2c', '\x28', '\x41', '\xca', '\x3e', '\x8e', '\x1c', '\x9b',
          '\x30', '\x8e', '\x88', '\x1f', '\xc4', '\x2d', '\xd2', '\x8e', '\x4a', '\x66', '\xa7', '\xb0', '\x74',
          '\xe2', '\x0b', '\x12', '\x46', '\xed', '\x0a', '\x44', '\x18', '\xc7', '\x24', '\xf0', '\xa3', '\xc8',
          '\x09', '\x69', '\x6e', '\xbf', '\x56', '\xbb', '\x2f', '\xf9', '\xd2', '\xfb', '\x67', '\x7f', '\x49',
          '\x4b', '\x16', '\x16' };

    QByteArray retVal( stateData, sizeof( stateData ) );

    return retVal;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QByteArray RiuDockWidgetTools::defaultPlotDockState()
{
    static const char stateData[] =
        { '\x00', '\x00', '\x04', '\xf5', '\x78', '\xda', '\x9d', '\x54', '\x51', '\x6f', '\x82', '\x30', '\x10',
          '\x7e', '\xdf', '\xaf', '\x68', '\xfa', '\xee', '\x04', '\x99', '\x13', '\x13', '\xd4', '\x18', '\x9c',
          '\x6f', '\x6e', '\x2e', '\xe8', '\x7c', '\x34', '\x1d', '\x5c', '\x48', '\x23', '\xb4', '\xa4', '\x2d',
          '\x6e', '\x2e', '\xfb', '\xf1', '\x3b', '\x70', '\x61', '\x9b', '\x01', '\xc5', '\x3d', '\xb5', '\xbd',
          '\xfb', '\xbe', '\xbb', '\xef', '\x72', '\x5f', '\xea', '\x4d', '\xde', '\xd3', '\x84', '\xec', '\x41',
          '\x69', '\x2e', '\xc5', '\x88', '\xda', '\xb7', '\x16', '\x25', '\x20', '\x42', '\x19', '\x71', '\x11',
          '\x8f', '\xe8', '\x7a', '\x35', '\xef', '\xb8', '\x74', '\x32', '\xf6', '\x9e', '\xcd', '\x34', '\xda',
          '\x33', '\x11', '\x42', '\x34', '\x93', '\xe1', '\x0e', '\x73', '\xc1', '\x41', '\x1b', '\x48', '\xc9',
          '\x4b', '\x45', '\xa4', '\x64', '\xad', '\x41', '\x55', '\xef', '\x1e', '\x25', '\xbe', '\x14', '\x86',
          '\x71', '\x81', '\x91', '\x32', '\xed', '\x83', '\x30', '\x8a', '\x25', '\x1b', '\x1e', '\xc5', '\x60',
          '\x46', '\x34', '\xc2', '\x3a', '\xcb', '\x44', '\x9a', '\x0d', '\x17', '\x91', '\x7c', '\xdb', '\xa6',
          '\x88', '\xfc', '\x79', '\xd2', '\xb1', '\x57', '\xb1', '\xc9', '\x3c', '\x91', '\xcc', '\x94', '\x72',
          '\x2c', '\x8c', '\x07', '\x59', '\xc2', '\x8d', '\xc1', '\xf0', '\x93', '\xe2', '\x58', '\x11', '\x33',
          '\x45', '\xbb', '\xcf', '\xa2', '\x5d', '\x2e', '\xb0', '\xae', '\xd3', '\x88', '\xe9', '\x54', '\x98',
          '\x1e', '\x62', '\xa6', '\x0a', '\x18', '\x59', '\xb1', '\x57', '\x5d', '\x50', '\x88', '\x9f', '\x2b',
          '\x05', '\xe2', '\x97', '\x2c', '\xbd', '\x52', '\x00', '\xdb', '\x0c', '\x6f', '\x0b', '\x54', '\x51',
          '\xa9', '\x3a', '\xaa', '\x27', '\x8f', '\x2c', '\x85', '\x0b', '\x58', '\xe2', '\x27', '\x52', '\x43',
          '\x54', '\x88', '\xee', '\xd6', '\xf0', '\x56', '\x90', '\x66', '\x09', '\x33', '\xf0', '\x1f', '\x6e',
          '\x10', '\x2a', '\x9e', '\xb5', '\xe9', '\xda', '\x2d', '\x86', '\xfc', '\x33', '\xaa', '\x7d', '\x3a',
          '\xaa', '\x92', '\x19', '\x28', '\x73', '\x78', '\x88', '\xb8', '\x91', '\xaa', '\xcd', '\xbc', '\x67',
          '\x09', '\xb5', '\xed', '\x03', '\xfe', '\x01', '\x7a', '\xec', '\xda', '\x43', '\x72', '\xd7', '\x77',
          '\x89', '\xd7', '\x3d', '\xbe', '\xf1', '\xfc', '\xde', '\xd2', '\x79', '\x81', '\x8d', '\x16', '\x41',
          '\x5f', '\xb0', '\xb8', '\x64', '\x34', '\xac', '\xa5', '\x81', '\x56', '\x2b', '\xf1', '\x3a', '\xc3',
          '\xf4', '\x4e', '\x44', '\xce', '\x98', '\x61', '\x81', '\xcc', '\x55', '\x08', '\x2d', '\x5d', '\x73',
          '\x9e', '\x70', '\x69', '\xfd', '\x79', '\x9a', '\x32', '\x75', '\x58', '\x96', '\x24', '\xc1', '\x62',
          '\x50', '\x17', '\xd7', '\x7e', '\x2a', '\x78', '\x01', '\x5a', '\x23', '\x51', '\xb7', '\x90', '\xda',
          '\x04', '\xbd', '\x20', '\x72', '\x8d', '\xa0', '\xc0', '\xb0', '\x70', '\x77', '\x8d', '\x45', '\x86',
          '\x16', '\x71', '\xdc', '\x41', '\xad', '\x45', '\x8e', '\x11', '\xc7', '\xe9', '\x13', '\x7b', '\x70',
          '\x6f', '\xa1', '\x93', '\x9c', '\x5a', '\x58', '\xb7', '\xfa', '\x36', '\xf0', '\xde', '\xf0', '\x69',
          '\x8d', '\x6f', '\xbe', '\x00', '\x0a', '\x89', '\xb7', '\xa8' };

    QByteArray retVal( stateData, sizeof( stateData ) );

    return retVal;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QByteArray RiuDockWidgetTools::hideAllDocking3DState()
{
    static const char stateData[] =
        { '\x00', '\x00', '\x06', '\x11', '\x78', '\xda', '\x95', '\x54', '\x4d', '\x53', '\xc2', '\x30', '\x10',
          '\xbd', '\xfb', '\x2b', '\x32', '\xbd', '\x2b', '\x05', '\xd4', '\xf1', '\xc0', '\xc7', '\x30', '\xa0',
          '\x33', '\x1e', '\x50', '\xb4', '\x20', '\x47', '\x27', '\xa6', '\x2b', '\x46', '\xd3', '\x84', '\xd9',
          '\x2c', '\xf8', '\x31', '\xfe', '\x78', '\xb7', '\x05', '\x19', '\x29', '\xa5', '\x95', '\x53', '\xd3',
          '\xdd', '\xf7', '\xf2', '\xde', '\x26', '\xbb', '\x69', '\x75', '\x3f', '\x12', '\x23', '\x96', '\x80',
          '\x5e', '\x3b', '\xdb', '\x0e', '\xea', '\x27', '\x61', '\x20', '\xc0', '\x2a', '\x17', '\x6b', '\x3b',
          '\x6b', '\x07', '\x93', '\xf1', '\xd5', '\xf1', '\x45', '\xd0', '\xed', '\xb4', '\xee', '\xa8', '\x17',
          '\x2f', '\xa5', '\x55', '\x10', '\x0f', '\x9c', '\x7a', '\xe3', '\x5c', '\xf4', '\xe9', '\x09', '\x12',
          '\xf1', '\xb0', '\x21', '\x06', '\x62', '\xe2', '\x01', '\x37', '\xff', '\x8d', '\x40', '\xf4', '\x9d',
          '\x25', '\xa9', '\x2d', '\x47', '\xb2', '\x74', '\x1f', '\x2c', '\xa1', '\x34', '\x53', '\x1d', '\xcf',
          '\x80', '\xda', '\x41', '\xcc', '\xfb', '\x34', '\x07', '\x53', '\x6d', '\x63', '\xf7', '\xfe', '\x98',
          '\x30', '\x6e', '\xb5', '\x0c', '\x3a', '\xad', '\x0d', '\x4f', '\x5c', '\x19', '\x27', '\x29', '\x33',
          '\x12', '\x72', '\x3c', '\x9a', '\x1b', '\x4d', '\xc4', '\xe1', '\x5b', '\xd4', '\xbc', '\x17', '\x67',
          '\x52', '\xa1', '\xef', '\x54', '\x68', '\x61', '\x29', '\x95', '\xdc', '\x87', '\x39', '\xde', '\xc2',
          '\xf4', '\x10', '\xa4', '\x18', '\xcb', '\x27', '\xb6', '\xd5', '\xe4', '\xc4', '\x02', '\x11', '\xec',
          '\xda', '\x50', '\xa4', '\x50', '\xcf', '\xc9', '\x8f', '\x11', '\x60', '\xdb', '\xd3', '\xca', '\xb5',
          '\xb8', '\x91', '\x09', '\xac', '\x90', '\x23', '\x74', '\xaf', '\xa0', '\x28', '\x8f', '\x14', '\x7d',
          '\xe3', '\x3c', '\xc4', '\x69', '\xc1', '\xb5', '\x02', '\xd6', '\x40', '\x92', '\x8c', '\xdc', '\x02',
          '\x15', '\x1c', '\x48', '\xdc', '\x63', '\x6c', '\x9b', '\x55', '\x4b', '\x2b', '\xdb', '\xaa', '\xaf',
          '\x9e', '\xab', '\x8f', '\x5d', '\xcf', '\x01', '\xe9', '\xf3', '\x32', '\xd6', '\xe4', '\xb0', '\xba',
          '\xc4', '\x3d', '\xe0', '\x42', '\xd9', '\x48', '\x7f', '\x81', '\xef', '\x84', '\xa2', '\xde', '\x38',
          '\x3b', '\x17', '\xad', '\xda', '\xea', '\x97', '\xbf', '\xeb', '\x1b', '\x39', '\xf8', '\x6e', '\xf2',
          '\xde', '\x8b', '\x9a', '\x85', '\x3b', '\x44', '\xce', '\x32', '\x6c', '\x81', '\xff', '\x42', '\xc2',
          '\xaf', '\xf3', '\xf0', '\xaf', '\xf3', '\x8a', '\xc6', '\x6a', '\x96', '\x1b', '\xbb', '\x07', '\xbf',
          '\x30', '\x74', '\x6d', '\x9f', '\x5d', '\xc5', '\x81', '\x16', '\x03', '\xab', '\xee', '\xf0', '\x34',
          '\x27', '\x37', '\x74', '\x2f', '\xe8', '\xfb', '\x1a', '\x95', '\x81', '\x91', '\x71', '\xf4', '\x2f',
          '\xcd', '\x3c', '\xb0', '\xa2', '\xdb', '\x4a', '\x24', '\x2a', '\x98', '\xf7', '\x60', '\x46', '\x80',
          '\xc9', '\x81', '\xac', '\xd1', '\xb2', '\xc2', '\xe1', '\xee', '\xa9', '\xe4', '\x27', '\x77', '\xc2',
          '\xac', '\x88', '\xa4', '\x7a', '\xab', '\x38', '\x8f', '\x21', '\x78', '\x2f', '\x67', '\xe0', '\x0f',
          '\x71', '\x87', '\x4e', '\x31', '\x6b', '\xe8', '\x6c', '\xf9', '\x1c', '\xec', '\x10', '\x0b', '\x3d',
          '\x95', '\xce', '\x4e', '\x28', '\x78', '\x78', '\xc2', '\xe2', '\xe9', '\xc9', '\x22', '\xd9', '\x70',
          '\x95', '\x01', '\x4a', '\x76', '\xa8', '\x6d', '\x1e', '\x56', '\x5e', '\xef', '\x79', '\xd0', '\x3b',
          '\x47', '\x3f', '\x6f', '\xaf', '\x16', '\x81' };

    QByteArray retVal( stateData, sizeof( stateData ) );

    return retVal;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QByteArray RiuDockWidgetTools::hideAllDockingPlotState()
{
    static const char stateData[] =
        { '\x00', '\x00', '\x04', '\xe7', '\x78', '\xda', '\x9d', '\x94', '\x51', '\x6f', '\x82', '\x30', '\x14',
          '\x85', '\xdf', '\xf7', '\x2b', '\x9a', '\xbe', '\x3b', '\x10', '\xb3', '\xc5', '\x07', '\xc4', '\x18',
          '\x9c', '\x6f', '\x6e', '\x2e', '\xe0', '\x7c', '\x34', '\x1d', '\xdc', '\x90', '\x46', '\x68', '\x49',
          '\x5b', '\xdc', '\x5c', '\xf6', '\xe3', '\x77', '\x41', '\xc5', '\xe9', '\xc0', '\xa9', '\x4f', '\xd0',
          '\xf6', '\x9c', '\x9e', '\xef', '\x72', '\x6f', '\x70', '\x87', '\x9f', '\x59', '\x4a', '\xd6', '\xa0',
          '\x34', '\x97', '\x62', '\x40', '\xbb', '\xf7', '\x36', '\x25', '\x20', '\x22', '\x19', '\x73', '\x91',
          '\x0c', '\xe8', '\x3c', '\x9c', '\x74', '\xfa', '\x74', '\xe8', '\xb9', '\xaf', '\x66', '\x14', '\xaf',
          '\x99', '\x88', '\x20', '\x1e', '\xcb', '\x68', '\x85', '\x67', '\xc1', '\x46', '\x1b', '\xc8', '\xc8',
          '\x5b', '\x6d', '\xa4', '\x64', '\xae', '\x41', '\xd5', '\x6b', '\x87', '\x12', '\x5f', '\x0a', '\xc3',
          '\xb8', '\xc0', '\x9d', '\xea', '\xd8', '\x07', '\x61', '\x14', '\x4b', '\x17', '\x3c', '\x4e', '\xc0',
          '\x0c', '\x68', '\x8c', '\xf7', '\xcc', '\x52', '\x69', '\x16', '\x5c', '\xc4', '\xf2', '\x63', '\x99',
          '\xa1', '\xf2', '\xb0', '\xa4', '\x9e', '\x5b', '\xbb', '\xc9', '\x24', '\x95', '\xcc', '\x54', '\x38',
          '\x36', '\xee', '\x07', '\x79', '\xca', '\x8d', '\xc1', '\xed', '\x17', '\xc5', '\xf1', '\x46', '\x3c',
          '\x29', '\xe3', '\xbe', '\xcb', '\xb8', '\x42', '\xe0', '\xbd', '\xbd', '\x56', '\x4d', '\xa7', '\xd6',
          '\x38', '\xa8', '\x19', '\x29', '\x60', '\x24', '\x64', '\xef', '\xba', '\xb4', '\x10', '\xbf', '\x50',
          '\x0a', '\xc4', '\x0e', '\x2b', '\x88', '\x14', '\xcf', '\x8d', '\x0e', '\x15', '\xc0', '\x32', '\x47',
          '\xa6', '\x29', '\x72', '\xd4', '\x5c', '\x5b', '\x7e', '\xf2', '\xcc', '\x32', '\x38', '\x14', '\xd1',
          '\xa8', '\x25', '\x7e', '\x2a', '\x35', '\xc4', '\x65', '\xf1', '\x56', '\x83', '\x2f', '\x84', '\x2c',
          '\x4f', '\x99', '\x81', '\x5b', '\xbc', '\x67', '\x08', '\x8f', '\x9d', '\x56', '\x59', '\xe6', '\x51',
          '\xb1', '\xdd', '\x93', '\x62', '\x67', '\x4a', '\xe6', '\xa0', '\xcc', '\xe6', '\x29', '\xe6', '\x46',
          '\xaa', '\x4b', '\xea', '\x3d', '\x6b', '\x68', '\x8c', '\x0f', '\xf8', '\x17', '\x68', '\xcf', '\x26',
          '\x5d', '\xa7', '\xdf', '\x23', '\xae', '\xb5', '\x5d', '\xe2', '\x73', '\xd7', '\xa6', '\xf3', '\x7c',
          '\xad', '\x33', '\x82', '\x83', '\xc1', '\x92', '\xca', '\xd1', '\xd2', '\x95', '\x16', '\xdb', '\x9e',
          '\xd0', '\xfe', '\x4d', '\x78', '\xdd', '\xc4', '\x38', '\xa7', '\x13', '\x53', '\x64', '\x19', '\x53',
          '\x9b', '\x59', '\xf5', '\x2d', '\x04', '\x4b', '\x40', '\x35', '\x31', '\x8d', '\x99', '\x61', '\x81',
          '\x2c', '\x54', '\x04', '\xb7', '\xb4', '\xfc', '\x6f', '\xc4', '\x7f', '\xad', '\x3e', '\xa5', '\x9c',
          '\x63', '\x4e', '\x60', '\x58', '\xb4', '\xba', '\xa0', '\xcb', '\x53', '\xd0', '\x1a', '\x43', '\xf4',
          '\x95', '\x94', '\xad', '\x11', '\xb7', '\xcd', '\xc5', '\x5e', '\xe0', '\x3c', '\x3c', '\xda', '\xc4',
          '\x6e', '\x94', '\x58', '\xf5', '\x8f', '\x02', '\xdf', '\x5b', '\x7e', '\x53', '\xde', '\xdd', '\x0f',
          '\x36', '\x41', '\xb3', '\x2a' };

    QByteArray retVal( stateData, sizeof( stateData ) );

    return retVal;
}
