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

#include "RiuMainWindow.h"
#include "RiuPlotMainWindow.h"

#include "DockManager.h"
#include "DockWidget.h"

#include "cafAssert.h"
#include "cafPdmUiTreeView.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ads::CDockWidget* RiuDockWidgetTools::createDockWidget( QString title, QString dockName, QWidget* parent )
{
    ads::CDockWidget* dockWidget = new ads::CDockWidget( title, parent );
    dockWidget->setObjectName( dockName );
    dockWidget->setToggleViewActionMode( ads::CDockWidget::ActionModeToggle );
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
QString RiuDockWidgetTools::mainWindowQuickAccessName()
{
    return "dockQuickAccess_mainWindow";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuDockWidgetTools::mainWindowCellSelectionToolName()
{
    return "dockCellSelectionTool_mainWindow";
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
QString RiuDockWidgetTools::plotWindowQuickAccessName()
{
    return "dockQuickAccess_plotWindow";
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
QString RiuDockWidgetTools::plotMainWindowCloudTreeName()
{
    return "dockCloudTree_plotMainWindow";
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
QString RiuDockWidgetTools::plotMainWindowPropertyEditorRightName()
{
    return "dockPropertyEditorRight_plotMainWindow";
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
    else if ( dockWidgetName == plotMainWindowPropertyEditorRightName() )
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
        return QIcon( ":/svg/Calculator.svg" );
    else if ( dockWidgetName == mainWindowScriptsTreeName() )
        return QIcon( ":/scripts.svg" );
    else if ( dockWidgetName == mainPlotWindowName() )
        return QIcon( ":/window-management.svg" );
    else if ( dockWidgetName == main3DWindowName() )
        return QIcon( ":/window-management.svg" );
    else if ( dockWidgetName == mainWindowSeismicHistogramName() )
        return QIcon( ":/graph.svg" );
    else if ( dockWidgetName == plotMainWindowCloudTreeName() )
        return QIcon( ":/SummaryEnsemble.svg" );
    else if ( dockWidgetName == plotWindowQuickAccessName() || dockWidgetName == mainWindowQuickAccessName() )
        return QIcon( ":/pinned.svg" );

    return QIcon( ":/view.svg" );
}

//--------------------------------------------------------------------------------------------------
// When a user clicks on an item in a tree view, or right-clicks for the context menu, the global selection is set to the activated object.
// This function can be used to get the selection from a different tree view
//--------------------------------------------------------------------------------------------------
std::vector<caf::PdmUiItem*> RiuDockWidgetTools::selectedItemsInTreeView( const QString& dockWidgetName )
{
    ads::CDockWidget* dockWidget = nullptr;
    if ( auto mainWindow = RiuMainWindow::instance() )
    {
        dockWidget = RiuDockWidgetTools::findDockWidget( mainWindow->dockManager(), dockWidgetName );
    }

    if ( !dockWidget )
    {
        if ( auto plotWindow = RiuPlotMainWindow::instance() )
        {
            dockWidget = RiuDockWidgetTools::findDockWidget( plotWindow->dockManager(), dockWidgetName );
        }
    }

    if ( dockWidget )
    {
        if ( auto tree = dynamic_cast<caf::PdmUiTreeView*>( dockWidget->widget() ) )
        {
            std::vector<caf::PdmUiItem*> uiItems;
            tree->selectedUiItems( uiItems );

            return uiItems;
        }
    }

    return {};
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
        { '\x00', '\x00', '\x06', '\xec', '\x78', '\xda', '\x95', '\x55', '\x5d', '\x4f', '\xc2', '\x30', '\x14', '\x7d', '\xf7', '\x57',
          '\x34', '\x7b', '\x57', '\x98', '\x43', '\x85', '\x04', '\x30', '\x04', '\x34', '\xfa', '\x80', '\x02', '\x43', '\x7d', '\x34',
          '\xb5', '\xbb', '\x62', '\xb5', '\x6b', '\x49', '\xdb', '\xe1', '\x47', '\xfc', '\xf1', '\xde', '\x82', '\x59', '\x14', '\x3b',
          '\x66', '\x9f', '\xd6', '\xf5', '\x9e', '\xd3', '\x9e', '\xd3', '\x73', '\x9b', '\x76', '\x4f', '\xdf', '\x72', '\x41', '\x56',
          '\xa0', '\x0d', '\x57', '\xb2', '\x17', '\xc5', '\x07', '\xcd', '\x88', '\x80', '\x64', '\x2a', '\xe3', '\x72', '\xd1', '\x8b',
          '\x6e', '\xe6', '\xe7', '\xfb', '\xed', '\xe8', '\xb4', '\xdf', '\x9d', '\xda', '\x41', '\xb6', '\xa2', '\x92', '\x41', '\x36',
          '\x52', '\xec', '\x05', '\x6b', '\xe9', '\xbb', '\xb1', '\x90', '\x93', '\xdb', '\x92', '\x18', '\x91', '\x1b', '\x03', '\xba',
          '\xfc', '\x4f', '\x22', '\x32', '\x54', '\xd2', '\x52', '\x2e', '\x71', '\x66', '\x5d', '\x1e', '\x82', '\xb4', '\x9a', '\x8a',
          '\x3b', '\x9e', '\x2d', '\xc0', '\xf6', '\xa2', '\x0c', '\xd7', '\x49', '\x46', '\x77', '\x5c', '\x66', '\xea', '\xf5', '\x3e',
          '\x47', '\xdc', '\x66', '\x18', '\xf5', '\xbb', '\x25', '\x8f', '\x9c', '\x0b', '\x45', '\xed', '\x5a', '\x48', '\x13', '\xe7',
          '\xd3', '\xa5', '\xe0', '\xd6', '\xe2', '\xf4', '\xb5', '\xe6', '\xb8', '\x16', '\x56', '\xdc', '\x46', '\x9f', '\x6e', '\xa3',
          '\x42', '\xe2', '\x8a', '\x87', '\x95', '\x98', '\xfd', '\x5f', '\x98', '\x81', '\x06', '\x4a', '\xe6', '\xf4', '\xc1', '\x6c',
          '\x54', '\x16', '\x5a', '\x83', '\x2b', '\x8d', '\x4b', '\x0d', '\x07', '\x67', '\x4c', '\xcc', '\x0a', '\x89', '\x0a', '\x10',
          '\xbd', '\xd1', '\x4b', '\xae', '\x68', '\x0e', '\x15', '\x18', '\x32', '\x14', '\xca', '\x40', '\xe6', '\x44', '\x36', '\xb6',
          '\xf0', '\xce', '\xe4', '\xb4', '\xe0', '\xec', '\x65', '\xc0', '\x18', '\x18', '\xf3', '\xd3', '\x67', '\x0d', '\x6b', '\x08',
          '\x42', '\xa4', '\x20', '\x80', '\x39', '\x07', '\x73', '\xa5', '\x44', '\x35', '\xb7', '\xe1', '\x0c', '\x55', '\xda', '\x72',
          '\x8b', '\xcd', '\xc0', '\x14', '\xc2', '\x5e', '\xca', '\x47', '\xf5', '\xfb', '\xa4', '\xff', '\xec', '\x3a', '\xd1', '\xca',
          '\xe9', '\x1c', '\x2b', '\xc9', '\xad', '\xd2', '\x01', '\x72', '\xc7', '\xc8', '\xa2', '\x0b', '\x08', '\x71', '\xe8', '\x17',
          '\xe5', '\xb5', '\x96', '\xf2', '\x0f', '\x30', '\xfd', '\x56', '\xa7', '\x43', '\x92', '\x56', '\x4c', '\xba', '\x8d', '\xcd',
          '\x3f', '\x7e', '\xbf', '\xd3', '\xfe', '\x5f', '\xee', '\xff', '\xe9', '\x9f', '\x1f', '\x87', '\x18', '\x6f', '\x1d', '\xa2',
          '\xaf', '\x59', '\xb1', '\x43', '\xe9', '\x62', '\x8d', '\xf5', '\x18', '\xf4', '\x12', '\x02', '\x93', '\xc3', '\x40', '\x9e',
          '\xb1', '\x07', '\xe6', '\x1a', '\xa0', '\x26', '\xba', '\x11', '\xb5', '\x34', '\x55', '\x85', '\x66', '\xb0', '\x0d', '\xae',
          '\xc9', '\x21', '\x65', '\x9a', '\x2f', '\xad', '\x09', '\x64', '\x55', '\x08', '\xdb', '\x11', '\x5f', '\xbb', '\x13', '\x93',
          '\xc3', '\x76', '\x12', '\x10', '\x5f', '\x65', '\x34', '\x47', '\xde', '\xfe', '\x9e', '\x08', '\x65', '\x6b', '\x0e', '\xc9',
          '\x0f', '\xac', '\xb1', '\x3a', '\x82', '\xa5', '\x7d', '\x0a', '\xe4', '\xcc', '\x40', '\x4c', '\x40', '\xe7', '\x81', '\xac',
          '\xc9', '\x2a', '\x54', '\x5b', '\x0a', '\xdc', '\x5c', '\x70', '\x63', '\x43', '\x7a', '\x2c', '\xfe', '\xdb', '\x63', '\x4b',
          '\xd0', '\xf6', '\xfd', '\x2c', '\xdb', '\xbe', '\xf4', '\xfe', '\xdc', '\x2b', '\xc0', '\x3b', '\xa3', '\x6f', '\x62', '\xf4',
          '\x2d', '\x7f', '\xf4', '\xdf', '\x77', '\xfb', '\x04', '\xef', '\x76', '\xb2', '\x0b', '\x11', '\x1f', '\x93', '\x38', '\x3e',
          '\xe9', '\x78', '\x21', '\x8d', '\xf2', '\xcd', '\xc0', '\x71', '\xc5', '\x5b', '\xd5', '\xdf', '\xfb', '\x02', '\xeb', '\x9f',
          '\x5f', '\xdf' };


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
        { '\x00', '\x00', '\x05', '\x83', '\x78', '\xda', '\xa5', '\x54', '\x5d', '\x6f', '\x82', '\x30', '\x14', '\x7d', '\xdf', '\xaf',
          '\x68', '\xfa', '\xee', '\x00', '\x75', '\x8e', '\x25', '\xa8', '\x31', '\x3a', '\xdf', '\xdc', '\x17', '\x38', '\x1f', '\x4d',
          '\x47', '\x6f', '\x58', '\x33', '\x68', '\x49', '\x5b', '\xdc', '\x5c', '\xf6', '\xe3', '\x57', '\x60', '\x61', '\x1f', '\x01',
          '\x44', '\xf7', '\x40', '\x68', '\xef', '\x3d', '\xa7', '\xe7', '\xdc', '\xf4', '\xa4', '\xde', '\xf4', '\x2d', '\x89', '\xd1',
          '\x0e', '\xa4', '\x62', '\x82', '\x8f', '\xb1', '\x73', '\x6e', '\x63', '\x04', '\x3c', '\x14', '\x94', '\xf1', '\x68', '\x8c',
          '\xd7', '\xc1', '\xb2', '\xe7', '\xe2', '\xe9', '\xc4', '\xbb', '\xd7', '\x33', '\xba', '\x23', '\x3c', '\x04', '\xba', '\x10',
          '\xe1', '\x8b', '\xe9', '\xf9', '\x7b', '\xa5', '\x21', '\x41', '\x8f', '\x15', '\x11', '\xa3', '\xb5', '\x02', '\x59', '\xed',
          '\x07', '\x18', '\xcd', '\x05', '\xd7', '\x84', '\x71', '\x53', '\x29', '\xda', '\x73', '\xe0', '\x5a', '\x92', '\x78', '\xc3',
          '\x68', '\x04', '\x7a', '\x8c', '\xa9', '\x39', '\xe7', '\x2e', '\x16', '\x7a', '\xc3', '\x38', '\x15', '\xaf', '\xdb', '\xc4',
          '\x20', '\xbf', '\xb7', '\x78', '\xe2', '\x55', '\x6c', '\xb4', '\x8c', '\x05', '\xd1', '\x85', '\x1d', '\xdb', '\xd4', '\xfd',
          '\x34', '\x66', '\x5a', '\x9b', '\xf2', '\xad', '\x64', '\xe6', '\x44', '\xd3', '\xc9', '\xe5', '\x3e', '\x72', '\xb9', '\x8c',
          '\xeb', '\x5c', '\xb8', '\x09', '\xd3', '\xab', '\x30', '\x7d', '\x83', '\x99', '\x49', '\x20', '\x28', '\x20', '\x4f', '\xaa',
          '\xf4', '\x9a', '\x49', '\x09', '\xfc', '\x87', '\x2d', '\x15', '\x48', '\x80', '\x6d', '\x6a', '\x56', '\x2b', '\xe3', '\xa2',
          '\x72', '\x55', '\xba', '\x47', '\x37', '\x24', '\x81', '\x03', '\x58', '\x34', '\x8f', '\x85', '\x02', '\x9a', '\x9b', '\xb6',
          '\x6a', '\x78', '\x01', '\x24', '\x69', '\x4c', '\x34', '\x9c', '\xc2', '\xf5', '\x43', '\xc9', '\xd2', '\x2e', '\xaa', '\x56',
          '\x3e', '\xe4', '\xaf', '\x51', '\x9d', '\xbf', '\xa3', '\x4a', '\x91', '\x82', '\xd4', '\xfb', '\x6b', '\xca', '\xb4', '\x90',
          '\x5d', '\xe6', '\x6d', '\x25', '\xd4', '\xca', '\xfb', '\xec', '\x1d', '\xd4', '\xc4', '\xb1', '\xed', '\x0b', '\x34', '\x18',
          '\x5d', '\x22', '\xcf', '\x2a', '\x0b', '\xe6', '\xff', '\x75', '\x4d', '\xed', '\x0e', '\x1b', '\x33', '\x62', '\x82', '\x41',
          '\xa2', '\x82', '\xd1', '\x70', '\x2f', '\x0d', '\xb4', '\x5a', '\x8f', '\x07', '\x12', '\x33', '\x68', '\x4f', '\xcc', '\x82',
          '\x68', '\xe2', '\x8b', '\x4c', '\x86', '\xd0', '\x31', '\x36', '\xed', '\x84', '\x03', '\xf7', '\x6f', '\xba', '\x19', '\x3d',
          '\x25', '\x37', '\x59', '\x92', '\x10', '\xb9', '\xbf', '\x2b', '\x48', '\x9c', '\x44', '\x20', '\xff', '\x99', '\x97', '\x07',
          '\x16', '\x3d', '\xeb', '\xa3', '\x43', '\x53', '\xcb', '\x3a', '\xd6', '\xc8', '\x0a', '\x94', '\x32', '\x13', '\xa8', '\x0e',
          '\xea', '\x4d', '\xd0', '\x96', '\xb0', '\xba', '\xb6', '\x8d', '\xfa', '\x6e', '\xfe', '\xb9', '\xb5', '\x79', '\x2d', '\x2b',
          '\x43', '\xdb', '\x41', '\xce', '\x68', '\x74', '\x85', '\x86', '\x6e', '\xbf', '\x16', '\x66', '\x55', '\x8f', '\x98', '\x59',
          '\x37', '\x3c', '\xa1', '\x93', '\xb3', '\x4f', '\x33', '\x96', '\xe9', '\xd8' };

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
