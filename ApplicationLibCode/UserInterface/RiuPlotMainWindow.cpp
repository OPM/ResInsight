/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016 Statoil ASA
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

#include "RiuPlotMainWindow.h"

#include "RiaApplication.h"
#include "RiaBaseDefs.h"
#include "RiaGuiApplication.h"
#include "RiaPreferences.h"
#include "RiaPreferencesSystem.h"
#include "RiaRegressionTestRunner.h"
#include "Summary/RiaSummaryPlotTools.h"
#include "Summary/RiaSummaryTools.h"

#include "Histogram/RimHistogramMultiPlot.h"
#include "Histogram/RimHistogramPlot.h"
#include "RimEnsembleCurveSetCollection.h"
#include "RimMainPlotCollection.h"
#include "RimMultiPlot.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryCurveCollection.h"
#include "RimSummaryEnsembleTools.h"
#include "RimSummaryMultiPlot.h"
#include "RimSummaryMultiPlotCollection.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotManager.h"
#include "RimViewWindow.h"
#include "RimWellAllocationPlot.h"
#include "RimWellLogCurveCommonDataSource.h"
#include "RimWellLogPlot.h"
#include "RimWellRftPlot.h"
#include "Tools/RimAutomationSettings.h"

#include "RicReloadSummaryCaseFeature.h"
#include "SummaryPlotCommands/RicSummaryCurveCalculatorDialog.h"
#include "SummaryPlotCommands/RicSummaryPlotEditorDialog.h"

#include "RiuContextMenuLauncher.h"
#include "RiuDockWidgetTools.h"
#include "RiuDragDrop.h"
#include "RiuMenuBarBuildTools.h"
#include "RiuMessagePanel.h"
#include "RiuMultiPlotBook.h"
#include "RiuMultiPlotPage.h"
#include "RiuPlotMainWindowTools.h"
#include "RiuToolTipMenu.h"
#include "RiuTools.h"
#include "RiuTreeViewEventFilter.h"
#include "RiuWellAllocationPlot.h"

#include "cafCmdExecCommandManager.h"
#include "cafCmdFeatureManager.h"
#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmObjectHandle.h"
#include "cafPdmUiPropertyView.h"
#include "cafPdmUiToolBarEditor.h"
#include "cafPdmUiTreeView.h"
#include "cafSelectionManager.h"

#include "DockAreaWidget.h"
#include "DockManager.h"

