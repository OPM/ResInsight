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
#include "RiaPreferencesSystem.h"
#include "RiaRegressionTest.h"
#include "RiaRegressionTestRunner.h"

#include "RicGridCalculatorDialog.h"

#include "Rim2dIntersectionView.h"
#include "Rim3dView.h"
#include "RimCellEdgeColors.h"
#include "RimCommandObject.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseContourMapView.h"
#include "RimEclipseFaultColors.h"
#include "RimEclipsePropertyFilter.h"
#include "RimEclipseResultDefinition.h"
#include "RimEclipseView.h"
#include "RimExtrudedCurveIntersection.h"
#include "RimFaultInViewCollection.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechView.h"
#include "RimProject.h"
#include "RimSimWellInViewCollection.h"
#include "RimViewLinker.h"
#include "RimViewLinkerCollection.h"
#include "RimViewWindow.h"

#include "RiuDepthQwtPlot.h"
#include "RiuDockWidgetTools.h"
#include "RiuMdiArea.h"
#include "RiuMdiSubWindow.h"
#include "RiuMenuBarBuildTools.h"
#include "RiuMessagePanel.h"
#include "RiuMohrsCirclePlot.h"
#include "RiuProcessMonitor.h"
#include "RiuProjectPropertyView.h"
#include "RiuPropertyViewTabWidget.h"
#include "RiuPvtPlotPanel.h"
#include "RiuRelativePermeabilityPlotPanel.h"
#include "RiuResultInfoPanel.h"
#include "RiuResultQwtPlot.h"
#include "RiuSeismicHistogramPanel.h"
#include "RiuToolTipMenu.h"
#include "RiuTools.h"
#include "RiuTreeViewEventFilter.h"
#include "RiuViewer.h"

#include "cafAnimationToolBar.h"
#include "cafCmdExecCommandManager.h"
#include "cafCmdFeatureManager.h"
#include "cafMemoryInspector.h"
#include "cafPdmUiPropertyView.h"
#include "cafPdmUiPropertyViewDialog.h"
#include "cafPdmUiTreeView.h"
#include "cafSelectionManager.h"
#include "cafUtils.h"

#include "ApplicationCommands/RicLaunchRegressionTestsFeature.h"
#include "ExportCommands/RicSnapshotAllViewsToFileFeature.h"
#include "MeasurementCommands/RicToggleMeasurementModeFeature.h"
#include "SummaryPlotCommands/RicEditSummaryPlotFeature.h"
#include "SummaryPlotCommands/RicShowSummaryCurveCalculatorFeature.h"

#include "cvfTimer.h"

#include "DockAreaWidget.h"
#include "DockManager.h"

