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

#include "RiaApplication.h"
#include "RiaBaseDefs.h"
#include "RiaPreferences.h"
#include "RiaRegressionTest.h"

#include "RimCommandObject.h"
#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimEclipseWellCollection.h"
#include "RimFaultCollection.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechView.h"
#include "RimProject.h"
#include "RimTreeViewStateSerializer.h"
#include "RimView.h"

#include "RiuDragDrop.h"
#include "RiuMdiSubWindow.h"
#include "RiuMessagePanel.h"
#include "RiuProcessMonitor.h"
#include "RiuProjectPropertyView.h"
#include "RiuPropertyViewTabWidget.h"
#include "RiuResultInfoPanel.h"
#include "RiuResultQwtPlot.h"
#include "RiuToolTipMenu.h"
#include "RiuTreeViewEventFilter.h"
#include "RiuViewer.h"

#include "cafAnimationToolBar.h"
#include "cafCmdExecCommandManager.h"
#include "cafCmdFeatureManager.h"
#include "cafPdmSettings.h"
#include "cafPdmUiPropertyView.h"
#include "cafPdmUiPropertyViewDialog.h"
#include "cafPdmUiTreeView.h"
#include "cafSelectionManager.h"
#include "cafUtils.h"

#include "ExportCommands/RicSnapshotAllViewsToFileFeature.h"

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
#include <QToolBar>
#include <QTreeView>
#include <QUndoStack>


#define DOCK_PANEL_NAME_PROCESS_MONITOR "dockProcessMonitor"



//==================================================================================================
///
/// \class RiuMainWindow
///
/// Contains our main window
///
//==================================================================================================


