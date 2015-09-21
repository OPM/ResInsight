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

#include "RiaStdInclude.h"

#include "RiuMainWindow.h"

#include "RiaApplication.h"
#include "RiaBaseDefs.h"
#include "RiaPreferences.h"
#include "RiaRegressionTest.h"

#include "RigCaseCellResultsData.h"
#include "RigFemPartResultsCollection.h"
#include "RigGeoMechCaseData.h"

#include "RimCaseCollection.h"
#include "RimCommandObject.h"
#include "RimEclipseCase.h"
#include "RimEclipseCaseCollection.h"
#include "RimEclipseCellColors.h"
#include "RimEclipsePropertyFilterCollection.h"
#include "RimEclipseView.h"
#include "RimEclipseWellCollection.h"
#include "RimFaultCollection.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechCellColors.h"
#include "RimGeoMechModels.h"
#include "RimGeoMechView.h"
#include "RimGeoMechView.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimReservoirCellResultsStorage.h"
#include "RimTools.h"
#include "RimWellPathCollection.h"
#include "RimWellPathImport.h"
#include "RimWellLogPlot.h"
#include "RimWellLogPlotCollection.h"
#include "RimMainPlotCollection.h"

#include "RiuMultiCaseImportDialog.h"
#include "RiuProcessMonitor.h"
#include "RiuResultInfoPanel.h"
#include "RiuViewer.h"
#include "RiuWellImportWizard.h"
#include "RiuDragDrop.h"
#include "RiuWellLogPlot.h"

#include "cafAboutDialog.h"
#include "cafAnimationToolBar.h"
#include "cafCmdExecCommandManager.h"
#include "cafCmdFeatureManager.h"
#include "cafPdmDefaultObjectFactory.h"
#include "cafPdmFieldCvfMat4d.h"
#include "cafPdmObjectGroup.h"
#include "cafPdmSettings.h"
#include "cafPdmUiPropertyView.h"
#include "cafPdmUiPropertyViewDialog.h"
#include "cafPdmUiTreeView.h"
#include "cafSelectionManager.h"