#include <QCloseEvent>
#include <QLayout>
#include <QMenuBar>
#include <QMessageBox>
#include <QSettings>
#include <QTimer>
#include <QToolBar>
#include <QTreeView>
#include <QUndoStack>
#include <QUndoView>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotMainWindow::RiuPlotMainWindow()
    : m_activePlotViewWindow( nullptr )
    , m_selection3DLinkEnabled( false )
    , m_toggleSelectionLinkAction( nullptr )
    , m_toggleAutoUpdateAction( nullptr )
    , m_autoUpdateEnabled( false )
    , m_autoUpdateTimerId( -1 )
{
    setAttribute( Qt::WA_DeleteOnClose );

    setUpCentralDockWidget();
    createActions();
    createMenus();
    createToolBars();
    createDockPanels();
    // Pre-populate the Windows menu so it is not empty on creation. On macOS, the native menu bar
    // hides empty menus, preventing the aboutToShow signal from ever firing.
    slotBuildWindowActions();

    setAcceptDrops( true );

    if ( m_undoView )
    {
        m_undoView->setStack( caf::CmdExecCommandManager::instance()->undoStack() );
    }
    connect( caf::CmdExecCommandManager::instance()->undoStack(), SIGNAL( indexChanged( int ) ), SLOT( slotRefreshUndoRedoActions() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotMainWindow::~RiuPlotMainWindow()
{
    setBeingDestroyed();

    m_summaryPlotManagerView->showProperties( nullptr );
    setPdmRoot( nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuPlotMainWindow::mainWindowName()
{
    return "RiuPlotMainWindow";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotMainWindow* RiuPlotMainWindow::instance()
{
    if ( RiaGuiApplication::isRunning() )
    {
        return RiaGuiApplication::instance()->mainPlotWindow();
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotMainWindow::onWellSelected( const QString& wellName, int timeStep )
{
    RiuPlotMainWindow* plotWnd = instance();
    if ( !plotWnd ) return;

    if ( !plotWnd->selection3DLinkEnabled() ) return;

    RimMainPlotCollection::current()->updateSelectedWell( wellName, timeStep );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotMainWindow::initializeGuiNewProjectLoaded()
{
    setPdmRoot( RimProject::current() );
    restoreTreeViewState();

    RimMainPlotCollection* mainPlotColl = RimMainPlotCollection::current();
    mainPlotColl->ensureDefaultFlowPlotsAreCreated();

    auto sumPlotManager = dynamic_cast<RimSummaryPlotManager*>( m_summaryPlotManager.get() );
    if ( sumPlotManager )
    {
        if ( auto* sumCaseMainColl = RiaSummaryTools::summaryCaseMainCollection() )
        {
            sumCaseMainColl->dataSourceHasChanged.connect( sumPlotManager, &RimSummaryPlotManager::onSummaryDataSourceHasChanged );
        }

        sumPlotManager->resetDataSourceSelection();
        sumPlotManager->updateConnectedEditors();
    }

    if ( auto* sumCaseMainColl = RiaSummaryTools::summaryCaseMainCollection() )
    {
        setExpanded( sumCaseMainColl );
    }

    {
        auto* obj = RiaSummaryTools::summaryMultiPlotCollection();
        if ( obj )
        {
            setExpanded( obj );
        }
    }

    for ( auto view : viewWindows() )
    {
        view->updateDockWindowVisibility();
    }

    refreshToolbars();

    // Sync selections with property views.
    // Go backwards as the most "important" tree view is first in the list
    // and we want that to use the property editor in case multiple tree views are visible
    for ( int i = (int)m_projectTreeViews.size() - 1; i >= 0; i-- )
    {
        auto projectTree = projectTreeView( i );
        if ( !projectTree->isVisible() ) continue;

        std::vector<caf::PdmUiItem*> uiItems;
        projectTree->selectedUiItems( uiItems );
        if ( !uiItems.empty() )
        {
            auto firstSelectedObject = dynamic_cast<caf::PdmObjectHandle*>( uiItems.front() );
            if ( i < (int)m_propertyViews.size() ) m_propertyViews[i]->showProperties( firstSelectedObject );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotMainWindow::cleanupGuiBeforeProjectClose()
{
    // Closing the last window can trigger a project close while this window is being destroyed. Accessing the
    // GUI of a window being destroyed is not safe.
    if ( isBeingDestroyed() ) return;

    for ( auto v : viewWindows() )
    {
        v->removeWindowFromDock();
    }

    setPdmRoot( nullptr );

    for ( auto pdmUiPropertyView : m_propertyViews )
    {
        pdmUiPropertyView->showProperties( nullptr );
    }

    cleanUpTemporaryWidgets();

    m_wellLogPlotToolBarEditor->clear();
    m_multiPlotToolBarEditor->clear();
    m_multiPlotLayoutToolBarEditor->clear();

    setWindowTitle( "Plots - ResInsight" );

    if ( m_messagePanel ) m_messagePanel->slotClearMessages();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotMainWindow::cleanUpTemporaryWidgets()
{
    for ( QWidget* w : m_temporaryWidgets )
    {
        w->close();
        w->deleteLater();
    }

    m_temporaryWidgets.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotMainWindow::closeEvent( QCloseEvent* event )
{
    RiaGuiApplication* app = RiaGuiApplication::instance();
    if ( !app->isMain3dWindowVisible() )
    {
        if ( !app->checkWithUserBeforeClose() )
        {
            event->ignore();
            return;
        }
    }

    RiuPlotMainWindowTools::remove3dViewsFromDocking();

    if ( auto proj = RimProject::current() )
    {
        proj->plotWindowDockState = dockWidgetStateString();
    }

    RiuMainWindowBase::closeEvent( event );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotMainWindow::keyPressEvent( QKeyEvent* keyEvent )
{
    if ( RiuTreeViewEventFilter::activateFeatureFromKeyEvent( keyEvent ) )
    {
        return;
    }

    RiuMainWindowBase::keyPressEvent( keyEvent );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotMainWindow::createActions()
{
    m_toggleSelectionLinkAction = new QAction( QIcon( ":/Link3DandPlots.png" ), tr( "Link With Selection in 3D" ), this );
    m_toggleSelectionLinkAction->setToolTip( "Update wells used in plots from well selections in 3D view." );
    m_toggleSelectionLinkAction->setCheckable( true );
    m_toggleSelectionLinkAction->setChecked( m_selection3DLinkEnabled );
    connect( m_toggleSelectionLinkAction, SIGNAL( triggered() ), SLOT( slotToggleSelectionLink() ) );

    m_toggleAutoUpdateAction = new QAction( QIcon( ":/TimedRefresh.png" ), tr( "Auto-update plots." ), this );
    m_toggleAutoUpdateAction->setToolTip( "Reload cases at interval specified in Automation Settings." );
    m_toggleAutoUpdateAction->setCheckable( true );
    m_toggleAutoUpdateAction->setChecked( m_autoUpdateEnabled );
    connect( m_toggleAutoUpdateAction, SIGNAL( triggered() ), SLOT( slotToggleAutoUpdate() ) );

    m_reloadSelectedCasesAction = new QAction( QIcon( ":/Refresh.svg" ), tr( "Reload Selected Cases" ), this );
    m_reloadSelectedCasesAction->setToolTip( "Reload selected summary and/or ensemble cases." );
    m_reloadSelectedCasesAction->setCheckable( false );
    connect( m_reloadSelectedCasesAction, SIGNAL( triggered() ), SLOT( slotReloadSelectedCases() ) );

    m_viewFullScreenAction = new QAction( QIcon( ":/Fullscreen.png" ), "Full Screen", this );
    m_viewFullScreenAction->setToolTip( "Full Screen (Ctrl+Alt+F)" );
    m_viewFullScreenAction->setCheckable( true );
    caf::CmdFeature::applyShortcutWithHintToAction( m_viewFullScreenAction, QKeySequence( tr( "Ctrl+Alt+F" ) ) );
    connect( m_viewFullScreenAction, SIGNAL( toggled( bool ) ), SLOT( slotViewFullScreen( bool ) ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotMainWindow::createMenus()
{
    caf::CmdFeatureManager* cmdFeatureMgr = caf::CmdFeatureManager::instance();
    CAF_ASSERT( cmdFeatureMgr );

    // File menu
    QMenu* fileMenu = RiuMenuBarBuildTools::createDefaultFileMenu( menuBar() );
    fileMenu->addSeparator();

    // Import menu actions
    RiuMenuBarBuildTools::addImportMenuForPlotWindow( this, fileMenu );

    // Export menu actions
    QMenu* exportMenu = fileMenu->addMenu( QIcon( ":/export.svg" ), "&Export" );
    exportMenu->addAction( cmdFeatureMgr->action( "RicSnapshotViewToFileFeature" ) );
    exportMenu->addAction( cmdFeatureMgr->action( "RicSnapshotViewToPdfFeature" ) );
    exportMenu->addAction( cmdFeatureMgr->action( "RicSnapshotAllPlotsToFileFeature" ) );
    exportMenu->addAction( cmdFeatureMgr->action( "RicSaveEclipseInputActiveVisibleCellsFeature" ) );

    // Save menu actions
    fileMenu->addSeparator();
    RiuMenuBarBuildTools::addSaveProjectActions( fileMenu );

    std::vector<QAction*> recentFileActions = RiaGuiApplication::instance()->recentFileActions();
    for ( auto act : recentFileActions )
    {
        fileMenu->addAction( act );
    }

    // Close and Exit actions
    fileMenu->addSeparator();
    RiuMenuBarBuildTools::addCloseAndExitActions( fileMenu );

    // Edit menu
    QMenu* editMenu = RiuMenuBarBuildTools::createDefaultEditMenu( menuBar() );
    if ( RiaPreferences::current()->useUndoRedo() )
    {
        editMenu->addSeparator();
        editMenu->addAction( m_undoAction );
        editMenu->addAction( m_redoAction );
    }
    connect( editMenu, SIGNAL( aboutToShow() ), SLOT( slotRefreshUndoRedoActions() ) );

    // View menu
    QMenu* viewMenu = RiuMenuBarBuildTools::createDefaultViewMenu( menuBar() );
    viewMenu->addSeparator();
    viewMenu->addAction( m_viewFullScreenAction );
    viewMenu->addAction( m_hideTabsAction );

    // Windows menu
    m_windowMenu = menuBar()->addMenu( "&Windows" );
    connect( m_windowMenu, SIGNAL( aboutToShow() ), SLOT( slotBuildWindowActions() ) );

    // Help menu
    QMenu* helpMenu = RiuMenuBarBuildTools::createDefaultHelpMenu( menuBar() );
    connect( helpMenu, SIGNAL( aboutToShow() ), SLOT( slotRefreshHelpActions() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RiuPlotMainWindow::toolbarCommandIds( const QString& toolbarName )
{
    QStringList commandIds;

    if ( toolbarName.isEmpty() || toolbarName == "Standard" )
    {
        commandIds << "RicImportSummaryCaseFeature";
        commandIds << "RicImportGridAndSummaryEnsembleFeature";
        commandIds << "RicImportEnsembleFeature";
        commandIds << "Separator";
        commandIds << "RicOpenProjectFeature";
        commandIds << "RicSaveProjectFeature";
    }

    if ( toolbarName.isEmpty() || toolbarName == "Window Management" )
    {
        commandIds << "RicShowMainWindowFeature";
        commandIds << "RicShowSummaryCurveCalculatorFeature";
    }

    if ( toolbarName.isEmpty() || toolbarName == "View Snapshots" )
    {
        commandIds << "RicSnapshotViewToClipboardFeature";
        commandIds << "RicSnapshotViewToFileFeature";
        commandIds << "RicSnapshotViewToPdfFeature";
        commandIds << "RicSnapshotAllPlotsToFileFeature";
    }

    if ( toolbarName.isEmpty() || toolbarName == "View" )
    {
        commandIds << "RicViewZoomAllFeature";
    }

    return commandIds;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotMainWindow::createToolBars()
{
    caf::CmdFeatureManager* cmdFeatureMgr = caf::CmdFeatureManager::instance();
    CAF_ASSERT( cmdFeatureMgr );

    QStringList toolbarNames;
    toolbarNames << "Standard" << "Window Management" << "View Snapshots" << "View";

    for ( QString toolbarName : toolbarNames )
    {
        QToolBar* toolbar = addToolBar( toolbarName );
        toolbar->setObjectName( toolbar->windowTitle() );

        QStringList toolbarCommands = toolbarCommandIds( toolbarName );
        for ( QString s : toolbarCommands )
        {
            if ( s.compare( "separator", Qt::CaseInsensitive ) == 0 )
            {
                toolbar->addSeparator();
            }
            else
            {
                toolbar->addAction( cmdFeatureMgr->action( s ) );
            }
        }
        if ( toolbarName == "View" )
        {
            toolbar->addAction( m_viewFullScreenAction );
            toolbar->addAction( m_hideTabsAction );
            toolbar->addAction( m_toggleSelectionLinkAction );
            toolbar->addAction( m_reloadSelectedCasesAction );
            toolbar->addAction( m_toggleAutoUpdateAction );
        }
    }

    m_wellLogPlotToolBarEditor = std::make_unique<caf::PdmUiToolBarEditor>( "Well Log Plot", this );
    m_wellLogPlotToolBarEditor->hide();

    m_multiPlotToolBarEditor = std::make_unique<caf::PdmUiToolBarEditor>( "Multi Plot", this );
    m_multiPlotToolBarEditor->hide();

    m_multiPlotLayoutToolBarEditor = std::make_unique<caf::PdmUiToolBarEditor>( "Multi Plot Layout", this );
    m_multiPlotLayoutToolBarEditor->hide();

    if ( RiaPreferences::current()->useUndoRedo() )
    {
        QToolBar* toolbar = addToolBar( tr( "Edit" ) );
        toolbar->setObjectName( toolbar->windowTitle() );
        toolbar->addAction( m_undoAction );
        toolbar->addAction( m_redoAction );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotMainWindow::refreshToolbars()
{
    QStringList allToolbarCommandNames = toolbarCommandIds();

    caf::CmdFeatureManager::instance()->refreshEnabledState( allToolbarCommandNames );
    caf::CmdFeatureManager::instance()->refreshCheckedState( allToolbarCommandNames );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotMainWindow::createDockPanels()
{
    const std::vector<QString> treeViewTitles    = { "Plots", "Data Sources", "Templates", "Scripts/Jobs", "Ensemble Data" };
    const std::vector<QString> treeViewConfigs   = { "PlotWindow.Plots",
                                                     "PlotWindow.DataSources",
                                                     "PlotWindow.Templates",
                                                     "PlotWindow.Scripts",
                                                     "PlotWindow.Cloud" };
    const std::vector<QString> treeViewDockNames = { RiuDockWidgetTools::plotMainWindowPlotsTreeName(),
                                                     RiuDockWidgetTools::plotMainWindowDataSourceTreeName(),
                                                     RiuDockWidgetTools::plotMainWindowTemplateTreeName(),
                                                     RiuDockWidgetTools::plotMainWindowScriptsTreeName(),
                                                     RiuDockWidgetTools::plotMainWindowCloudTreeName() };
    const int                  nTreeViews        = static_cast<int>( treeViewConfigs.size() );

    const std::vector<ads::DockWidgetArea> defaultDockWidgetArea{ ads::DockWidgetArea::LeftDockWidgetArea,
                                                                  ads::DockWidgetArea::RightDockWidgetArea,
                                                                  ads::DockWidgetArea::LeftDockWidgetArea,
                                                                  ads::DockWidgetArea::LeftDockWidgetArea,
                                                                  ads::DockWidgetArea::RightDockWidgetArea };

    createTreeViews( nTreeViews );

    std::vector<ads::CDockWidget*> rightWidgets;
    std::vector<ads::CDockWidget*> leftWidgets;
    std::vector<ads::CDockWidget*> bottomWidgets;

    // the project trees
    for ( int i = 0; i < nTreeViews; i++ )
    {
        auto dockWidget = RiuDockWidgetTools::createDockWidget( treeViewTitles[i], treeViewDockNames[i], dockManager() );

        caf::PdmUiTreeView* projectTree = projectTreeView( i );
        projectTree->enableSelectionManagerUpdating( true );
        projectTree->setObjectName( treeViewDockNames[i] );
        projectTree->enableAppendOfClassNameToUiItemText( RiaPreferencesSystem::current()->appendClassNameToUiText() );

        dockWidget->setWidget( projectTree );
        dockWidget->hide();

        projectTree->treeView()->setHeaderHidden( true );
        projectTree->treeView()->setSelectionMode( QAbstractItemView::ExtendedSelection );

        // Drag and drop configuration
        projectTree->treeView()->setDragEnabled( true );
        projectTree->treeView()->viewport()->setAcceptDrops( true );
        projectTree->treeView()->setDropIndicatorShown( true );
        projectTree->treeView()->setDragDropMode( QAbstractItemView::DragDrop );

        // Install event filter used to handle key press events
        RiuTreeViewEventFilter* treeViewEventFilter = new RiuTreeViewEventFilter( this, projectTree );
        projectTree->treeView()->installEventFilter( treeViewEventFilter );

        if ( defaultDockWidgetArea[i] == ads::DockWidgetArea::LeftDockWidgetArea ) leftWidgets.push_back( dockWidget );
        if ( defaultDockWidgetArea[i] == ads::DockWidgetArea::RightDockWidgetArea ) rightWidgets.push_back( dockWidget );

        connect( dockWidget, SIGNAL( visibilityChanged( bool ) ), projectTree, SLOT( treeVisibilityChanged( bool ) ) );

        projectTree->treeView()->setContextMenuPolicy( Qt::CustomContextMenu );
        connect( projectTree->treeView(), SIGNAL( customContextMenuRequested( const QPoint& ) ), SLOT( customMenuRequested( const QPoint& ) ) );

        projectTree->setUiConfigurationName( treeViewConfigs[i] );
    }

    // the plot manager
    {
        auto dockWidget =
            RiuDockWidgetTools::createDockWidget( "Plot Manager", RiuDockWidgetTools::plotMainWindowPlotManagerName(), dockManager() );

        m_summaryPlotManagerView = std::make_unique<caf::PdmUiPropertyView>( dockWidget );

        auto plotManager = std::make_unique<RimSummaryPlotManager>();
        m_summaryPlotManagerView->showProperties( plotManager.get() );
        m_summaryPlotManagerView->installEventFilter( plotManager.get() );
        m_summaryPlotManager = std::move( plotManager );

        dockWidget->setWidget( m_summaryPlotManagerView.get() );

        rightWidgets.push_back( dockWidget );
    }

    // the undo stack
    if ( m_undoView && RiaPreferences::current()->useUndoRedo() )
    {
        auto dockWidget =
            RiuDockWidgetTools::createDockWidget( "Undo Stack", RiuDockWidgetTools::plotMainWindowUndoStackName(), dockManager() );

        dockWidget->setWidget( m_undoView );
        rightWidgets.push_back( dockWidget );
    }

    ads::CDockAreaWidget* leftArea  = addTabbedWidgets( leftWidgets, ads::DockWidgetArea::LeftDockWidgetArea );
    ads::CDockAreaWidget* rightArea = addTabbedWidgets( rightWidgets, ads::DockWidgetArea::RightDockWidgetArea );
    ads::CDockAreaWidget* bottomArea =
        addTabbedWidgets( bottomWidgets, ads::DockWidgetArea::BottomDockWidgetArea, dockManager()->centralWidget()->dockAreaWidget() );

    // the log message view
    {
        auto dockWidget = RiuDockWidgetTools::createDockWidget( "Messages", RiuDockWidgetTools::plotMainWindowMessagesName(), dockManager() );

        m_messagePanel = new RiuMessagePanel( dockWidget );
        dockWidget->setWidget( m_messagePanel );
        dockManager()->addDockWidget( ads::DockWidgetArea::BottomDockWidgetArea, dockWidget, rightArea );
    }

    auto createPropertyView = [this]( const QString& displayName, const QString& internalName, ads::CDockAreaWidget* dockArea )
    {
        auto dockWidget        = RiuDockWidgetTools::createDockWidget( displayName, internalName, dockManager() );
        auto pdmUiPropertyView = std::make_shared<caf::PdmUiPropertyView>( dockWidget );
        dockWidget->setWidget( pdmUiPropertyView.get() );
        dockManager()->addDockWidget( ads::DockWidgetArea::BottomDockWidgetArea, dockWidget, dockArea );
        return pdmUiPropertyView;
    };

    auto leftPropertyView = createPropertyView( "Property Editor", RiuDockWidgetTools::plotMainWindowPropertyEditorName(), leftArea );
    auto rightPropertyView =
        createPropertyView( "Data Source Property Editor", RiuDockWidgetTools::plotMainWindowPropertyEditorRightName(), rightArea );

    // Connect project trees with property views
    for ( int i = 0; i < nTreeViews; i++ )
    {
        caf::PdmUiTreeView* projectTree = projectTreeView( i );

        auto pdmUiPropertyView = defaultDockWidgetArea[i] == ads::DockWidgetArea::LeftDockWidgetArea ? leftPropertyView : rightPropertyView;

        connect( projectTree,
                 &caf::PdmUiTreeView::selectionChanged,
                 [this, projectTree, pdmUiPropertyView]() { selectedObjectsChanged( projectTree, pdmUiPropertyView.get() ); } );

        m_propertyViews.push_back( pdmUiPropertyView );
    }

    if ( leftArea ) leftArea->setCurrentIndex( 0 );
    if ( rightArea ) rightArea->setCurrentIndex( 0 );
    if ( bottomArea ) bottomArea->setCurrentIndex( 0 );

    auto widgets = dockManager()->dockWidgetsMap().values();
    for ( ads::CDockWidget* dock : widgets )
    {
        connect( dock->toggleViewAction(), SIGNAL( triggered() ), SLOT( slotDockWidgetToggleViewActionTriggered() ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotMainWindow::addToTemporaryWidgets( QWidget* widget )
{
    CAF_ASSERT( widget );

    m_temporaryWidgets.push_back( widget );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotMainWindow::updateWellLogPlotToolBar()
{
    RimWellLogPlot* wellLogPlot = dynamic_cast<RimWellLogPlot*>( m_activePlotViewWindow.p() );
    RimWellRftPlot* wellRftPlot = dynamic_cast<RimWellRftPlot*>( wellLogPlot );

    if ( wellLogPlot && !wellRftPlot )
    {
        std::vector<caf::PdmFieldHandle*> toolBarFields;
        toolBarFields = wellLogPlot->commonDataSource()->fieldsToShowInToolbar();

        m_wellLogPlotToolBarEditor->setFields( toolBarFields );
        m_wellLogPlotToolBarEditor->updateUi();

        m_wellLogPlotToolBarEditor->show();
    }
    else
    {
        m_wellLogPlotToolBarEditor->clear();

        m_wellLogPlotToolBarEditor->hide();
    }

    refreshToolbars();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotMainWindow::updateMultiPlotToolBar()
{
    RimMultiPlot* plotWindow = dynamic_cast<RimMultiPlot*>( m_activePlotViewWindow.p() );
    if ( plotWindow )
    {
        std::vector<caf::PdmFieldHandle*> toolBarFields = plotWindow->fieldsToShowInToolbar();

        if ( toolBarFields.empty() )
        {
            m_multiPlotToolBarEditor->clear();
            m_multiPlotToolBarEditor->hide();
        }
        else
        {
            if ( !m_multiPlotToolBarEditor->isEditorDataEqualAndValid( toolBarFields ) )
            {
                m_multiPlotToolBarEditor->setFields( toolBarFields );
            }
            m_multiPlotToolBarEditor->updateUi();
            m_multiPlotToolBarEditor->show();
        }

        std::vector<caf::PdmFieldHandle*> layoutFields = plotWindow->fieldsToShowInLayoutToolbar();
        m_multiPlotLayoutToolBarEditor->setFields( layoutFields );
        m_multiPlotLayoutToolBarEditor->updateUi();
        m_multiPlotLayoutToolBarEditor->show();
    }
    else
    {
        m_multiPlotToolBarEditor->clear();
        m_multiPlotToolBarEditor->hide();
        m_multiPlotLayoutToolBarEditor->clear();
        m_multiPlotLayoutToolBarEditor->hide();
    }
    refreshToolbars();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicSummaryPlotEditorDialog* RiuPlotMainWindow::summaryCurveCreatorDialog( bool createIfNotPresent )
{
    if ( !m_summaryCurveCreatorDialog && createIfNotPresent )
    {
        m_summaryCurveCreatorDialog = std::make_unique<RicSummaryPlotEditorDialog>( this );
    }

    return m_summaryCurveCreatorDialog.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicSummaryCurveCalculatorDialog* RiuPlotMainWindow::summaryCurveCalculatorDialog( bool createIfNotPresent )
{
    if ( !m_summaryCurveCalculatorDialog && createIfNotPresent )
    {
        m_summaryCurveCalculatorDialog = std::make_unique<RicSummaryCurveCalculatorDialog>( this );
    }

    return m_summaryCurveCalculatorDialog.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuMessagePanel* RiuPlotMainWindow::messagePanel()
{
    return m_messagePanel;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotMainWindow::showAndSetKeyboardFocusToSummaryPlotManager()
{
    auto dockWidget = RiuDockWidgetTools::findDockWidget( dockManager(), RiuDockWidgetTools::plotMainWindowPlotManagerName() );
    if ( dockWidget )
    {
        dockWidget->setVisible( true );

        auto sumPlotManager = dynamic_cast<RimSummaryPlotManager*>( m_summaryPlotManager.get() );
        if ( sumPlotManager )
        {
            sumPlotManager->setFocusToFilterText();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotMainWindow::onViewerRemoved()
{
    refreshToolbars();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotMainWindow::initializeViewer( ads::CDockWidget* dockWidget, QWidget* viewer )
{
    dockManager()->addDockWidget( ads::DockWidgetArea::CenterDockWidgetArea, dockWidget, dockManager()->centralWidget()->dockAreaWidget() );

    if ( auto book = dynamic_cast<RiuMultiPlotBook*>( viewer ) )
    {
        book->scheduleReplotOfAllPlots();
    }
    else
    {
        viewer->update();
    }

    refreshToolbars();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimViewWindow*> RiuPlotMainWindow::viewWindows()
{
    std::vector<RimViewWindow*> views;

    // Project can be null while it is being torn down.
    if ( auto project = RimProject::current() )
    {
        for ( auto v : project->descendantsOfType<RimPlotWindow>() )
        {
            if ( v->dockWidget() ) views.push_back( v );
        }
    }

    return views;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimViewWindow* RiuPlotMainWindow::activeViewer()
{
    for ( auto view : viewWindows() )
    {
        if ( view->isActiveViewer() )
        {
            return view;
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotMainWindow::setPdmRoot( caf::PdmObject* pdmRoot )
{
    for ( auto tv : projectTreeViews() )
    {
        tv->setPdmItem( pdmRoot );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotMainWindow::slotBuildWindowActions()
{
    m_windowMenu->clear();

    caf::CmdFeatureManager* cmdFeatureMgr = caf::CmdFeatureManager::instance();
    m_windowMenu->addAction( cmdFeatureMgr->action( "RicShowMainWindowFeature" ) );
    m_windowMenu->addSeparator();

    addDefaultEntriesToWindowsMenu();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotMainWindow::selectedObjectsChanged( caf::PdmUiTreeView* projectTree, caf::PdmUiPropertyView* propertyView )
{
    std::vector<caf::PdmUiItem*> uiItems;
    projectTree->selectedUiItems( uiItems );

    caf::PdmObjectHandle* firstSelectedObject = nullptr;
    if ( !uiItems.empty() )
    {
        firstSelectedObject = dynamic_cast<caf::PdmObjectHandle*>( uiItems.front() );
    }

    propertyView->showProperties( firstSelectedObject );

    std::vector<RimSummaryCase*> summaryCases;
    for ( auto uiItem : uiItems )
    {
        if ( auto summaryCase = dynamic_cast<RimSummaryCase*>( uiItem ) )
        {
            summaryCases.push_back( summaryCase );
        }
    }

    RimSummaryEnsembleTools::highlightCurvesForSummaryCases( summaryCases );

    if ( uiItems.size() == 1 && m_allowActiveViewChangeFromSelection )
    {
        // Find the plot view that the selected item is within
        if ( !firstSelectedObject )
        {
            caf::PdmFieldHandle* selectedField = dynamic_cast<caf::PdmFieldHandle*>( uiItems.front() );
            if ( selectedField ) firstSelectedObject = selectedField->ownerObject();
        }

        if ( !firstSelectedObject ) return;

        RimViewWindow* selectedWindow = dynamic_cast<RimViewWindow*>( firstSelectedObject );
        if ( !selectedWindow )
        {
            selectedWindow = firstSelectedObject->firstAncestorOrThisOfType<RimViewWindow>();
        }

        if ( selectedWindow )
        {
            m_activePlotViewWindow = selectedWindow;

            if ( auto multiSummaryPlot = firstSelectedObject->firstAncestorOrThisOfType<RimSummaryMultiPlot>() )
            {
                // The toolbar shows fields from the summary multi plot. When a child object is selected, the first
                // ancestor view window is the sub plot, not the multi plot. Use the multi plot as active plot view
                // window to make sure the toolbar is available for any object in the multi plot.
                m_activePlotViewWindow = multiSummaryPlot;

                setBlockViewSelectionOnSubWindowActivated( true );
                setActiveViewer( multiSummaryPlot->dockWindowName() );
                setBlockViewSelectionOnSubWindowActivated( false );

                updateMultiPlotToolBar();

                auto summaryPlot = firstSelectedObject->firstAncestorOrThisOfType<RimSummaryPlot>();
                if ( summaryPlot )
                {
                    multiSummaryPlot->makeSureIsVisible( summaryPlot );
                }
            }
            else if ( auto multiHistogramPlot = firstSelectedObject->firstAncestorOrThisOfType<RimHistogramMultiPlot>() )
            {
                // The toolbar shows fields from the histogram multi plot. When a child object is selected, the first
                // ancestor view window is the sub plot, not the multi plot. Use the multi plot as active plot view
                // window to make sure the toolbar is available for any object in the multi plot.
                m_activePlotViewWindow = multiHistogramPlot;

                setBlockViewSelectionOnSubWindowActivated( true );
                setActiveViewer( multiHistogramPlot->dockWindowName() );
                setBlockViewSelectionOnSubWindowActivated( false );

                updateMultiPlotToolBar();

                auto histogramPlot = firstSelectedObject->firstAncestorOrThisOfType<RimHistogramPlot>();
                if ( histogramPlot )
                {
                    multiHistogramPlot->makeSureIsVisible( histogramPlot );
                }
            }
            else
            {
                if ( selectedWindow->dockWidget() )
                {
                    setBlockViewSelectionOnSubWindowActivated( true );
                    setActiveViewer( selectedWindow->dockWindowName() );
                    setBlockViewSelectionOnSubWindowActivated( false );
                }
            }

            // The only way to get to this code is by selection change initiated from the project tree view
            // As we are activating an view window, the focus might be given to this window
            // Set focus back to the tree view to be able to continue keyboard tree view navigation
            projectTree->treeView()->setFocus();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotMainWindow::restoreTreeViewState()
{
    restoreTreeViewStates( RimProject::current()->plotWindowTreeViewStates(), RimProject::current()->plotWindowCurrentModelIndexPaths() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotMainWindow::setDefaultWindowSize()
{
    resize( 1000, 810 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotMainWindow::customMenuRequested( const QPoint& pos )
{
    QMenu menu;

    RiaApplication* app = RiaApplication::instance();
    app->project()->actionsBasedOnSelection( menu );

    // Qt doc: QAbstractScrollArea and its subclasses that map the context menu event to coordinates of the
    // viewport(). Since we might get this signal from different treeViews, we need to map the position accordingly.
    QObject*   senderObj = sender();
    QTreeView* treeView  = dynamic_cast<QTreeView*>( senderObj );
    if ( treeView )
    {
        QPoint globalPos = treeView->viewport()->mapToGlobal( pos );
        menu.exec( globalPos );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotMainWindow::dragEnterEvent( QDragEnterEvent* event )
{
    if ( m_centralDockWidget != nullptr )
    {
        QPoint curpos = m_centralDockWidget->mapFromGlobal( QCursor::pos() );
        auto   rect   = m_centralDockWidget->widget()->rect();
        rect.adjust( -10, -10, 10, 10 ); // allow some tolerance outside the widget
        if ( rect.contains( curpos ) )
        {
            event->acceptProposedAction();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotMainWindow::dropEvent( QDropEvent* event )
{
    std::vector<caf::PdmObjectHandle*> objects;

    if ( RiuDragDrop::handleGenericDropEvent( event, objects ) )
    {
        RiaSummaryPlotTools::createAndAppendSummaryMultiPlot( objects );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RiuPlotMainWindow::defaultDockStateNames()
{
    QStringList retList = { RiuDockWidgetTools::dockStatePlotWindowName(), RiuDockWidgetTools::dockStateHideAllPlotWindowName() };
    return retList;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotMainWindow::enable3DSelectionLink( bool enable )
{
    m_selection3DLinkEnabled = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuPlotMainWindow::selection3DLinkEnabled()
{
    return m_selection3DLinkEnabled;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotMainWindow::slotToggleSelectionLink()
{
    m_selection3DLinkEnabled = !m_selection3DLinkEnabled;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotMainWindow::slotToggleAutoUpdate()
{
    m_autoUpdateEnabled = !m_autoUpdateEnabled;

    if ( ( m_autoUpdateEnabled ) && ( m_autoUpdateTimerId == -1 ) )
    {
        auto intervalMs     = RimProject::current()->automationSettings()->caseReloadIntervalMs();
        m_autoUpdateTimerId = startTimer( intervalMs );
    }
    else
    {
        if ( m_autoUpdateTimerId != -1 )
        {
            killTimer( m_autoUpdateTimerId );
            m_autoUpdateTimerId = -1;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotMainWindow::slotReloadSelectedCases()
{
    RicReloadSummaryCaseFeature::reloadSelectedCasesAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotMainWindow::timerEvent( QTimerEvent* event )
{
    if ( event->timerId() == m_autoUpdateTimerId )
    {
        RicReloadSummaryCaseFeature::reloadTaggedSummaryCasesAndUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotMainWindow::slotViewFullScreen( bool showFullScreen )
{
    if ( showFullScreen )
    {
        m_lastDockState = dockManager()->saveState( DOCKSTATE_VERSION );

        QString activeViewerName;
        if ( auto activeViewer = RiaGuiApplication::instance()->activePlotWindow() )
        {
            activeViewerName = activeViewer->dockWindowName();
        }

        dockManager()->restoreState( RiuDockWidgetTools::hideAllDockingPlotState(), DOCKSTATE_VERSION );

        if ( !activeViewerName.isEmpty() )
        {
            if ( auto dw = dockManager()->findDockWidget( activeViewerName ) )
            {
                dockManager()->addDockWidget( ads::DockWidgetArea::CenterDockWidgetArea, dw, dockManager()->centralWidget()->dockAreaWidget() );
            }
        }
    }
    else
    {
        QString activeViewerName;
        if ( auto activeViewer = RiaGuiApplication::instance()->activePlotWindow() )
        {
            activeViewerName = activeViewer->dockWindowName();
        }

        dockManager()->restoreState( m_lastDockState, DOCKSTATE_VERSION );

        if ( !activeViewerName.isEmpty() )
        {
            if ( auto dw = dockManager()->findDockWidget( activeViewerName ) )
            {
                dw->setAsCurrentTab();
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotMainWindow::onCentralWidgetContextMenu( QMenu& menu )
{
    QStringList commandIds;
    commandIds << "RicNewEmptySummaryMultiPlotFeature";
    commandIds << "RicOpenSummaryPlotEditorFromDockAreaFeature";
    caf::CmdFeatureMenuBuilder::appendToMenu( &menu, commandIds );
}