#include <QAction>
#include <QActionGroup>
#include <QCloseEvent>
#include <QComboBox>
#include <QDir>
#include <QLabel>
#include <QLayout>
#include <QMdiSubWindow>
#include <QMenuBar>
#include <QMimeData>
#include <QSpinBox>
#include <QStatusBar>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QTreeView>
#include <QUndoStack>
#include <QUndoView>

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
    , m_holoLensToolBar( nullptr )
    , m_seismicHistogramPanel( nullptr )
{
    setAttribute( Qt::WA_DeleteOnClose );

    m_mdiArea = new RiuMdiArea( this );
    connect( m_mdiArea, SIGNAL( subWindowActivated( QMdiSubWindow* ) ), SLOT( slotSubWindowActivated( QMdiSubWindow* ) ) );

    ads::CDockWidget* cWidget = RiuDockWidgetTools::createDockWidget( "3D Views", RiuDockWidgetTools::main3DWindowName(), this );
    cWidget->setWidget( m_mdiArea );
    dockManager()->setCentralWidget( cWidget );

    createActions();
    createMenus();
    createToolBars();
    createDockPanels();

    setAcceptDrops( true );

    if ( m_undoView )
    {
        m_undoView->setStack( caf::CmdExecCommandManager::instance()->undoStack() );
    }
    connect( caf::CmdExecCommandManager::instance()->undoStack(), SIGNAL( indexChanged( int ) ), SLOT( slotRefreshUndoRedoActions() ) );

    initializeGuiNewProjectLoaded();

    QString versionText = RiaApplication::getVersionStringApp( false );

    m_versionInfo           = new QLabel( versionText );
    m_memoryCriticalWarning = new QLabel( "" );
    m_memoryUsedButton      = new QToolButton( nullptr );
    m_memoryTotalStatus     = new QLabel( "" );

    m_memoryUsedButton->setDefaultAction( caf::CmdFeatureManager::instance()->action( "RicShowMemoryCleanupDialogFeature" ) );

    statusBar()->addPermanentWidget( m_versionInfo );
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
RiuMainWindow::~RiuMainWindow()
{
    setPdmRoot( nullptr );

    if ( m_pdmUiPropertyView )
    {
        m_pdmUiPropertyView->showProperties( nullptr );
    }
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
void RiuMainWindow::closeIfOpen()
{
    if ( instance() != nullptr ) instance()->close();
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
    setPdmRoot( RimProject::current() );
    restoreTreeViewState();

    m_mdiArea->applyTiling();

    slotRefreshFileActions();
    slotRefreshUndoRedoActions();
    slotRefreshViewActions();
    refreshAnimationActions();
    refreshDrawStyleActions();

    if ( m_pdmUiPropertyView && m_pdmUiPropertyView->currentObject() )
    {
        m_pdmUiPropertyView->currentObject()->uiCapability()->updateConnectedEditors();
    }

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
    m_depthQwtPlot->deleteAllCurves();
    if ( m_relPermPlotPanel ) m_relPermPlotPanel->clearPlot();
    if ( m_pvtPlotPanel ) m_pvtPlotPanel->clearPlot();
    if ( m_mohrsCirclePlot ) m_mohrsCirclePlot->clearPlot();
    if ( m_seismicHistogramPanel ) m_seismicHistogramPanel->clearPlot();

    if ( m_pdmUiPropertyView )
    {
        m_pdmUiPropertyView->showProperties( nullptr );
    }

    for ( auto& additionalProjectView : m_additionalProjectViews )
    {
        RiuProjectAndPropertyView* projPropView = dynamic_cast<RiuProjectAndPropertyView*>( additionalProjectView->widget() );
        if ( projPropView )
        {
            projPropView->showProperties( nullptr );
        }
    }

    RicEditSummaryPlotFeature* editSumCurves =
        dynamic_cast<RicEditSummaryPlotFeature*>( caf::CmdFeatureManager::instance()->getCommandFeature( "RicEditSummaryPlotFeature" ) );
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
    RiaGuiApplication* app = RiaGuiApplication::instance();

    if ( !app->isMainPlotWindowVisible() )
    {
        if ( !app->askUserToSaveModifiedProject() )
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
void RiuMainWindow::createActions()
{
    // File actions
    m_mockModelAction             = new QAction( "&Mock Model", this );
    m_mockResultsModelAction      = new QAction( "Mock Model With &Results", this );
    m_mockLargeResultsModelAction = new QAction( "Large Mock Model", this );
    m_mockModelCustomizedAction   = new QAction( "Customized Mock Model", this );
    m_mockInputModelAction        = new QAction( "Input Mock Model", this );

    m_snapshotAllViewsToFile = new QAction( QIcon( ":/SnapShotSaveViews.svg" ), "Snapshot All Views To File", this );

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
    m_viewFullScreen = new QAction( QIcon( ":/Fullscreen.png" ), "Full Screen", this );
    m_viewFullScreen->setToolTip( "Full Screen (Ctrl+Alt+F)" );
    m_viewFullScreen->setCheckable( true );
    caf::CmdFeature::applyShortcutWithHintToAction( m_viewFullScreen, QKeySequence( tr( "Ctrl+Alt+F" ) ) );

    m_viewFromNorth = new QAction( QIcon( ":/SouthView.svg" ), "Look South", this );
    m_viewFromNorth->setToolTip( "Look South (Ctrl+Alt+S)" );
    caf::CmdFeature::applyShortcutWithHintToAction( m_viewFromNorth, QKeySequence( tr( "Ctrl+Alt+S" ) ) );

    m_viewFromSouth = new QAction( QIcon( ":/NorthView.svg" ), "Look North", this );
    m_viewFromSouth->setToolTip( "Look North (Ctrl+Alt+N)" );
    caf::CmdFeature::applyShortcutWithHintToAction( m_viewFromSouth, QKeySequence( tr( "Ctrl+Alt+N" ) ) );

    m_viewFromEast = new QAction( QIcon( ":/WestView.svg" ), "Look West", this );
    m_viewFromEast->setToolTip( "Look West (Ctrl+Alt+W)" );
    caf::CmdFeature::applyShortcutWithHintToAction( m_viewFromEast, QKeySequence( tr( "Ctrl+Alt+W" ) ) );

    m_viewFromWest = new QAction( QIcon( ":/EastView.svg" ), "Look East", this );
    m_viewFromWest->setToolTip( "Look East (Ctrl+Alt+E)" );
    caf::CmdFeature::applyShortcutWithHintToAction( m_viewFromWest, QKeySequence( tr( "Ctrl+Alt+E" ) ) );

    m_viewFromAbove = new QAction( QIcon( ":/DownView.svg" ), "Look Down", this );
    m_viewFromAbove->setToolTip( "Look Down (Ctrl+Alt+D)" );
    caf::CmdFeature::applyShortcutWithHintToAction( m_viewFromAbove, QKeySequence( tr( "Ctrl+Alt+D" ) ) );

    m_viewFromBelow = new QAction( QIcon( ":/UpView.svg" ), "Look Up", this );
    m_viewFromBelow->setToolTip( "Look Up (Ctrl+Alt+U)" );
    caf::CmdFeature::applyShortcutWithHintToAction( m_viewFromBelow, QKeySequence( tr( "Ctrl+Alt+U" ) ) );

    connect( m_viewFromNorth, SIGNAL( triggered() ), SLOT( slotViewFromNorth() ) );
    connect( m_viewFromSouth, SIGNAL( triggered() ), SLOT( slotViewFromSouth() ) );
    connect( m_viewFromEast, SIGNAL( triggered() ), SLOT( slotViewFromEast() ) );
    connect( m_viewFromWest, SIGNAL( triggered() ), SLOT( slotViewFromWest() ) );
    connect( m_viewFromAbove, SIGNAL( triggered() ), SLOT( slotViewFromAbove() ) );
    connect( m_viewFromBelow, SIGNAL( triggered() ), SLOT( slotViewFromBelow() ) );
    connect( m_viewFullScreen, SIGNAL( toggled( bool ) ), SLOT( slotViewFullScreen( bool ) ) );

    // Debug actions
    m_newPropertyView = new QAction( "New Project and Property View", this );
    connect( m_newPropertyView, SIGNAL( triggered() ), SLOT( slotNewObjectPropertyView() ) );

    // Draw style actions
    m_dsActionGroup = new QActionGroup( this );

    m_drawStyleLinesAction = new QAction( QIcon( ":/DrawStyleLines.svg" ), "&Mesh Only", this );
    m_dsActionGroup->addAction( m_drawStyleLinesAction );

    m_drawStyleLinesSolidAction = new QAction( QIcon( ":/DrawStyleMeshLines.svg" ), "Mesh And Surfaces", this );
    m_dsActionGroup->addAction( m_drawStyleLinesSolidAction );

    m_drawStyleSurfOnlyAction = new QAction( QIcon( ":/DrawStyleSurface.svg" ), "&Surface Only", this );
    m_dsActionGroup->addAction( m_drawStyleSurfOnlyAction );

    m_drawStyleDeformationsAction = new QAction( QIcon( ":/draw_style_deformation_24x24.png" ), "Show &Displacements", this );
    m_drawStyleDeformationsAction->setCheckable( true );
    m_dsActionGroup->addAction( m_drawStyleDeformationsAction );

    connect( m_dsActionGroup, SIGNAL( triggered( QAction* ) ), SLOT( slotDrawStyleChanged( QAction* ) ) );

    m_drawStyleFaultLinesSolidAction = new QAction( QIcon( ":/draw_style_surface_w_fault_mesh_24x24.png" ), "Fault Mesh And Surfaces", this );
    m_dsActionGroup->addAction( m_drawStyleFaultLinesSolidAction );

    m_drawStyleHideGridCellsAction = new QAction( QIcon( ":/draw_style_faults_24x24.png" ), "&Hide Grid Cells", this );
    m_drawStyleHideGridCellsAction->setCheckable( true );
    connect( m_drawStyleHideGridCellsAction, SIGNAL( toggled( bool ) ), SLOT( slotToggleHideGridCellsAction( bool ) ) );

    m_toggleFaultsLabelAction = new QAction( QIcon( ":/draw_style_faults_label_24x24.png" ), "&Show Fault Labels", this );
    m_toggleFaultsLabelAction->setCheckable( true );
    connect( m_toggleFaultsLabelAction, SIGNAL( toggled( bool ) ), SLOT( slotToggleFaultLabelsAction( bool ) ) );

    m_showWellCellsAction = new QAction( QIcon( ":/draw_style_WellCellsToRangeFilter_24x24.png" ), "&Show Well Cells", this );
    m_showWellCellsAction->setCheckable( true );
    m_showWellCellsAction->setToolTip( "Show Well Cells" );
    connect( m_showWellCellsAction, SIGNAL( toggled( bool ) ), SLOT( slotShowWellCellsAction( bool ) ) );

    m_enableLightingAction = new QAction( QIcon( ":/DrawLightingEnabled.svg" ), "&Enable Results Lighting", this );
    m_enableLightingAction->setCheckable( true );
    connect( m_enableLightingAction, SIGNAL( toggled( bool ) ), SLOT( slotToggleLightingAction( bool ) ) );
    // m_dsActionGroup->addAction( m_enableLightingAction );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::createMenus()
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
    exportMenu->addAction( m_snapshotAllViewsToFile );
    exportMenu->addAction( cmdFeatureMgr->action( "RicAdvancedSnapshotExportFeature" ) );
    exportMenu->addSeparator();
    exportMenu->addAction( cmdFeatureMgr->action( "RicExportEclipseInputGridFeature" ) );
    exportMenu->addAction( cmdFeatureMgr->action( "RicSaveEclipseInputActiveVisibleCellsFeature" ) );
    exportMenu->addAction( cmdFeatureMgr->action( "RicExportCompletionsForVisibleWellPathsFeature" ) );
    exportMenu->addAction( cmdFeatureMgr->action( "RicExportVisibleWellPathsFeature" ) );

    // Save menu actions
    fileMenu->addSeparator();
    RiuMenuBarBuildTools::addSaveProjectActions( fileMenu );

    std::vector<QAction*> recentFileActions = RiaGuiApplication::instance()->recentFileActions();
    for ( auto act : recentFileActions )
    {
        fileMenu->addAction( act );
    }

    fileMenu->addSeparator();
    QMenu* testMenu = fileMenu->addMenu( "&Testing" );

    // Close and Exit actions
    fileMenu->addSeparator();
    RiuMenuBarBuildTools::addCloseAndExitActions( fileMenu );

    connect( fileMenu, SIGNAL( aboutToShow() ), SLOT( slotRefreshFileActions() ) );

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
    viewMenu->addAction( m_viewFullScreen );
    viewMenu->addSeparator();
    viewMenu->addAction( m_viewFromSouth );
    viewMenu->addAction( m_viewFromNorth );
    viewMenu->addAction( m_viewFromWest );
    viewMenu->addAction( m_viewFromEast );
    viewMenu->addAction( m_viewFromBelow );
    viewMenu->addAction( m_viewFromAbove );

    connect( viewMenu, SIGNAL( aboutToShow() ), SLOT( slotRefreshViewActions() ) );

    // Test menu
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
    testMenu->addAction( cmdFeatureMgr->action( "RicRunCommandFileFeature" ) );
    testMenu->addAction( cmdFeatureMgr->action( "RicExportObjectAndFieldKeywordsFeature" ) );
    testMenu->addAction( cmdFeatureMgr->action( "RicSaveProjectNoGlobalPathsFeature" ) );
    testMenu->addAction( cmdFeatureMgr->action( "RicExecuteLastUsedScriptFeature" ) );

    testMenu->addSeparator();
    testMenu->addAction( cmdFeatureMgr->action( "RicHoloLensExportToFolderFeature" ) );
    testMenu->addAction( cmdFeatureMgr->action( "RicHoloLensCreateDummyFiledBackedSessionFeature" ) );

    testMenu->addSeparator();
    testMenu->addAction( cmdFeatureMgr->action( "RicThemeColorEditorFeature" ) );

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
void RiuMainWindow::createToolBars()
{
    caf::CmdFeatureManager* cmdFeatureMgr = caf::CmdFeatureManager::instance();
    CVF_ASSERT( cmdFeatureMgr );

    {
        QToolBar* toolbar = addToolBar( tr( "Standard" ) );
        toolbar->setObjectName( toolbar->windowTitle() );
        toolbar->addAction( cmdFeatureMgr->action( "RicImportEclipseCaseFeature" ) );
        toolbar->addAction( cmdFeatureMgr->action( "RicOpenProjectFeature" ) );
        toolbar->addAction( cmdFeatureMgr->action( "RicSaveProjectFeature" ) );
    }

    if ( RiaPreferences::current()->useUndoRedo() )
    {
        QToolBar* toolbar = addToolBar( tr( "Edit" ) );
        toolbar->setObjectName( toolbar->windowTitle() );
        toolbar->addAction( m_undoAction );
        toolbar->addAction( m_redoAction );
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
        QToolBar* toolbar = addToolBar( tr( "Import GeoMech" ) );
        toolbar->setObjectName( toolbar->windowTitle() );
        toolbar->addAction( cmdFeatureMgr->action( "RicImportGeoMechCaseFeature" ) );
        toolbar->addAction( cmdFeatureMgr->action( "RicImportGeoMechCaseTimeStepFilterFeature" ) );
        toolbar->addAction( cmdFeatureMgr->action( "RicImportElementPropertyFeature" ) );
        toolbar->hide();
    }

    {
        QToolBar* toolbar = addToolBar( tr( "Window Management" ) );
        toolbar->setObjectName( toolbar->windowTitle() );
        toolbar->addAction( cmdFeatureMgr->action( "RicShowPlotWindowFeature" ) );
        toolbar->addAction( cmdFeatureMgr->action( "RicLinkVisibleViewsFeature" ) );
        toolbar->addAction( cmdFeatureMgr->action( "RicTileWindowsFeature" ) );
        toolbar->addAction( cmdFeatureMgr->action( "RicShowGridCalculatorFeature" ) );
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
        toolbar->addAction( m_viewFullScreen );
        toolbar->addAction( m_viewFromNorth );
        toolbar->addAction( m_viewFromSouth );
        toolbar->addAction( m_viewFromEast );
        toolbar->addAction( m_viewFromWest );
        toolbar->addAction( m_viewFromAbove );
        toolbar->addAction( m_viewFromBelow );

        QLabel* scaleLabel = new QLabel( toolbar );
        scaleLabel->setText( "Scale" );
        toolbar->addWidget( scaleLabel );

        m_scaleFactor = new QComboBox( toolbar );
        QStringList scaleItems;
        for ( auto d : RiaDefines::viewScaleOptions() )
        {
            m_scaleFactor->addItem( QString::number( d ), QVariant( d ) );
        }
        toolbar->addWidget( m_scaleFactor );
        connect( m_scaleFactor, SIGNAL( currentIndexChanged( int ) ), SLOT( slotScaleChanged( int ) ) );
    }

    {
        QToolBar* dsToolBar = addToolBar( tr( "Draw Style" ) );
        dsToolBar->setObjectName( dsToolBar->windowTitle() );
        dsToolBar->addAction( m_drawStyleLinesAction );
        dsToolBar->addAction( m_drawStyleLinesSolidAction );
        dsToolBar->addAction( m_drawStyleSurfOnlyAction );
        dsToolBar->addAction( m_enableLightingAction );
        dsToolBar->addAction( m_drawStyleFaultLinesSolidAction );
        dsToolBar->addAction( m_drawStyleHideGridCellsAction );
        dsToolBar->addAction( m_toggleFaultsLabelAction );
        dsToolBar->addAction( m_showWellCellsAction );
        dsToolBar->addAction( m_drawStyleDeformationsAction );
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

    if ( RiaPreferencesSystem::current()->showTestToolbar() )
    {
        QToolBar* toolbar = addToolBar( tr( "Test" ) );
        toolbar->setObjectName( toolbar->windowTitle() );
        toolbar->addAction( cmdFeatureMgr->action( "RicLaunchRegressionTestsFeature" ) );
        toolbar->addAction( cmdFeatureMgr->action( "RicLaunchRegressionTestDialogFeature" ) );
        toolbar->addAction( cmdFeatureMgr->action( "RicShowClassNamesFeature" ) );
        toolbar->addAction( cmdFeatureMgr->action( "RicRunCommandFileFeature" ) );
        toolbar->addAction( cmdFeatureMgr->action( "RicExecuteLastUsedScriptFeature" ) );
        toolbar->addAction( cmdFeatureMgr->action( "RicExportCompletionsForVisibleWellPathsFeature" ) );
        toolbar->addAction( cmdFeatureMgr->action( "RicShowMemoryReportFeature" ) );
    }

    // Create animation toolbar
    m_animationToolBar = new caf::AnimationToolBar( "Animation", this );
    addToolBar( m_animationToolBar );

    {
        QToolBar* toolbar = addToolBar( tr( "Timestep Slider" ) );
        toolbar->setObjectName( toolbar->windowTitle() );

        m_animationSlider = new QSlider( Qt::Horizontal, toolbar );
        m_animationSlider->setToolTip( "Current Time Step" );

        m_animationSliderAction = toolbar->addWidget( m_animationSlider );

        connect( m_animationSlider, SIGNAL( valueChanged( int ) ), SLOT( slotAnimationSliderMoved( int ) ) );
    }

    // make sure slider updates if user uses animation toolbar
    connect( m_animationToolBar, SIGNAL( frameChanged( int ) ), SLOT( slotAnimationControlFrameChanged( int ) ) );

    refreshAnimationActions();
    refreshDrawStyleActions();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::createDockPanels()
{
    const int                  nTreeViews        = 3;
    const std::vector<QString> treeViewTitles    = { "Project Tree", "Calculator Data ", "Scripts" };
    const std::vector<QString> treeViewConfigs   = { "MainWindow.ProjectTree", "MainWindow.DataSources", "MainWindow.Scripts" };
    const std::vector<QString> treeViewDockNames = { RiuDockWidgetTools::mainWindowProjectTreeName(),
                                                     RiuDockWidgetTools::mainWindowDataSourceTreeName(),
                                                     RiuDockWidgetTools::mainWindowScriptsTreeName() };

    const std::vector<ads::DockWidgetArea> defaultDockWidgetArea{ ads::DockWidgetArea::LeftDockWidgetArea,
                                                                  ads::DockWidgetArea::LeftDockWidgetArea,
                                                                  ads::DockWidgetArea::LeftDockWidgetArea };

    createTreeViews( nTreeViews );

    std::vector<ads::CDockWidget*> rightWidgets;
    std::vector<ads::CDockWidget*> leftWidgets;
    std::vector<ads::CDockWidget*> bottomWidgets;

    for ( int i = 0; i < nTreeViews; i++ )
    {
        auto dockWidget = RiuDockWidgetTools::createDockWidget( treeViewTitles[i], treeViewDockNames[i], dockManager() );

        caf::PdmUiTreeView* projectTree = projectTreeView( i );
        projectTree->enableSelectionManagerUpdating( true );
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
        connect( projectTree, SIGNAL( selectionChanged() ), this, SLOT( selectedObjectsChanged() ) );

        projectTree->treeView()->setContextMenuPolicy( Qt::CustomContextMenu );
        connect( projectTree->treeView(), SIGNAL( customContextMenuRequested( const QPoint& ) ), SLOT( customMenuRequested( const QPoint& ) ) );

        projectTree->setUiConfigurationName( treeViewConfigs[i] );
    }

    // undo/redo view
    if ( m_undoView && RiaPreferences::current()->useUndoRedo() )
    {
        auto dockWidget = RiuDockWidgetTools::createDockWidget( "Undo Stack", RiuDockWidgetTools::mainWindowUndoStackName(), dockManager() );
        dockWidget->setWidget( m_undoView );
        rightWidgets.push_back( dockWidget );
    }

    {
        auto dockWidget = RiuDockWidgetTools::createDockWidget( "Result Plot", RiuDockWidgetTools::mainWindowResultPlotName(), dockManager() );

        m_resultQwtPlot = new RiuResultQwtPlot( dockWidget );
        dockWidget->setWidget( m_resultQwtPlot );
        bottomWidgets.push_back( dockWidget );
    }

    {
        auto dockWidget = RiuDockWidgetTools::createDockWidget( "Depth Plot", RiuDockWidgetTools::mainWindowDepthPlotName(), dockManager() );

        m_depthQwtPlot = new RiuDepthQwtPlot( dockWidget );
        dockWidget->setWidget( m_depthQwtPlot );
        rightWidgets.push_back( dockWidget );
    }

    ads::CDockAreaWidget* leftArea  = addTabbedWidgets( leftWidgets, ads::DockWidgetArea::LeftDockWidgetArea );
    ads::CDockAreaWidget* rightArea = addTabbedWidgets( rightWidgets, ads::DockWidgetArea::RightDockWidgetArea );
    ads::CDockAreaWidget* bottomArea =
        addTabbedWidgets( bottomWidgets, ads::DockWidgetArea::BottomDockWidgetArea, dockManager()->centralWidget()->dockAreaWidget() );

    {
        auto dockWidget =
            RiuDockWidgetTools::createDockWidget( "Property Editor", RiuDockWidgetTools::mainWindowPropertyEditorName(), dockManager() );

        m_pdmUiPropertyView = new caf::PdmUiPropertyView( dockWidget );
        dockWidget->setWidget( m_pdmUiPropertyView );
        dockManager()->addDockWidget( ads::DockWidgetArea::BottomDockWidgetArea, dockWidget, leftArea );
    }

#ifdef USE_ODB_API
    {
        auto dockWidget =
            RiuDockWidgetTools::createDockWidget( "Mohr's Circle Plot", RiuDockWidgetTools::mainWindowMohrsCirclePlotName(), dockManager() );

        m_mohrsCirclePlot = new RiuMohrsCirclePlot( dockWidget );
        dockWidget->setWidget( m_mohrsCirclePlot );
        dockManager()->addDockWidgetTabToArea( dockWidget, bottomArea );
    }
#endif

    {
        auto dockWidget = RiuDockWidgetTools::createDockWidget( "Relative Permeability Plot",
                                                                RiuDockWidgetTools::mainWindowRelPermPlotName(),
                                                                dockManager() );

        m_relPermPlotPanel = new RiuRelativePermeabilityPlotPanel( dockWidget );
        dockWidget->setWidget( m_relPermPlotPanel );
        dockManager()->addDockWidgetTabToArea( dockWidget, bottomArea );
    }

    {
        auto dockWidget = RiuDockWidgetTools::createDockWidget( "PVT Plot", RiuDockWidgetTools::mainWindowPvtPlotName(), dockManager() );

        m_pvtPlotPanel = new RiuPvtPlotPanel( dockWidget );
        dockWidget->setWidget( m_pvtPlotPanel );
        dockManager()->addDockWidgetTabToArea( dockWidget, bottomArea );
    }

    {
        auto dockWidget =
            RiuDockWidgetTools::createDockWidget( "Seismic Histogram", RiuDockWidgetTools::mainWindowSeismicHistogramName(), dockManager() );

        m_seismicHistogramPanel = new RiuSeismicHistogramPanel( dockWidget );
        dockWidget->setWidget( m_seismicHistogramPanel );
        dockManager()->addDockWidgetTabToArea( dockWidget, bottomArea );
    }

    // result info
    {
        auto dockWidget = RiuDockWidgetTools::createDockWidget( "Result Info", RiuDockWidgetTools::mainWindowResultInfoName(), dockManager() );

        m_resultInfoPanel = new RiuResultInfoPanel( dockWidget );
        dockWidget->setWidget( m_resultInfoPanel );
        dockManager()->addDockWidget( ads::DockWidgetArea::LeftDockWidgetArea, dockWidget, bottomArea );
    }

    ads::CDockAreaWidget* procAndMsgTabs = nullptr;
    // process monitor
    {
        auto dockWidget =
            RiuDockWidgetTools::createDockWidget( "Process Monitor", RiuDockWidgetTools::mainWindowProcessMonitorName(), dockManager() );

        m_processMonitor = new RiuProcessMonitor( dockWidget );
        dockWidget->setWidget( m_processMonitor );
        procAndMsgTabs = dockManager()->addDockWidget( ads::DockWidgetArea::RightDockWidgetArea, dockWidget, bottomArea );
    }

    {
        auto dockWidget = RiuDockWidgetTools::createDockWidget( "Messages", RiuDockWidgetTools::mainWindowMessagesName(), dockManager() );

        m_messagePanel = new RiuMessagePanel( dockWidget );
        dockWidget->setWidget( m_messagePanel );
        dockManager()->addDockWidgetTabToArea( dockWidget, procAndMsgTabs );
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
void RiuMainWindow::setResultInfo( const QString& info ) const
{
    m_resultInfoPanel->setInfo( info );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::refreshViewActions()
{
    slotRefreshViewActions();
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
    if ( activeView && activeView->ownerCase() && activeView->viewer() && activeView->viewer()->frameCount() )
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

    m_animationSliderAction->setEnabled( enableAnimControls );
    m_animationSlider->blockSignals( true );
    m_animationSlider->setMaximum( timeStepStrings.size() - 1 );
    m_animationSlider->setMinimum( 0 );
    m_animationSlider->setSingleStep( 1 );
    m_animationSlider->setPageStep( 1 );
    m_animationSlider->setValue( currentTimeStepIndex );
    m_animationSlider->blockSignals( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotAnimationControlFrameChanged( int frameIndex )
{
    if ( m_animationSlider )
    {
        m_animationSlider->blockSignals( true );
        m_animationSlider->setValue( frameIndex );
        m_animationSlider->blockSignals( false );
    }
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
        std::vector<RimViewWindow*> allViewWindows = RimProject::current()->descendantsIncludingThisOfType<RimViewWindow>();

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
RiuDepthQwtPlot* RiuMainWindow::depthPlot()
{
    return m_depthQwtPlot;
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
RiuSeismicHistogramPanel* RiuMainWindow::seismicHistogramPanel()
{
    return m_seismicHistogramPanel;
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
void RiuMainWindow::initializeViewer( QMdiSubWindow* subWindow, QWidget* viewer, const RimMdiWindowGeometry& windowsGeometry )
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

    slotRefreshViewActions();
}

//--------------------------------------------------------------------------------------------------
/// This method needs to handle memory deallocation !!!
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::setPdmRoot( caf::PdmObject* pdmRoot )
{
    m_pdmRoot = pdmRoot;

    for ( auto tv : projectTreeViews() )
    {
        tv->setPdmItem( pdmRoot );
    }

    for ( auto& additionalProjectView : m_additionalProjectViews )
    {
        if ( !additionalProjectView ) continue;

        RiuProjectAndPropertyView* projPropView = dynamic_cast<RiuProjectAndPropertyView*>( additionalProjectView->widget() );
        if ( projPropView )
        {
            projPropView->setPdmItem( pdmRoot );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotViewFromNorth()
{
    if ( RiaApplication::instance()->activeReservoirView() && RiaApplication::instance()->activeReservoirView()->viewer() )
    {
        RiaApplication::instance()->activeReservoirView()->viewer()->setView( cvf::Vec3d( 0, -1, 0 ), cvf::Vec3d( 0, 0, 1 ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotViewFullScreen( bool showFullScreen )
{
    if ( showFullScreen )
    {
        m_lastDockState = dockManager()->saveState( DOCKSTATE_VERSION );
        dockManager()->restoreState( RiuDockWidgetTools::hideAllDocking3DState(), DOCKSTATE_VERSION );
    }
    else
    {
        dockManager()->restoreState( m_lastDockState, DOCKSTATE_VERSION );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotViewFromSouth()
{
    if ( RiaApplication::instance()->activeReservoirView() && RiaApplication::instance()->activeReservoirView()->viewer() )
    {
        RiaApplication::instance()->activeReservoirView()->viewer()->setView( cvf::Vec3d( 0, 1, 0 ), cvf::Vec3d( 0, 0, 1 ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotViewFromEast()
{
    if ( RiaApplication::instance()->activeReservoirView() && RiaApplication::instance()->activeReservoirView()->viewer() )
    {
        RiaApplication::instance()->activeReservoirView()->viewer()->setView( cvf::Vec3d( -1, 0, 0 ), cvf::Vec3d( 0, 0, 1 ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotViewFromWest()
{
    if ( RiaApplication::instance()->activeReservoirView() && RiaApplication::instance()->activeReservoirView()->viewer() )
    {
        RiaApplication::instance()->activeReservoirView()->viewer()->setView( cvf::Vec3d( 1, 0, 0 ), cvf::Vec3d( 0, 0, 1 ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotViewFromAbove()
{
    if ( RiaApplication::instance()->activeReservoirView() && RiaApplication::instance()->activeReservoirView()->viewer() )
    {
        RiaApplication::instance()->activeReservoirView()->viewer()->setView( cvf::Vec3d( 0, 0, -1 ), cvf::Vec3d( 0, 1, 0 ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotViewFromBelow()
{
    if ( RiaApplication::instance()->activeReservoirView() && RiaApplication::instance()->activeReservoirView()->viewer() )
    {
        RiaApplication::instance()->activeReservoirView()->viewer()->setView( cvf::Vec3d( 0, 0, 1 ), cvf::Vec3d( 0, 1, 0 ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotSubWindowActivated( QMdiSubWindow* subWindow )
{
    if ( isBlockingSubWindowActivatedSignal() ) return;

    Rim3dView* previousActiveReservoirView = RiaApplication::instance()->activeReservoirView();
    Rim3dView* activatedView               = dynamic_cast<Rim3dView*>( findViewWindowFromSubWindow( subWindow ) );

    if ( !activatedView || ( previousActiveReservoirView == activatedView ) ) return;

    RiaApplication::instance()->setActiveReservoirView( activatedView );

    if ( !isBlockingViewSelectionOnSubWindowActivated() )
    {
        selectViewInProjectTreePreservingSubItemSelection( previousActiveReservoirView, activatedView );
    }

    slotRefreshViewActions();
    refreshAnimationActions();
    refreshDrawStyleActions();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::selectViewInProjectTreePreservingSubItemSelection( const Rim3dView* previousActiveReservoirView, Rim3dView* activatedView )
{
    bool is3dViewCurrentlySelected = false;
    if ( caf::SelectionManager::instance()->selectedItem() )
    {
        if ( caf::SelectionManager::instance()->selectedItemAncestorOfType<Rim3dView>() )
        {
            is3dViewCurrentlySelected = true;
        }
    }

    auto tv = getTreeViewWithItem( activatedView );
    if ( !tv ) return;

    QModelIndex newViewModelIndex = tv->findModelIndex( activatedView );
    if ( !newViewModelIndex.isValid() ) return;

    QModelIndex newSelectionIndex = newViewModelIndex;

    if ( !is3dViewCurrentlySelected )
    {
        std::vector<RimEclipseCellColors*> objects = activatedView->descendantsIncludingThisOfType<RimEclipseCellColors>();
        if ( !objects.empty() )
        {
            auto candidate = tv->findModelIndex( objects.front() );
            if ( candidate.isValid() ) newSelectionIndex = candidate;
        }
    }
    else if ( previousActiveReservoirView && is3dViewCurrentlySelected )
    {
        // Try to select the same entry in the new View, as was selected in the previous

        QModelIndex previousViewModelIndex = tv->findModelIndex( previousActiveReservoirView );
        QModelIndex currentSelectionIndex  = tv->treeView()->selectionModel()->currentIndex();

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
                    newSelectionIndex = tv->treeView()->model()->index( tmp.row(), tmp.column(), newSelectionIndex );
                }
            }

            // Use view model index if anything goes wrong
            if ( !newSelectionIndex.isValid() )
            {
                newSelectionIndex = newViewModelIndex;
            }
        }
    }

    tv->treeView()->setCurrentIndex( newSelectionIndex );
    if ( newSelectionIndex != newViewModelIndex )
    {
        tv->treeView()->setExpanded( newViewModelIndex, true );
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
void RiuMainWindow::slotBuildWindowActions()
{
    m_windowMenu->clear();

    {
        caf::CmdFeatureManager* cmdFeatureMgr = caf::CmdFeatureManager::instance();
        m_windowMenu->addAction( cmdFeatureMgr->action( "RicShowPlotWindowFeature" ) );
        m_windowMenu->addSeparator();
    }

    addDefaultEntriesToWindowsMenu();

    m_windowMenu->addSeparator();
    m_windowMenu->addAction( m_newPropertyView );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::selectedObjectsChanged()
{
    caf::PdmUiTreeView* projectTree = dynamic_cast<caf::PdmUiTreeView*>( sender() );
    if ( !projectTree ) return;

    std::vector<caf::PdmUiItem*> uiItems;
    projectTree->selectedUiItems( uiItems );

    caf::PdmObjectHandle* firstSelectedObject = nullptr;
    if ( !uiItems.empty() )
    {
        firstSelectedObject = dynamic_cast<caf::PdmObjectHandle*>( uiItems.front() );
    }

    updateUiFieldsFromActiveResult( firstSelectedObject );

    m_pdmUiPropertyView->showProperties( firstSelectedObject );

    m_seismicHistogramPanel->showHistogram( firstSelectedObject );

    if ( uiItems.size() == 1 && m_allowActiveViewChangeFromSelection )
    {
        // Find the reservoir view or the Plot that the selected item is within

        if ( !firstSelectedObject )
        {
            caf::PdmFieldHandle* selectedField = dynamic_cast<caf::PdmFieldHandle*>( uiItems.front() );
            if ( selectedField ) firstSelectedObject = selectedField->ownerObject();
        }

        if ( !firstSelectedObject ) return;

        // First check if we are within a RimView
        Rim3dView* selectedReservoirView = dynamic_cast<Rim3dView*>( firstSelectedObject );
        if ( !selectedReservoirView )
        {
            selectedReservoirView = firstSelectedObject->firstAncestorOrThisOfType<Rim3dView>();
        }

        bool isActiveViewChanged = false;

        if ( selectedReservoirView )
        {
            // Set focus in MDI area to this window if it exists
            if ( selectedReservoirView->viewer() )
            {
                setBlockViewSelectionOnSubWindowActivated( true );
                setActiveViewer( selectedReservoirView->viewer()->layoutWidget() );
                setBlockViewSelectionOnSubWindowActivated( false );

                isActiveViewChanged = true;
            }
        }

        if ( isActiveViewChanged )
        {
            RiaApplication::instance()->setActiveReservoirView( selectedReservoirView );
            refreshDrawStyleActions();
            refreshAnimationActions();
            slotRefreshFileActions();
            slotRefreshUndoRedoActions();
            slotRefreshViewActions();

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
void RiuMainWindow::slotNewObjectPropertyView()
{
    ads::CDockWidget* dockWidget =
        new ads::CDockWidget( QString( "Additional Project Tree (%1)" ).arg( m_additionalProjectViews.size() + 1 ), this );
    dockWidget->setObjectName( "AdditionalDockWidget" );

    RiuProjectAndPropertyView* projPropView = new RiuProjectAndPropertyView( dockWidget );
    dockWidget->setWidget( projPropView );
    projPropView->setPdmItem( m_pdmRoot );

    dockManager()->addDockWidget( ads::DockWidgetArea::RightDockWidgetArea, dockWidget );

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
    else if ( activatedAction == m_drawStyleDeformationsAction )
    {
        RimGeoMechView* geoMechView = dynamic_cast<RimGeoMechView*>( RiaApplication::instance()->activeReservoirView() );
        if ( geoMechView )
        {
            geoMechView->setShowDisplacementsAndUpdate( !geoMechView->showDisplacements() );
            m_drawStyleDeformationsAction->blockSignals( true );
            m_drawStyleDeformationsAction->setChecked( geoMechView->showDisplacements() );
            m_drawStyleDeformationsAction->blockSignals( false );
        }
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
            activeRiv = isectView->intersection()->firstAncestorOrThisOfType<RimEclipseView>();
        }
    }

    if ( !activeRiv ) return;

    activeRiv->faultCollection()->setShowFaultLabelWithFieldChanged( showLabels );

    refreshDrawStyleActions();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::refreshDrawStyleActions()
{
    RimGridView*              gridView     = RiaApplication::instance()->activeGridView();
    RimEclipseContourMapView* view2d       = dynamic_cast<RimEclipseContourMapView*>( gridView );
    RimGeoMechView*           geoMechView  = dynamic_cast<RimGeoMechView*>( gridView );
    bool                      is2dMap      = view2d != nullptr;
    bool                      is3dGridView = gridView != nullptr && !is2dMap;

    Rim3dView* view     = RiaApplication::instance()->activeReservoirView();
    bool       is3dView = view != nullptr && !is2dMap;

    m_drawStyleLinesAction->setEnabled( is3dView );
    m_drawStyleLinesSolidAction->setEnabled( is3dView );
    m_drawStyleSurfOnlyAction->setEnabled( is3dView );
    m_drawStyleFaultLinesSolidAction->setEnabled( is3dView );
    m_drawStyleDeformationsAction->setVisible( geoMechView != nullptr );
    m_enableLightingAction->setEnabled( is3dView );

    bool lightingEnabled = view ? !view->isLightingDisabled() : true;

    m_enableLightingAction->blockSignals( true );
    m_enableLightingAction->setChecked( lightingEnabled );

    if ( lightingEnabled )
    {
        m_enableLightingAction->setIcon( QIcon( ":/DrawLightingEnabled.svg" ) );
    }
    else
    {
        m_enableLightingAction->setIcon( QIcon( ":/DrawLightingDisabled.svg" ) );
    }

    m_enableLightingAction->blockSignals( false );
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
            eclView = intView->intersection()->firstAncestorOrThisOfType<RimEclipseView>();
        }
    }

    m_toggleFaultsLabelAction->setEnabled( eclView != nullptr );

    if ( eclView )
    {
        m_toggleFaultsLabelAction->blockSignals( true );
        m_toggleFaultsLabelAction->setChecked( eclView->faultCollection()->showFaultLabel() );
        m_toggleFaultsLabelAction->blockSignals( false );
    }

    if ( geoMechView )
    {
        m_drawStyleDeformationsAction->blockSignals( true );
        m_drawStyleDeformationsAction->setChecked( geoMechView->showDisplacements() );
        m_drawStyleDeformationsAction->blockSignals( false );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotToggleLightingAction( bool enable )
{
    Rim3dView* view = RiaApplication::instance()->activeReservoirView();
    if ( view )
    {
        view->disableLighting( !enable );
    }
    refreshDrawStyleActions();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::restoreTreeViewState()
{
    restoreTreeViewStates( RimProject::current()->mainWindowTreeViewStates(), RimProject::current()->mainWindowCurrentModelIndexPaths() );
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
                       (int)( okColor.green() * ( 1.0 - currentUsageFraction ) + warningColor.green() * currentUsageFraction ),
                       (int)( okColor.blue() * ( 1.0 - currentUsageFraction ) + warningColor.blue() * currentUsageFraction ) );

    m_memoryCriticalWarning->setText( QString( "" ) );
    if ( availVirtualFraction < caf::MemoryInspector::getRemainingMemoryCriticalThresholdFraction() )
    {
        m_memoryCriticalWarning->setText( QString( "Available System Memory Critically Low!" ) );
        m_memoryCriticalWarning->setProperty( "styleRole", "memoryCriticalWarning" );
    }
    else
    {
        m_memoryCriticalWarning->setText( QString( "" ) );
    }

    m_memoryUsedButton->setText( QString( "Memory Used: %1 MiB" ).arg( currentUsage ) );
    m_memoryUsedButton->setProperty( "styleRole", "memoryUsedButton" );
    m_memoryTotalStatus->setText( QString( "Total Physical Memory: %1 MiB" ).arg( totalPhysicalMemory ) );
    m_memoryTotalStatus->setProperty( "styleRole", "memoryTotalStatus" );

    m_memoryUsedButton->setStyleSheet( QString( "QLabel {color: %1;}" ).arg( usageColor.name() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::showProcessMonitorDockPanel()
{
    RiuDockWidgetTools::showDockWidget( dockManager(), RiuDockWidgetTools::mainWindowProcessMonitorName() );
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
    if ( m_resultQwtPlot )
    {
        m_resultQwtPlot->applyFontSizes( true );
    }
    if ( m_depthQwtPlot )
    {
        m_depthQwtPlot->applyFontSizes( true );
    }
    if ( m_mohrsCirclePlot )
    {
        m_mohrsCirclePlot->applyFontSizes( true );
    }
    if ( m_relPermPlotPanel )
    {
        m_relPermPlotPanel->applyFontSizes( true );
    }
    if ( m_pvtPlotPanel )
    {
        m_pvtPlotPanel->applyFontSizes( true );
    }
    if ( m_seismicHistogramPanel )
    {
        m_seismicHistogramPanel->applyFontSizes( true );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotScaleChanged( int index )
{
    if ( RiaApplication::instance()->activeReservoirView() )
    {
        double scaleValue = m_scaleFactor->currentData().toDouble();

        RiaApplication::instance()->activeReservoirView()->setScaleZAndUpdate( scaleValue );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::updateScaleValue()
{
    Rim3dView* view                   = RiaApplication::instance()->activeReservoirView();
    bool       isRegularReservoirView = view && dynamic_cast<RimEclipseContourMapView*>( view ) == nullptr;
    if ( isRegularReservoirView && view->isScaleZEditable() )
    {
        m_scaleFactor->setEnabled( true );
        m_scaleFactor->blockSignals( true );

        int index = m_scaleFactor->findData( QVariant( view->scaleZ() ) );
        if ( index < 0 )
        {
            m_scaleFactor->addItem( QString::number( view->scaleZ() ), QVariant( view->scaleZ() ) );
            index = m_scaleFactor->findData( QVariant( view->scaleZ() ) );
        }
        m_scaleFactor->setCurrentIndex( index );

        m_scaleFactor->blockSignals( false );
    }
    else
    {
        m_scaleFactor->setEnabled( false );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotAnimationSliderMoved( int newValue )
{
    if ( RiaApplication::instance()->activeReservoirView() && RiaApplication::instance()->activeReservoirView()->viewer() )
    {
        RiaApplication::instance()->activeReservoirView()->viewer()->slotSetCurrentFrame( newValue );
    }
    m_animationToolBar->setCurrentTimeStepIndex( newValue );
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

    caf::PdmUiTreeView* projectTree = dynamic_cast<caf::PdmUiTreeView*>( sender() );
    if ( !projectTree ) return;

    std::vector<caf::PdmUiItem*> selectedUiItems;
    projectTree->selectedUiItems( selectedUiItems );

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
    RicLaunchRegressionTestDialogFeature::showRegressionTestDialog();
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

        QString resultInfo =
            QString( "Total time '%1 ms' for %2 number of redraws, frame time '%3 ms'" ).arg( totalTimeMS ).arg( redrawCount ).arg( msPerFrame );
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
bool RiuMainWindow::isAnyMdiSubWindowVisible()
{
    return !m_mdiArea->subWindowList().empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicGridCalculatorDialog* RiuMainWindow::gridCalculatorDialog( bool createIfNotPresent )
{
    if ( !m_gridCalculatorDialog && createIfNotPresent )
    {
        m_gridCalculatorDialog = std::make_unique<RicGridCalculatorDialog>( this );
    }

    return m_gridCalculatorDialog.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RiuMainWindow::defaultDockStateNames()
{
    QStringList retList = { RiuDockWidgetTools::dockState3DEclipseName(),
                            RiuDockWidgetTools::dockState3DGeoMechName(),
                            RiuDockWidgetTools::dockStateHideAll3DWindowName() };
    return retList;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RiuMainWindow::windowsMenuFeatureNames()
{
    return { "RicTileWindowsFeature", "RicTileWindowsVerticallyFeature", "RicTileWindowsHorizontallyFeature" };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::dragEnterEvent( QDragEnterEvent* event )
{
    if ( event->mimeData()->hasUrls() ) event->acceptProposedAction();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::dropEvent( QDropEvent* event )
{
    if ( !event ) return;
    if ( !event->mimeData()->hasUrls() ) return;

    for ( const auto& url : event->mimeData()->urls() )
    {
        QString fileName = url.toLocalFile();

        QFileInfo fi( fileName );
        if ( fi.exists() )
        {
            if ( RiaGuiApplication::instance()->openFile( fileName ) )
            {
                RiaGuiApplication::instance()->addToRecentFiles( fileName );
            }
        }
    }

    event->acceptProposedAction();
}
