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

#include "RiaGuiApplication.h"

#include "Rim3dView.h"
#include "RimViewWindow.h"

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
QString RiuDockWidgetTools::welcomeScreenName()
{
    return "dockWelcomeScreen";
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
QString RiuDockWidgetTools::viewWindowPrefix()
{
    return "DockViewWindow_";
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
void RiuDockWidgetTools::setDockLayout( RiuMainWindowBase* mainWindow, const QString& layoutName )
{
    if ( mainWindow == nullptr ) return;

    QString activeViewerName;
    if ( auto activeViewer = RiaGuiApplication::instance()->activeReservoirView() )
    {
        activeViewerName = activeViewer->dockWindowName();
    }

    if ( auto dm = mainWindow->dockManager() )
    {
        QByteArray state = RiuDockWidgetTools::defaultDockState( layoutName );
        if ( dm->restoreState( state, RiuMainWindowBase::DOCKSTATE_VERSION ) )
        {
            for ( auto view : mainWindow->viewWindows() )
            {
                if ( view->showWindow() && view->dockWidget() )
                {
                    dm->addDockWidget( ads::DockWidgetArea::CenterDockWidgetArea, view->dockWidget(), dm->centralWidget()->dockAreaWidget() );
                }
            }
        }

        if ( auto dw = dm->findDockWidget( activeViewerName ) )
        {
            dw->setAsCurrentTab();
        }
    }
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
        return QIcon( ":/data-sources.svg" );
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
void RiuDockWidgetTools::selectItemsInTreeView( const QString& dockWidgetName, const std::vector<const caf::PdmUiItem*>& items )
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
            tree->selectItems( items );
        }
    }
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
        { '\x00', '\x00', '\x07', '\x27', '\x78', '\xda', '\x95', '\x55', '\xc1', '\x72', '\xda', '\x30', '\x10', '\xbd', '\xf7', '\x2b',
          '\x34', '\xbe', '\xa7', '\x60', '\x3b', '\xc5', '\xce', '\x0c', '\x90', '\x61', '\x9c', '\x66', '\xda', '\x03', '\x2d', '\x89',
          '\x49', '\x38', '\x76', '\x54', '\x79', '\x4b', '\xd4', '\xc8', '\x5a', '\x46', '\x92', '\x69', '\xd3', '\xe9', '\xc7', '\x77',
          '\x0d', '\x19', '\x3a', '\x10', '\xd9', '\xc6', '\x27', '\x5b', '\xd2', '\x7b', '\xd2', '\xdb', '\xa7', '\xdd', '\xd5', '\xf8',
          '\xfa', '\x77', '\xa9', '\xd8', '\x16', '\x8c', '\x95', '\xa8', '\x27', '\x41', '\xf8', '\x7e', '\x18', '\x30', '\xd0', '\x02',
          '\x0b', '\xa9', '\xd7', '\x93', '\xe0', '\x61', '\x79', '\x7b', '\x91', '\x06', '\xd7', '\xd3', '\xf1', '\x9d', '\x9b', '\x15',
          '\x5b', '\xae', '\x05', '\x14', '\x37', '\x28', '\x9e', '\x69', '\x2d', '\x7f', '\xb1', '\x0e', '\x4a', '\xf6', '\x78', '\x20',
          '\x06', '\xec', '\xc1', '\x82', '\x39', '\x8c', '\x2f', '\x03', '\x96', '\xa1', '\x76', '\x5c', '\x6a', '\x9a', '\xd9', '\x2d',
          '\x67', '\xa0', '\x9d', '\xe1', '\x6a', '\x25', '\x8b', '\x35', '\xb8', '\x49', '\x50', '\xd0', '\x3e', '\x2b', '\x50', '\x02',
          '\x4b', '\xc8', '\x85', '\x01', '\xd0', '\xc1', '\x74', '\x7c', '\x60', '\xb0', '\x5b', '\x85', '\xdc', '\xed', '\x24', '\x0c',
          '\x69', '\x3e', '\xdf', '\x28', '\xe9', '\x1c', '\x4d', '\x7f', '\x35', '\x92', '\x76', '\xa1', '\x95', '\xfa', '\x88', '\xbf',
          '\xf5', '\x11', '\x95', '\xa6', '\xbd', '\xa2', '\x46', '\xcc', '\xc5', '\x11', '\x66', '\x66', '\x80', '\xb3', '\x25', '\xff',
          '\x4e', '\x82', '\x62', '\x5a', '\xa8', '\x8c', '\x01', '\xfd', '\x2a', '\x65', '\x61', '\xf0', '\x27', '\x08', '\xb7', '\x24',
          '\x25', '\xdf', '\x4a', '\xd2', '\xb0', '\x92', '\xba', '\xc0', '\x5f', '\xc4', '\xd9', '\xeb', '\x65', '\x5f', '\x78', '\x09',
          '\xad', '\x48', '\x96', '\x29', '\xb4', '\x50', '\xd4', '\x82', '\x07', '\x1e', '\xd6', '\x0d', '\x77', '\x3c', '\xc7', '\xca',
          '\x08', '\xe8', '\x49', '\x24', '\x73', '\xe4', '\xc6', '\xd9', '\x76', '\xd6', '\xa0', '\x8e', '\xec', '\x28', '\xbe', '\xf0',
          '\x6d', '\x7c', '\x1b', '\x30', '\xee', '\xe5', '\x63', '\x21', '\x1d', '\x9a', '\xee', '\x10', '\x1b', '\xc0', '\xde', '\x63',
          '\x73', '\xf9', '\x07', '\xec', '\x74', '\x34', '\x4c', '\x59', '\x9c', '\x44', '\x6c', '\x3c', '\xd8', '\x8f', '\xe9', '\xfb',
          '\x7a', '\x25', '\xe7', '\x5d', '\xce', '\x39', '\x97', '\xdc', '\x12', '\xe0', '\x71', '\x2e', '\x51', '\x02', '\xf1', '\xf5',
          '\x0e', '\xe5', '\x09', '\xef', '\x04', '\xda', '\xd3', '\xc9', '\xbb', '\x4a', '\x8a', '\xe7', '\x99', '\x10', '\x60', '\x6d',
          '\x87', '\x8d', '\x0d', '\xc8', '\x16', '\x0f', '\xc3', '\x28', '\x49', '\x58', '\x9c', '\x5e', '\xf5', '\x30', '\xf1', '\xbf',
          '\x41', '\xf1', '\xb1', '\x41', '\xd1', '\x89', '\xee', '\x7b', '\xb0', '\x95', '\x72', '\x9f', '\xf5', '\x0f', '\xec', '\x90',
          '\xed', '\x07', '\x76', '\xa4', '\x69', '\x06', '\x4a', '\xe5', '\xa0', '\xa8', '\x30', '\x48', '\xd5', '\x12', '\x51', '\x79',
          '\xb9', '\xa1', '\xdf', '\xe2', '\x91', '\x57', '\xea', '\x42', '\xa1', '\x3b', '\x4b', '\xea', '\x29', '\xb0', '\xab', '\x14',
          '\x61', '\xe3', '\x9e', '\x7a', '\x72', '\xee', '\x41', '\x2d', '\xc0', '\x94', '\x3d', '\x59', '\x8b', '\x6d', '\x5f', '\x6d',
          '\x39', '\x48', '\xfb', '\x49', '\xda', '\x3e', '\x94', '\x39', '\x3e', '\x19', '\x9b', '\x49', '\x23', '\x14', '\x34', '\x1e',
          '\xd6', '\xe0', '\xfb', '\x69', '\x8a', '\xcc', '\x29', '\x53', '\xf9', '\x1a', '\xba', '\xf2', '\xda', '\x07', '\xeb', '\x72',
          '\xc2', '\x60', '\x5d', '\x06', '\x73', '\xd4', '\x3d', '\x3a', '\x4a', '\x1c', '\xa5', '\xec', '\xea', '\xf2', '\x03', '\x15',
          '\x44', '\xea', '\x2f', '\x88', '\x7d', '\xdf', '\x49', '\xa8', '\xef', '\x0c', '\xa3', '\x16', '\x44', '\xdd', '\x95', '\xc2',
          '\x51', '\x12', '\x7a', '\x21', '\x83', '\xc3', '\xa3', '\x43', '\xff', '\x0d', '\xcf', '\xdc', '\xf4', '\xdd', '\x3f', '\x7f',
          '\xa4', '\x74', '\x6f' };

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
        { '\x00', '\x00', '\x07', '\x25', '\x78', '\xda', '\x95', '\x55', '\x4d', '\x6f', '\xdb', '\x30', '\x0c', '\xbd', '\xef', '\x57',
          '\x08', '\xbe', '\x77', '\xf1', '\x47', '\x97', '\x78', '\x40', '\x92', '\x22', '\x70', '\x57', '\x6c', '\x87', '\x6c', '\x69',
          '\x9d', '\x2e', '\xc7', '\x41', '\x93', '\xb9', '\x54', '\xab', '\x2c', '\x05', '\x12', '\x9d', '\xad', '\xc5', '\x7e', '\xfc',
          '\xe8', '\xa4', '\xf0', '\x96', '\x54', '\x8e', '\xe3', '\x93', '\x2d', '\xf1', '\x3d', '\xf1', '\x91', '\x22', '\xa9', '\xf1',
          '\xd5', '\xef', '\x52', '\xb1', '\x2d', '\x58', '\x27', '\x8d', '\x9e', '\x04', '\xd1', '\xdb', '\x30', '\x60', '\xa0', '\x85',
          '\x29', '\xa4', '\x5e', '\x4f', '\x82', '\xfb', '\xe5', '\xcd', '\x45', '\x1a', '\x5c', '\x4d', '\xc7', '\xb7', '\x38', '\x2b',
          '\xb6', '\x5c', '\x0b', '\x28', '\xae', '\x8d', '\x78', '\x24', '\x5b', '\xfe', '\xe4', '\x10', '\x4a', '\xf6', '\xb5', '\x21',
          '\x06', '\xec', '\xde', '\x81', '\x6d', '\xd6', '\x97', '\x01', '\xcb', '\x8c', '\x46', '\x2e', '\x35', '\xed', '\xec', '\xcc',
          '\x19', '\x68', '\xb4', '\x5c', '\xad', '\x64', '\xb1', '\x06', '\x9c', '\x04', '\x05', '\x9d', '\xb3', '\x02', '\x25', '\x4c',
          '\x09', '\xb9', '\xb0', '\x00', '\x3a', '\x98', '\x8e', '\x1b', '\x06', '\xbb', '\x51', '\x86', '\xe3', '\x4e', '\x42', '\x48',
          '\xfb', '\xf9', '\x46', '\x49', '\x44', '\xda', '\xfe', '\x62', '\x25', '\x9d', '\x42', '\x96', '\xda', '\xc5', '\x9f', '\xda',
          '\x45', '\xa5', '\xe9', '\xac', '\xb8', '\x15', '\x73', '\x71', '\x80', '\x99', '\x59', '\xe0', '\x6c', '\xc9', '\xbf', '\x93',
          '\xa0', '\x84', '\x0c', '\x95', '\xb5', '\xa0', '\x5f', '\xa4', '\x2c', '\xac', '\xf9', '\x09', '\x02', '\x97', '\xa4', '\xe4',
          '\x5b', '\x49', '\x1a', '\x56', '\x52', '\x17', '\xe6', '\x17', '\x71', '\xf6', '\x7a', '\xd9', '\x67', '\x5e', '\xc2', '\x49',
          '\x24', '\xcb', '\x94', '\x71', '\x50', '\xd4', '\x82', '\x07', '\x1e', '\xd6', '\x35', '\x47', '\x9e', '\x9b', '\xca', '\x0a',
          '\xe8', '\x49', '\xa4', '\xe4', '\xc8', '\x0d', '\xba', '\xd3', '\xac', '\x41', '\x1d', '\xd9', '\x41', '\x7c', '\xd1', '\xeb',
          '\xf8', '\x36', '\x60', '\xf1', '\xe9', '\x43', '\x21', '\xd1', '\xd8', '\xee', '\x10', '\x5b', '\xc0', '\x5e', '\xb7', '\xb9',
          '\x7c', '\x06', '\x37', '\x1d', '\x86', '\x29', '\x4b', '\x46', '\x31', '\x1b', '\x0f', '\xf6', '\x6b', '\xfa', '\xbe', '\x5c',
          '\xc9', '\x79', '\x97', '\x73', '\xce', '\x25', '\x9f', '\x08', '\xf0', '\xb0', '\x96', '\xa8', '\x80', '\xf8', '\x7a', '\x87',
          '\xf2', '\x84', '\x77', '\x04', '\xed', '\x99', '\xc9', '\xdb', '\x4a', '\x8a', '\xc7', '\x99', '\x10', '\xe0', '\x5c', '\x47',
          '\x1a', '\x5b', '\x90', '\x8d', '\xc3', '\xe8', '\x55', '\x0e', '\xa3', '\xe1', '\x28', '\x62', '\x61', '\x8f', '\x14', '\xfe',
          '\x4b', '\x4f', '\x72', '\x98', '\x9e', '\xf8', '\x48', '\xf5', '\x1d', '\xb8', '\x4a', '\xe1', '\x27', '\xfd', '\xc3', '\x74',
          '\x88', '\xf6', '\x03', '\x3b', '\x8a', '\x34', '\x03', '\xa5', '\x72', '\x50', '\xd4', '\x16', '\xa4', '\x6a', '\x69', '\x8c',
          '\xea', '\x8c', '\xf7', '\x3f', '\xa9', '\x43', '\xaf', '\xd4', '\x85', '\x32', '\x78', '\x96', '\xd4', '\x63', '\x60', '\x57',
          '\x23', '\xc2', '\x06', '\x1f', '\x5a', '\x39', '\x91', '\x97', '\x73', '\x07', '\x6a', '\x01', '\xb6', '\xec', '\xc9', '\x5a',
          '\x6c', '\xb1', '\x27', '\x63', '\x6e', '\x1e', '\xac', '\xcb', '\xa4', '\x15', '\x0a', '\x7a', '\x46', '\x95', '\x83', '\x74',
          '\x1f', '\xa5', '\xc3', '\x3e', '\x23', '\xe2', '\xb8', '\x44', '\xe6', '\x54', '\xa7', '\x7c', '\x0d', '\x5d', '\x55', '\xed',
          '\x83', '\x75', '\xa8', '\xa3', '\x79', '\x52', '\x37', '\xc1', '\xdc', '\xe8', '\x1e', '\xf3', '\x24', '\x89', '\x53', '\xf6',
          '\xfe', '\xf2', '\x1d', '\x4b', '\xd2', '\xd4', '\xdf', '\x10', '\xfb', '\xa9', '\x33', '\xa2', '\xa9', '\x13', '\xc6', '\x27',
          '\x10', '\xf5', '\x4c', '\xda', '\xf5', '\x95', '\x0f', '\x32', '\x68', '\x9e', '\x1c', '\xfa', '\x6f', '\x79', '\xe4', '\xa6',
          '\x6f', '\xfe', '\x02', '\x82', '\xdf', '\x73', '\xfc' };

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
        { '\x00', '\x00', '\x05', '\x5e', '\x78', '\xda', '\xa5', '\x54', '\x51', '\x4f', '\xc2', '\x30', '\x10', '\x7e', '\xf7', '\x57',
          '\x34', '\x7b', '\x47', '\xb6', '\x81', '\x80', '\xc9', '\x18', '\x21', '\x20', '\x6f', '\x28', '\x3a', '\x90', '\x47', '\x53',
          '\xd7', '\xcb', '\x6c', '\xdc', '\xda', '\xa5', '\xed', '\x50', '\x8c', '\x3f', '\xde', '\xdb', '\x20', '\x4b', '\x20', '\x63',
          '\x0c', '\x7c', '\x6a', '\x7b', '\xf7', '\x7d', '\x77', '\xdf', '\xb5', '\x5f', '\xea', '\x8d', '\xbe', '\x93', '\x98', '\x6c',
          '\x40', '\x69', '\x2e', '\xc5', '\xd0', '\x72', '\x6e', '\x6d', '\x8b', '\x80', '\x08', '\x25', '\xe3', '\x22', '\x1a', '\x5a',
          '\xab', '\xe5', '\xac', '\x35', '\xb0', '\x46', '\xbe', '\xf7', '\x6c', '\xc6', '\x6c', '\x43', '\x45', '\x08', '\x6c', '\x2a',
          '\xc3', '\x4f', '\xcc', '\x05', '\x5b', '\x6d', '\x20', '\x21', '\xaf', '\x25', '\xd1', '\x22', '\x2b', '\x0d', '\xaa', '\x3c',
          '\x77', '\x2d', '\x32', '\x91', '\xc2', '\x50', '\x2e', '\x30', '\x52', '\xa4', '\x27', '\x20', '\x8c', '\xa2', '\xf1', '\x9a',
          '\xb3', '\x08', '\xcc', '\xd0', '\x62', '\x58', '\x67', '\x0d', '\x71', '\x28', '\x13', '\x08', '\x42', '\x05', '\x20', '\x2c',
          '\xdf', '\x2b', '\x19', '\x64', '\x16', '\x4b', '\x6a', '\x0a', '\x09', '\x36', '\xc6', '\x83', '\x34', '\xe6', '\xc6', '\x60',
          '\xf8', '\x49', '\x71', '\xac', '\x82', '\x99', '\xbc', '\xc5', '\x6f', '\xde', '\x22', '\x13', '\x58', '\xab', '\x73', '\x12',
          '\xd3', '\x2a', '\x31', '\x2e', '\x62', '\xc6', '\x0a', '\x28', '\x59', '\xd2', '\x77', '\x9d', '\x53', '\xc8', '\x24', '\x53',
          '\x0a', '\xc4', '\x5e', '\xca', '\x22', '\x96', '\x46', '\x2f', '\x51', '\xc7', '\x5b', '\x8a', '\xbb', '\x39', '\xaa', '\x58',
          '\x73', '\xc1', '\xe4', '\x17', '\xb2', '\x76', '\x8a', '\xc9', '\x23', '\x4d', '\xe0', '\x0c', '\x96', '\x4c', '\x62', '\xa9',
          '\x81', '\xe5', '\xa2', '\xdb', '\x15', '\xbc', '\x25', '\x24', '\x69', '\x4c', '\x0d', '\x5c', '\xc3', '\xc5', '\x3b', '\xe2',
          '\x69', '\x93', '\xae', '\xed', '\x7c', '\xc8', '\x83', '\x51', '\x9d', '\xe3', '\x51', '\x95', '\x4c', '\x41', '\x99', '\xed',
          '\x03', '\xe3', '\x46', '\xaa', '\x26', '\xf3', '\xd6', '\x12', '\x2a', '\xdb', '\x07', '\xfc', '\x07', '\xb4', '\xdf', '\xbb',
          '\xef', '\x93', '\x8e', '\x63', '\x13', '\xaf', '\xbd', '\x3b', '\xe3', '\xba', '\x7f', '\xa5', '\x5a', '\x81', '\x87', '\xb6',
          '\x40', '\x2f', '\xd0', '\xa8', '\x40', '\x55', '\x48', '\x3b', '\x82', '\x56', '\x4a', '\x39', '\x63', '\x8c', '\x4e', '\xbd',
          '\x31', '\xa6', '\xd4', '\xd0', '\x40', '\x66', '\x2a', '\x84', '\x86', '\xee', '\xa8', '\x27', '\x9c', '\x79', '\x66', '\xcc',
          '\x66', '\xec', '\x1a', '\x7b', '\x64', '\x49', '\x42', '\xd5', '\x76', '\x51', '\x90', '\x04', '\x8d', '\x40', '\xfd', '\xd3',
          '\x16', '\x2f', '\x3c', '\xfa', '\x30', '\x17', '\x7b', '\xa3', '\x92', '\x75', '\xa9', '\x90', '\x39', '\x68', '\x8d', '\x13',
          '\xe8', '\x06', '\xdd', '\x4f', '\x41', '\xeb', '\x3c', '\xe9', '\xf4', '\x88', '\x33', '\xb8', '\x23', '\xae', '\xed', '\x54',
          '\xfa', '\x72', '\x17', '\xe9', '\xf4', '\x6d', '\xe2', '\xb8', '\xae', '\x43', '\xba', '\xdd', '\x7e', '\x25', '\xac', '\x5d',
          '\xfe', '\x55', '\xb8', '\x3f', '\xf1', '\x3b', '\xfa', '\x37', '\x7f', '\xf8', '\x8b', '\xda', '\x8b' };

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
        { '\x00', '\x00', '\x07', '\x20', '\x78', '\xda', '\x95', '\x55', '\x4d', '\x4f', '\xe3', '\x30', '\x10', '\xbd', '\xef', '\xaf',
          '\xb0', '\x72', '\x87', '\x86', '\xf2', '\xa1', '\xae', '\xd4', '\x16', '\x55', '\x01', '\x04', '\x87', '\x42', '\x21', '\x65',
          '\x7b', '\x5c', '\x79', '\x9d', '\xd9', '\x62', '\x70', '\xec', '\xca', '\x9e', '\x94', '\x65', '\xc5', '\x8f', '\x67', '\x92',
          '\x42', '\x57', '\xc9', '\xe6', '\xcb', '\xa7', '\x24', '\x9e', '\xf7', '\xc6', '\x6f', '\x9e', '\x67', '\x9c', '\xf1', '\xf9',
          '\x9f', '\x54', '\xb1', '\x2d', '\x58', '\x27', '\x8d', '\x9e', '\x04', '\x47', '\x87', '\x61', '\xc0', '\x40', '\x0b', '\x93',
          '\x48', '\xbd', '\x9e', '\x04', '\x8f', '\xcb', '\xab', '\x83', '\x51', '\x70', '\x3e', '\x1d', '\xdf', '\xe3', '\x2c', '\xd9',
          '\x72', '\x2d', '\x20', '\xb9', '\x30', '\xe2', '\x85', '\x62', '\xf1', '\x9b', '\x43', '\x48', '\xd9', '\x8f', '\x3d', '\x31',
          '\x60', '\x8f', '\x0e', '\xec', '\xfe', '\xfb', '\x24', '\x60', '\x91', '\xd1', '\xc8', '\xa5', '\xa6', '\x95', '\x22', '\x1c',
          '\x81', '\x46', '\xcb', '\xd5', '\x4a', '\x26', '\x6b', '\xc0', '\x49', '\x90', '\x50', '\x9e', '\x15', '\x28', '\x61', '\x52',
          '\x88', '\x85', '\x05', '\xd0', '\xc1', '\x74', '\xbc', '\x67', '\xb0', '\x2b', '\x65', '\x38', '\x16', '\x12', '\x42', '\x5a',
          '\x8f', '\x37', '\x4a', '\x22', '\xd2', '\xf2', '\x9d', '\x95', '\x94', '\x85', '\x22', '\xf9', '\x16', '\xef', '\xf9', '\x16',
          '\x99', '\xa6', '\x5c', '\xc3', '\x46', '\xcc', '\x41', '\x09', '\x33', '\xb3', '\xc0', '\xd9', '\x92', '\xff', '\x22', '\x41',
          '\xc7', '\x14', '\xc8', '\xac', '\x05', '\xfd', '\x29', '\x85', '\x34', '\xc8', '\x0d', '\xba', '\x25', '\x29', '\xf9', '\x99',
          '\x92', '\x86', '\x95', '\xd4', '\x89', '\x79', '\x25', '\xce', '\x4e', '\x2f', '\xbb', '\xe5', '\x29', '\xec', '\x90', '\x0b',
          '\x6b', '\x9e', '\x41', '\x60', '\x15', '\xc9', '\x22', '\x65', '\x1c', '\x24', '\x79', '\xa9', '\x83', '\x1a', '\xd6', '\x05',
          '\x47', '\x1e', '\x9b', '\xcc', '\x0a', '\xf0', '\x24', '\x36', '\x08', '\x2b', '\xb3', '\x06', '\x79', '\x65', '\xa5', '\xfa',
          '\x8e', '\x2a', '\xf5', '\x91', '\xea', '\x0d', '\x58', '\x7c', '\xbb', '\x4c', '\x24', '\x1a', '\xdb', '\x5d', '\x62', '\x03',
          '\xb8', '\x76', '\xdb', '\x58', '\xfe', '\x05', '\x37', '\x0d', '\xd9', '\xf7', '\xd1', '\x29', '\x1b', '\x0f', '\x76', '\x5f',
          '\xf4', '\xfc', '\x3c', '\x90', '\x7e', '\x47', '\xd3', '\xe7', '\x88', '\x5b', '\xca', '\x2b', '\x77', '\x12', '\xb5', '\x0f',
          '\x5f', '\x17', '\xa8', '\x9a', '\xe2', '\x2a', '\xd0', '\xaf', '\x82', '\xc2', '\x7e', '\x3e', '\xde', '\x67', '\x52', '\xbc',
          '\xcc', '\x84', '\x00', '\xe7', '\x3a', '\x4c', '\x6c', '\x40', '\xb6', '\x38', '\x38', '\x0c', '\x4f', '\x46', '\x2c', '\xf4',
          '\xb0', '\xf0', '\x9f', '\x3d', '\xc7', '\x65', '\x7b', '\x86', '\x15', '\xd5', '\x0f', '\xe0', '\x32', '\x85', '\x37', '\xfa',
          '\xb7', '\xe9', '\x10', '\x5d', '\x0f', '\xec', '\x68', '\xd1', '\x08', '\x94', '\x8a', '\x41', '\xd1', '\x50', '\x90', '\xaa',
          '\xa5', '\x31', '\xca', '\xa7', '\x51', '\xcf', '\xaa', '\x83', '\x08', '\xd2', '\x5d', '\x4b', '\x87', '\xbd', '\x84', '\x2e',
          '\x94', '\x41', '\x9f', '\x21', '\x84', '\x0d', '\x3e', '\x79', '\x72', '\x1e', '\x40', '\x2d', '\xc0', '\xa6', '\x9e', '\xac',
          '\xc5', '\xd6', '\x57', '\xdb', '\xdc', '\x3c', '\x59', '\x17', '\x49', '\x2b', '\x14', '\x78', '\x32', '\xeb', '\x1c', '\xeb',
          '\x72', '\x7d', '\xf8', '\xff', '\xf5', '\x90', '\x37', '\xea', '\xdc', '\xe8', '\x1e', '\xd7', '\xc3', '\x9c', '\x90', '\x7c',
          '\x0d', '\xce', '\xc7', '\x8f', '\xc6', '\xfc', '\xad', '\x37', '\x4a', '\xc8', '\x8a', '\x99', '\xa8', '\x9d', '\x88', '\x62',
          '\x25', '\xbf', '\x72', '\xc2', '\x96', '\x78', '\x4b', '\x82', '\xc1', '\xfe', '\x6f', '\x43', '\xef', '\x0d', '\xff', '\xb7',
          '\xe9', '\xb7', '\x0f', '\x67', '\x50', '\x73', '\xca' };

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
        { '\x00', '\x00', '\x05', '\x4d', '\x78', '\xda', '\xa5', '\x94', '\x4f', '\x73', '\x82', '\x30', '\x10', '\xc5', '\xef', '\xfd',
          '\x14', '\x19', '\xee', '\x16', '\x50', '\x0f', '\x1e', '\x10', '\xc7', '\xc1', '\x7a', '\xb3', '\xb5', '\x05', '\xeb', '\xb1',
          '\x93', '\x92', '\x1d', '\x9a', '\x29', '\x24', '\x4c', '\x12', '\x6c', '\xed', '\xf4', '\xc3', '\x77', '\x41', '\x65', '\xaa',
          '\x45', '\xfc', '\xd3', '\x13', '\x24', '\xfb', '\x5e', '\xf6', '\xb7', '\x93', '\x37', '\xf1', '\x46', '\x9f', '\x59', '\x4a',
          '\x56', '\xa0', '\x34', '\x97', '\x62', '\x68', '\xb9', '\xb7', '\x8e', '\x45', '\x40', '\xc4', '\x92', '\x71', '\x91', '\x0c',
          '\xad', '\x45', '\x34', '\xed', '\x0c', '\xac', '\x91', '\xef', '\x3d', '\x9a', '\x31', '\x5b', '\x51', '\x11', '\x03', '\x9b',
          '\xc8', '\xf8', '\x1d', '\x6b', '\xe1', '\x5a', '\x1b', '\xc8', '\xc8', '\x73', '\x6d', '\xb4', '\xc8', '\x42', '\x83', '\xaa',
          '\xd7', '\x7d', '\x8b', '\x04', '\x52', '\x18', '\xca', '\x05', '\xee', '\x54', '\xe5', '\x00', '\x84', '\x51', '\x34', '\x5d',
          '\x72', '\x96', '\x80', '\x19', '\x5a', '\x0c', '\xcf', '\x59', '\x42', '\x1a', '\xcb', '\x0c', '\xc2', '\x58', '\x01', '\x08',
          '\xcb', '\xf7', '\x6a', '\x07', '\x99', '\xa6', '\x92', '\x9a', '\x0a', '\xc1', '\xc1', '\xfd', '\x30', '\x4f', '\xb9', '\x31',
          '\xb8', '\xfd', '\xa0', '\x38', '\x9e', '\x82', '\x95', '\xb2', '\xc5', '\x77', '\xd9', '\xa2', '\x10', '\x78', '\x56', '\xef',
          '\xa8', '\xa6', '\x53', '\x6b', '\xba', '\xa8', '\x19', '\x2b', '\xa0', '\x24', '\xa2', '\xaf', '\xba', '\xb4', '\x90', '\xa0',
          '\x50', '\x0a', '\xc4', '\x16', '\x05', '\x19', '\x78', '\x6e', '\x74', '\x84', '\x24', '\x2f', '\x79', '\x2a', '\xcd', '\x0c',
          '\x39', '\x96', '\x5c', '\x30', '\xf9', '\x81', '\xbe', '\x0d', '\x33', '\xb9', '\xa7', '\x19', '\x6c', '\xd4', '\x73', '\x54',
          '\x34', '\x6a', '\x49', '\x90', '\x4a', '\x0d', '\xac', '\x1c', '\xd8', '\x6e', '\xf0', '\x45', '\x90', '\xe5', '\x29', '\x35',
          '\x70', '\x8d', '\xb7', '\x85', '\x70', '\xdf', '\x69', '\x97', '\x63', '\xee', '\x0d', '\xeb', '\x1e', '\x0c', '\x3b', '\x57',
          '\x32', '\x07', '\x65', '\xd6', '\x77', '\x8c', '\x1b', '\xa9', '\xce', '\x99', '\xb7', '\xd5', '\xd0', '\xd8', '\x3e', '\xe4',
          '\x5f', '\xa0', '\x7d', '\x87', '\xb8', '\x8e', '\xdb', '\x25', '\x9e', '\xbd', '\x59', '\xe2', '\x77', '\x7b', '\x4d', '\xad',
          '\x7c', '\xfb', '\xb9', '\xc0', '\x30', '\xd0', '\xa4', '\x52', '\x35', '\x90', '\x1d', '\x48', '\x77', '\x24', '\xce', '\x6f',
          '\x92', '\x13', '\xc9', '\xe8', '\x9d', '\x48', '\x46', '\x91', '\x65', '\x54', '\xad', '\xe7', '\xd5', '\xcc', '\x82', '\x26',
          '\xa0', '\x9a', '\x38', '\x26', '\xd4', '\xd0', '\x50', '\x16', '\x2a', '\x86', '\x2b', '\xae', '\x16', '\xab', '\x05', '\xbb',
          '\x26', '\x12', '\x7f', '\xd1', '\xfe', '\x17', '\x85', '\x27', '\x9e', '\xbc', '\x99', '\x8b', '\xf3', '\xd0', '\xe8', '\xba',
          '\x14', '\x64', '\x06', '\x5a', '\xe3', '\x04', '\xfa', '\x8c', '\xee', '\xc7', '\xa4', '\xad', '\x39', '\x6c', '\x49', '\xe2',
          '\x4e', '\xd2', '\x75', '\xfa', '\x03', '\xd4', '\x35', '\x49', '\xec', '\xfa', '\x69', '\xc2', '\xff', '\x23', '\x8f', '\xa1',
          '\x7f', '\xf3', '\x03', '\xbc', '\x34', '\xd5', '\x5b' };

    // end paste

    QByteArray retVal( stateData, sizeof( stateData ) );

    return retVal;
}
