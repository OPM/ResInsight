/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RiuMainWindow.h"

#include "RiaBaseDefs.h"
#include "RiaGuiApplication.h"
#include "RiaPreferences.h"
#include "RiaRegressionTest.h"
#include "RiaRegressionTestRunner.h"

#include "Rim2dIntersectionView.h"
#include "Rim3dView.h"
#include "RimCellEdgeColors.h"
#include "RimCommandObject.h"
#include "RimEclipseCase.h"
#include "RimEclipseContourMapView.h"
#include "RimEclipseFaultColors.h"
#include "RimEclipsePropertyFilter.h"
#include "RimEclipseResultDefinition.h"
#include "RimEclipseView.h"
#include "RimFaultInViewCollection.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechView.h"
#include "RimIntersection.h"
#include "RimProject.h"
#include "RimSimWellInViewCollection.h"
#include "RimViewLinker.h"
#include "RimViewLinkerCollection.h"
#include "RimViewWindow.h"

#include "RiuDockWidgetTools.h"
#include "RiuDragDrop.h"
#include "RiuMdiSubWindow.h"
#include "RiuMessagePanel.h"
#include "RiuMohrsCirclePlot.h"
#include "RiuProcessMonitor.h"
#include "RiuProjectPropertyView.h"
#include "RiuPropertyViewTabWidget.h"
#include "RiuPvtPlotPanel.h"
#include "RiuRelativePermeabilityPlotPanel.h"
#include "RiuResultInfoPanel.h"
#include "RiuResultQwtPlot.h"
#include "RiuToolTipMenu.h"
#include "RiuTreeViewEventFilter.h"
#include "RiuViewer.h"

#include "cafAnimationToolBar.h"
#include "cafCmdExecCommandManager.h"
#include "cafCmdFeatureManager.h"
#include "cafMemoryInspector.h"
#include "cafPdmSettings.h"
#include "cafPdmUiPropertyView.h"
#include "cafPdmUiPropertyViewDialog.h"
#include "cafPdmUiTreeView.h"
#include "cafQTreeViewStateSerializer.h"
#include "cafSelectionManager.h"
#include "cafUtils.h"

#include "ExportCommands/RicSnapshotAllViewsToFileFeature.h"
#include "MeasurementCommands/RicToggleMeasurementModeFeature.h"
#include "SummaryPlotCommands/RicEditSummaryPlotFeature.h"
#include "SummaryPlotCommands/RicShowSummaryCurveCalculatorFeature.h"

#include "cvfTimer.h"

#include <QAction>
#include <QCloseEvent>
#include <QDir>
#include <QDockWidget>
#include <QLabel>
#include <QLayout>
#include <QMdiSubWindow>
#include <QMenuBar>
#include <QSpinBox>
#include <QStatusBar>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QTreeView>
#include <QUndoStack>

#include <QDebug>

#include <algorithm>