RiuMainWindow* RiuMainWindow::sm_mainWindowInstance = NULL;

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuMainWindow::RiuMainWindow()
    : m_pdmRoot(NULL),
    m_mainViewer(NULL),
    m_windowMenu(NULL),
    m_blockSlotSubWindowActivated(false)
{
    CVF_ASSERT(sm_mainWindowInstance == NULL);

    m_mdiArea = new QMdiArea;
    m_mdiArea->setOption(QMdiArea::DontMaximizeSubWindowOnActivation, true);
    connect(m_mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow *)), SLOT(slotSubWindowActivated(QMdiSubWindow*)));
    setCentralWidget(m_mdiArea);

    //m_mainViewer = createViewer();

    createActions();
    createMenus();
    createToolBars();
    createDockPanels();

    // Store the layout so we can offer reset option
    m_initialDockAndToolbarLayout = saveState(0);

    sm_mainWindowInstance = this;

    m_dragDropInterface = std::unique_ptr<caf::PdmUiDragDropInterface>(new RiuDragDrop());

    initializeGuiNewProjectLoaded();

    // Enabling the line below will activate the undo stack
    // When enableUndoCommandSystem is set false, all commands are executed and deleted immediately
    //caf::CmdExecCommandManager::instance()->enableUndoCommandSystem(true);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuMainWindow* RiuMainWindow::instance()
{
    return sm_mainWindowInstance;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::initializeGuiNewProjectLoaded()
{
    setPdmRoot(RiaApplication::instance()->project());
    restoreTreeViewState();
    slotRefreshFileActions();
    slotRefreshEditActions();
    slotRefreshViewActions();
    refreshAnimationActions();
    refreshDrawStyleActions();

    if (m_pdmUiPropertyView && m_pdmUiPropertyView->currentObject())
    {
        m_pdmUiPropertyView->currentObject()->uiCapability()->updateConnectedEditors();
    }

    m_processMonitor->slotClearTextEdit();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::cleanupGuiCaseClose()
{
    caf::CmdExecCommandManager::instance()->undoStack()->clear();

    setResultInfo("");

    m_resultQwtPlot->deleteAllCurves();

    if (m_pdmUiPropertyView)
    {
        m_pdmUiPropertyView->showProperties(NULL);
    }

    for (size_t i = 0; i < additionalProjectViews.size(); i++)
    {
        RiuProjectAndPropertyView* projPropView = dynamic_cast<RiuProjectAndPropertyView*>(additionalProjectViews[i]->widget());
        if (projPropView)
        {
            projPropView->showProperties(NULL);
        }
    }
    m_processMonitor->startMonitorWorkProcess(NULL);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::cleanupGuiBeforeProjectClose()
{
    setPdmRoot(NULL);

    cleanupGuiCaseClose();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::closeEvent(QCloseEvent* event)
{
    RiaApplication* app = RiaApplication::instance();

    if (app->isMainPlotWindowVisible())
    {
        return;
    }

    if (!app->askUserToSaveModifiedProject())
    {
        event->ignore();
        return;
    }

    saveWinGeoAndDockToolBarLayout();

    if (!app->tryClosePlotWindow()) return;

    app->closeProject();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::createActions()
{
    // File actions
    m_mockModelAction           = new QAction("&Mock Model", this);
    m_mockResultsModelAction    = new QAction("Mock Model With &Results", this);
    m_mockLargeResultsModelAction = new QAction("Large Mock Model", this);
    m_mockModelCustomizedAction = new QAction("Customized Mock Model", this);
    m_mockInputModelAction      = new QAction("Input Mock Model", this);

    m_snapshotAllViewsToFile    = new QAction(QIcon(":/SnapShotSaveViews.png"), "Snapshot All Views To File", this);

    m_createCommandObject       = new QAction("Create Command Object", this);
    m_showRegressionTestDialog  = new QAction("Regression Test Dialog", this);
    m_executePaintEventPerformanceTest = new QAction("&Paint Event Performance Test", this);

    connect(m_mockModelAction,            SIGNAL(triggered()), SLOT(slotMockModel()));
    connect(m_mockResultsModelAction,    SIGNAL(triggered()), SLOT(slotMockResultsModel()));
    connect(m_mockLargeResultsModelAction,    SIGNAL(triggered()), SLOT(slotMockLargeResultsModel()));
    connect(m_mockModelCustomizedAction,    SIGNAL(triggered()), SLOT(slotMockModelCustomized()));
    connect(m_mockInputModelAction,        SIGNAL(triggered()), SLOT(slotInputMockModel()));

    connect(m_snapshotAllViewsToFile,   SIGNAL(triggered()), SLOT(slotSnapshotAllViewsToFile()));

    connect(m_createCommandObject,      SIGNAL(triggered()), SLOT(slotCreateCommandObject()));
    connect(m_showRegressionTestDialog, SIGNAL(triggered()), SLOT(slotShowRegressionTestDialog()));
    connect(m_executePaintEventPerformanceTest, SIGNAL(triggered()), SLOT(slotExecutePaintEventPerformanceTest()));
    
    // View actions
    m_viewFromNorth                = new QAction(QIcon(":/SouthViewArrow.png"), "Look South", this);
    m_viewFromNorth->setToolTip("Look South");
    m_viewFromSouth                = new QAction(QIcon(":/NorthViewArrow.png"),"Look North", this);
    m_viewFromSouth->setToolTip("Look North");
    m_viewFromEast                 = new QAction(QIcon(":/WestViewArrow.png"),"Look West", this);
    m_viewFromEast->setToolTip("Look West");
    m_viewFromWest                 = new QAction(QIcon(":/EastViewArrow.png"),"Look East", this);
    m_viewFromWest->setToolTip("Look East");
    m_viewFromAbove                = new QAction(QIcon(":/DownViewArrow.png"),"Look Down", this);
    m_viewFromAbove->setToolTip("Look Down");
    m_viewFromBelow                = new QAction(QIcon(":/UpViewArrow.png"),"Look Up", this);
    m_viewFromBelow->setToolTip("Look Up");

    connect(m_viewFromNorth,                SIGNAL(triggered()), SLOT(slotViewFromNorth()));
    connect(m_viewFromSouth,                SIGNAL(triggered()), SLOT(slotViewFromSouth()));
    connect(m_viewFromEast,                 SIGNAL(triggered()), SLOT(slotViewFromEast()));
    connect(m_viewFromWest,                 SIGNAL(triggered()), SLOT(slotViewFromWest()));
    connect(m_viewFromAbove,                SIGNAL(triggered()), SLOT(slotViewFromAbove()));
    connect(m_viewFromBelow,                SIGNAL(triggered()), SLOT(slotViewFromBelow()));

    // Debug actions
    m_newPropertyView = new QAction("New Project and Property View", this);
    connect(m_newPropertyView, SIGNAL(triggered()), SLOT(slotNewObjectPropertyView()));

    // Draw style actions
    m_dsActionGroup = new QActionGroup(this);

    m_drawStyleLinesAction                = new QAction(QIcon(":/draw_style_lines_24x24.png"), "&Mesh Only", this);
    //connect(m_drawStyleLinesAction,        SIGNAL(triggered()), SLOT(slotDrawStyleLines()));
    m_dsActionGroup->addAction(m_drawStyleLinesAction);

     m_drawStyleLinesSolidAction           = new QAction(QIcon(":/draw_style_meshlines_24x24.png"), "Mesh And Surfaces", this);
    //connect(m_drawStyleLinesSolidAction,    SIGNAL(triggered()), SLOT(slotDrawStyleLinesSolid()));
     m_dsActionGroup->addAction(m_drawStyleLinesSolidAction);

     m_drawStyleFaultLinesSolidAction           = new QAction(QIcon(":/draw_style_surface_w_fault_mesh_24x24.png"), "Fault Mesh And Surfaces", this);
     m_dsActionGroup->addAction(m_drawStyleFaultLinesSolidAction);

    m_drawStyleSurfOnlyAction             = new QAction(QIcon(":/draw_style_surface_24x24.png"), "&Surface Only", this);
    //connect(m_drawStyleSurfOnlyAction,    SIGNAL(triggered()), SLOT(slotDrawStyleSurfOnly()));
    m_dsActionGroup->addAction(m_drawStyleSurfOnlyAction);


    connect(m_dsActionGroup, SIGNAL(triggered(QAction*)), SLOT(slotDrawStyleChanged(QAction*)));

    m_disableLightingAction = new QAction(QIcon(":/disable_lighting_24x24.png"), "&Disable Results Lighting", this);
    m_disableLightingAction->setCheckable(true);
    connect(m_disableLightingAction,    SIGNAL(toggled(bool)), SLOT(slotDisableLightingAction(bool)));


    m_drawStyleHideGridCellsAction             = new QAction( QIcon(":/draw_style_faults_24x24.png"), "&Hide Grid Cells", this);
    m_drawStyleHideGridCellsAction->setCheckable(true);
    connect(m_drawStyleHideGridCellsAction,    SIGNAL(toggled(bool)), SLOT(slotToggleHideGridCellsAction(bool)));

    m_toggleFaultsLabelAction             = new QAction( QIcon(":/draw_style_faults_label_24x24.png"), "&Show Fault Labels", this);
    m_toggleFaultsLabelAction->setCheckable(true);
    connect(m_toggleFaultsLabelAction,    SIGNAL(toggled(bool)), SLOT(slotToggleFaultLabelsAction(bool)));

    m_showWellCellsAction = new QAction(QIcon(":/draw_style_WellCellsToRangeFilter_24x24.png"), "&Show Well Cells", this);
    m_showWellCellsAction->setCheckable(true);
    m_showWellCellsAction->setToolTip("Show Well Cells");
    connect(m_showWellCellsAction,    SIGNAL(toggled(bool)), SLOT(slotShowWellCellsAction(bool)));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::createMenus()
{
    caf::CmdFeatureManager* cmdFeatureMgr = caf::CmdFeatureManager::instance();
    CVF_ASSERT(cmdFeatureMgr);

    // File menu
    QMenu* fileMenu = new RiuToolTipMenu(menuBar());
    fileMenu->setTitle("&File");

    menuBar()->addMenu(fileMenu);
  
    fileMenu->addAction(cmdFeatureMgr->action("RicOpenProjectFeature"));
    fileMenu->addAction(cmdFeatureMgr->action("RicOpenLastUsedFileFeature"));
    fileMenu->addSeparator();


    QMenu* importMenu = fileMenu->addMenu("&Import");
    importMenu->addAction(cmdFeatureMgr->action("RicImportEclipseCaseFeature"));
    importMenu->addAction(cmdFeatureMgr->action("RicImportInputEclipseCaseFeature"));
    importMenu->addAction(cmdFeatureMgr->action("RicImportSummaryCaseFeature"));
    importMenu->addAction(cmdFeatureMgr->action("RicCreateGridCaseGroupFeature"));
    importMenu->addSeparator();
    importMenu->addAction(cmdFeatureMgr->action("RicImportEclipseCaseTimeStepFilterFeature"));
    importMenu->addSeparator();
    #ifdef USE_ODB_API
    importMenu->addAction(cmdFeatureMgr->action("RicImportGeoMechCaseFeature"));
    importMenu->addSeparator();
    #endif
    importMenu->addAction(cmdFeatureMgr->action("RicWellPathsImportFileFeature"));
    importMenu->addAction(cmdFeatureMgr->action("RicWellPathsImportSsihubFeature"));
    importMenu->addAction(cmdFeatureMgr->action("RicWellLogsImportFileFeature"));
    importMenu->addSeparator();
    importMenu->addAction(cmdFeatureMgr->action("RicImportFormationNamesFeature"));

    QMenu* exportMenu = fileMenu->addMenu("&Export");
    exportMenu->addAction(cmdFeatureMgr->action("RicSnapshotViewToFileFeature"));
    exportMenu->addAction(m_snapshotAllViewsToFile);
    exportMenu->addAction(cmdFeatureMgr->action("RicExportMultipleSnapshotsFeature"));

    fileMenu->addSeparator();
    fileMenu->addAction(cmdFeatureMgr->action("RicSaveProjectFeature"));
    fileMenu->addAction(cmdFeatureMgr->action("RicSaveProjectAsFeature"));

    std::vector<QAction*> recentFileActions = RiaApplication::instance()->recentFileActions();
    for (auto act : recentFileActions)
    {
        fileMenu->addAction(act);
    }
    
    fileMenu->addSeparator();
    QMenu* testMenu = fileMenu->addMenu("&Testing");

    fileMenu->addSeparator();
    fileMenu->addAction(cmdFeatureMgr->action("RicCloseProjectFeature"));
    fileMenu->addSeparator();
    fileMenu->addAction(cmdFeatureMgr->action("RicExitApplicationFeature"));

    connect(fileMenu, SIGNAL(aboutToShow()), SLOT(slotRefreshFileActions()));

    // Edit menu
    QMenu* editMenu = menuBar()->addMenu("&Edit");
    editMenu->addAction(cmdFeatureMgr->action("RicSnapshotViewToClipboardFeature"));
    editMenu->addSeparator();
    editMenu->addAction(cmdFeatureMgr->action("RicEditPreferencesFeature"));

    connect(editMenu, SIGNAL(aboutToShow()), SLOT(slotRefreshEditActions()));


    // View menu
    QMenu* viewMenu = menuBar()->addMenu("&View");
    viewMenu->addAction(cmdFeatureMgr->action("RicViewZoomAllFeature"));
    viewMenu->addSeparator();
    viewMenu->addAction(m_viewFromSouth);
    viewMenu->addAction(m_viewFromNorth);
    viewMenu->addAction(m_viewFromWest);
    viewMenu->addAction(m_viewFromEast);
    viewMenu->addAction(m_viewFromBelow);
    viewMenu->addAction(m_viewFromAbove);

    connect(viewMenu, SIGNAL(aboutToShow()), SLOT(slotRefreshViewActions()));

    // Debug menu
    testMenu->addAction(m_mockModelAction);
    testMenu->addAction(m_mockResultsModelAction);
    testMenu->addAction(m_mockLargeResultsModelAction);
    testMenu->addAction(m_mockModelCustomizedAction);
    testMenu->addAction(m_mockInputModelAction);
    testMenu->addSeparator();
    testMenu->addAction(m_createCommandObject);
    testMenu->addSeparator();
    testMenu->addAction(m_showRegressionTestDialog);
    testMenu->addAction(m_executePaintEventPerformanceTest);
    testMenu->addAction(cmdFeatureMgr->action("RicLaunchUnitTestsFeature"));

    // Windows menu
    m_windowMenu = menuBar()->addMenu("&Windows");
    connect(m_windowMenu, SIGNAL(aboutToShow()), SLOT(slotBuildWindowActions()));

    // Help menu
    QMenu* helpMenu = menuBar()->addMenu("&Help");
    helpMenu->addAction(cmdFeatureMgr->action("RicHelpAboutFeature"));
    helpMenu->addAction(cmdFeatureMgr->action("RicHelpCommandLineFeature"));
    helpMenu->addSeparator();
    helpMenu->addAction(cmdFeatureMgr->action("RicHelpOpenUsersGuideFeature"));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::createToolBars()
{
    caf::CmdFeatureManager* cmdFeatureMgr = caf::CmdFeatureManager::instance();
    CVF_ASSERT(cmdFeatureMgr);

    {
        QToolBar* toolbar = addToolBar(tr("Standard"));
        toolbar->setObjectName(toolbar->windowTitle());
        toolbar->addAction(cmdFeatureMgr->action("RicImportEclipseCaseFeature"));
        toolbar->addAction(cmdFeatureMgr->action("RicImportInputEclipseCaseFeature"));
        toolbar->addAction(cmdFeatureMgr->action("RicImportSummaryCaseFeature"));
        toolbar->addAction(cmdFeatureMgr->action("RicOpenProjectFeature"));
        toolbar->addAction(cmdFeatureMgr->action("RicSaveProjectFeature"));
    }

    {
        QToolBar* toolbar = addToolBar(tr("Window Management"));
        toolbar->setObjectName(toolbar->windowTitle());
        toolbar->addAction(cmdFeatureMgr->action("RicShowPlotWindowFeature"));
        toolbar->addAction(cmdFeatureMgr->action("RicLinkVisibleViewsFeature"));
        toolbar->addAction(cmdFeatureMgr->action("RicTileWindowsFeature"));
    }

    {
        QToolBar* toolbar = addToolBar(tr("View Snapshots"));
        toolbar->setObjectName(toolbar->windowTitle());
        toolbar->addAction(cmdFeatureMgr->action("RicSnapshotViewToClipboardFeature"));
        toolbar->addAction(cmdFeatureMgr->action("RicSnapshotViewToFileFeature"));
        toolbar->addAction(m_snapshotAllViewsToFile);
    }


    // View toolbar
    {
        QToolBar* toolbar = addToolBar(tr("View"));
        toolbar->setObjectName(toolbar->windowTitle());
        toolbar->addAction(cmdFeatureMgr->action("RicTogglePerspectiveViewFeature"));
        toolbar->addAction(cmdFeatureMgr->action("RicViewZoomAllFeature"));
        toolbar->addAction(m_viewFromNorth);
        toolbar->addAction(m_viewFromSouth);
        toolbar->addAction(m_viewFromEast);
        toolbar->addAction(m_viewFromWest);
        toolbar->addAction(m_viewFromAbove);
        toolbar->addAction(m_viewFromBelow);

        QLabel* scaleLabel = new QLabel(toolbar);
        scaleLabel->setText("Scale");
        toolbar->addWidget(scaleLabel);

        m_scaleFactor = new QSpinBox(toolbar);
        m_scaleFactor->setValue(0);
        toolbar->addWidget(m_scaleFactor);
        connect(m_scaleFactor, SIGNAL(valueChanged(int)), SLOT(slotScaleChanged(int)));
    }
    
    {
        QToolBar* dsToolBar = addToolBar(tr("Draw Style"));
        dsToolBar->setObjectName(dsToolBar->windowTitle());
        dsToolBar->addAction(m_drawStyleLinesAction);
        dsToolBar->addAction(m_drawStyleLinesSolidAction);
        dsToolBar->addAction(m_drawStyleSurfOnlyAction);
        dsToolBar->addAction(m_drawStyleFaultLinesSolidAction);
        dsToolBar->addAction(m_disableLightingAction);
        dsToolBar->addAction(m_drawStyleHideGridCellsAction);
        dsToolBar->addAction(m_toggleFaultsLabelAction);
        dsToolBar->addAction(m_showWellCellsAction);
    }

    // Create animation toolbar
    m_animationToolBar = new caf::AnimationToolBar("Animation", this);
    addToolBar(m_animationToolBar);
    //connect(m_animationToolBar, SIGNAL(signalFrameRateChanged(double)), SLOT(slotFramerateChanged(double)));

    refreshAnimationActions();
    refreshDrawStyleActions();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------

void RiuMainWindow::createDockPanels()
{
    {
        QDockWidget* dockWidget = new QDockWidget("Project Tree", this);
        dockWidget->setObjectName("dockWidget");
        dockWidget->setAllowedAreas(Qt::AllDockWidgetAreas);

        m_projectTreeView = new caf::PdmUiTreeView(this);
        m_projectTreeView->enableSelectionManagerUpdating(true);

        RiaApplication* app = RiaApplication::instance();
        m_projectTreeView->enableAppendOfClassNameToUiItemText(app->preferences()->appendClassNameToUiText());

        dockWidget->setWidget(m_projectTreeView);

        m_projectTreeView->treeView()->setHeaderHidden(true);
        m_projectTreeView->treeView()->setSelectionMode(QAbstractItemView::ExtendedSelection);

        // Drag and drop configuration
        m_projectTreeView->treeView()->setDragEnabled(true);
        m_projectTreeView->treeView()->viewport()->setAcceptDrops(true);
        m_projectTreeView->treeView()->setDropIndicatorShown(true);
        m_projectTreeView->treeView()->setDragDropMode(QAbstractItemView::DragDrop);

        // Install event filter used to handle key press events
        RiuTreeViewEventFilter* treeViewEventFilter = new RiuTreeViewEventFilter(this);
        m_projectTreeView->treeView()->installEventFilter(treeViewEventFilter);

        addDockWidget(Qt::LeftDockWidgetArea, dockWidget);

        connect(m_projectTreeView, SIGNAL(selectionChanged()), this, SLOT(selectedObjectsChanged()));
        m_projectTreeView->treeView()->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(m_projectTreeView->treeView(), SIGNAL(customContextMenuRequested(const QPoint&)), SLOT(customMenuRequested(const QPoint&)));
    }
    
/*
    {
        QDockWidget* dockWidget = new QDockWidget("Undo stack", this);
        dockWidget->setObjectName("dockWidget");
        dockWidget->setAllowedAreas(Qt::AllDockWidgetAreas);

        m_undoView = new QUndoView(this);
        m_undoView->setStack(caf::CmdExecCommandManager::instance()->undoStack());
        //connect(caf::CmdExecCommandManager::instance()->undoStack(), SIGNAL(indexChanged(int)), SLOT(slotIndexChanged()));

        dockWidget->setWidget(m_undoView);

        addDockWidget(Qt::RightDockWidgetArea, dockWidget);

        dockWidget->hide();

        //m_windowsMenu->addAction(dockWidget->toggleViewAction());
    }
*/

    {
        QDockWidget* dockWidget = new QDockWidget("Property Editor", this);
        dockWidget->setObjectName("dockWidget");
        dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

        m_pdmUiPropertyView = new caf::PdmUiPropertyView(dockWidget);
        dockWidget->setWidget(m_pdmUiPropertyView);

        addDockWidget(Qt::LeftDockWidgetArea, dockWidget);
    }

    {
        QDockWidget* dockPanel = new QDockWidget("Result Info", this);
        dockPanel->setObjectName("dockResultInfoPanel");
        dockPanel->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
        m_resultInfoPanel = new RiuResultInfoPanel(dockPanel);
        dockPanel->setWidget(m_resultInfoPanel);

        addDockWidget(Qt::BottomDockWidgetArea, dockPanel);
    }

    {
        QDockWidget* dockPanel = new QDockWidget("Process Monitor", this);
        dockPanel->setObjectName(DOCK_PANEL_NAME_PROCESS_MONITOR);
        dockPanel->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
        m_processMonitor = new RiuProcessMonitor(dockPanel);
        dockPanel->setWidget(m_processMonitor);

        addDockWidget(Qt::BottomDockWidgetArea, dockPanel);
        dockPanel->hide();
    }

    {
        QDockWidget* dockPanel = new QDockWidget("Result Plot", this);
        dockPanel->setObjectName("dockTimeHistoryPanel");
        dockPanel->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
        m_resultQwtPlot = new RiuResultQwtPlot(dockPanel);
        dockPanel->setWidget(m_resultQwtPlot);

        addDockWidget(Qt::BottomDockWidgetArea, dockPanel);
    }
 
    {
        QDockWidget* dockWidget = new QDockWidget("Messages", this);
        dockWidget->setObjectName("dockMessages");
        m_messagePanel = new RiuMessagePanel(dockWidget);
        dockWidget->setWidget(m_messagePanel);
        addDockWidget(Qt::BottomDockWidgetArea, dockWidget);
        dockWidget->hide();
    }

    setCorner(Qt::BottomLeftCorner,    Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomRightCorner, Qt::BottomDockWidgetArea);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::setResultInfo(const QString& info) const
{
    m_resultInfoPanel->setInfo(info);
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
    RiaApplication* app = RiaApplication::instance();

    bool projectFileExists = caf::Utils::fileExists(app->project()->fileName());

    caf::CmdFeatureManager* cmdFeatureMgr = caf::CmdFeatureManager::instance();
    CVF_ASSERT(cmdFeatureMgr);

    cmdFeatureMgr->action("RicWellPathsImportSsihubFeature")->setEnabled(projectFileExists);
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
    bool enabled = true;
    m_viewFromNorth->setEnabled(enabled);
    m_viewFromSouth->setEnabled(enabled);
    m_viewFromEast->setEnabled(enabled);
    m_viewFromWest->setEnabled(enabled);
    m_viewFromAbove->setEnabled(enabled);
    m_viewFromBelow->setEnabled(enabled);

    updateScaleValue();

    caf::CmdFeatureManager::instance()->refreshEnabledState(QStringList() << "RicLinkVisibleViewsFeature" << "RicTileWindowsFeature" << "RicTogglePerspectiveViewFeature");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::refreshAnimationActions()
{
    caf::FrameAnimationControl* animationControl = NULL;
    if (RiaApplication::instance()->activeReservoirView() && RiaApplication::instance()->activeReservoirView()->viewer())
    {
        animationControl = RiaApplication::instance()->activeReservoirView()->viewer()->animationControl();
    }

    m_animationToolBar->connectAnimationControl(animationControl);

    QStringList timeStepStrings;

    int currentTimeStepIndex = 0;

    bool enableAnimControls = false;
    RimView * activeView = RiaApplication::instance()->activeReservoirView();
    if (activeView && 
        activeView->viewer() &&
        activeView->viewer()->frameCount())
    {
        enableAnimControls = true;
        RimEclipseView * activeRiv = dynamic_cast<RimEclipseView*>(activeView);

        if (activeRiv)
        {
            if (activeRiv->currentGridCellResults())
            {
                if (activeRiv->isTimeStepDependentDataVisible())
                {
                    timeStepStrings = activeRiv->eclipseCase()->timeStepStrings();
                }
                else
                {
                    timeStepStrings.push_back(tr("Static Property"));
                }
            }
        }
        else
        {
            RimGeoMechView * activeGmv = dynamic_cast<RimGeoMechView*>(activeView);
            if (activeGmv)
            {
                if (activeGmv->isTimeStepDependentDataVisible())
                {
                    timeStepStrings = activeGmv->geoMechCase()->timeStepStrings();
                }
            }
        }

        currentTimeStepIndex = activeView->currentTimeStep();

        // Animation control is only relevant for more than one time step

        if (timeStepStrings.size() < 2)
        {
            enableAnimControls = false;
        }

        m_animationToolBar->setFrameRate(activeView->maximumFrameRate());
    }

    m_animationToolBar->setTimeStepStrings(timeStepStrings);
    m_animationToolBar->setCurrentTimeStepIndex(currentTimeStepIndex);

    m_animationToolBar->setEnabled(enableAnimControls);
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
QMdiSubWindow* RiuMainWindow::findMdiSubWindow(QWidget* viewer)
{
    QList<QMdiSubWindow*> subws = m_mdiArea->subWindowList();
    int i; 
    for (i = 0; i < subws.size(); ++i)
    {
        if (subws[i]->widget() == viewer)
        {
            return subws[i];
        }
    }

    return NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<QMdiSubWindow*> RiuMainWindow::subWindowList(QMdiArea::WindowOrder order)
{
	return m_mdiArea->subWindowList(order);
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
RiuMessagePanel* RiuMainWindow::messagePanel()
{
    return m_messagePanel;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::removeViewer(QWidget* viewer)
{
    m_blockSlotSubWindowActivated = true;
    m_mdiArea->removeSubWindow(findMdiSubWindow(viewer));
    m_blockSlotSubWindowActivated = false;

    slotRefreshViewActions();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::addViewer(QWidget* viewer, const RimMdiWindowGeometry& windowsGeometry)
{
    RiuMdiSubWindow* subWin = new RiuMdiSubWindow(m_mdiArea);
    subWin->setAttribute(Qt::WA_DeleteOnClose); // Make sure the contained widget is destroyed when the MDI window is closed
    subWin->setWidget(viewer);

    QSize subWindowSize;
    QPoint subWindowPos(-1, -1);
    bool initialStateMaximized = false;

    if (windowsGeometry.isValid())
    {
        subWindowPos = QPoint(windowsGeometry.x, windowsGeometry.y);
        subWindowSize = QSize(windowsGeometry.width, windowsGeometry.height);

        initialStateMaximized = windowsGeometry.isMaximized;
    }
    else
    {
        subWindowSize = QSize(400, 400);

        if (m_mdiArea->subWindowList().size() < 1)
        {
            // Show first 3D view maximized
            initialStateMaximized = true;
        }
    }

    if (m_mdiArea->currentSubWindow() && m_mdiArea->currentSubWindow()->isMaximized())
    {
        initialStateMaximized = true;
    }
        
    subWin->show();

    // Move and resize must be done after window is visible
    // If not, the position and size of the window is different to specification (Windows 7)
    // Might be a Qt bug, must be tested on Linux
    if (subWindowPos.x() > -1)
    {
        subWin->move(subWindowPos);
    }
    subWin->resize(subWindowSize);

    if (initialStateMaximized)
    {
        subWin->showMaximized();
    }

    slotRefreshViewActions();
}

//--------------------------------------------------------------------------------------------------
/// This method needs to handle memory deallocation !!!
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::setPdmRoot(caf::PdmObject* pdmRoot)
{
    m_pdmRoot = pdmRoot;

    m_projectTreeView->setPdmItem(pdmRoot);
    // For debug only : m_projectTreeView->treeView()->expandAll();
    m_projectTreeView->setDragDropInterface(m_dragDropInterface.get());

    for (size_t i = 0; i < additionalProjectViews.size(); i++)
    {
        if (!additionalProjectViews[i]) continue;

        RiuProjectAndPropertyView* projPropView = dynamic_cast<RiuProjectAndPropertyView*>(additionalProjectViews[i]->widget());
        if (projPropView)
        {
            projPropView->setPdmItem(pdmRoot);
        }
    }

    caf::SelectionManager::instance()->setPdmRootObject(pdmRoot);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotViewFromNorth()
{
    if (RiaApplication::instance()->activeReservoirView() &&  RiaApplication::instance()->activeReservoirView()->viewer())
    {
        RiaApplication::instance()->activeReservoirView()->viewer()->setView(cvf::Vec3d(0,-1,0), cvf::Vec3d(0,0,1));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotViewFromSouth()
{
    if (RiaApplication::instance()->activeReservoirView() &&  RiaApplication::instance()->activeReservoirView()->viewer())
    {
        RiaApplication::instance()->activeReservoirView()->viewer()->setView(cvf::Vec3d(0,1,0), cvf::Vec3d(0,0,1));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotViewFromEast()
{
    if (RiaApplication::instance()->activeReservoirView() &&  RiaApplication::instance()->activeReservoirView()->viewer())
    {
        RiaApplication::instance()->activeReservoirView()->viewer()->setView(cvf::Vec3d(-1,0,0), cvf::Vec3d(0,0,1));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotViewFromWest()
{
    if (RiaApplication::instance()->activeReservoirView() &&  RiaApplication::instance()->activeReservoirView()->viewer())
    {
        RiaApplication::instance()->activeReservoirView()->viewer()->setView(cvf::Vec3d(1,0,0), cvf::Vec3d(0,0,1));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotViewFromAbove()
{
    if (RiaApplication::instance()->activeReservoirView() &&  RiaApplication::instance()->activeReservoirView()->viewer())
    {
        RiaApplication::instance()->activeReservoirView()->viewer()->setView(cvf::Vec3d(0,0,-1), cvf::Vec3d(0,1,0));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotViewFromBelow()
{
    if (RiaApplication::instance()->activeReservoirView() &&  RiaApplication::instance()->activeReservoirView()->viewer())
    {
        RiaApplication::instance()->activeReservoirView()->viewer()->setView(cvf::Vec3d(0,0,1), cvf::Vec3d(0,1,0));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotSubWindowActivated(QMdiSubWindow* subWindow)
{
    if (!subWindow) return;
    if (m_blockSlotSubWindowActivated) return;

    RimProject * proj = RiaApplication::instance()->project();
    if (!proj) return;

    // Find the activated 3D view
    
    RimView* activatedView = NULL;

    std::vector<RimCase*> allCases;
    proj->allCases(allCases);

    for (size_t caseIdx = 0; caseIdx < allCases.size(); ++caseIdx)
    {
        RimCase* reservoirCase = allCases[caseIdx];
        if (reservoirCase == NULL) continue;

        std::vector<RimView*> views = reservoirCase->views();

        size_t viewIdx;
        for (viewIdx = 0; viewIdx < views.size(); viewIdx++)
        {
            RimView* riv = views[viewIdx];

            if (riv &&
                riv->viewer() &&
                riv->viewer()->layoutWidget() &&
                riv->viewer()->layoutWidget()->parent() == subWindow)
            {
                activatedView = riv;
                break;
            }
        }
    }

    {
        RimView* previousActiveReservoirView = RiaApplication::instance()->activeReservoirView();
        RiaApplication::instance()->setActiveReservoirView(activatedView);

        if (previousActiveReservoirView != activatedView)
        {
            QModelIndex newViewModelIndex = m_projectTreeView->findModelIndex(activatedView);
            QModelIndex newSelectionIndex = newViewModelIndex;

            if (previousActiveReservoirView)
            {
                // Try to select the same entry in the new View, as was selected in the previous

                QModelIndex previousViewModelIndex = m_projectTreeView->findModelIndex(previousActiveReservoirView);
                QModelIndex currentSelectionIndex = m_projectTreeView->treeView()->selectionModel()->currentIndex();

                if (currentSelectionIndex != newViewModelIndex &&
                    currentSelectionIndex.isValid())
                {
                    QVector<QModelIndex> route; // Contains all model indices from current selection up to previous view

                    QModelIndex tmpModelIndex = currentSelectionIndex;

                    while (tmpModelIndex.isValid() && tmpModelIndex != previousViewModelIndex)
                    {
                        // NB! Add model index to front of vector to be able to do a for-loop with correct ordering
                        route.push_front(tmpModelIndex);

                        tmpModelIndex = tmpModelIndex.parent();
                    }

                    // Traverse model indices from new view index to currently selected item
                    int i;
                    for (i = 0; i < route.size(); i++)
                    {
                        QModelIndex tmp = route[i];
                        if (newSelectionIndex.isValid())
                        {
                            newSelectionIndex = m_projectTreeView->treeView()->model()->index(tmp.row(), tmp.column(), newSelectionIndex);
                        }
                    }

                    // Use view model index if anything goes wrong
                    if (!newSelectionIndex.isValid())
                    {
                        newSelectionIndex = newViewModelIndex;
                    }
                }
            }

            m_projectTreeView->treeView()->setCurrentIndex(newSelectionIndex);
            if (newSelectionIndex != newViewModelIndex)
            {
                m_projectTreeView->treeView()->setExpanded(newViewModelIndex, true);
            }

        }

        slotRefreshViewActions();
        refreshAnimationActions();
        refreshDrawStyleActions();
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotUseShaders(bool enable)
{
    RiaApplication::instance()->setUseShaders(enable);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotShowPerformanceInfo(bool enable)
{
    RiaApplication::instance()->setShowPerformanceInfo(enable);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::setActiveViewer(QWidget* viewer)
{
   m_blockSlotSubWindowActivated = true;
   
   QMdiSubWindow * swin = findMdiSubWindow(viewer);
   if (swin) m_mdiArea->setActiveSubWindow(swin);
   
   m_blockSlotSubWindowActivated = false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotFramerateChanged(double frameRate)
{
    if (RiaApplication::instance()->activeReservoirView() != NULL)
    {
        RiaApplication::instance()->activeReservoirView()->maximumFrameRate.setValueWithFieldChanged(frameRate);
    }
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
    m_windowMenu->addAction(m_newPropertyView);
    m_windowMenu->addSeparator();

    QList<QDockWidget*> dockWidgets = findChildren<QDockWidget*>();

    int i = 0;
    foreach (QDockWidget* dock, dockWidgets)
    {
        if (dock)
        {
            if (i == 4) m_windowMenu->addSeparator();
            m_windowMenu->addAction(dock->toggleViewAction());
            ++i;
        }
    }

    m_windowMenu->addSeparator();
    QAction* cascadeWindowsAction = new QAction("Cascade Windows", this);
    connect(cascadeWindowsAction, SIGNAL(triggered()), m_mdiArea, SLOT(cascadeSubWindows()));

    QAction* closeAllSubWindowsAction = new QAction("Close All Windows", this);
    connect(closeAllSubWindowsAction, SIGNAL(triggered()), m_mdiArea, SLOT(closeAllSubWindows()));

    m_windowMenu->addAction(caf::CmdFeatureManager::instance()->action("RicTileWindowsFeature"));
    m_windowMenu->addAction(cascadeWindowsAction);
    m_windowMenu->addAction(closeAllSubWindowsAction);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::selectedObjectsChanged()
{
    std::vector<caf::PdmUiItem*> uiItems;
    m_projectTreeView->selectedUiItems(uiItems);

    caf::PdmObjectHandle* firstSelectedObject = NULL;

    if (uiItems.size() == 1)
    {
        firstSelectedObject = dynamic_cast<caf::PdmObjectHandle*>(uiItems[0]);
    }
    m_pdmUiPropertyView->showProperties(firstSelectedObject);

    if (uiItems.size() == 1)
    {
        // Find the reservoir view or the Plot that the selected item is within 

        if (!firstSelectedObject)
        {
            caf::PdmFieldHandle* selectedField = dynamic_cast<caf::PdmFieldHandle*>(uiItems[0]);
            if (selectedField) firstSelectedObject = selectedField->ownerObject();
        }

        if (!firstSelectedObject) return;

        // First check if we are within a RimView
        RimView* selectedReservoirView = dynamic_cast<RimView*>(firstSelectedObject);
        if (!selectedReservoirView)
        {
            firstSelectedObject->firstAncestorOrThisOfType(selectedReservoirView);
        }

        bool isActiveViewChanged = false;
        
        if (selectedReservoirView)
        {
            // Set focus in MDI area to this window if it exists
            if (selectedReservoirView->viewer())
            {
                setActiveViewer(selectedReservoirView->viewer()->layoutWidget());
            }
            isActiveViewChanged = true;
        }

        if (isActiveViewChanged)
        {
            RiaApplication::instance()->setActiveReservoirView(selectedReservoirView);
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
    QDockWidget* dockWidget = new QDockWidget(QString("Additional Project Tree (%1)").arg(additionalProjectViews.size() + 1), this);
    dockWidget->setObjectName("dockWidget");
    dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    RiuProjectAndPropertyView* projPropView = new RiuProjectAndPropertyView(dockWidget);
    dockWidget->setWidget(projPropView);
    projPropView->setPdmItem(m_pdmRoot);

    addDockWidget(Qt::RightDockWidgetArea, dockWidget);

    additionalProjectViews.push_back(dockWidget);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotSnapshotAllViewsToFile()
{
    RiaApplication* app = RiaApplication::instance();

    // Save images in snapshot catalog relative to project directory
    QString absolutePathToSnapshotDir = app->createAbsolutePathFromProjectRelativePath("snapshots");
    RicSnapshotAllViewsToFileFeature::exportSnapshotOfAllViewsIntoFolder(absolutePathToSnapshotDir);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::hideAllDockWindows()
{
    QList<QDockWidget*> dockWidgets = findChildren<QDockWidget*>();

    for (int i = 0; i < dockWidgets.size(); i++)
    {
        dockWidgets[i]->close();
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
/*void RiuMainWindow::slotOpenMultipleCases()
{
#if 1
    QAction* action = caf::CmdFeatureManager::instance()->action("RicCreateGridCaseGroupFeature");
    CVF_ASSERT(action);

    action->trigger();

#else  // Code to fast generate a test project
    RiaApplication* app = RiaApplication::instance();

    QStringList gridFileNames;

    if (1)
    {
        gridFileNames += RiaDefines::mockModelBasicWithResults();
        gridFileNames += RiaDefines::mockModelBasicWithResults();
        gridFileNames += RiaDefines::mockModelBasicWithResults();
    }
    else
    {
        gridFileNames += "d:/Models/Statoil/MultipleRealisations/Case_with_10_timesteps/Real0/BRUGGE_0000.EGRID";
        gridFileNames += "d:/Models/Statoil/MultipleRealisations/Case_with_10_timesteps/Real10/BRUGGE_0010.EGRID";
        gridFileNames += "d:/Models/Statoil/MultipleRealisations/Case_with_10_timesteps/Real30/BRUGGE_0030.EGRID";
        gridFileNames += "d:/Models/Statoil/MultipleRealisations/Case_with_10_timesteps/Real40/BRUGGE_0040.EGRID";
    }

    app->addEclipseCases(gridFileNames);
#endif

}
*/
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotDrawStyleChanged(QAction* activatedAction)
{
    if (!RiaApplication::instance()->activeReservoirView()) return;

    if (activatedAction == m_drawStyleLinesAction)
    {
        RiaApplication::instance()->activeReservoirView()->setMeshOnlyDrawstyle();
    }
    else if (activatedAction == m_drawStyleLinesSolidAction)
    {
        RiaApplication::instance()->activeReservoirView()->setMeshSurfDrawstyle();
    }
    else if (activatedAction == m_drawStyleSurfOnlyAction)
    {
        RiaApplication::instance()->activeReservoirView()->setSurfOnlyDrawstyle();
    }
    else if (activatedAction == m_drawStyleFaultLinesSolidAction)
    {
        RiaApplication::instance()->activeReservoirView()->setFaultMeshSurfDrawstyle();
    }

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotToggleHideGridCellsAction(bool hideGridCells)
{
    if (!RiaApplication::instance()->activeReservoirView()) return;

    RiaApplication::instance()->activeReservoirView()->showGridCells(!hideGridCells);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotToggleFaultLabelsAction(bool showLabels)
{
    RimEclipseView* activeRiv = dynamic_cast<RimEclipseView*>(RiaApplication::instance()->activeReservoirView());
    if (!activeRiv) return;

    activeRiv->faultCollection->showFaultLabel.setValueWithFieldChanged(showLabels);

    refreshDrawStyleActions();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::refreshDrawStyleActions()
{
    RimView* view = RiaApplication::instance()->activeReservoirView();
    bool enable = view != NULL;

    m_drawStyleLinesAction->setEnabled(enable);
    m_drawStyleLinesSolidAction->setEnabled(enable);
    m_drawStyleSurfOnlyAction->setEnabled(enable);
    m_drawStyleFaultLinesSolidAction->setEnabled(enable);
    m_disableLightingAction->setEnabled(enable);

    bool lightingDisabledInView = view ? view->isLightingDisabled() : false;

    m_disableLightingAction->blockSignals(true);
    m_disableLightingAction->setChecked(lightingDisabledInView);
    m_disableLightingAction->blockSignals(false);

    if (enable)
    {
        m_drawStyleHideGridCellsAction->setEnabled(true);
        m_drawStyleHideGridCellsAction->blockSignals(true);
        m_drawStyleHideGridCellsAction->setChecked(!view->isGridVisualizationMode());
        m_drawStyleHideGridCellsAction->blockSignals(false);
    }

    RimEclipseView* eclView = dynamic_cast<RimEclipseView*>(view);
    enable = enable && eclView;

    m_toggleFaultsLabelAction->setEnabled(enable);
    m_showWellCellsAction->setEnabled(enable);

    if (enable) 
    {   
        m_toggleFaultsLabelAction->blockSignals(true);
        m_toggleFaultsLabelAction->setChecked(eclView->faultCollection()->showFaultLabel());
        m_toggleFaultsLabelAction->blockSignals(false);

        m_showWellCellsAction->blockSignals(true);
        eclView->wellCollection()->updateStateForVisibilityCheckboxes();
        m_showWellCellsAction->setChecked(eclView->wellCollection()->showWellCells());
        m_showWellCellsAction->blockSignals(false);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotDisableLightingAction(bool disable)
{
    RimView* view = RiaApplication::instance()->activeReservoirView();
    if (view)
    {
        view->disableLighting(disable);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::restoreTreeViewState()
{
    if (m_projectTreeView)
    {
        QString stateString = RiaApplication::instance()->project()->mainWindowTreeViewState;
        if (!stateString.isEmpty())
        {
            m_projectTreeView->treeView()->collapseAll();
            RimTreeViewStateSerializer::applyTreeViewStateFromString(m_projectTreeView->treeView(), stateString);
        }

        QString currentIndexString = RiaApplication::instance()->project()->mainWindowCurrentModelIndexPath;
        if (!currentIndexString.isEmpty())
        {
            QModelIndex mi = RimTreeViewStateSerializer::getModelIndexFromString(m_projectTreeView->treeView()->model(), currentIndexString);
            m_projectTreeView->treeView()->setCurrentIndex(mi);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::showDockPanel(const QString &dockPanelName)
{
    QList<QDockWidget*> dockWidgets = findChildren<QDockWidget*>();

    foreach (QDockWidget* dock, dockWidgets)
    {
        if (dock && dock->objectName() == dockPanelName)
        {
            dock->show();
            return;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::showProcessMonitorDockPanel()
{
    showDockPanel(DOCK_PANEL_NAME_PROCESS_MONITOR);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::selectAsCurrentItem(caf::PdmObject* object)
{
    m_projectTreeView->selectAsCurrentItem(object);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotScaleChanged(int scaleValue)
{
    if (RiaApplication::instance()->activeReservoirView())
    {
        RiaApplication::instance()->activeReservoirView()->scaleZ.setValueWithFieldChanged(scaleValue);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::updateScaleValue()
{
    if (RiaApplication::instance()->activeReservoirView())
    {
        m_scaleFactor->setEnabled(true);

        int scaleValue = static_cast<int>(RiaApplication::instance()->activeReservoirView()->scaleZ()); // Round down is probably ok.
        m_scaleFactor->blockSignals(true);
        m_scaleFactor->setValue(scaleValue);
        m_scaleFactor->blockSignals(false);
    }
    else
    {
        m_scaleFactor->setEnabled(false);
    }
}

//--------------------------------------------------------------------------------------------------
/// TODO: This function will be moved to a class responsible for handling the application selection concept
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::selectedCases(std::vector<RimCase*>& cases)
{
    caf::SelectionManager::instance()->objectsByType(&cases);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotCreateCommandObject()
{
    RiaApplication* app = RiaApplication::instance();
    if (!app->project()) return;

    std::vector<caf::PdmUiItem*> selectedUiItems;
    m_projectTreeView->selectedUiItems(selectedUiItems);

    caf::PdmObjectGroup selectedObjects;
    for (size_t i = 0; i < selectedUiItems.size(); ++i)
    {
        caf::PdmUiObjectHandle* uiObj = dynamic_cast<caf::PdmUiObjectHandle*>(selectedUiItems[i]);
        if (uiObj) 
        {
            selectedObjects.addObject(uiObj->objectHandle());
        }
    }

    if (selectedObjects.objects.size())
    { 
        std::vector<RimCommandObject*> commandObjects;
        RimCommandFactory::createCommandObjects(selectedObjects, &commandObjects);

        for (size_t i = 0; i < commandObjects.size(); i++)
        {
            app->project()->commandObjects.push_back(commandObjects[i]);
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

    RiaApplication* app = RiaApplication::instance();
    caf::PdmSettings::readFieldsFromApplicationStore(&regTestConfig);

    caf::PdmUiPropertyViewDialog regressionTestDialog(this, &regTestConfig, "Regression Test", "");
    regressionTestDialog.resize(QSize(600, 200));

    if (regressionTestDialog.exec() == QDialog::Accepted)
    {
        // Write preferences using QSettings and apply them to the application
        caf::PdmSettings::writeFieldsToApplicationStore(&regTestConfig);

        QString currentApplicationPath = QDir::currentPath();

        QDir::setCurrent(regTestConfig.applicationWorkingFolder);
        app->executeRegressionTests(regTestConfig.regressionTestFolder);

        QDir::setCurrent(currentApplicationPath);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotExecutePaintEventPerformanceTest()
{

    if (RiaApplication::instance()->activeReservoirView() &&  RiaApplication::instance()->activeReservoirView()->viewer())
    {
        size_t redrawCount = 50;

        caf::Viewer* viewer = RiaApplication::instance()->activeReservoirView()->viewer();

        cvf::Timer timer;
        for (size_t i = 0; i < redrawCount; i++)
        {
            viewer->repaint();
        }

        double totalTimeMS = timer.time() * 1000.0;

        double msPerFrame = totalTimeMS  / redrawCount;

        QString resultInfo = QString("Total time '%1 ms' for %2 number of redraws, frame time '%3 ms'").arg(totalTimeMS).arg(redrawCount).arg(msPerFrame);
        setResultInfo(resultInfo);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::setDefaultWindowSize()
{
    resize(1000, 810);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotShowWellCellsAction(bool doAdd)
{
    RimEclipseView* riv = dynamic_cast<RimEclipseView*>(RiaApplication::instance()->activeReservoirView());
    if (riv)
    {
        riv->wellCollection()->setShowWellCellsState(doAdd);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::setExpanded(const caf::PdmUiItem* uiItem, bool expanded)
{
    m_projectTreeView->setExpanded(uiItem, expanded);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::customMenuRequested(const QPoint& pos)
{
    QMenu menu;

    RiaApplication* app = RiaApplication::instance();
    app->project()->actionsBasedOnSelection(menu);

    // Qt doc: QAbstractScrollArea and its subclasses that map the context menu event to coordinates of the viewport().
    // Since we might get this signal from different treeViews, we need to map the position accordingly.  
    QObject* senderObj = this->sender();
    QTreeView* treeView = dynamic_cast<QTreeView*>(senderObj); 
    if (treeView)
    {
        QPoint globalPos = treeView->viewport()->mapToGlobal(pos);
        menu.exec(globalPos);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimMdiWindowGeometry RiuMainWindow::windowGeometryForViewer(QWidget* viewer)
{
    QMdiSubWindow* mdiWindow = findMdiSubWindow(viewer);
    if (mdiWindow)
    {
        return RiuMdiSubWindow::windowGeometryForWidget(mdiWindow);
    }

    RimMdiWindowGeometry geo;
    return geo;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::tileWindows()
{
    // Based on workaround described here
    // https://forum.qt.io/topic/50053/qmdiarea-tilesubwindows-always-places-widgets-in-activationhistoryorder-in-subwindowview-mode

    QMdiSubWindow *a = m_mdiArea->activeSubWindow();
    QList<QMdiSubWindow *> list = m_mdiArea->subWindowList(m_mdiArea->activationOrder());
    for (int i = 0; i < list.count(); i++)
    {
        m_mdiArea->setActiveSubWindow(list[i]);
    }

    m_mdiArea->tileSubWindows();
    m_mdiArea->setActiveSubWindow(a);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiuMainWindow::isAnyMdiSubWindowVisible()
{
    return m_mdiArea->subWindowList().size() > 0;
}

