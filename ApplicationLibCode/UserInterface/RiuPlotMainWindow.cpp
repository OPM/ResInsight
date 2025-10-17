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

#include "SummaryPlotCommands/RicSummaryCurveCalculatorDialog.h"
#include "SummaryPlotCommands/RicSummaryPlotEditorDialog.h"

#include "RiuContextMenuLauncher.h"
#include "RiuDockWidgetTools.h"
#include "RiuDragDrop.h"
#include "RiuMdiArea.h"
#include "RiuMdiSubWindow.h"
#include "RiuMenuBarBuildTools.h"
#include "RiuMessagePanel.h"
#include "RiuMultiPlotPage.h"
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
#include <QMdiSubWindow>
#include <QMenuBar>
#include <QMessageBox>
#include <QSettings>
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
{
    m_mdiArea = new RiuMdiArea( this );
    connect( m_mdiArea, SIGNAL( subWindowActivated( QMdiSubWindow* ) ), SLOT( slotSubWindowActivated( QMdiSubWindow* ) ) );

    caf::CmdFeatureMenuBuilder menuForMdiArea;
    menuForMdiArea << "RicNewEmptySummaryMultiPlotFeature";
    menuForMdiArea << "RicOpenSummaryPlotEditorFromMdiAreaFeature";
    new RiuContextMenuLauncher( m_mdiArea, menuForMdiArea );

    ads::CDockWidget* cWidget = RiuDockWidgetTools::createDockWidget( "Plot Window", RiuDockWidgetTools::mainPlotWindowName(), this );

    cWidget->setWidget( m_mdiArea );
    auto dockArea = dockManager()->setCentralWidget( cWidget );
    dockArea->setVisible( true );

    m_toggleSelectionLinkAction = new QAction( QIcon( ":/Link3DandPlots.png" ), tr( "Link With Selection in 3D" ), this );
    m_toggleSelectionLinkAction->setToolTip( "Update wells used in plots from well selections in 3D view." );
    m_toggleSelectionLinkAction->setCheckable( true );
    m_toggleSelectionLinkAction->setChecked( m_selection3DLinkEnabled );
    connect( m_toggleSelectionLinkAction, SIGNAL( triggered() ), SLOT( slotToggleSelectionLink() ) );

    createMenus();
    createToolBars();
    createDockPanels();

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

    m_mdiArea->applyTiling();

    if ( m_activePlotViewWindow && m_activePlotViewWindow->viewWidget() && !RiaRegressionTestRunner::instance()->isRunningRegressionTests() )
    {
        if ( m_activePlotViewWindow->mdiWindowGeometry().isMaximized )
        {
            auto subWin = findMdiSubWindow( m_activePlotViewWindow->viewWidget() );
            if ( subWin )
            {
                subWin->showMaximized();
            }
        }
    }

    refreshToolbars();

    // Find the project tree and reselect items to trigger the selectionChanged signal. This will make sure that the property view is
    // updated based on the selection in the main project tree.
    if ( auto dockWidget = RiuDockWidgetTools::findDockWidget( dockManager(), RiuDockWidgetTools::plotMainWindowPlotsTreeName() ) )
    {
        if ( auto tree = dynamic_cast<caf::PdmUiTreeView*>( dockWidget->widget() ) )
        {
            std::vector<caf::PdmUiItem*> uiItems;
            tree->selectedUiItems( uiItems );

            std::vector<const caf::PdmUiItem*> constSelectedItems;
            for ( auto item : uiItems )
            {
                constSelectedItems.push_back( item );
            }

            tree->selectItems( constSelectedItems );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotMainWindow::cleanupGuiBeforeProjectClose()
{
    m_mdiArea->closeAllSubWindows();

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
    saveWinGeoAndDockToolBarLayout();
    QMainWindow::closeEvent( event );
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
void RiuPlotMainWindow::createMenus()
{
    caf::CmdFeatureManager* cmdFeatureMgr = caf::CmdFeatureManager::instance();
    CVF_ASSERT( cmdFeatureMgr );

    // File menu
    QMenu* fileMenu = RiuMenuBarBuildTools::createDefaultFileMenu( menuBar() );
    fileMenu->addSeparator();

    // Import menu actions
    RiuMenuBarBuildTools::addImportMenuWithActions( this, fileMenu );

    // Export menu actions
    QMenu* exportMenu = fileMenu->addMenu( "&Export" );
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
    RiuMenuBarBuildTools::createDefaultViewMenu( menuBar() );

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
        commandIds << "RicImportEclipseCaseFeature";
        commandIds << "RicImportSummaryCaseFeature";
        commandIds << "RicImportEnsembleFeature";
        commandIds << "RicOpenProjectFeature";
        commandIds << "RicSaveProjectFeature";
    }

    if ( toolbarName.isEmpty() || toolbarName == "Window Management" )
    {
        commandIds << "RicShowMainWindowFeature";
        commandIds << "RicTilePlotWindowsFeature";
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
    CVF_ASSERT( cmdFeatureMgr );

    QStringList toolbarNames;
    toolbarNames << "Standard" << "Window Management" << "View Snapshots" << "View";

    for ( QString toolbarName : toolbarNames )
    {
        QToolBar* toolbar = addToolBar( toolbarName );
        toolbar->setObjectName( toolbar->windowTitle() );

        QStringList toolbarCommands = toolbarCommandIds( toolbarName );
        for ( QString s : toolbarCommands )
        {
            toolbar->addAction( cmdFeatureMgr->action( s ) );
        }
        if ( toolbarName == "View" )
        {
            toolbar->addAction( m_toggleSelectionLinkAction );
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

        projectTree->enableAppendOfClassNameToUiItemText( RiaPreferencesSystem::current()->appendClassNameToUiText() );

        dockWidget->setWidget( projectTree );

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
QMdiSubWindow* RiuPlotMainWindow::findMdiSubWindow( QWidget* viewer )
{
    QList<QMdiSubWindow*> subws = m_mdiArea->subWindowList();
    int                   i;
    for ( i = 0; i < subws.size(); ++i )
    {
        if ( subws[i]->widget() == viewer )
        {
            return subws[i];
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimViewWindow* RiuPlotMainWindow::findViewWindowFromSubWindow( QMdiSubWindow* subWindow )
{
    RimProject* proj = RimProject::current();
    if ( subWindow && proj )
    {
        return RiuInterfaceToViewWindow::viewWindowFromWidget( subWindow->widget() );
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<QMdiSubWindow*> RiuPlotMainWindow::subWindowList( QMdiArea::WindowOrder order )
{
    return m_mdiArea->subWindowList( order );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotMainWindow::setWidthOfMdiWindow( QWidget* mdiWindowWidget, int newWidth )
{
    QMdiSubWindow* mdiWindow = findMdiSubWindow( mdiWindowWidget );
    if ( mdiWindow )
    {
        QSize subWindowSize = mdiWindow->size();

        subWindowSize.setWidth( std::max( newWidth, 100 ) );
        mdiWindow->resize( subWindowSize );

        if ( mdiWindow->isMaximized() )
        {
            // Set window temporarily to normal state and back to maximized
            // to redo layout so the whole window canvas is filled
            // Tried to activate layout, did not work as expected
            // Tested code:
            //   m_layout->activate();
            //   mdiWindow->layout()->activate();

            mdiWindow->showNormal();
            mdiWindow->showMaximized();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotMainWindow::addToTemporaryWidgets( QWidget* widget )
{
    CVF_ASSERT( widget );

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
                m_multiPlotToolBarEditor->updateUi();
            }
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
void RiuPlotMainWindow::removeViewer( QWidget* viewer )
{
    removeViewerFromMdiArea( m_mdiArea, viewer );
    refreshToolbars();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotMainWindow::initializeViewer( QMdiSubWindow* subWindow, QWidget* viewer, const RimMdiWindowGeometry& windowsGeometry )
{
    QSize  subWindowSize;
    QPoint subWindowPos( -1, -1 );

    if ( windowsGeometry.isValid() )
    {
        subWindowPos  = QPoint( windowsGeometry.x, windowsGeometry.y );
        subWindowSize = QSize( windowsGeometry.width, windowsGeometry.height );
    }
    else
    {
        subWindowSize = QSize( 400, 400 );
    }

    initializeSubWindow( m_mdiArea, subWindow, subWindowPos, subWindowSize );
    subWindow->setWidget( viewer );

    refreshToolbars();
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
void RiuPlotMainWindow::slotSubWindowActivated( QMdiSubWindow* subWindow )
{
    if ( isBlockingSubWindowActivatedSignal() ) return;

    RimViewWindow* activatedView = findViewWindowFromSubWindow( subWindow );

    if ( !activatedView ) return;
    m_activePlotViewWindow = activatedView;

    if ( !isBlockingViewSelectionOnSubWindowActivated() )
    {
        caf::PdmUiTreeView* projectTree = getTreeViewWithItem( activatedView );
        if ( projectTree )
        {
            std::vector<caf::PdmUiItem*> currentSelection;
            projectTree->selectedUiItems( currentSelection );
            bool childSelected = false;
            for ( caf::PdmUiItem* uiItem : currentSelection )
            {
                caf::PdmObject* pdmObject = dynamic_cast<caf::PdmObject*>( uiItem );
                if ( pdmObject )
                {
                    std::vector<RimViewWindow*> ancestralViews = pdmObject->allAncestorsOrThisOfType<RimViewWindow>();
                    for ( auto ancestralView : ancestralViews )
                    {
                        if ( ancestralView == activatedView )
                        {
                            childSelected = true;
                        }
                    }
                }
            }
            if ( !childSelected )
            {
                selectAsCurrentItem( activatedView );
            }
        }
    }

    updateWellLogPlotToolBar();
    updateMultiPlotToolBar();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotMainWindow::setActiveViewer( QWidget* viewer )
{
    QMdiSubWindow* swin = findMdiSubWindow( viewer );
    if ( swin ) m_mdiArea->setActiveSubWindow( swin );
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
        // Find the reservoir view or the Plot that the selected item is within
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

        // If we cant find the view window as an MDI sub window, we search higher in the
        // project tree to find a possible parent view window that has.
        if ( selectedWindow && !findMdiSubWindow( selectedWindow->viewWidget() ) )
        {
            if ( selectedWindow->parentField() && selectedWindow->parentField()->ownerObject() )
            {
                selectedWindow = selectedWindow->parentField()->ownerObject()->firstAncestorOrThisOfType<RimViewWindow>();
            }
        }

        if ( selectedWindow )
        {
            if ( selectedWindow->viewWidget() )
            {
                setBlockViewSelectionOnSubWindowActivated( true );
                setActiveViewer( selectedWindow->viewWidget() );
                setBlockViewSelectionOnSubWindowActivated( false );
            }

            m_activePlotViewWindow = selectedWindow;

            if ( firstSelectedObject )
            {
                auto multiSummaryPlot = firstSelectedObject->firstAncestorOrThisOfType<RimSummaryMultiPlot>();
                if ( multiSummaryPlot )
                {
                    updateMultiPlotToolBar();

                    auto summaryPlot = firstSelectedObject->firstAncestorOrThisOfType<RimSummaryPlot>();
                    if ( summaryPlot )
                    {
                        multiSummaryPlot->makeSureIsVisible( summaryPlot );
                    }
                }
            }

            // The only way to get to this code is by selection change initiated from the project tree view
            // As we are activating an MDI-window, the focus is given to this MDI-window
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
bool RiuPlotMainWindow::isAnyMdiSubWindowVisible()
{
    return !m_mdiArea->subWindowList().empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotMainWindow::dragEnterEvent( QDragEnterEvent* event )
{
    QPoint curpos = m_mdiArea->mapFromGlobal( QCursor::pos() );

    if ( m_mdiArea->rect().contains( curpos ) ) event->acceptProposedAction();
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
QStringList RiuPlotMainWindow::windowsMenuFeatureNames()
{
    return { "RicTilePlotWindowsFeature", "RicTilePlotWindowsVerticallyFeature", "RicTilePlotWindowsHorizontallyFeature" };
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