//==================================================================================================
///
/// \class RiuMainWindow
///
/// Contains our main window
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuMainWindow::RiuMainWindow()
    : m_pdmRoot( nullptr )
    , m_relPermPlotPanel( nullptr )
    , m_pvtPlotPanel( nullptr )
    , m_mohrsCirclePlot( nullptr )
    , m_windowMenu( nullptr )
    , m_holoLensToolBar( nullptr )
{
    m_mdiArea = new RiuMdiArea;
    connect( m_mdiArea, SIGNAL( subWindowActivated( QMdiSubWindow* ) ), SLOT( slotSubWindowActivated( QMdiSubWindow* ) ) );
    setCentralWidget( m_mdiArea );

    // m_mainViewer = createViewer();

    createActions();
    createMenus();
    createToolBars();
    createDockPanels();

    m_dragDropInterface = std::unique_ptr<caf::PdmUiDragDropInterface>( new RiuDragDrop() );

    initializeGuiNewProjectLoaded();

    // Enabling the line below will activate the undo stack
    // When enableUndoCommandSystem is set false, all commands are executed and deleted immediately
    // caf::CmdExecCommandManager::instance()->enableUndoCommandSystem(true);

    m_memoryCriticalWarning = new QLabel( "" );
    m_memoryUsedButton      = new QToolButton( nullptr );
    m_memoryTotalStatus     = new QLabel( "" );

    m_memoryUsedButton->setDefaultAction(
        caf::CmdFeatureManager::instance()->action( "RicShowMemoryCleanupDialogFeature" ) );

    statusBar()->addPermanentWidget( m_memoryCriticalWarning );
    statusBar()->addPermanentWidget( m_memoryUsedButton );
    statusBar()->addPermanentWidget( m_memoryTotalStatus );

    updateMemoryUsage();

    m_memoryRefreshTimer = new QTimer( this );
    connect( m_memoryRefreshTimer, SIGNAL( timeout() ), this, SLOT( updateMemoryUsage() ) );
    m_memoryRefreshTimer->start( 1000 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuMainWindow* RiuMainWindow::instance()
{
    if ( RiaGuiApplication::isRunning() )
    {
        return RiaGuiApplication::instance()->mainWindow();
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuMainWindow::mainWindowName()
{
    return "RiuMainWindow";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::initializeGuiNewProjectLoaded()
{
    setPdmRoot( RiaApplication::instance()->project() );
    restoreTreeViewState();

    if ( subWindowsAreTiled() )
    {
        tileSubWindows();
    }

    slotRefreshFileActions();
    slotRefreshEditActions();
    slotRefreshViewActions();
    refreshAnimationActions();
    refreshDrawStyleActions();

    if ( m_pdmUiPropertyView && m_pdmUiPropertyView->currentObject() )
    {
        m_pdmUiPropertyView->currentObject()->uiCapability()->updateConnectedEditors();
    }

    m_processMonitor->slotClearTextEdit();

    if ( statusBar() && !RiaRegressionTestRunner::instance()->isRunningRegressionTests() )
    {
        statusBar()->showMessage( "Ready ..." );
    }

    QMdiSubWindow* activeSubWindow = m_mdiArea->activeSubWindow();
    if ( activeSubWindow )
    {
        auto w = findViewWindowFromSubWindow( activeSubWindow );
        if ( w && w->mdiWindowGeometry().isMaximized )
        {
            activeSubWindow->showMaximized();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::cleanupGuiCaseClose()
{
    caf::CmdExecCommandManager::instance()->undoStack()->clear();

    setResultInfo( "" );

    m_resultQwtPlot->deleteAllCurves();
    if ( m_relPermPlotPanel ) m_relPermPlotPanel->clearPlot();
    if ( m_pvtPlotPanel ) m_pvtPlotPanel->clearPlot();
    if ( m_mohrsCirclePlot ) m_mohrsCirclePlot->clearPlot();

    if ( m_pdmUiPropertyView )
    {
        m_pdmUiPropertyView->showProperties( nullptr );
    }

    for ( auto& additionalProjectView : m_additionalProjectViews )
    {
        RiuProjectAndPropertyView* projPropView = dynamic_cast<RiuProjectAndPropertyView*>(
            additionalProjectView->widget() );
        if ( projPropView )
        {
            projPropView->showProperties( nullptr );
        }
    }
    m_processMonitor->startMonitorWorkProcess( nullptr );

    RicEditSummaryPlotFeature* editSumCurves = dynamic_cast<RicEditSummaryPlotFeature*>(
        caf::CmdFeatureManager::instance()->getCommandFeature( "RicEditSummaryPlotFeature" ) );
    if ( editSumCurves )
    {
        editSumCurves->closeDialogAndResetTargetPlot();
    }

    RicShowSummaryCurveCalculatorFeature::hideCurveCalculatorDialog();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::cleanupGuiBeforeProjectClose()
{
    setPdmRoot( nullptr );

    cleanupGuiCaseClose();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::closeEvent( QCloseEvent* event )
{
    this->saveWinGeoAndDockToolBarLayout();

    RiaGuiApplication* app = RiaGuiApplication::instance();
    if ( app->isMainPlotWindowVisible() )
    {
        event->ignore(); // Make Qt think we don't do anything, otherwise it closes the window.
        this->hide(); // Instead we just hide it.
        return;
    }

    if ( !app->askUserToSaveModifiedProject() )
    {
        event->ignore();
        return;
    }

    this->hideAllDockWidgets();
    app->closeMainPlotWindowIfOpenButHidden();
    app->closeProject();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::createActions()
{
    // File actions
    m_mockModelAction             = new QAction( "&Mock Model", this );
    m_mockResultsModelAction      = new QAction( "Mock Model With &Results", this );
    m_mockLargeResultsModelAction = new QAction( "Large Mock Model", this );
    m_mockModelCustomizedAction   = new QAction( "Customized Mock Model", this );
    m_mockInputModelAction        = new QAction( "Input Mock Model", this );

    m_snapshotAllViewsToFile = new QAction( QIcon( ":/SnapShotSaveViews.png" ), "Snapshot All Views To File", this );

    m_createCommandObject              = new QAction( "Create Command Object", this );
    m_showRegressionTestDialog         = new QAction( "Regression Test Dialog", this );
    m_executePaintEventPerformanceTest = new QAction( "&Paint Event Performance Test", this );

    connect( m_mockModelAction, SIGNAL( triggered() ), SLOT( slotMockModel() ) );
    connect( m_mockResultsModelAction, SIGNAL( triggered() ), SLOT( slotMockResultsModel() ) );
    connect( m_mockLargeResultsModelAction, SIGNAL( triggered() ), SLOT( slotMockLargeResultsModel() ) );
    connect( m_mockModelCustomizedAction, SIGNAL( triggered() ), SLOT( slotMockModelCustomized() ) );
    connect( m_mockInputModelAction, SIGNAL( triggered() ), SLOT( slotInputMockModel() ) );

    connect( m_snapshotAllViewsToFile, SIGNAL( triggered() ), SLOT( slotSnapshotAllViewsToFile() ) );

    connect( m_createCommandObject, SIGNAL( triggered() ), SLOT( slotCreateCommandObject() ) );
    connect( m_showRegressionTestDialog, SIGNAL( triggered() ), SLOT( slotShowRegressionTestDialog() ) );
    connect( m_executePaintEventPerformanceTest, SIGNAL( triggered() ), SLOT( slotExecutePaintEventPerformanceTest() ) );

    // View actions
    m_viewFromNorth = new QAction( QIcon( ":/SouthViewArrow.png" ), "Look South", this );
    m_viewFromNorth->setToolTip( "Look South (Ctrl+Alt+S)" );
    m_viewFromNorth->setShortcut( QKeySequence( tr( "Ctrl+Alt+S" ) ) );

    m_viewFromSouth = new QAction( QIcon( ":/NorthViewArrow.png" ), "Look North", this );
    m_viewFromSouth->setToolTip( "Look North (Ctrl+Alt+N)" );
    m_viewFromSouth->setShortcut( QKeySequence( tr( "Ctrl+Alt+N" ) ) );

    m_viewFromEast = new QAction( QIcon( ":/WestViewArrow.png" ), "Look West", this );
    m_viewFromEast->setToolTip( "Look West (Ctrl+Alt+W)" );
    m_viewFromEast->setShortcut( QKeySequence( tr( "Ctrl+Alt+W" ) ) );

    m_viewFromWest = new QAction( QIcon( ":/EastViewArrow.png" ), "Look East", this );
    m_viewFromWest->setToolTip( "Look East (Ctrl+Alt+E)" );
    m_viewFromWest->setShortcut( QKeySequence( tr( "Ctrl+Alt+E" ) ) );

    m_viewFromAbove = new QAction( QIcon( ":/DownViewArrow.png" ), "Look Down", this );
    m_viewFromAbove->setToolTip( "Look Down (Ctrl+Alt+D)" );
    m_viewFromAbove->setShortcut( QKeySequence( tr( "Ctrl+Alt+D" ) ) );

    m_viewFromBelow = new QAction( QIcon( ":/UpViewArrow.png" ), "Look Up", this );
    m_viewFromBelow->setToolTip( "Look Up (Ctrl+Alt+U)" );
    m_viewFromBelow->setShortcut( QKeySequence( tr( "Ctrl+Alt+U" ) ) );

    connect( m_viewFromNorth, SIGNAL( triggered() ), SLOT( slotViewFromNorth() ) );
    connect( m_viewFromSouth, SIGNAL( triggered() ), SLOT( slotViewFromSouth() ) );
    connect( m_viewFromEast, SIGNAL( triggered() ), SLOT( slotViewFromEast() ) );
    connect( m_viewFromWest, SIGNAL( triggered() ), SLOT( slotViewFromWest() ) );
    connect( m_viewFromAbove, SIGNAL( triggered() ), SLOT( slotViewFromAbove() ) );
    connect( m_viewFromBelow, SIGNAL( triggered() ), SLOT( slotViewFromBelow() ) );

    // Debug actions
    m_newPropertyView = new QAction( "New Project and Property View", this );
    connect( m_newPropertyView, SIGNAL( triggered() ), SLOT( slotNewObjectPropertyView() ) );

    // Draw style actions
    m_dsActionGroup = new QActionGroup( this );

    m_drawStyleLinesAction = new QAction( QIcon( ":/draw_style_lines_24x24.png" ), "&Mesh Only", this );
    // connect(m_drawStyleLinesAction,        SIGNAL(triggered()), SLOT(slotDrawStyleLines()));
    m_dsActionGroup->addAction( m_drawStyleLinesAction );

    m_drawStyleLinesSolidAction = new QAction( QIcon( ":/draw_style_meshlines_24x24.png" ), "Mesh And Surfaces", this );
    // connect(m_drawStyleLinesSolidAction,    SIGNAL(triggered()), SLOT(slotDrawStyleLinesSolid()));
    m_dsActionGroup->addAction( m_drawStyleLinesSolidAction );

    m_drawStyleFaultLinesSolidAction = new QAction( QIcon( ":/draw_style_surface_w_fault_mesh_24x24.png" ),
                                                    "Fault Mesh And Surfaces",
                                                    this );
    m_dsActionGroup->addAction( m_drawStyleFaultLinesSolidAction );

    m_drawStyleSurfOnlyAction = new QAction( QIcon( ":/draw_style_surface_24x24.png" ), "&Surface Only", this );
    // connect(m_drawStyleSurfOnlyAction,    SIGNAL(triggered()), SLOT(slotDrawStyleSurfOnly()));
    m_dsActionGroup->addAction( m_drawStyleSurfOnlyAction );

    connect( m_dsActionGroup, SIGNAL( triggered( QAction* ) ), SLOT( slotDrawStyleChanged( QAction* ) ) );

    m_disableLightingAction = new QAction( QIcon( ":/disable_lighting_24x24.png" ), "&Disable Results Lighting", this );
    m_disableLightingAction->setCheckable( true );
    connect( m_disableLightingAction, SIGNAL( toggled( bool ) ), SLOT( slotDisableLightingAction( bool ) ) );

    m_drawStyleHideGridCellsAction = new QAction( QIcon( ":/draw_style_faults_24x24.png" ), "&Hide Grid Cells", this );
    m_drawStyleHideGridCellsAction->setCheckable( true );
    connect( m_drawStyleHideGridCellsAction, SIGNAL( toggled( bool ) ), SLOT( slotToggleHideGridCellsAction( bool ) ) );

    m_toggleFaultsLabelAction = new QAction( QIcon( ":/draw_style_faults_label_24x24.png" ), "&Show Fault Labels", this );
    m_toggleFaultsLabelAction->setCheckable( true );
    connect( m_toggleFaultsLabelAction, SIGNAL( toggled( bool ) ), SLOT( slotToggleFaultLabelsAction( bool ) ) );

    m_showWellCellsAction = new QAction( QIcon( ":/draw_style_WellCellsToRangeFilter_24x24.png" ),
                                         "&Show Well Cells",
                                         this );
    m_showWellCellsAction->setCheckable( true );
    m_showWellCellsAction->setToolTip( "Show Well Cells" );
    connect( m_showWellCellsAction, SIGNAL( toggled( bool ) ), SLOT( slotShowWellCellsAction( bool ) ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::createMenus()
{
    caf::CmdFeatureManager* cmdFeatureMgr = caf::CmdFeatureManager::instance();
    CVF_ASSERT( cmdFeatureMgr );

    // File menu
    QMenu* fileMenu = new RiuToolTipMenu( menuBar() );
    fileMenu->setTitle( "&File" );

    menuBar()->addMenu( fileMenu );

    fileMenu->addAction( cmdFeatureMgr->action( "RicOpenProjectFeature" ) );
    fileMenu->addAction( cmdFeatureMgr->action( "RicOpenLastUsedFileFeature" ) );
    fileMenu->addSeparator();

    QMenu* importMenu = fileMenu->addMenu( "&Import" );

    QMenu* importEclipseMenu = importMenu->addMenu( QIcon( ":/Case48x48.png" ), "Eclipse Cases" );
    importEclipseMenu->addAction( cmdFeatureMgr->action( "RicImportEclipseCaseFeature" ) );
    importEclipseMenu->addAction( cmdFeatureMgr->action( "RicImportEclipseCasesFeature" ) );
    importEclipseMenu->addAction( cmdFeatureMgr->action( "RicImportEclipseCaseTimeStepFilterFeature" ) );
    importEclipseMenu->addAction( cmdFeatureMgr->action( "RicImportInputEclipseCaseFeature" ) );
    importEclipseMenu->addAction( cmdFeatureMgr->action( "RicCreateGridCaseGroupFeature" ) );
    importEclipseMenu->addAction( cmdFeatureMgr->action( "RicCreateGridCaseGroupFromFilesFeature" ) );

    importMenu->addSeparator();
    QMenu* importSummaryMenu = importMenu->addMenu( QIcon( ":/SummaryCase48x48.png" ), "Summary Cases" );
    importSummaryMenu->addAction( cmdFeatureMgr->action( "RicImportSummaryCaseFeature" ) );
    importSummaryMenu->addAction( cmdFeatureMgr->action( "RicImportSummaryCasesFeature" ) );
    importSummaryMenu->addAction( cmdFeatureMgr->action( "RicImportSummaryGroupFeature" ) );
    importSummaryMenu->addAction( cmdFeatureMgr->action( "RicImportEnsembleFeature" ) );

#ifdef USE_ODB_API
    importMenu->addSeparator();
    QMenu* importGeoMechMenu = importMenu->addMenu( QIcon( ":/GeoMechCase24x24.png" ), "Geo Mechanical Cases" );
    importGeoMechMenu->addAction( cmdFeatureMgr->action( "RicImportGeoMechCaseFeature" ) );
    importGeoMechMenu->addAction( cmdFeatureMgr->action( "RicImportGeoMechCaseTimeStepFilterFeature" ) );
    importGeoMechMenu->addAction( cmdFeatureMgr->action( "RicImportElementPropertyFeature" ) );
#endif

    importMenu->addSeparator();
    QMenu* importWellMenu = importMenu->addMenu( QIcon( ":/Well.png" ), "Well Data" );
    importWellMenu->addAction( cmdFeatureMgr->action( "RicWellPathsImportFileFeature" ) );
    importWellMenu->addAction( cmdFeatureMgr->action( "RicWellPathsImportSsihubFeature" ) );
    importWellMenu->addAction( cmdFeatureMgr->action( "RicWellLogsImportFileFeature" ) );
    importWellMenu->addAction( cmdFeatureMgr->action( "RicWellPathFormationsImportFileFeature" ) );

    importMenu->addSeparator();
    importMenu->addAction( cmdFeatureMgr->action( "RicImportObservedDataInMenuFeature" ) );
    importMenu->addAction( cmdFeatureMgr->action( "RicImportObservedFmuDataInMenuFeature" ) );
    importMenu->addAction( cmdFeatureMgr->action( "RicImportFormationNamesFeature" ) );

    QMenu* exportMenu = fileMenu->addMenu( "&Export" );
    exportMenu->addAction( cmdFeatureMgr->action( "RicSnapshotViewToFileFeature" ) );
    exportMenu->addAction( m_snapshotAllViewsToFile );
    exportMenu->addAction( cmdFeatureMgr->action( "RicAdvancedSnapshotExportFeature" ) );
    exportMenu->addSeparator();
    exportMenu->addAction( cmdFeatureMgr->action( "RicExportEclipseInputGridFeature" ) );
    exportMenu->addAction( cmdFeatureMgr->action( "RicSaveEclipseInputActiveVisibleCellsFeature" ) );
    exportMenu->addAction( cmdFeatureMgr->action( "RicExportCompletionsForVisibleWellPathsFeature" ) );
    exportMenu->addAction( cmdFeatureMgr->action( "RicExportVisibleWellPathsFeature" ) );

    fileMenu->addSeparator();
    fileMenu->addAction( cmdFeatureMgr->action( "RicSaveProjectFeature" ) );
    fileMenu->addAction( cmdFeatureMgr->action( "RicSaveProjectAsFeature" ) );

    std::vector<QAction*> recentFileActions = RiaGuiApplication::instance()->recentFileActions();
    for ( auto act : recentFileActions )
    {
        fileMenu->addAction( act );
    }

    fileMenu->addSeparator();
    QMenu* testMenu = fileMenu->addMenu( "&Testing" );

    fileMenu->addSeparator();
    fileMenu->addAction( cmdFeatureMgr->action( "RicCloseProjectFeature" ) );
    fileMenu->addSeparator();
    fileMenu->addAction( cmdFeatureMgr->action( "RicExitApplicationFeature" ) );

    connect( fileMenu, SIGNAL( aboutToShow() ), SLOT( slotRefreshFileActions() ) );

    // Edit menu
    QMenu* editMenu = menuBar()->addMenu( "&Edit" );
    editMenu->addAction( cmdFeatureMgr->action( "RicSnapshotViewToClipboardFeature" ) );
    editMenu->addSeparator();
    editMenu->addAction( cmdFeatureMgr->action( "RicShowMemoryCleanupDialogFeature" ) );
    editMenu->addSeparator();
    editMenu->addAction( cmdFeatureMgr->action( "RicEditPreferencesFeature" ) );

    connect( editMenu, SIGNAL( aboutToShow() ), SLOT( slotRefreshEditActions() ) );

    // View menu
    QMenu* viewMenu = menuBar()->addMenu( "&View" );
    viewMenu->addAction( cmdFeatureMgr->action( "RicViewZoomAllFeature" ) );
    viewMenu->addSeparator();
    viewMenu->addAction( m_viewFromSouth );
    viewMenu->addAction( m_viewFromNorth );
    viewMenu->addAction( m_viewFromWest );
    viewMenu->addAction( m_viewFromEast );
    viewMenu->addAction( m_viewFromBelow );
    viewMenu->addAction( m_viewFromAbove );

    connect( viewMenu, SIGNAL( aboutToShow() ), SLOT( slotRefreshViewActions() ) );

    // Debug menu
    testMenu->addAction( m_mockModelAction );
    testMenu->addAction( m_mockResultsModelAction );
    testMenu->addAction( m_mockLargeResultsModelAction );
    testMenu->addAction( m_mockModelCustomizedAction );
    testMenu->addAction( m_mockInputModelAction );
    testMenu->addSeparator();
    testMenu->addAction( m_createCommandObject );
    testMenu->addSeparator();
    testMenu->addAction( m_showRegressionTestDialog );
    testMenu->addAction( m_executePaintEventPerformanceTest );
    testMenu->addAction( cmdFeatureMgr->action( "RicLaunchUnitTestsFeature" ) );
    testMenu->addAction( cmdFeatureMgr->action( "RicRunCommandFileFeature" ) );
    testMenu->addSeparator();

    testMenu->addAction( cmdFeatureMgr->action( "RicHoloLensExportToFolderFeature" ) );
    testMenu->addAction( cmdFeatureMgr->action( "RicHoloLensCreateDummyFiledBackedSessionFeature" ) );

    // Windows menu
    m_windowMenu = menuBar()->addMenu( "&Windows" );
    connect( m_windowMenu, SIGNAL( aboutToShow() ), SLOT( slotBuildWindowActions() ) );

    // Help menu
    QMenu* helpMenu = menuBar()->addMenu( "&Help" );
    helpMenu->addAction( cmdFeatureMgr->action( "RicHelpAboutFeature" ) );
    helpMenu->addAction( cmdFeatureMgr->action( "RicHelpCommandLineFeature" ) );
    helpMenu->addAction( cmdFeatureMgr->action( "RicHelpSummaryCommandLineFeature" ) );
    helpMenu->addSeparator();
    helpMenu->addAction( cmdFeatureMgr->action( "RicHelpOpenUsersGuideFeature" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::createToolBars()
{
    caf::CmdFeatureManager* cmdFeatureMgr = caf::CmdFeatureManager::instance();
    CVF_ASSERT( cmdFeatureMgr );

    {
        QToolBar* toolbar = addToolBar( tr( "Standard" ) );
        toolbar->setObjectName( toolbar->windowTitle() );
        toolbar->addAction( cmdFeatureMgr->action( "RicImportGeneralDataFeature" ) );
        toolbar->addAction( cmdFeatureMgr->action( "RicOpenProjectFeature" ) );
        toolbar->addAction( cmdFeatureMgr->action( "RicSaveProjectFeature" ) );
    }

    {
        QToolBar* toolbar = addToolBar( tr( "Import" ) );
        toolbar->setObjectName( toolbar->windowTitle() );
        toolbar->addAction( cmdFeatureMgr->action( "RicImportEclipseCaseFeature" ) );
        toolbar->addAction( cmdFeatureMgr->action( "RicImportInputEclipseCaseFeature" ) );
        toolbar->addAction( cmdFeatureMgr->action( "RicImportSummaryCaseFeature" ) );
        toolbar->addAction( cmdFeatureMgr->action( "RicImportEnsembleFeature" ) );
        toolbar->hide();
    }
    {
#ifdef USE_ODB_API
        QToolBar* toolbar = addToolBar( tr( "Import GeoMech" ) );
        toolbar->setObjectName( toolbar->windowTitle() );
        toolbar->addAction( cmdFeatureMgr->action( "RicImportGeoMechCaseFeature" ) );
        toolbar->addAction( cmdFeatureMgr->action( "RicImportGeoMechCaseTimeStepFilterFeature" ) );
        toolbar->addAction( cmdFeatureMgr->action( "RicImportElementPropertyFeature" ) );
        toolbar->hide();
#endif
    }

    {
        QToolBar* toolbar = addToolBar( tr( "Window Management" ) );
        toolbar->setObjectName( toolbar->windowTitle() );
        toolbar->addAction( cmdFeatureMgr->action( "RicShowPlotWindowFeature" ) );
        toolbar->addAction( cmdFeatureMgr->action( "RicLinkVisibleViewsFeature" ) );
        toolbar->addAction( cmdFeatureMgr->action( "RicTileWindowsFeature" ) );
    }

    {
        QToolBar* toolbar = addToolBar( tr( "View Snapshots" ) );
        toolbar->setObjectName( toolbar->windowTitle() );
        toolbar->addAction( cmdFeatureMgr->action( "RicSnapshotViewToClipboardFeature" ) );
        toolbar->addAction( cmdFeatureMgr->action( "RicSnapshotViewToFileFeature" ) );
        toolbar->addAction( m_snapshotAllViewsToFile );

        toolbar->hide();
    }

    // View toolbar
    {
        QToolBar* toolbar = addToolBar( tr( "View" ) );
        toolbar->setObjectName( toolbar->windowTitle() );
        toolbar->addAction( cmdFeatureMgr->action( "RicTogglePerspectiveViewFeature" ) );
        toolbar->addAction( cmdFeatureMgr->action( "RicViewZoomAllFeature" ) );
        toolbar->addAction( m_viewFromNorth );
        toolbar->addAction( m_viewFromSouth );
        toolbar->addAction( m_viewFromEast );
        toolbar->addAction( m_viewFromWest );
        toolbar->addAction( m_viewFromAbove );
        toolbar->addAction( m_viewFromBelow );

        QLabel* scaleLabel = new QLabel( toolbar );
        scaleLabel->setText( "Scale" );
        toolbar->addWidget( scaleLabel );

        m_scaleFactor = new QSpinBox( toolbar );
        m_scaleFactor->setValue( 0 );
        toolbar->addWidget( m_scaleFactor );
        connect( m_scaleFactor, SIGNAL( valueChanged( int ) ), SLOT( slotScaleChanged( int ) ) );
    }

    {
        QToolBar* dsToolBar = addToolBar( tr( "Draw Style" ) );
        dsToolBar->setObjectName( dsToolBar->windowTitle() );
        dsToolBar->addAction( m_drawStyleLinesAction );
        dsToolBar->addAction( m_drawStyleLinesSolidAction );
        dsToolBar->addAction( m_drawStyleSurfOnlyAction );
        dsToolBar->addAction( m_drawStyleFaultLinesSolidAction );
        dsToolBar->addAction( m_disableLightingAction );
        dsToolBar->addAction( m_drawStyleHideGridCellsAction );
        dsToolBar->addAction( m_toggleFaultsLabelAction );
        dsToolBar->addAction( m_showWellCellsAction );
    }

    {
        m_holoLensToolBar = addToolBar( tr( "HoloLens" ) );
        m_holoLensToolBar->setObjectName( m_holoLensToolBar->windowTitle() );

        m_holoLensToolBar->addAction( cmdFeatureMgr->action( "RicHoloLensCreateSessionFeature" ) );
        m_holoLensToolBar->addAction( cmdFeatureMgr->action( "RicHoloLensTerminateSessionFeature" ) );
        m_holoLensToolBar->addAction( cmdFeatureMgr->action( "RicHoloLensAutoExportToSharingServerFeature" ) );
        m_holoLensToolBar->addAction( cmdFeatureMgr->action( "RicHoloLensExportToSharingServerFeature" ) );
    }

    {
        QToolBar* toolbar = addToolBar( tr( "Measurement" ) );
        toolbar->setObjectName( toolbar->windowTitle() );
        auto measureAction = cmdFeatureMgr->action( "RicToggleMeasurementModeFeature" );
        toolbar->addAction( measureAction );
        auto polyMeasureAction = cmdFeatureMgr->action( "RicTogglePolyMeasurementModeFeature" );
        toolbar->addAction( polyMeasureAction );
    }

    RiaApplication* app = RiaApplication::instance();
    if ( app->preferences()->showTestToolbar() )
    {
        QToolBar* toolbar = addToolBar( tr( "Test" ) );
        toolbar->setObjectName( toolbar->windowTitle() );
        toolbar->addAction( cmdFeatureMgr->action( "RicLaunchUnitTestsFeature" ) );
        toolbar->addAction( cmdFeatureMgr->action( "RicLaunchRegressionTestsFeature" ) );
        toolbar->addAction( cmdFeatureMgr->action( "RicRunCommandFileFeature" ) );
    }

    // Create animation toolbar
    m_animationToolBar = new caf::AnimationToolBar( "Animation", this );
    addToolBar( m_animationToolBar );

    refreshAnimationActions();
    refreshDrawStyleActions();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::createDockPanels()
{
    {
        QDockWidget* dockWidget = new QDockWidget( "Project Tree", this );
        dockWidget->setObjectName( RiuDockWidgetTools::projectTreeName() );
        dockWidget->setAllowedAreas( Qt::AllDockWidgetAreas );

        m_projectTreeView = new caf::PdmUiTreeView( this );
        m_projectTreeView->enableSelectionManagerUpdating( true );

        RiaApplication* app = RiaApplication::instance();
        m_projectTreeView->enableAppendOfClassNameToUiItemText( app->preferences()->appendClassNameToUiText() );

        dockWidget->setWidget( m_projectTreeView );

        m_projectTreeView->treeView()->setHeaderHidden( true );
        m_projectTreeView->treeView()->setSelectionMode( QAbstractItemView::ExtendedSelection );

        // Drag and drop configuration
        m_projectTreeView->treeView()->setDragEnabled( true );
        m_projectTreeView->treeView()->viewport()->setAcceptDrops( true );
        m_projectTreeView->treeView()->setDropIndicatorShown( true );
        m_projectTreeView->treeView()->setDragDropMode( QAbstractItemView::DragDrop );

        // Install event filter used to handle key press events
        RiuTreeViewEventFilter* treeViewEventFilter = new RiuTreeViewEventFilter( this );
        m_projectTreeView->treeView()->installEventFilter( treeViewEventFilter );

        addDockWidget( Qt::LeftDockWidgetArea, dockWidget );

        connect( m_projectTreeView, SIGNAL( selectionChanged() ), this, SLOT( selectedObjectsChanged() ) );
        m_projectTreeView->treeView()->setContextMenuPolicy( Qt::CustomContextMenu );
        connect( m_projectTreeView->treeView(),
                 SIGNAL( customContextMenuRequested( const QPoint& ) ),
                 SLOT( customMenuRequested( const QPoint& ) ) );
    }

    QDockWidget* resultPlotDock  = nullptr;
    QDockWidget* relPermPlotDock = nullptr;
    QDockWidget* pvtPlotDock     = nullptr;
#ifdef USE_ODB_API
    QDockWidget* mohrsCirclePlotDock = nullptr;
#endif

    {
        QDockWidget* dockWidget = new QDockWidget( "Property Editor", this );
        dockWidget->setObjectName( RiuDockWidgetTools::propertyEditorName() );
        dockWidget->setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );

        m_pdmUiPropertyView = new caf::PdmUiPropertyView( dockWidget );
        dockWidget->setWidget( m_pdmUiPropertyView );

        addDockWidget( Qt::LeftDockWidgetArea, dockWidget );
    }

    {
        QDockWidget* dockWidget = new QDockWidget( "Result Info", this );
        dockWidget->setObjectName( RiuDockWidgetTools::resultInfoName() );
        dockWidget->setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea );
        m_resultInfoPanel = new RiuResultInfoPanel( dockWidget );
        dockWidget->setWidget( m_resultInfoPanel );

        addDockWidget( Qt::BottomDockWidgetArea, dockWidget );
    }

    {
        QDockWidget* dockWidget = new QDockWidget( "Process Monitor", this );
        dockWidget->setObjectName( RiuDockWidgetTools::processMonitorName() );
        dockWidget->setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea );
        m_processMonitor = new RiuProcessMonitor( dockWidget );
        dockWidget->setWidget( m_processMonitor );

        addDockWidget( Qt::BottomDockWidgetArea, dockWidget );
        dockWidget->hide();
    }

    {
        QDockWidget* dockWidget = new QDockWidget( "Result Plot", this );
        dockWidget->setObjectName( RiuDockWidgetTools::resultPlotName() );
        dockWidget->setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea );
        m_resultQwtPlot = new RiuResultQwtPlot( dockWidget );
        dockWidget->setWidget( m_resultQwtPlot );

        addDockWidget( Qt::BottomDockWidgetArea, dockWidget );
        resultPlotDock = dockWidget;
    }

#ifdef USE_ODB_API
    {
        QDockWidget* dockWidget = new QDockWidget( "Mohr's Circle Plot", this );
        dockWidget->setObjectName( RiuDockWidgetTools::mohrsCirclePlotName() );
        dockWidget->setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea );
        m_mohrsCirclePlot = new RiuMohrsCirclePlot( dockWidget );
        dockWidget->setWidget( m_mohrsCirclePlot );

        addDockWidget( Qt::BottomDockWidgetArea, dockWidget );
        mohrsCirclePlotDock = dockWidget;

        dockWidget->hide();
    }
#endif

    {
        QDockWidget* dockWidget = new QDockWidget( "Relative Permeability Plot", this );
        dockWidget->setObjectName( RiuDockWidgetTools::relPermPlotName() );
        dockWidget->setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea );
        m_relPermPlotPanel = new RiuRelativePermeabilityPlotPanel( dockWidget );
        dockWidget->setWidget( m_relPermPlotPanel );

        addDockWidget( Qt::BottomDockWidgetArea, dockWidget );
        relPermPlotDock = dockWidget;
    }

    {
        QDockWidget* dockWidget = new QDockWidget( "PVT Plot", this );
        dockWidget->setObjectName( RiuDockWidgetTools::pvtPlotName() );
        dockWidget->setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea );
        m_pvtPlotPanel = new RiuPvtPlotPanel( dockWidget );
        dockWidget->setWidget( m_pvtPlotPanel );

        addDockWidget( Qt::BottomDockWidgetArea, dockWidget );
        pvtPlotDock = dockWidget;
    }

    {
        QDockWidget* dockWidget = new QDockWidget( "Messages", this );
        dockWidget->setObjectName( RiuDockWidgetTools::messagesName() );
        m_messagePanel = new RiuMessagePanel( dockWidget );
        dockWidget->setWidget( m_messagePanel );
        addDockWidget( Qt::BottomDockWidgetArea, dockWidget );
        dockWidget->hide();
    }

    setCorner( Qt::BottomLeftCorner, Qt::LeftDockWidgetArea );
    setCorner( Qt::BottomRightCorner, Qt::BottomDockWidgetArea );

    // Tabify docks
    tabifyDockWidget( pvtPlotDock, relPermPlotDock );
#ifdef USE_ODB_API
    tabifyDockWidget( relPermPlotDock, mohrsCirclePlotDock );
    tabifyDockWidget( mohrsCirclePlotDock, resultPlotDock );
#else
    tabifyDockWidget( relPermPlotDock, resultPlotDock );
#endif

    QList<QDockWidget*> dockWidgets = findChildren<QDockWidget*>();
    for ( QDockWidget* dock : dockWidgets )
    {
        connect( dock->toggleViewAction(), SIGNAL( triggered() ), SLOT( slotDockWidgetToggleViewActionTriggered() ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::setResultInfo( const QString& info ) const
{
    m_resultInfoPanel->setInfo( info );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::refreshViewActions()
{
    this->slotRefreshViewActions();
}

//==================================================================================================
//
// Action slots
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotRefreshFileActions()
{
    caf::CmdFeatureManager* cmdFeatureMgr = caf::CmdFeatureManager::instance();
    CVF_ASSERT( cmdFeatureMgr );

    QStringList commandIdList;
    commandIdList << "RicExportEclipseInputGridFeature";
    commandIdList << "RicSaveEclipseInputVisibleCellsFeature";
    commandIdList << "RicSaveEclipseInputActiveVisibleCellsFeature";
    commandIdList << "RicExportCompletionsForVisibleWellPathsFeature";
    commandIdList << "RicExportVisibleWellPathsFeature";
    cmdFeatureMgr->refreshStates( commandIdList );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotRefreshEditActions()
{
    //     RiaApplication* app = RiaApplication::instance();
    //     RISceneManager* proj = app->project();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotRefreshViewActions()
{
    RimGridView*              gridView = RiaApplication::instance()->activeGridView();
    RimEclipseContourMapView* view2d   = dynamic_cast<RimEclipseContourMapView*>( gridView );
    bool                      enabled  = gridView != nullptr && view2d == nullptr;
    m_viewFromNorth->setEnabled( enabled );
    m_viewFromSouth->setEnabled( enabled );
    m_viewFromEast->setEnabled( enabled );
    m_viewFromWest->setEnabled( enabled );
    m_viewFromAbove->setEnabled( enabled );
    m_viewFromBelow->setEnabled( enabled );

    updateScaleValue();

    {
        QStringList commandIds;
        commandIds << "RicLinkVisibleViewsFeature"
                   << "RicTileWindowsFeature"
                   << "RicTogglePerspectiveViewFeature"
                   << "RicViewZoomAllFeature";

        caf::CmdFeatureManager::instance()->refreshEnabledState( commandIds );
    }

    {
        QStringList commandIds;
        commandIds << "RicTileWindowsFeature";
        commandIds << "RicToggleMeasurementModeFeature";
        commandIds << "RicTogglePolyMeasurementModeFeature";

        caf::CmdFeatureManager::instance()->refreshCheckedState( commandIds );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::refreshAnimationActions()
{
    caf::FrameAnimationControl* animationControl = nullptr;
    Rim3dView*                  activeView       = RiaApplication::instance()->activeReservoirView();

    if ( activeView && activeView->viewer() )
    {
        animationControl = activeView->viewer()->animationControl();
    }

    m_animationToolBar->connectAnimationControl( animationControl );

    QStringList timeStepStrings;

    int currentTimeStepIndex = 0;

    bool enableAnimControls = false;
    if ( activeView && activeView->viewer() && activeView->viewer()->frameCount() )
    {
        enableAnimControls = true;

        if ( activeView->isTimeStepDependentDataVisibleInThisOrComparisonView() )
        {
            timeStepStrings = activeView->ownerCase()->timeStepStrings();
        }
        else
        {
            RimEclipseView* activeRiv = dynamic_cast<RimEclipseView*>( activeView );
            if ( activeRiv && activeRiv->currentGridCellResults() )
            {
                timeStepStrings.push_back( tr( "Static Property" ) );
            }
        }

        currentTimeStepIndex = activeView->currentTimeStep();

        // Animation control is only relevant for more than one time step

        if ( timeStepStrings.size() < 2 )
        {
            enableAnimControls = false;
        }

        m_animationToolBar->setFrameRate( activeView->maximumFrameRate() );
    }

    m_animationToolBar->setTimeStepStrings( timeStepStrings );
    m_animationToolBar->setCurrentTimeStepIndex( currentTimeStepIndex );

    m_animationToolBar->setEnabled( enableAnimControls );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotMockModel()
{
    RiaApplication* app = RiaApplication::instance();
    app->createMockModel();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotMockResultsModel()
{
    RiaApplication* app = RiaApplication::instance();
    app->createResultsMockModel();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotMockLargeResultsModel()
{
    RiaApplication* app = RiaApplication::instance();
    app->createLargeResultsMockModel();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotMockModelCustomized()
{
    RiaApplication* app = RiaApplication::instance();
    app->createMockModelCustomized();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotInputMockModel()
{
    RiaApplication* app = RiaApplication::instance();
    app->createInputMockModel();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QMdiSubWindow* RiuMainWindow::findMdiSubWindow( QWidget* viewer )
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
RimViewWindow* RiuMainWindow::findViewWindowFromSubWindow( QMdiSubWindow* subWindow )
{
    if ( subWindow )
    {
        std::vector<RimViewWindow*> allViewWindows;
        RiaApplication::instance()->project()->descendantsIncludingThisOfType( allViewWindows );

        for ( RimViewWindow* viewWindow : allViewWindows )
        {
            if ( viewWindow->viewWidget() == subWindow->widget() )
            {
                return viewWindow;
            }
        }
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<QMdiSubWindow*> RiuMainWindow::subWindowList( QMdiArea::WindowOrder order )
{
    return m_mdiArea->subWindowList( order );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuResultQwtPlot* RiuMainWindow::resultPlot()
{
    return m_resultQwtPlot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuRelativePermeabilityPlotPanel* RiuMainWindow::relativePermeabilityPlotPanel()
{
    return m_relPermPlotPanel;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPvtPlotPanel* RiuMainWindow::pvtPlotPanel()
{
    return m_pvtPlotPanel;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuMohrsCirclePlot* RiuMainWindow::mohrsCirclePlot()
{
    return m_mohrsCirclePlot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuMessagePanel* RiuMainWindow::messagePanel()
{
    return m_messagePanel;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::removeViewer( QWidget* viewer )
{
    removeViewerFromMdiArea( m_mdiArea, viewer );
    slotRefreshViewActions();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::addViewer( QWidget* viewer, const RimMdiWindowGeometry& windowsGeometry )
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

    addViewerToMdiArea( m_mdiArea, viewer, subWindowPos, subWindowSize );

    slotRefreshViewActions();
}

//--------------------------------------------------------------------------------------------------
/// This method needs to handle memory deallocation !!!
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::setPdmRoot( caf::PdmObject* pdmRoot )
{
    m_pdmRoot = pdmRoot;

    m_projectTreeView->setPdmItem( pdmRoot );
    // For debug only : m_projectTreeView->treeView()->expandAll();
    m_projectTreeView->setDragDropInterface( m_dragDropInterface.get() );

    for ( auto& additionalProjectView : m_additionalProjectViews )
    {
        if ( !additionalProjectView ) continue;

        RiuProjectAndPropertyView* projPropView = dynamic_cast<RiuProjectAndPropertyView*>(
            additionalProjectView->widget() );
        if ( projPropView )
        {
            projPropView->setPdmItem( pdmRoot );
        }
    }

    caf::SelectionManager::instance()->setPdmRootObject( pdmRoot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotViewFromNorth()
{
    if ( RiaApplication::instance()->activeReservoirView() && RiaApplication::instance()->activeReservoirView()->viewer() )
    {
        RiaApplication::instance()->activeReservoirView()->viewer()->setView( cvf::Vec3d( 0, -1, 0 ),
                                                                              cvf::Vec3d( 0, 0, 1 ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotViewFromSouth()
{
    if ( RiaApplication::instance()->activeReservoirView() && RiaApplication::instance()->activeReservoirView()->viewer() )
    {
        RiaApplication::instance()->activeReservoirView()->viewer()->setView( cvf::Vec3d( 0, 1, 0 ),
                                                                              cvf::Vec3d( 0, 0, 1 ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotViewFromEast()
{
    if ( RiaApplication::instance()->activeReservoirView() && RiaApplication::instance()->activeReservoirView()->viewer() )
    {
        RiaApplication::instance()->activeReservoirView()->viewer()->setView( cvf::Vec3d( -1, 0, 0 ),
                                                                              cvf::Vec3d( 0, 0, 1 ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotViewFromWest()
{
    if ( RiaApplication::instance()->activeReservoirView() && RiaApplication::instance()->activeReservoirView()->viewer() )
    {
        RiaApplication::instance()->activeReservoirView()->viewer()->setView( cvf::Vec3d( 1, 0, 0 ),
                                                                              cvf::Vec3d( 0, 0, 1 ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotViewFromAbove()
{
    if ( RiaApplication::instance()->activeReservoirView() && RiaApplication::instance()->activeReservoirView()->viewer() )
    {
        RiaApplication::instance()->activeReservoirView()->viewer()->setView( cvf::Vec3d( 0, 0, -1 ),
                                                                              cvf::Vec3d( 0, 1, 0 ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotViewFromBelow()
{
    if ( RiaApplication::instance()->activeReservoirView() && RiaApplication::instance()->activeReservoirView()->viewer() )
    {
        RiaApplication::instance()->activeReservoirView()->viewer()->setView( cvf::Vec3d( 0, 0, 1 ),
                                                                              cvf::Vec3d( 0, 1, 0 ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotSubWindowActivated( QMdiSubWindow* subWindow )
{
    if ( blockSubWindowActivation() ) return;

    Rim3dView* previousActiveReservoirView = RiaApplication::instance()->activeReservoirView();
    Rim3dView* activatedView               = dynamic_cast<Rim3dView*>( findViewWindowFromSubWindow( subWindow ) );

    if ( !activatedView ) return;
    RiaApplication::instance()->setActiveReservoirView( activatedView );

    if ( !blockSubWindowProjectTreeSelection() )
    {
        selectViewInProjectTree( previousActiveReservoirView, activatedView );
    }

    slotRefreshViewActions();
    refreshAnimationActions();
    refreshDrawStyleActions();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::selectViewInProjectTree( const Rim3dView* previousActiveReservoirView, Rim3dView* activatedView )
{
    bool is3dViewCurrentlySelected = false;
    if ( caf::SelectionManager::instance()->selectedItem() )
    {
        if ( caf::SelectionManager::instance()->selectedItemAncestorOfType<Rim3dView>() )
        {
            is3dViewCurrentlySelected = true;
        }
    }

    if ( is3dViewCurrentlySelected && ( previousActiveReservoirView != activatedView ) )
    {
        QModelIndex newViewModelIndex = m_projectTreeView->findModelIndex( activatedView );
        QModelIndex newSelectionIndex = newViewModelIndex;

        if ( previousActiveReservoirView && is3dViewCurrentlySelected )
        {
            // Try to select the same entry in the new View, as was selected in the previous

            QModelIndex previousViewModelIndex = m_projectTreeView->findModelIndex( previousActiveReservoirView );
            QModelIndex currentSelectionIndex  = m_projectTreeView->treeView()->selectionModel()->currentIndex();

            if ( currentSelectionIndex != newViewModelIndex && currentSelectionIndex.isValid() )
            {
                QVector<QModelIndex> route; // Contains all model indices from current selection up to previous view

                QModelIndex tmpModelIndex = currentSelectionIndex;

                while ( tmpModelIndex.isValid() && tmpModelIndex != previousViewModelIndex )
                {
                    // NB! Add model index to front of vector to be able to do a for-loop with correct ordering
                    route.push_front( tmpModelIndex );

                    tmpModelIndex = tmpModelIndex.parent();
                }

                // Traverse model indices from new view index to currently selected item
                int i;
                for ( i = 0; i < route.size(); i++ )
                {
                    QModelIndex tmp = route[i];
                    if ( newSelectionIndex.isValid() )
                    {
                        newSelectionIndex = m_projectTreeView->treeView()->model()->index( tmp.row(),
                                                                                           tmp.column(),
                                                                                           newSelectionIndex );
                    }
                }

                // Use view model index if anything goes wrong
                if ( !newSelectionIndex.isValid() )
                {
                    newSelectionIndex = newViewModelIndex;
                }
            }
        }

        m_projectTreeView->treeView()->setCurrentIndex( newSelectionIndex );
        if ( newSelectionIndex != newViewModelIndex )
        {
            m_projectTreeView->treeView()->setExpanded( newViewModelIndex, true );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::setActiveViewer( QWidget* viewer )
{
    QMdiSubWindow* swin = findMdiSubWindow( viewer );
    if ( swin ) m_mdiArea->setActiveSubWindow( swin );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuProcessMonitor* RiuMainWindow::processMonitor()
{
    return m_processMonitor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void appendToggleActionForDockingWidget( QMenu* menu, QWidget* parent, const QString& dockWidgetName )
{
    if ( menu )
    {
        QAction* action = RiuDockWidgetTools::toggleActionForWidget( parent, dockWidgetName );
        if ( action )
        {
            // Some dock windows are depending on configuration (mohrs circle plot), so do not assert they exist
            menu->addAction( action );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotBuildWindowActions()
{
    m_windowMenu->clear();

    {
        caf::CmdFeatureManager* cmdFeatureMgr = caf::CmdFeatureManager::instance();
        m_windowMenu->addAction( cmdFeatureMgr->action( "RicShowPlotWindowFeature" ) );
        m_windowMenu->addSeparator();

        m_windowMenu->addAction( cmdFeatureMgr->action( "RicDefaultDockConfigEclipseFeature" ) );
#ifdef USE_ODB_API
        m_windowMenu->addAction( cmdFeatureMgr->action( "RicDefaultDockConfigGeoMechFeature" ) );
#endif
        m_windowMenu->addSeparator();
    }

    appendToggleActionForDockingWidget( m_windowMenu, this, RiuDockWidgetTools::projectTreeName() );
    appendToggleActionForDockingWidget( m_windowMenu, this, RiuDockWidgetTools::propertyEditorName() );
    appendToggleActionForDockingWidget( m_windowMenu, this, RiuDockWidgetTools::messagesName() );
    appendToggleActionForDockingWidget( m_windowMenu, this, RiuDockWidgetTools::processMonitorName() );

    m_windowMenu->addSeparator();

    appendToggleActionForDockingWidget( m_windowMenu, this, RiuDockWidgetTools::resultInfoName() );
    appendToggleActionForDockingWidget( m_windowMenu, this, RiuDockWidgetTools::resultPlotName() );
    appendToggleActionForDockingWidget( m_windowMenu, this, RiuDockWidgetTools::relPermPlotName() );
    appendToggleActionForDockingWidget( m_windowMenu, this, RiuDockWidgetTools::pvtPlotName() );
    appendToggleActionForDockingWidget( m_windowMenu, this, RiuDockWidgetTools::mohrsCirclePlotName() );

    m_windowMenu->addSeparator();
    QAction* cascadeWindowsAction = new QAction( "Cascade Windows", this );
    connect( cascadeWindowsAction, SIGNAL( triggered() ), m_mdiArea, SLOT( cascadeSubWindows() ) );

    QAction* closeAllSubWindowsAction = new QAction( "Close All Windows", this );
    connect( closeAllSubWindowsAction, SIGNAL( triggered() ), m_mdiArea, SLOT( closeAllSubWindows() ) );

    m_windowMenu->addAction( caf::CmdFeatureManager::instance()->action( "RicTileWindowsFeature" ) );
    m_windowMenu->addAction( cascadeWindowsAction );
    m_windowMenu->addAction( closeAllSubWindowsAction );

    m_windowMenu->addSeparator();
    m_windowMenu->addAction( m_newPropertyView );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::selectedObjectsChanged()
{
    std::vector<caf::PdmUiItem*> uiItems;
    m_projectTreeView->selectedUiItems( uiItems );

    caf::PdmObjectHandle* firstSelectedObject = nullptr;

    if ( uiItems.size() == 1 )
    {
        firstSelectedObject = dynamic_cast<caf::PdmObjectHandle*>( uiItems[0] );
    }

    updateUiFieldsFromActiveResult( firstSelectedObject );

    m_pdmUiPropertyView->showProperties( firstSelectedObject );

    if ( uiItems.size() == 1 && m_allowActiveViewChangeFromSelection )
    {
        // Find the reservoir view or the Plot that the selected item is within

        if ( !firstSelectedObject )
        {
            caf::PdmFieldHandle* selectedField = dynamic_cast<caf::PdmFieldHandle*>( uiItems[0] );
            if ( selectedField ) firstSelectedObject = selectedField->ownerObject();
        }

        if ( !firstSelectedObject ) return;

        // First check if we are within a RimView
        Rim3dView* selectedReservoirView = dynamic_cast<Rim3dView*>( firstSelectedObject );
        if ( !selectedReservoirView )
        {
            firstSelectedObject->firstAncestorOrThisOfType( selectedReservoirView );
        }

        bool isActiveViewChanged = false;

        if ( selectedReservoirView )
        {
            // Set focus in MDI area to this window if it exists
            if ( selectedReservoirView->viewer() )
            {
                setBlockSubWindowProjectTreeSelection( true );
                setActiveViewer( selectedReservoirView->viewer()->layoutWidget() );
                setBlockSubWindowProjectTreeSelection( false );

                isActiveViewChanged = true;
            }
        }

        if ( isActiveViewChanged )
        {
            RiaApplication::instance()->setActiveReservoirView( selectedReservoirView );
            refreshDrawStyleActions();
            refreshAnimationActions();
            slotRefreshFileActions();
            slotRefreshEditActions();
            slotRefreshViewActions();

            // The only way to get to this code is by selection change initiated from the project tree view
            // As we are activating an MDI-window, the focus is given to this MDI-window
            // Set focus back to the tree view to be able to continue keyboard tree view navigation
            m_projectTreeView->treeView()->setFocus();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotNewObjectPropertyView()
{
    QDockWidget* dockWidget =
        new QDockWidget( QString( "Additional Project Tree (%1)" ).arg( m_additionalProjectViews.size() + 1 ), this );
    dockWidget->setObjectName( "dockWidget" );
    dockWidget->setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );

    RiuProjectAndPropertyView* projPropView = new RiuProjectAndPropertyView( dockWidget );
    dockWidget->setWidget( projPropView );
    projPropView->setPdmItem( m_pdmRoot );

    addDockWidget( Qt::RightDockWidgetArea, dockWidget );

    m_additionalProjectViews.push_back( dockWidget );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotSnapshotAllViewsToFile()
{
    RiaApplication* app = RiaApplication::instance();

    // Save images in snapshot catalog relative to project directory
    QString absolutePathToSnapshotDir = app->createAbsolutePathFromProjectRelativePath( "snapshots" );
    RicSnapshotAllViewsToFileFeature::exportSnapshotOfViewsIntoFolder( absolutePathToSnapshotDir );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotDrawStyleChanged( QAction* activatedAction )
{
    if ( !RiaApplication::instance()->activeReservoirView() ) return;

    if ( activatedAction == m_drawStyleLinesAction )
    {
        RiaApplication::instance()->activeReservoirView()->setMeshOnlyDrawstyle();
    }
    else if ( activatedAction == m_drawStyleLinesSolidAction )
    {
        RiaApplication::instance()->activeReservoirView()->setMeshSurfDrawstyle();
    }
    else if ( activatedAction == m_drawStyleSurfOnlyAction )
    {
        RiaApplication::instance()->activeReservoirView()->setSurfOnlyDrawstyle();
    }
    else if ( activatedAction == m_drawStyleFaultLinesSolidAction )
    {
        RiaApplication::instance()->activeReservoirView()->setFaultMeshSurfDrawstyle();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotToggleHideGridCellsAction( bool hideGridCells )
{
    if ( !RiaApplication::instance()->activeReservoirView() ) return;

    RimGridView* rigv = RiaApplication::instance()->activeGridView();
    if ( rigv ) rigv->showGridCells( !hideGridCells );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotToggleFaultLabelsAction( bool showLabels )
{
    Rim3dView* activeView = RiaApplication::instance()->activeReservoirView();

    RimEclipseView* activeRiv = dynamic_cast<RimEclipseView*>( activeView );
    if ( !activeRiv )
    {
        Rim2dIntersectionView* isectView = dynamic_cast<Rim2dIntersectionView*>( activeView );
        if ( isectView )
        {
            isectView->intersection()->firstAncestorOrThisOfType( activeRiv );
        }
    }

    if ( !activeRiv ) return;

    activeRiv->faultCollection()->showFaultLabel.setValueWithFieldChanged( showLabels );

    refreshDrawStyleActions();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::refreshDrawStyleActions()
{
    RimGridView*              gridView     = RiaApplication::instance()->activeGridView();
    RimEclipseContourMapView* view2d       = dynamic_cast<RimEclipseContourMapView*>( gridView );
    bool                      is2dMap      = view2d != nullptr;
    bool                      is3dGridView = gridView != nullptr && !is2dMap;

    Rim3dView* view     = RiaApplication::instance()->activeReservoirView();
    bool       is3dView = view != nullptr && !is2dMap;

    m_drawStyleLinesAction->setEnabled( is3dView );
    m_drawStyleLinesSolidAction->setEnabled( is3dView );
    m_drawStyleSurfOnlyAction->setEnabled( is3dView );
    m_drawStyleFaultLinesSolidAction->setEnabled( is3dView );
    m_disableLightingAction->setEnabled( is3dView );

    bool lightingDisabledInView = view ? view->isLightingDisabled() : false;

    m_disableLightingAction->blockSignals( true );
    m_disableLightingAction->setChecked( lightingDisabledInView );
    m_disableLightingAction->blockSignals( false );

    m_drawStyleHideGridCellsAction->setEnabled( is3dGridView );
    if ( is3dGridView )
    {
        m_drawStyleHideGridCellsAction->blockSignals( true );
        m_drawStyleHideGridCellsAction->setChecked( !view->isGridVisualizationMode() );
        m_drawStyleHideGridCellsAction->blockSignals( false );
    }

    RimEclipseView* eclView = dynamic_cast<RimEclipseView*>( view );

    bool hasEclipseView = eclView != nullptr;
    m_showWellCellsAction->setEnabled( hasEclipseView && !is2dMap );

    if ( hasEclipseView && !is2dMap )
    {
        m_showWellCellsAction->blockSignals( true );
        eclView->wellCollection()->updateStateForVisibilityCheckboxes();
        m_showWellCellsAction->setChecked( eclView->wellCollection()->showWellCells() );
        m_showWellCellsAction->blockSignals( false );
    }

    if ( !eclView )
    {
        Rim2dIntersectionView* intView = dynamic_cast<Rim2dIntersectionView*>( view );
        if ( intView && intView->intersection() )
        {
            intView->intersection()->firstAncestorOrThisOfType( eclView );
        }
    }

    m_toggleFaultsLabelAction->setEnabled( eclView != nullptr );

    if ( eclView )
    {
        m_toggleFaultsLabelAction->blockSignals( true );
        m_toggleFaultsLabelAction->setChecked( eclView->faultCollection()->showFaultLabel() );
        m_toggleFaultsLabelAction->blockSignals( false );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotDisableLightingAction( bool disable )
{
    Rim3dView* view = RiaApplication::instance()->activeReservoirView();
    if ( view )
    {
        view->disableLighting( disable );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::restoreTreeViewState()
{
    if ( m_projectTreeView )
    {
        QString stateString = RiaApplication::instance()->project()->mainWindowTreeViewState;
        if ( !stateString.isEmpty() )
        {
            m_projectTreeView->treeView()->collapseAll();
            caf::QTreeViewStateSerializer::applyTreeViewStateFromString( m_projectTreeView->treeView(), stateString );
        }

        QString currentIndexString = RiaApplication::instance()->project()->mainWindowCurrentModelIndexPath;
        if ( !currentIndexString.isEmpty() )
        {
            QModelIndex mi = caf::QTreeViewStateSerializer::getModelIndexFromString( m_projectTreeView->treeView()->model(),
                                                                                     currentIndexString );
            m_projectTreeView->treeView()->setCurrentIndex( mi );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::updateUiFieldsFromActiveResult( caf::PdmObjectHandle* objectToUpdate )
{
    RimEclipseResultDefinition* resultDefinition = nullptr;
    resultDefinition                             = dynamic_cast<RimEclipseResultDefinition*>( objectToUpdate );
    if ( resultDefinition )
    {
        resultDefinition->updateUiFieldsFromActiveResult();
    }

    RimEclipsePropertyFilter* eclPropFilter = nullptr;
    eclPropFilter                           = dynamic_cast<RimEclipsePropertyFilter*>( objectToUpdate );
    if ( eclPropFilter )
    {
        eclPropFilter->updateUiFieldsFromActiveResult();
    }

    RimEclipseFaultColors* eclFaultColors = nullptr;
    eclFaultColors                        = dynamic_cast<RimEclipseFaultColors*>( objectToUpdate );
    if ( eclFaultColors )
    {
        eclFaultColors->updateUiFieldsFromActiveResult();
    }

    RimCellEdgeColors* cellEdgeColors = nullptr;
    cellEdgeColors                    = dynamic_cast<RimCellEdgeColors*>( objectToUpdate );
    if ( cellEdgeColors )
    {
        cellEdgeColors->updateUiFieldsFromActiveResult();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::updateMemoryUsage()
{
    uint64_t currentUsage        = caf::MemoryInspector::getApplicationPhysicalMemoryUsageMiB();
    uint64_t totalPhysicalMemory = caf::MemoryInspector::getTotalPhysicalMemoryMiB();
    uint64_t totalVirtualMemory  = caf::MemoryInspector::getTotalVirtualMemoryMiB();
    uint64_t availVirtualMemory  = caf::MemoryInspector::getAvailableVirtualMemoryMiB();

    QColor okColor( 0, 150, 0 );
    QColor warningColor( 200, 0, 0 );
    QColor criticalColor( 255, 100, 0 );

    float currentUsageFraction = 0.0f;
    float availVirtualFraction = 1.0f;
    if ( currentUsage > 0u && totalPhysicalMemory > 0u )
    {
        currentUsageFraction = std::min( 1.0f, static_cast<float>( currentUsage ) / totalPhysicalMemory );
    }
    if ( availVirtualMemory > 0u && totalVirtualMemory > 0u )
    {
        availVirtualFraction = static_cast<float>( availVirtualMemory ) / totalVirtualMemory;
    }

    QColor usageColor( (int)( okColor.red() * ( 1.0 - currentUsageFraction ) + warningColor.red() * currentUsageFraction ),
                       (int)( okColor.green() * ( 1.0 - currentUsageFraction ) +
                              warningColor.green() * currentUsageFraction ),
                       (int)( okColor.blue() * ( 1.0 - currentUsageFraction ) +
                              warningColor.blue() * currentUsageFraction ) );

    m_memoryCriticalWarning->setText( QString( "" ) );
    if ( availVirtualFraction < caf::MemoryInspector::getRemainingMemoryCriticalThresholdFraction() )
    {
        m_memoryCriticalWarning->setText( QString( "Available System Memory Critically Low!" ) );
        m_memoryCriticalWarning->setStyleSheet(
            QString( "QLabel {color: %1; padding: 0px 5px 0px 0px;}" ).arg( criticalColor.name() ) );
    }
    else
    {
        m_memoryCriticalWarning->setText( QString( "" ) );
    }

    m_memoryUsedButton->setText( QString( "Memory Used: %1 MiB" ).arg( currentUsage ) );
    m_memoryTotalStatus->setText( QString( "Total Physical Memory: %1 MiB" ).arg( totalPhysicalMemory ) );

    m_memoryUsedButton->setStyleSheet(
        QString( "QLabel {color: %1; padding: 0px 5px 0px 0px;}" ).arg( usageColor.name() ) );
    m_memoryTotalStatus->setStyleSheet( QString( "QLabel {padding: 0px 5px 0px 0px; }" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::showProcessMonitorDockPanel()
{
    RiuDockWidgetTools::setDockWidgetVisibility( this, RiuDockWidgetTools::processMonitorName(), true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::setDefaultToolbarVisibility()
{
    m_holoLensToolBar->hide();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::applyFontSizesToDockedPlots()
{
    m_resultQwtPlot->applyFontSizes( true );
    m_mohrsCirclePlot->applyFontSizes( true );
    m_relPermPlotPanel->applyFontSizes( true );
    m_pvtPlotPanel->applyFontSizes( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotScaleChanged( int scaleValue )
{
    if ( RiaApplication::instance()->activeReservoirView() )
    {
        RiaApplication::instance()->activeReservoirView()->scaleZ.setValueWithFieldChanged( scaleValue );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::updateScaleValue()
{
    Rim3dView* view                   = RiaApplication::instance()->activeReservoirView();
    bool       isRegularReservoirView = view && dynamic_cast<RimEclipseContourMapView*>( view ) == nullptr;
    if ( isRegularReservoirView )
    {
        m_scaleFactor->setEnabled( true );

        int scaleValue = static_cast<int>( view->scaleZ() ); // Round down is probably ok.
        m_scaleFactor->blockSignals( true );
        m_scaleFactor->setValue( scaleValue );
        m_scaleFactor->blockSignals( false );
    }
    else
    {
        m_scaleFactor->setEnabled( false );
    }
}

//--------------------------------------------------------------------------------------------------
/// TODO: This function will be moved to a class responsible for handling the application selection concept
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::selectedCases( std::vector<RimCase*>& cases )
{
    caf::SelectionManager::instance()->objectsByType( &cases );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotCreateCommandObject()
{
    RiaApplication* app = RiaApplication::instance();
    if ( !app->project() ) return;

    std::vector<caf::PdmUiItem*> selectedUiItems;
    m_projectTreeView->selectedUiItems( selectedUiItems );

    caf::PdmObjectGroup selectedObjects;
    for ( auto* selectedUiItem : selectedUiItems )
    {
        caf::PdmUiObjectHandle* uiObj = dynamic_cast<caf::PdmUiObjectHandle*>( selectedUiItem );
        if ( uiObj )
        {
            selectedObjects.addObject( uiObj->objectHandle() );
        }
    }

    if ( !selectedObjects.objects.empty() )
    {
        std::vector<RimCommandObject*> commandObjects;
        RimCommandFactory::createCommandObjects( selectedObjects, &commandObjects );

        for ( auto* commandObject : commandObjects )
        {
            app->project()->commandObjects.push_back( commandObject );
        }

        app->project()->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotShowRegressionTestDialog()
{
    RiaRegressionTest regTestConfig;
    regTestConfig.readSettingsFromApplicationStore();

    caf::PdmUiPropertyViewDialog regressionTestDialog( this, &regTestConfig, "Regression Test", "" );
    regressionTestDialog.resize( QSize( 600, 350 ) );

    if ( regressionTestDialog.exec() == QDialog::Accepted )
    {
        // Write preferences using QSettings and apply them to the application
        regTestConfig.writeSettingsToApplicationStore();

        RiaRegressionTestRunner::instance()->executeRegressionTests();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotExecutePaintEventPerformanceTest()
{
    if ( RiaApplication::instance()->activeReservoirView() && RiaApplication::instance()->activeReservoirView()->viewer() )
    {
        size_t redrawCount = 50;

        caf::Viewer* viewer = RiaApplication::instance()->activeReservoirView()->viewer();

        cvf::Timer timer;
        for ( size_t i = 0; i < redrawCount; i++ )
        {
            viewer->repaint();
        }

        double totalTimeMS = timer.time() * 1000.0;

        double msPerFrame = totalTimeMS / redrawCount;

        QString resultInfo = QString( "Total time '%1 ms' for %2 number of redraws, frame time '%3 ms'" )
                                 .arg( totalTimeMS )
                                 .arg( redrawCount )
                                 .arg( msPerFrame );
        setResultInfo( resultInfo );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::setDefaultWindowSize()
{
    resize( 1000, 810 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotShowWellCellsAction( bool doAdd )
{
    RimEclipseView* riv = dynamic_cast<RimEclipseView*>( RiaApplication::instance()->activeReservoirView() );
    if ( riv )
    {
        riv->wellCollection()->setShowWellCellsState( doAdd );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::customMenuRequested( const QPoint& pos )
{
    QMenu menu;

    RiaApplication* app = RiaApplication::instance();
    app->project()->actionsBasedOnSelection( menu );

    // Qt doc: QAbstractScrollArea and its subclasses that map the context menu event to coordinates of the viewport().
    // Since we might get this signal from different treeViews, we need to map the position accordingly.
    QObject*   senderObj = this->sender();
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
void RiuMainWindow::tileSubWindows()
{
    QMdiArea::WindowOrder currentActivationOrder = m_mdiArea->activationOrder();

    // Tile Windows so the one with the leftmost left edge gets sorted first.
    std::list<QMdiSubWindow*> windowList;
    for ( QMdiSubWindow* subWindow : m_mdiArea->subWindowList( currentActivationOrder ) )
    {
        windowList.push_back( subWindow );
    }

    // Get the active view linker if there is one
    RimProject*              proj                 = RiaApplication::instance()->project();
    RimViewLinkerCollection* viewLinkerCollection = proj->viewLinkerCollection();
    RimViewLinker*           viewLinker           = nullptr;
    if ( viewLinkerCollection && viewLinkerCollection->isActive() )
    {
        viewLinker = viewLinkerCollection->viewLinker();
    }

    // Perform stable sort of list so we first sort by window position but retain activation order
    // for windows with the same position.
    windowList.sort( [this, viewLinker]( QMdiSubWindow* lhs, QMdiSubWindow* rhs ) {
        RimViewWindow* lhsViewWindow = findViewWindowFromSubWindow( lhs );
        RimViewWindow* rhsViewWindow = findViewWindowFromSubWindow( rhs );
        RimGridView*   lhsGridView   = dynamic_cast<RimGridView*>( lhsViewWindow );
        RimGridView*   rhsGridView   = dynamic_cast<RimGridView*>( rhsViewWindow );

        if ( viewLinker )
        {
            if ( viewLinker->isFirstViewDependentOnSecondView( lhsGridView, rhsGridView ) )
            {
                return true;
            }
            else if ( viewLinker->isFirstViewDependentOnSecondView( rhsGridView, lhsGridView ) )
            {
                return false;
            }
        }
        if ( lhs->frameGeometry().topLeft().ry() == rhs->frameGeometry().topLeft().ry() )
        {
            return lhs->frameGeometry().topLeft().rx() < rhs->frameGeometry().topLeft().rx();
        }
        return lhs->frameGeometry().topLeft().ry() < rhs->frameGeometry().topLeft().ry();
    } );

    // Based on workaround described here
    // https://forum.qt.io/topic/50053/qmdiarea-tilesubwindows-always-places-widgets-in-activationhistoryorder-in-subwindowview-mode

    bool prevActivationBlock = blockSubWindowActivation();

    QMdiSubWindow* a = m_mdiArea->activeSubWindow();

    // Force activation order so they end up in the order of the loop.
    m_mdiArea->setActivationOrder( QMdiArea::ActivationHistoryOrder );

    setBlockSubWindowActivation( true );

    // Activate in reverse order
    for ( auto it = windowList.rbegin(); it != windowList.rend(); ++it )
    {
        m_mdiArea->setActiveSubWindow( *it );
    }

    m_mdiArea->tileSubWindows();
    // Set back the original activation order to avoid messing with the standard ordering
    m_mdiArea->setActivationOrder( currentActivationOrder );
    m_mdiArea->setActiveSubWindow( a );
    setBlockSubWindowActivation( prevActivationBlock );

    storeSubWindowTiling( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::storeSubWindowTiling( bool tiled )
{
    RiaApplication::instance()->project()->setSubWindowsTiledIn3DWindow( tiled );
    refreshViewActions();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::clearWindowTiling()
{
    setBlockSubWindowActivation( true );
    QMdiArea::WindowOrder currentActivationOrder = m_mdiArea->activationOrder();

    for ( QMdiSubWindow* subWindow : m_mdiArea->subWindowList( currentActivationOrder ) )
    {
        subWindow->hide();
        subWindow->showNormal();
    }
    storeSubWindowTiling( false );
    setBlockSubWindowActivation( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuMainWindow::subWindowsAreTiled() const
{
    if ( RiaApplication::instance()->project() )
    {
        return RiaApplication::instance()->project()->subWindowsTiled3DWindow();
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuMainWindow::isAnyMdiSubWindowVisible()
{
    return !m_mdiArea->subWindowList().empty();
}