#include "cvfTimer.h"
#include "RimTreeViewStateSerializer.h"
#include "RiuTreeViewEventFilter.h"
#include "RiuProjectPropertyView.h"


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

    m_dragDrop = new RiuDragDrop;

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

    m_processMonitor->slotClearTextEdit();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::cleanupGuiBeforeProjectClose()
{
    caf::CmdExecCommandManager::instance()->undoStack()->clear();

    setPdmRoot(NULL);
    setResultInfo("");
    
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
void RiuMainWindow::closeEvent(QCloseEvent* event)
{
    if (!RiaApplication::instance()->closeProject(true))
    {
        event->ignore();
        return;
    }

    delete m_dragDrop;
    
    saveWinGeoAndDockToolBarLayout();
        
    event->accept();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::createActions()
{
    // File actions
    m_openProjectAction         = new QAction(style()->standardIcon(QStyle::SP_DirOpenIcon), "&Open Project", this);
    m_openLastUsedProjectAction = new QAction("Open &Last Used Project", this);

    m_importGeoMechCaseAction     = new QAction(QIcon(":/GeoMechCase48x48.png"), "Import &Geo Mechanical Model", this);

    m_mockModelAction           = new QAction("&Mock Model", this);
    m_mockResultsModelAction    = new QAction("Mock Model With &Results", this);
    m_mockLargeResultsModelAction = new QAction("Large Mock Model", this);
    m_mockModelCustomizedAction = new QAction("Customized Mock Model", this);
    m_mockInputModelAction      = new QAction("Input Mock Model", this);

    m_snapshotToFile            = new QAction(QIcon(":/SnapShotSave.png"), "Snapshot To File", this);
    m_snapshotToClipboard       = new QAction(QIcon(":/SnapShot.png"), "Copy Snapshot To Clipboard", this);
    m_snapshotAllViewsToFile    = new QAction(QIcon(":/SnapShotSaveViews.png"), "Snapshot All Views To File", this);

    m_createCommandObject       = new QAction("Create Command Object", this);
    m_showRegressionTestDialog  = new QAction("Regression Test Dialog", this);
    m_executePaintEventPerformanceTest = new QAction("&Paint Event Performance Test", this);

    m_saveProjectAction         = new QAction(QIcon(":/Save.png"), "&Save Project", this);
    m_saveProjectAsAction       = new QAction(QIcon(":/Save.png"), "Save Project &As", this);

    m_closeProjectAction               = new QAction("&Close Project", this);

    for (int i = 0; i < MaxRecentFiles; ++i)
    {
        m_recentFileActions[i] = new QAction(this);
        m_recentFileActions[i]->setVisible(false);
        connect(m_recentFileActions[i], SIGNAL(triggered()), this, SLOT(slotOpenRecentFile()));
    }

    m_exitAction = new QAction("E&xit", this);

    connect(m_openProjectAction,	            SIGNAL(triggered()), SLOT(slotOpenProject()));
    connect(m_openLastUsedProjectAction,        SIGNAL(triggered()), SLOT(slotOpenLastUsedProject()));

    connect(m_importGeoMechCaseAction,          SIGNAL(triggered()), SLOT(slotImportGeoMechModel()));
    
    connect(m_mockModelAction,	        SIGNAL(triggered()), SLOT(slotMockModel()));
    connect(m_mockResultsModelAction,	SIGNAL(triggered()), SLOT(slotMockResultsModel()));
    connect(m_mockLargeResultsModelAction,	SIGNAL(triggered()), SLOT(slotMockLargeResultsModel()));
    connect(m_mockModelCustomizedAction,	SIGNAL(triggered()), SLOT(slotMockModelCustomized()));
    connect(m_mockInputModelAction,	    SIGNAL(triggered()), SLOT(slotInputMockModel()));

    connect(m_snapshotToFile,	        SIGNAL(triggered()), SLOT(slotSnapshotToFile()));
    connect(m_snapshotToClipboard,	    SIGNAL(triggered()), SLOT(slotSnapshotToClipboard()));
    connect(m_snapshotAllViewsToFile,   SIGNAL(triggered()), SLOT(slotSnapshotAllViewsToFile()));

    connect(m_createCommandObject,      SIGNAL(triggered()), SLOT(slotCreateCommandObject()));
    connect(m_showRegressionTestDialog, SIGNAL(triggered()), SLOT(slotShowRegressionTestDialog()));
    connect(m_executePaintEventPerformanceTest, SIGNAL(triggered()), SLOT(slotExecutePaintEventPerformanceTest()));
    
    connect(m_saveProjectAction,	    SIGNAL(triggered()), SLOT(slotSaveProject()));
    connect(m_saveProjectAsAction,	    SIGNAL(triggered()), SLOT(slotSaveProjectAs()));

    connect(m_closeProjectAction,	            SIGNAL(triggered()), SLOT(slotCloseProject()));

    connect(m_exitAction, SIGNAL(triggered()), QApplication::instance(), SLOT(closeAllWindows()));

    // Edit actions
    m_editPreferences                = new QAction("&Preferences...", this);
    connect(m_editPreferences,	  SIGNAL(triggered()), SLOT(slotEditPreferences()));

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

    m_zoomAll                = new QAction(QIcon(":/ZoomAll16x16.png"),"Zoom all", this);
    m_zoomAll->setToolTip("Zoom to view all");

    connect(m_viewFromNorth,	            SIGNAL(triggered()), SLOT(slotViewFromNorth()));
    connect(m_viewFromSouth,	            SIGNAL(triggered()), SLOT(slotViewFromSouth()));
    connect(m_viewFromEast, 	            SIGNAL(triggered()), SLOT(slotViewFromEast()));
    connect(m_viewFromWest, 	            SIGNAL(triggered()), SLOT(slotViewFromWest()));
    connect(m_viewFromAbove,	            SIGNAL(triggered()), SLOT(slotViewFromAbove()));
    connect(m_viewFromBelow,	            SIGNAL(triggered()), SLOT(slotViewFromBelow()));
    connect(m_zoomAll,	                    SIGNAL(triggered()), SLOT(slotZoomAll()));

    // Debug actions
    m_newPropertyView = new QAction("New Project and Property View", this);
    connect(m_newPropertyView, SIGNAL(triggered()), SLOT(slotNewObjectPropertyView()));

    // Help actions
    m_aboutAction = new QAction("&About", this);    
    connect(m_aboutAction, SIGNAL(triggered()), SLOT(slotAbout()));
    m_commandLineHelpAction = new QAction("&Command Line Help", this);    
    connect(m_commandLineHelpAction, SIGNAL(triggered()), SLOT(slotShowCommandLineHelp()));
    m_openUsersGuideInBrowserAction = new QAction("&Users Guide", this);    
    connect(m_openUsersGuideInBrowserAction, SIGNAL(triggered()), SLOT(slotOpenUsersGuideInBrowserAction()));

    // Draw style actions
    m_dsActionGroup = new QActionGroup(this);

    m_drawStyleLinesAction                = new QAction(QIcon(":/draw_style_lines_24x24.png"), "&Mesh Only", this);
    //connect(m_drawStyleLinesAction,	    SIGNAL(triggered()), SLOT(slotDrawStyleLines()));
    m_dsActionGroup->addAction(m_drawStyleLinesAction);

     m_drawStyleLinesSolidAction           = new QAction(QIcon(":/draw_style_meshlines_24x24.png"), "Mesh And Surfaces", this);
    //connect(m_drawStyleLinesSolidAction,	SIGNAL(triggered()), SLOT(slotDrawStyleLinesSolid()));
     m_dsActionGroup->addAction(m_drawStyleLinesSolidAction);

     m_drawStyleFaultLinesSolidAction           = new QAction(QIcon(":/draw_style_surface_w_fault_mesh_24x24.png"), "Fault Mesh And Surfaces", this);
     m_dsActionGroup->addAction(m_drawStyleFaultLinesSolidAction);

    m_drawStyleSurfOnlyAction             = new QAction(QIcon(":/draw_style_surface_24x24.png"), "&Surface Only", this);
    //connect(m_drawStyleSurfOnlyAction,	SIGNAL(triggered()), SLOT(slotDrawStyleSurfOnly()));
    m_dsActionGroup->addAction(m_drawStyleSurfOnlyAction);


    connect(m_dsActionGroup, SIGNAL(triggered(QAction*)), SLOT(slotDrawStyleChanged(QAction*)));

    m_disableLightingAction = new QAction(QIcon(":/disable_lighting_24x24.png"), "&Disable Results Lighting", this);
    m_disableLightingAction->setCheckable(true);
    connect(m_disableLightingAction,	SIGNAL(toggled(bool)), SLOT(slotDisableLightingAction(bool)));


    m_drawStyleToggleFaultsAction             = new QAction( QIcon(":/draw_style_faults_24x24.png"), "&Show Faults Only", this);
    m_drawStyleToggleFaultsAction->setCheckable(true);
    connect(m_drawStyleToggleFaultsAction,	SIGNAL(toggled(bool)), SLOT(slotToggleFaultsAction(bool)));

    m_toggleFaultsLabelAction             = new QAction( QIcon(":/draw_style_faults_label_24x24.png"), "&Show Fault Labels", this);
    m_toggleFaultsLabelAction->setCheckable(true);
    connect(m_toggleFaultsLabelAction,	SIGNAL(toggled(bool)), SLOT(slotToggleFaultLabelsAction(bool)));

    m_addWellCellsToRangeFilterAction = new QAction(QIcon(":/draw_style_WellCellsToRangeFilter_24x24.png"), "&Add Well Cells To Range Filter", this);
    m_addWellCellsToRangeFilterAction->setCheckable(true);
    m_addWellCellsToRangeFilterAction->setToolTip("Add Well Cells To Range Filter based on the individual settings");
    connect(m_addWellCellsToRangeFilterAction,	SIGNAL(toggled(bool)), SLOT(slotAddWellCellsToRangeFilterAction(bool)));

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class ToolTipableMenu : public QMenu
{
public:
    ToolTipableMenu( QWidget * parent ) : QMenu( parent ) {};
    
    bool event(QEvent* e)
    {
        if (e->type() == QEvent::ToolTip)
        {
            QHelpEvent* he = dynamic_cast<QHelpEvent*>(e);
            QAction* act = actionAt(he->pos());
            if (act)
            {
                // Remove ampersand as this is used to define shortcut key
                QString actionTextWithoutAmpersand = act->text().remove("&");

                if (actionTextWithoutAmpersand != act->toolTip())
                {
                    QToolTip::showText(he->globalPos(), act->toolTip(), this);
                }
                
                return true;
            }
        }
        else if (e->type() == QEvent::Paint && QToolTip::isVisible())
        {
            QToolTip::hideText();
        }

        return QMenu::event(e);
    }
};


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::createMenus()
{
    caf::CmdFeatureManager* cmdFeatureMgr = caf::CmdFeatureManager::instance();
    CVF_ASSERT(cmdFeatureMgr);

    // File menu
    QMenu* fileMenu = new ToolTipableMenu(menuBar());
    fileMenu->setTitle("&File");

    menuBar()->addMenu(fileMenu);
  
    fileMenu->addAction(m_openProjectAction);
    fileMenu->addAction(m_openLastUsedProjectAction);
    fileMenu->addSeparator();

    QMenu* importMenu = fileMenu->addMenu("&Import");
    importMenu->addAction(cmdFeatureMgr->action("RicImportEclipseCaseFeature"));
    importMenu->addAction(cmdFeatureMgr->action("RicImportInputEclipseCaseFeature"));
    importMenu->addAction(cmdFeatureMgr->action("RicCreateGridCaseGroupFeature"));
    importMenu->addSeparator();
    #ifdef USE_ODB_API
    importMenu->addAction(m_importGeoMechCaseAction);
    importMenu->addSeparator();
    #endif
    importMenu->addAction(cmdFeatureMgr->action("RicWellPathsImportFileFeature"));
    importMenu->addAction(cmdFeatureMgr->action("RicWellPathsImportSsihubFeature"));
    importMenu->addAction(cmdFeatureMgr->action("RicWellLogsImportFileFeature"));

    QMenu* exportMenu = fileMenu->addMenu("&Export");
    exportMenu->addAction(m_snapshotToFile);
    exportMenu->addAction(m_snapshotAllViewsToFile);

    fileMenu->addSeparator();
    fileMenu->addAction(m_saveProjectAction);
    fileMenu->addAction(m_saveProjectAsAction);

    m_recentFilesSeparatorAction = fileMenu->addSeparator();
    for (int i = 0; i < MaxRecentFiles; ++i)
        fileMenu->addAction(m_recentFileActions[i]);
    
    updateRecentFileActions();

    fileMenu->addSeparator();
    QMenu* testMenu = fileMenu->addMenu("&Testing");

    fileMenu->addSeparator();
    fileMenu->addAction(m_closeProjectAction);
    fileMenu->addSeparator();
    fileMenu->addAction(m_exitAction);

    connect(fileMenu, SIGNAL(aboutToShow()), SLOT(slotRefreshFileActions()));

    // Edit menu
    QMenu* editMenu = menuBar()->addMenu("&Edit");
    editMenu->addAction(m_snapshotToClipboard);
    editMenu->addSeparator();
    editMenu->addAction(m_editPreferences);

    connect(editMenu, SIGNAL(aboutToShow()), SLOT(slotRefreshEditActions()));


    // View menu
    QMenu* viewMenu = menuBar()->addMenu("&View");
    viewMenu->addAction(m_zoomAll);
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

    // Windows menu
    m_windowMenu = menuBar()->addMenu("&Windows");
    connect(m_windowMenu, SIGNAL(aboutToShow()), SLOT(slotBuildWindowActions()));

    // Help menu
    QMenu* helpMenu = menuBar()->addMenu("&Help");
    helpMenu->addAction(m_openUsersGuideInBrowserAction);
    helpMenu->addAction(m_commandLineHelpAction);
    helpMenu->addSeparator();
    helpMenu->addAction(m_aboutAction);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::createToolBars()
{
    caf::CmdFeatureManager* cmdFeatureMgr = caf::CmdFeatureManager::instance();
    CVF_ASSERT(cmdFeatureMgr);

    m_standardToolBar = addToolBar(tr("Standard"));
    m_standardToolBar->setObjectName(m_standardToolBar->windowTitle());

    m_standardToolBar->addAction(cmdFeatureMgr->action("RicImportEclipseCaseFeature"));
    m_standardToolBar->addAction(cmdFeatureMgr->action("RicImportInputEclipseCaseFeature"));
    m_standardToolBar->addAction(m_openProjectAction);
    //m_standardToolBar->addAction(m_openLastUsedProjectAction);
    m_standardToolBar->addAction(m_saveProjectAction);

    // Snapshots
    m_snapshotToolbar = addToolBar(tr("View Snapshots"));
    m_snapshotToolbar->setObjectName(m_snapshotToolbar->windowTitle());
    m_snapshotToolbar->addAction(m_snapshotToClipboard);
    m_snapshotToolbar->addAction(m_snapshotToFile);
    m_snapshotToolbar->addAction(m_snapshotAllViewsToFile);

   // View toolbar
    m_viewToolBar = addToolBar(tr("View"));
    m_viewToolBar->setObjectName(m_viewToolBar->windowTitle());
    m_viewToolBar->addAction(m_zoomAll);
    m_viewToolBar->addAction(m_viewFromNorth);
    m_viewToolBar->addAction(m_viewFromSouth);
    m_viewToolBar->addAction(m_viewFromEast);
    m_viewToolBar->addAction(m_viewFromWest);
    m_viewToolBar->addAction(m_viewFromAbove);
    m_viewToolBar->addAction(m_viewFromBelow);
    m_viewToolBar->addSeparator();
    m_viewToolBar->addAction(cmdFeatureMgr->action("RicLinkVisibleViewsFeature"));
    m_viewToolBar->addAction(cmdFeatureMgr->action("RicTileWindowsFeature"));
    m_viewToolBar->addSeparator();
    m_viewToolBar->addAction(m_drawStyleLinesAction);
    m_viewToolBar->addAction(m_drawStyleLinesSolidAction);
    m_viewToolBar->addAction(m_drawStyleSurfOnlyAction);
    m_viewToolBar->addAction(m_drawStyleFaultLinesSolidAction);
    m_viewToolBar->addAction(m_disableLightingAction);
    m_viewToolBar->addAction(m_drawStyleToggleFaultsAction);
    m_viewToolBar->addAction(m_toggleFaultsLabelAction);
    m_viewToolBar->addAction(m_addWellCellsToRangeFilterAction);

    QLabel* scaleLabel = new QLabel(m_viewToolBar);
    scaleLabel->setText("Scale");
    m_viewToolBar->addWidget(scaleLabel);

    m_scaleFactor = new QSpinBox(m_viewToolBar);
    m_scaleFactor->setValue(0);
    m_viewToolBar->addWidget(m_scaleFactor);
    connect(m_scaleFactor, SIGNAL(valueChanged(int)), SLOT(slotScaleChanged(int)));

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

		addDockWidget(Qt::RightDockWidgetArea, dockWidget);

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

        m_pdmUiPropertyView->layout()->setContentsMargins(5,0,0,0);

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
        dockPanel->setObjectName("dockProcessMonitor");
        dockPanel->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
        m_processMonitor = new RiuProcessMonitor(dockPanel);
        dockPanel->setWidget(m_processMonitor);

        addDockWidget(Qt::BottomDockWidgetArea, dockPanel);
    }

    // Test - create well log viewer in a dock widget
    // TODO: remove after making MDI widgets for well log viewers
//     {
//         QDockWidget* dockPanel = new QDockWidget("TEST - Well Log Viewer", this);
//         dockPanel->setObjectName("dockWellLogViewer");
//         dockPanel->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
//         
//         RiuWellLogViewer* wellLogViewer = new RiuWellLogViewer(dockPanel);
//         dockPanel->setWidget(wellLogViewer);
// 
//         addDockWidget(Qt::BottomDockWidgetArea, dockPanel);
//     }

 
    setCorner(Qt::BottomLeftCorner,	Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomRightCorner, Qt::BottomDockWidgetArea);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::saveWinGeoAndDockToolBarLayout()
{
    // Company and appname set through QCoreApplication
    QSettings settings;

    QByteArray winGeo = saveGeometry();
    settings.setValue("winGeometry", winGeo);

    QByteArray layout = saveState(0);
    settings.setValue("dockAndToolBarLayout", layout);

    settings.setValue("isMaximized", isMaximized());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::loadWinGeoAndDockToolBarLayout()
{
    // Company and appname set through QCoreApplication
    QSettings settings;

    QVariant winGeo = settings.value("winGeometry");
    QVariant layout = settings.value("dockAndToolBarLayout");

    if (winGeo.isValid())
    {
        if (restoreGeometry(winGeo.toByteArray()))
        {
            if (layout.isValid())
            {
                restoreState(layout.toByteArray(), 0);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::showWindow()
{
    // Company and appname set through QCoreApplication
    QSettings settings;

    showNormal();

    QVariant isMax = settings.value("isMaximized", false);
    if (isMax.toBool())
    {
        showMaximized();
    }
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

    bool projectExists = true;
    m_saveProjectAction->setEnabled(projectExists);
    m_saveProjectAsAction->setEnabled(projectExists);
    m_closeProjectAction->setEnabled(projectExists);
    
    bool projectFileExists = QFile::exists(app->project()->fileName());

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

    caf::CmdFeatureManager::instance()->refreshEnabledState(QStringList() << "RicLinkVisibleViewsFeature" << "RicTileWindowsFeature");
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
                    activeGmv->geoMechCase()->timeStepStrings();
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
void RiuMainWindow::slotAbout()
{
    caf::AboutDialog dlg(this);

    dlg.setApplicationName(RI_APPLICATION_NAME);
    dlg.setApplicationVersion(RiaApplication::getVersionStringApp(true));
    dlg.setCopyright("Copyright Statoil ASA, Ceetron Solutions AS, Ceetron AS");
    dlg.showQtVersion(false);
#ifdef _DEBUG
    dlg.setIsDebugBuild(true);
#endif

    dlg.addVersionEntry(" ", "ResInsight is made available under the GNU General Public License v. 3");
    dlg.addVersionEntry(" ", "See http://www.gnu.org/licenses/gpl.html");
    dlg.addVersionEntry(" ", " ");
    dlg.addVersionEntry(" ", " ");
    dlg.addVersionEntry(" ", "Technical Information");
    dlg.addVersionEntry(" ", QString("   Qt ") + qVersion());
    dlg.addVersionEntry(" ", QString("   ") + caf::AboutDialog::versionStringForcurrentOpenGLContext());
    dlg.addVersionEntry(" ", caf::Viewer::isShadersSupported() ? "   Hardware OpenGL" : "   Software OpenGL");

    dlg.create();
    dlg.resize(300, 200);

    dlg.exec();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotImportGeoMechModel()
{
    if (checkForDocumentModifications())
    {
        RiaApplication* app = RiaApplication::instance();

        QString defaultDir = app->defaultFileDialogDirectory("GEOMECH_MODEL");
        QStringList fileNames = QFileDialog::getOpenFileNames(this, "Import Geo-Mechanical Model", defaultDir, "Abaqus results (*.odb)");
        if (fileNames.size()) defaultDir = QFileInfo(fileNames.last()).absolutePath();
        app->setDefaultFileDialogDirectory("GEOMECH_MODEL", defaultDir);

        int i;
        for (i = 0; i < fileNames.size(); i++)
        {
            QString fileName = fileNames[i];

            if (!fileNames.isEmpty())
            {
                if (app->openOdbCaseFromFile(fileName))
                {
                    addRecentFiles(fileName);
                }
            }
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotOpenProject()
{
    if (checkForDocumentModifications())
    {
        RiaApplication* app = RiaApplication::instance();
        QString defaultDir = app->defaultFileDialogDirectory("BINARY_GRID");
        QString fileName = QFileDialog::getOpenFileName(this, "Open ResInsight Project", defaultDir, "ResInsight project (*.rsp *.rip);;All files(*.*)");

        if (fileName.isEmpty()) return;

        // Remember the path to next time
        app->setDefaultFileDialogDirectory("BINARY_GRID", QFileInfo(fileName).absolutePath());

        if (app->loadProject(fileName))
        {
            addRecentFiles(fileName);
        }
    }

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotOpenLastUsedProject()
{
    RiaApplication* app = RiaApplication::instance();
    QString fileName = app->preferences()->lastUsedProjectFileName;
    
    if (app->loadProject(fileName))
    {
        addRecentFiles(fileName);
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
bool RiuMainWindow::checkForDocumentModifications()
{
    RiaApplication* app = RiaApplication::instance();
//     RISceneManager* project = app->sceneManager();
//     if (project && project->isModified())
//     {
//         QMessageBox msgBox(this);
//         msgBox.setIcon(QMessageBox::Warning);
//         msgBox.setText("The project has been modified.");
//         msgBox.setInformativeText("Do you want to save your changes?");
//         msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
// 
//         int ret = msgBox.exec();
//         if (ret == QMessageBox::Save)
//         {
//             project->saveAll();
//         }
//         else if (ret == QMessageBox::Cancel)
//         {
//             return false;
//         }
//     }

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotCloseProject()
{
    RiaApplication* app = RiaApplication::instance();
    bool ret = app->closeProject(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotOpenRecentFile()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action)
    {
        QString filename = action->data().toString();
        bool loadingSucceded = false;

        if (filename.contains(".rsp", Qt::CaseInsensitive) || filename.contains(".rip", Qt::CaseInsensitive) )
        {
            loadingSucceded = RiaApplication::instance()->loadProject(action->data().toString());
        }
        else if ( filename.contains(".egrid", Qt::CaseInsensitive) || filename.contains(".grid", Qt::CaseInsensitive) )
        {
            loadingSucceded = RiaApplication::instance()->openEclipseCaseFromFile(filename);
        }
        else if (filename.contains(".odb", Qt::CaseInsensitive) )
        {
            loadingSucceded = RiaApplication::instance()->openOdbCaseFromFile(filename);
        }

        if (loadingSucceded)
        {
            addRecentFiles(filename);
        }
        else
        {
            removeRecentFiles(filename);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::updateRecentFileActions()
{
    QSettings settings;
    QStringList files = settings.value("recentFileList").toStringList();

    int numRecentFiles = qMin(files.size(), (int)MaxRecentFiles);

    for (int i = 0; i < numRecentFiles; ++i) {
        QString text = tr("&%1 %2").arg(i + 1).arg(QFileInfo(files[i]).fileName());
        m_recentFileActions[i]->setText(text);
        m_recentFileActions[i]->setData(files[i]);
        m_recentFileActions[i]->setToolTip(files[i]);
        m_recentFileActions[i]->setVisible(true);
    }
    for (int j = numRecentFiles; j < MaxRecentFiles; ++j)
        m_recentFileActions[j]->setVisible(false);

    m_recentFilesSeparatorAction->setVisible(numRecentFiles > 0);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::addRecentFiles(const QString& file)
{
    QSettings settings;
    QStringList files = settings.value("recentFileList").toStringList();
    files.removeAll(file);
    files.prepend(file);
    while (files.size() > MaxRecentFiles)
        files.removeLast();

    settings.setValue("recentFileList", files);

    updateRecentFileActions();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::removeRecentFiles(const QString& file)
{
    QSettings settings;
    QStringList files = settings.value("recentFileList").toStringList();
    files.removeAll(file);

    settings.setValue("recentFileList", files);

    updateRecentFileActions();
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
void RiuMainWindow::removeViewer(QWidget* viewer)
{
    m_blockSlotSubWindowActivated = true;
    m_mdiArea->removeSubWindow(findMdiSubWindow(viewer));
    m_blockSlotSubWindowActivated = false;

    slotRefreshViewActions();
}


// Helper class used to trap the close event of an QMdiSubWindow
class RiuMdiSubWindow : public QMdiSubWindow
{
public:
    RiuMdiSubWindow(QWidget* parent = 0, Qt::WindowFlags flags = 0)
        : QMdiSubWindow(parent, flags)
    {
    }

    ~RiuMdiSubWindow()
    {
        RiuMainWindow::instance()->slotRefreshViewActions();
    }

protected:
    virtual void closeEvent(QCloseEvent* event)
    {
        QWidget* mainWidget = widget();

        RiuWellLogPlot* wellLogPlot = dynamic_cast<RiuWellLogPlot*>(mainWidget);
        if (wellLogPlot)
        {
            wellLogPlot->ownerPlotDefinition()->windowGeometry = RiuMainWindow::instance()->windowGeometryForWidget(this);
        }
        else
        {
            RiuViewer* viewer = mainWidget->findChild<RiuViewer*>();
            if (viewer)
            {
                viewer->ownerReservoirView()->windowGeometry = RiuMainWindow::instance()->windowGeometryForWidget(this);
            }
        }

        QMdiSubWindow::closeEvent(event);
    }
};

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::addViewer(QWidget* viewer, const std::vector<int>& windowsGeometry)
{
    RiuMdiSubWindow* subWin = new RiuMdiSubWindow(m_mdiArea);
    subWin->setAttribute(Qt::WA_DeleteOnClose); // Make sure the contained widget is destroyed when the MDI window is closed
    subWin->setWidget(viewer);

    QSize subWindowSize;
    QPoint subWindowPos(-1, -1);
    bool initialStateMaximized = false;

    if (windowsGeometry.size() == 5)
    {
        subWindowPos = QPoint(windowsGeometry[0], windowsGeometry[1]);
        subWindowSize = QSize(windowsGeometry[2], windowsGeometry[3]);

        if (windowsGeometry[4] > 0)
        {
            initialStateMaximized = true;
        }
    }
    else
    {
        RiuWellLogPlot* wellLogPlot = dynamic_cast<RiuWellLogPlot*>(subWin->widget());
        if (wellLogPlot)
        {
            subWindowSize = QSize(200, m_mdiArea->height());
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
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotSaveProject()
{
    RiaApplication* app = RiaApplication::instance();

    storeTreeViewState();

    app->saveProject();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotSaveProjectAs()
{
    RiaApplication* app = RiaApplication::instance();

    storeTreeViewState();

    app->saveProjectPromptForFileName();
}


//--------------------------------------------------------------------------------------------------
/// This method needs to handle memory deallocation !!!
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::setPdmRoot(caf::PdmObject* pdmRoot)
{
    m_pdmRoot = pdmRoot;

	m_projectTreeView->setPdmItem(pdmRoot);
    // For debug only : m_projectTreeView->treeView()->expandAll();
    m_projectTreeView->setDragDropHandle(m_dragDrop);

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
void RiuMainWindow::slotZoomAll()
{
    if (RiaApplication::instance()->activeReservoirView() &&  RiaApplication::instance()->activeReservoirView()->viewer())
    {
        RiaApplication::instance()->activeReservoirView()->viewer()->zoomAll();
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

    RiuWellLogPlot* wellLogPlotViewer = dynamic_cast<RiuWellLogPlot*>(subWindow->widget());

    if (wellLogPlotViewer)
    {
        RimWellLogPlot* wellLogPlot = wellLogPlotViewer->ownerPlotDefinition();
        
        if (wellLogPlot != RiaApplication::instance()->activeWellLogPlot())
        {
            RiaApplication::instance()->setActiveWellLogPlot(wellLogPlot);
            if (wellLogPlot)
            {
                projectTreeView()->selectAsCurrentItem(wellLogPlot);
            }
        }
        return;
    }

    RiaApplication::instance()->setActiveWellLogPlot(NULL);

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
void RiuMainWindow::slotEditPreferences()
{
	RiaApplication* app = RiaApplication::instance();
	caf::PdmUiPropertyViewDialog propertyDialog(this, app->preferences(), "Preferences", "");
    if (propertyDialog.exec() == QDialog::Accepted)
    {
        // Write preferences using QSettings  and apply them to the application
		caf::PdmSettings::writeFieldsToApplicationStore(app->preferences());
        app->applyPreferences();
    }
    else
    {
        // Read back currently stored values using QSettings
		caf::PdmSettings::readFieldsFromApplicationStore(app->preferences());
    }
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
		caf::PdmUiFieldHandle* uiFieldHandle = RiaApplication::instance()->activeReservoirView()->maximumFrameRate.uiCapability();
        uiFieldHandle->setValueFromUi(QVariant(frameRate));
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
            firstSelectedObject->firstAnchestorOrThisOfType(selectedReservoirView);
        }

        bool isActiveViewChanged = false;
        
        RimWellLogPlot* selectedWellLogPlot = NULL;

        if (selectedReservoirView)
        {
            // Set focus in MDI area to this window if it exists
            if (selectedReservoirView->viewer())
            {
                setActiveViewer(selectedReservoirView->viewer()->layoutWidget());
            }
            isActiveViewChanged = true;
        }
        else // Check if we are winthin a Well Log plot
        {
            selectedWellLogPlot = dynamic_cast<RimWellLogPlot*>(firstSelectedObject);
            if (!selectedWellLogPlot)
            {
                firstSelectedObject->firstAnchestorOrThisOfType(selectedWellLogPlot);
            }

            if (selectedWellLogPlot)
            {
                if (selectedWellLogPlot->viewer())
                {
                    setActiveViewer(selectedWellLogPlot->viewer());

                }
                isActiveViewChanged = true;
            }
        }

        if (isActiveViewChanged)
        {
            RiaApplication::instance()->setActiveReservoirView(selectedReservoirView);
            RiaApplication::instance()->setActiveWellLogPlot(selectedWellLogPlot);
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
void RiuMainWindow::slotSnapshotToFile()
{
    RiaApplication* app = RiaApplication::instance();

    app->saveSnapshotPromtpForFilename();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotSnapshotToClipboard()
{
    RiaApplication* app = RiaApplication::instance();

    app->copySnapshotToClipboard();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotSnapshotAllViewsToFile()
{
    RiaApplication* app = RiaApplication::instance();

    // Save images in snapshot catalog relative to project directory
    QString absolutePathToSnapshotDir = app->createAbsolutePathFromProjectRelativePath("snapshots");
    app->saveSnapshotForAllViews(absolutePathToSnapshotDir);
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
        gridFileNames += RimDefines::mockModelBasicWithResults();
        gridFileNames += RimDefines::mockModelBasicWithResults();
        gridFileNames += RimDefines::mockModelBasicWithResults();
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
void RiuMainWindow::slotToggleFaultsAction(bool showFaults)
{
    if (!RiaApplication::instance()->activeReservoirView()) return;

    RiaApplication::instance()->activeReservoirView()->setShowFaultsOnly(showFaults);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotToggleFaultLabelsAction(bool showLabels)
{
    RimEclipseView* activeRiv = dynamic_cast<RimEclipseView*>(RiaApplication::instance()->activeReservoirView());
    if (!activeRiv) return;

	caf::PdmUiFieldHandle* uiFieldHandle = activeRiv->faultCollection->showFaultLabel.uiCapability();
    uiFieldHandle->setValueFromUi(showLabels);

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

    RimGeoMechView* geoMechView = dynamic_cast<RimGeoMechView*>(view);
    bool lightingDisabledInView = view ? view->isLightingDisabled() : false;

    m_disableLightingAction->blockSignals(true);
    m_disableLightingAction->setChecked(lightingDisabledInView);
    m_disableLightingAction->blockSignals(false);

    RimEclipseView* eclView = dynamic_cast<RimEclipseView*>(view);
    enable = enable && eclView;

    m_drawStyleToggleFaultsAction->setEnabled(enable);
    m_toggleFaultsLabelAction->setEnabled(enable);

    m_addWellCellsToRangeFilterAction->setEnabled(enable);

    if (enable) 
    {   
        m_drawStyleToggleFaultsAction->blockSignals(true);
        m_drawStyleToggleFaultsAction->setChecked(!eclView->isGridVisualizationMode());
        m_drawStyleToggleFaultsAction->blockSignals(false);

        m_toggleFaultsLabelAction->blockSignals(true);
        m_toggleFaultsLabelAction->setChecked(eclView->faultCollection()->showFaultLabel());
        m_toggleFaultsLabelAction->blockSignals(false);

        m_addWellCellsToRangeFilterAction->blockSignals(true);
        m_addWellCellsToRangeFilterAction->setChecked(eclView->wellCollection()->wellCellsToRangeFilterMode() != RimEclipseWellCollection::RANGE_ADD_NONE);
        m_addWellCellsToRangeFilterAction->blockSignals(false);
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
void RiuMainWindow::storeTreeViewState()
{
    if (m_projectTreeView)
    {
        QString treeViewState;
        RimTreeViewStateSerializer::storeTreeViewStateToString(m_projectTreeView->treeView(), treeViewState);

        QModelIndex mi = m_projectTreeView->treeView()->currentIndex();

        QString encodedModelIndexString;
        RimTreeViewStateSerializer::encodeStringFromModelIndex(mi, encodedModelIndexString);
        
        RiaApplication::instance()->project()->treeViewState = treeViewState;
        RiaApplication::instance()->project()->currentModelIndexPath = encodedModelIndexString;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::restoreTreeViewState()
{
    if (m_projectTreeView)
    {
        QString stateString = RiaApplication::instance()->project()->treeViewState;
        if (!stateString.isEmpty())
        {
            m_projectTreeView->treeView()->collapseAll();
            RimTreeViewStateSerializer::applyTreeViewStateFromString(m_projectTreeView->treeView(), stateString);
        }

        QString currentIndexString = RiaApplication::instance()->project()->currentModelIndexPath;
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
void RiuMainWindow::setCurrentObjectInTreeView(caf::PdmObject* object)
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
		caf::PdmUiFieldHandle* uiFieldHandle = RiaApplication::instance()->activeReservoirView()->scaleZ.uiCapability();
        uiFieldHandle->setValueFromUi(scaleValue);
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
void RiuMainWindow::slotShowCommandLineHelp()
{
    RiaApplication* app = RiaApplication::instance();
    QString text = app->commandLineParameterHelp();
    app->showFormattedTextInMessageBox(text);
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
void RiuMainWindow::slotAddWellCellsToRangeFilterAction(bool doAdd)
{
    RimEclipseView* riv = dynamic_cast<RimEclipseView*>(RiaApplication::instance()->activeReservoirView());
    if (riv)
    {
        caf::AppEnum<RimEclipseWellCollection::WellCellsRangeFilterType> rangeAddType;
        rangeAddType = doAdd ? RimEclipseWellCollection::RANGE_ADD_INDIVIDUAL : RimEclipseWellCollection::RANGE_ADD_NONE;

		caf::PdmUiFieldHandle* pdmUiFieldHandle = riv->wellCollection()->wellCellsToRangeFilterMode.uiCapability();
        if (pdmUiFieldHandle)
        {
            pdmUiFieldHandle->setValueFromUi(static_cast<unsigned int>(rangeAddType.index()));
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotOpenUsersGuideInBrowserAction()
{
    QString usersGuideUrl = "http://resinsight.org/docs/home";
    
    if (!QDesktopServices::openUrl(usersGuideUrl))
    {
        QErrorMessage* errorHandler = QErrorMessage::qtHandler();
        errorHandler->showMessage("Failed open browser with the following url\n\n" + usersGuideUrl);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::appendActionsContextMenuForPdmObject(caf::PdmObjectHandle* pdmObject, QMenu* menu)
{
    if (!menu)
    {
        return;
    }

    caf::CmdFeatureManager* cmdFeatureMgr = caf::CmdFeatureManager::instance();
    CVF_ASSERT(cmdFeatureMgr);

    if (dynamic_cast<RimWellPathCollection*>(pdmObject))
    {
        menu->addAction(cmdFeatureMgr->action("RicWellPathsImportFileFeature"));
        menu->addAction(cmdFeatureMgr->action("RicWellPathsImportSsihubFeature"));
    }
    else if (dynamic_cast<RimEclipseCaseCollection*>(pdmObject))
    {
        menu->addAction(cmdFeatureMgr->action("RicImportEclipseCaseFeature"));
        menu->addAction(cmdFeatureMgr->action("RicImportInputEclipseCaseFeature"));
        menu->addAction(cmdFeatureMgr->action("RicCreateGridCaseGroupFeature"));
    }
    else if (dynamic_cast<RimGeoMechModels*>(pdmObject))
    {
    #ifdef USE_ODB_API
        menu->addAction(m_importGeoMechCaseAction);
    #endif
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
void RiuMainWindow::forceProjectTreeRepaint()
{
    // This is a hack to force the treeview redraw. 
    // Needed for some reason when changing names and icons in the model
    m_projectTreeView->scroll(0,1);
    m_projectTreeView->scroll(0,-1);
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
std::vector<int> RiuMainWindow::windowGeometryForViewer(QWidget* viewer)
{
    QMdiSubWindow* mdiWindow = findMdiSubWindow(viewer);
    if (mdiWindow)
    {
        return windowGeometryForWidget(mdiWindow);
    }

    std::vector<int> geo;
    return geo;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<int> RiuMainWindow::windowGeometryForWidget(QWidget* widget)
{
    std::vector<int> geo;

    if (widget)
    {
        geo.push_back(widget->pos().x());
        geo.push_back(widget->pos().y());
        geo.push_back(widget->size().width());
        geo.push_back(widget->size().height());
        geo.push_back(widget->isMaximized());
    }

    return geo;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::tileWindows()
{
    m_mdiArea->tileSubWindows();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiuMainWindow::isAnyMdiSubWindowVisible()
{
    return m_mdiArea->subWindowList().size() > 0;
}

