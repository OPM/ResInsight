/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "RimProject.h"
#include "RimAnalysisModels.h"
#include "RimOilField.h"
#include "RimReservoirView.h"
#include "RimUiTreeView.h"
#include "RimCase.h"
#include "RimResultSlot.h"
#include "RimCellPropertyFilterCollection.h"
#include "RimWellCollection.h"
#include "RimReservoirCellResultsCacher.h"
#include "RimCaseCollection.h"
#include "RimWellPathCollection.h"

#include "RimUiTreeModelPdm.h"

#include "RiaBaseDefs.h"
#include "RiuViewer.h"
#include "RiuResultInfoPanel.h"
#include "RiuProcessMonitor.h"
#include "RiuMultiCaseImportDialog.h"

#include "RiaPreferences.h"
#include "RiuPreferencesDialog.h"

#include "RigCaseCellResultsData.h"

#include "cafAnimationToolBar.h"
#include "cafPdmUiPropertyView.h"
#include "cvfqtBasicAboutDialog.h"

#include "cafPdmFieldCvfMat4d.h"

#include "RimIdenticalGridCaseGroup.h"
#include "RimScriptCollection.h"
#include "RimCellEdgeResultSlot.h"
#include "RimCellRangeFilterCollection.h"
#include "Rim3dOverlayInfoConfig.h"

#include "ssihubInterface/ssihubInterface.h"



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
:   m_treeView(NULL),   
    m_pdmRoot(NULL),
    m_mainViewer(NULL),
    m_windowMenu(NULL)
{
    CVF_ASSERT(sm_mainWindowInstance == NULL);

#if 0
    m_CentralFrame = new QFrame;
    QHBoxLayout* frameLayout = new QHBoxLayout(m_CentralFrame);
    setCentralWidget(m_CentralFrame);
#else
    m_mdiArea = new QMdiArea;
    connect(m_mdiArea, SIGNAL(subWindowActivated ( QMdiSubWindow *)), SLOT(slotSubWindowActivated(QMdiSubWindow*)));
    setCentralWidget(m_mdiArea);
#endif

    //m_mainViewer = createViewer();


    m_treeModelPdm = new RimUiTreeModelPdm(this);

    m_ssihubInterface = new ssihub::Interface;

    createActions();
    createMenus();
    createToolBars();
    createDockPanels();

    // Store the layout so we can offer reset option
    m_initialDockAndToolbarLayout = saveState(0);
    loadWinGeoAndDockToolBarLayout();

    sm_mainWindowInstance = this;
    
    slotRefreshFileActions();
    slotRefreshEditActions();

    // Set pdm root so scripts are displayed
    setPdmRoot(RiaApplication::instance()->project());
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
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::cleanupGuiBeforeProjectClose()
{
    setPdmRoot(NULL);
    setResultInfo("");
    
    if (m_pdmUiPropertyView)
    {
        m_pdmUiPropertyView->showProperties(NULL);
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

    saveWinGeoAndDockToolBarLayout();


        
    event->accept();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::createActions()
{
    // File actions
    m_openEclipseCaseAction                = new QAction(QIcon(":/Case48x48.png"), "&Open Eclipse Case", this);
    m_openInputEclipseFileAction= new QAction(QIcon(":/EclipseInput48x48.png"), "&Open Input Eclipse Case", this);
    m_openMultipleEclipseCasesAction = new QAction(QIcon(":/CreateGridCaseGroup16x16.png"), "&Create Grid Case Group from Files", this);
    
    m_openProjectAction         = new QAction(style()->standardIcon(QStyle::SP_DirOpenIcon), "&Open Project", this);
    m_openLastUsedProjectAction = new QAction("Open &Last Used Project", this);
    m_importWellPathsFromFileAction       = new QAction(QIcon(":/Well.png"), "&Import Well Paths from File", this);
    m_importWellPathsFromSSIHubAction     = new QAction(QIcon(":/WellCollection.png"),"Import Well Paths from &SSI-hub", this);

    m_mockModelAction           = new QAction("&Mock Model", this);
    m_mockResultsModelAction    = new QAction("Mock Model With &Results", this);
    m_mockLargeResultsModelAction = new QAction("Large Mock Model", this);
    m_mockInputModelAction      = new QAction("Input Mock Model", this);

    m_snapshotToFile            = new QAction(QIcon(":/SnapShotSave.png"), "Snapshot To File", this);
    m_snapshotToClipboard       = new QAction(QIcon(":/SnapShot.png"), "Copy Snapshot To Clipboard", this);
    m_snapshotAllViewsToFile    = new QAction(QIcon(":/SnapShotSaveViews.png"), "Snapshot All Views To File", this);

    m_saveProjectAction         = new QAction(QIcon(":/Save.png"), "&Save Project", this);
    m_saveProjectAsAction       = new QAction(QIcon(":/Save.png"), "Save Project &As", this);

    m_closeProjectAction               = new QAction("&Close Project", this);
    m_exitAction		        = new QAction("E&xit", this);

    connect(m_openEclipseCaseAction,	            SIGNAL(triggered()), SLOT(slotOpenBinaryGridFiles()));
    connect(m_openInputEclipseFileAction,SIGNAL(triggered()), SLOT(slotOpenInputFiles()));
    connect(m_openMultipleEclipseCasesAction,SIGNAL(triggered()), SLOT(slotOpenMultipleCases()));
    connect(m_openProjectAction,	    SIGNAL(triggered()), SLOT(slotOpenProject()));
    connect(m_openLastUsedProjectAction,SIGNAL(triggered()), SLOT(slotOpenLastUsedProject()));
    connect(m_importWellPathsFromFileAction,	    SIGNAL(triggered()), SLOT(slotImportWellPathsFromFile()));
    connect(m_importWellPathsFromSSIHubAction,    SIGNAL(triggered()), SLOT(slotImportWellPathsFromSSIHub()));
    
    connect(m_mockModelAction,	        SIGNAL(triggered()), SLOT(slotMockModel()));
    connect(m_mockResultsModelAction,	SIGNAL(triggered()), SLOT(slotMockResultsModel()));
    connect(m_mockLargeResultsModelAction,	SIGNAL(triggered()), SLOT(slotMockLargeResultsModel()));
    connect(m_mockInputModelAction,	    SIGNAL(triggered()), SLOT(slotInputMockModel()));

    connect(m_snapshotToFile,	        SIGNAL(triggered()), SLOT(slotSnapshotToFile()));
    connect(m_snapshotToClipboard,	    SIGNAL(triggered()), SLOT(slotSnapshotToClipboard()));
    connect(m_snapshotAllViewsToFile,   SIGNAL(triggered()), SLOT(slotSnapshotAllViewsToFile()));
    
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

    // Draw style actions
    m_dsActionGroup = new QActionGroup(this);

    m_drawStyleLinesAction                = new QAction(QIcon(":/draw_style_lines_24x24.png"), "&Mesh Only", this);
    //connect(m_drawStyleLinesAction,	    SIGNAL(triggered()), SLOT(slotDrawStyleLines()));
    m_dsActionGroup->addAction(m_drawStyleLinesAction);

     m_drawStyleLinesSolidAction           = new QAction(QIcon(":/draw_style_meshlines_24x24.png"), "Mesh And Surfaces", this);
    //connect(m_drawStyleLinesSolidAction,	SIGNAL(triggered()), SLOT(slotDrawStyleLinesSolid()));
     m_dsActionGroup->addAction(m_drawStyleLinesSolidAction);

    m_drawStyleSurfOnlyAction             = new QAction(QIcon(":/draw_style_surface_24x24.png"), "&Surface Only", this);
    //connect(m_drawStyleSurfOnlyAction,	SIGNAL(triggered()), SLOT(slotDrawStyleSurfOnly()));
    m_dsActionGroup->addAction(m_drawStyleSurfOnlyAction);


    connect(m_dsActionGroup, SIGNAL(triggered(QAction*)), SLOT(slotDrawStyleChanged(QAction*)));

    m_drawStyleToggleFaultsAction             = new QAction( QIcon(":/draw_style_faults_24x24.png"), "&Show Faults Only", this);
    m_drawStyleToggleFaultsAction->setCheckable(true);
    connect(m_drawStyleToggleFaultsAction,	SIGNAL(toggled(bool)), SLOT(slotToggleFaultsAction(bool)));

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::createMenus()
{
    // File menu
    QMenu* fileMenu = menuBar()->addMenu("&File");
  
    fileMenu->addAction(m_openProjectAction);
    fileMenu->addAction(m_openLastUsedProjectAction);
    fileMenu->addSeparator();

    QMenu* importMenu = fileMenu->addMenu("&Import");
    importMenu->addAction(m_openEclipseCaseAction);
    importMenu->addAction(m_openInputEclipseFileAction);
    importMenu->addAction(m_openMultipleEclipseCasesAction);
    importMenu->addSeparator();
    importMenu->addAction(m_importWellPathsFromFileAction);
    importMenu->addAction(m_importWellPathsFromSSIHubAction);

    QMenu* exportMenu = fileMenu->addMenu("&Export");
    exportMenu->addAction(m_snapshotToFile);
    exportMenu->addAction(m_snapshotAllViewsToFile);

    fileMenu->addSeparator();
    fileMenu->addAction(m_saveProjectAction);
    fileMenu->addAction(m_saveProjectAsAction);

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
    QMenu* debugMenu = menuBar()->addMenu("&Debug");
    debugMenu->addAction(m_mockModelAction);
    debugMenu->addAction(m_mockResultsModelAction);
    debugMenu->addAction(m_mockLargeResultsModelAction);
    debugMenu->addAction(m_mockInputModelAction);

    connect(debugMenu, SIGNAL(aboutToShow()), SLOT(slotRefreshDebugActions()));

    m_windowMenu = menuBar()->addMenu("&Windows");
    connect(m_windowMenu, SIGNAL(aboutToShow()), SLOT(slotBuildWindowActions()));

    // Help menu
    QMenu* helpMenu = menuBar()->addMenu("&Help");
    helpMenu->addAction(m_aboutAction);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::createToolBars()
{

    m_standardToolBar = addToolBar(tr("Standard"));
    m_standardToolBar->setObjectName(m_standardToolBar->windowTitle());

    m_standardToolBar->addAction(m_openEclipseCaseAction);
    m_standardToolBar->addAction(m_openInputEclipseFileAction);
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
    m_viewToolBar->addAction(m_drawStyleLinesAction);
    m_viewToolBar->addAction(m_drawStyleLinesSolidAction);
    m_viewToolBar->addAction(m_drawStyleSurfOnlyAction);
    m_viewToolBar->addAction(m_drawStyleToggleFaultsAction);

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
        dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

        m_treeView = new RimUiTreeView(dockWidget);
        m_treeView->setModel(m_treeModelPdm);
        m_treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);

        // Drag and drop configuration
        m_treeView->setDragEnabled(true);
        m_treeView->viewport()->setAcceptDrops(true);
        m_treeView->setDropIndicatorShown(true);
        m_treeView->setDragDropMode(QAbstractItemView::DragDrop);

        dockWidget->setWidget(m_treeView);

        addDockWidget(Qt::LeftDockWidgetArea, dockWidget);
    }

    {
        QDockWidget* dockWidget = new QDockWidget("Property Editor", this);
        dockWidget->setObjectName("dockWidget");
        dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

        m_pdmUiPropertyView = new caf::PdmUiPropertyView(dockWidget);
        dockWidget->setWidget(m_pdmUiPropertyView);

        m_pdmUiPropertyView->layout()->setContentsMargins(5,0,0,0);
        connect(m_treeView, SIGNAL(selectedObjectChanged( caf::PdmObject* )), m_pdmUiPropertyView, SLOT(showProperties( caf::PdmObject* )));

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

    m_importWellPathsFromSSIHubAction->setEnabled(projectFileExists);
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
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::refreshAnimationActions()
{
    caf::FrameAnimationControl* ac = NULL;
    if (RiaApplication::instance()->activeReservoirView() && RiaApplication::instance()->activeReservoirView()->viewer())
    {
        ac = RiaApplication::instance()->activeReservoirView()->viewer()->animationControl();
    }

    m_animationToolBar->connectAnimationControl(ac);

    QStringList timeStepStrings;
    int currentTimeStepIndex = 0;

    RiaApplication* app = RiaApplication::instance();

    bool enableAnimControls = false;
    if (app->activeReservoirView() && 
        app->activeReservoirView()->viewer() &&
        app->activeReservoirView()->viewer()->frameCount())
    {
        enableAnimControls = true;

        if (app->activeReservoirView()->currentGridCellResults())
        {
            if (app->activeReservoirView()->cellResult()->hasDynamicResult() 
            || app->activeReservoirView()->propertyFilterCollection()->hasActiveDynamicFilters() 
            || app->activeReservoirView()->wellCollection()->hasVisibleWellPipes())
            {
                std::vector<QDateTime> timeStepDates = app->activeReservoirView()->currentGridCellResults()->cellResults()->timeStepDates(0);
                bool showHoursAndMinutes = false;
                for (size_t i = 0; i < timeStepDates.size(); i++)
                {
                    if (timeStepDates[i].time().hour() != 0.0 || timeStepDates[i].time().minute() != 0.0)
                    {
                        showHoursAndMinutes = true;
                    }
                }

                QString formatString = "dd.MMM yyyy";
                if (showHoursAndMinutes)
                {
                    formatString += " - hh:mm";
                }

                for (size_t i = 0; i < timeStepDates.size(); i++)
                {
                    timeStepStrings += timeStepDates[i].toString(formatString);
                }
                currentTimeStepIndex = RiaApplication::instance()->activeReservoirView()->currentTimeStep();
            }
            else
            {
                timeStepStrings.push_back(tr("Static Property"));
            }
        }

        // Animation control is only relevant for more than one time step
        if (timeStepStrings.size() < 2)
        {
            enableAnimControls = false;
        }

        m_animationToolBar->setFrameRate(app->activeReservoirView()->maximumFrameRate());
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
    cvfqt::BasicAboutDialog dlg(this);

    dlg.setApplicationName(RI_APPLICATION_NAME);
    dlg.setApplicationVersion(RiaApplication::getVersionStringApp(true));
    dlg.setCopyright("Copyright 2012 Statoil ASA, Ceetron AS");
    dlg.showCeeVizVersion(false);

#ifdef _DEBUG
    dlg.setIsDebugBuild(true);
#endif

    dlg.addVersionEntry(" ", "ResInsight is made available under the GNU General Public License v. 3");
    dlg.addVersionEntry(" ", "See http://www.gnu.org/licenses/gpl.html");
    dlg.addVersionEntry(" ", " ");
    dlg.addVersionEntry(" ", " ");
    dlg.addVersionEntry(" ", "Technical Information");
    dlg.addVersionEntry(" ", QString("   Qt ") + qVersion());
    dlg.addVersionEntry(" ", QString("   ") + dlg.openGLVersionString());
    dlg.addVersionEntry(" ", caf::Viewer::isShadersSupported() ? "   Hardware OpenGL" : "   Software OpenGL");

    dlg.create();
    dlg.resize(300, 200);

    dlg.exec();
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotOpenBinaryGridFiles()
{
    if (checkForDocumentModifications())
    {
        RiaApplication* app = RiaApplication::instance();

        QString defaultDir = app->defaultFileDialogDirectory("BINARY_GRID");
        QStringList fileNames = QFileDialog::getOpenFileNames(this, "Open Eclipse File", defaultDir, "Eclipse Grid Files (*.GRID *.EGRID)");
        if (fileNames.size()) defaultDir = QFileInfo(fileNames.last()).absolutePath();
        app->setDefaultFileDialogDirectory("BINARY_GRID", defaultDir);

        int i;
        for (i = 0; i < fileNames.size(); i++)
        {
            QString fileName = fileNames[i];

            if (!fileNames.isEmpty())
            {
                app->openEclipseCaseFromFile(fileName);
            }
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotOpenInputFiles()
{
    if (checkForDocumentModifications())
    {
        RiaApplication* app = RiaApplication::instance();
        QString defaultDir = app->defaultFileDialogDirectory("INPUT_FILES");
        QStringList fileNames = QFileDialog::getOpenFileNames(this, "Open Eclipse Input Files", defaultDir, "Eclipse Input Files and Input Properties (*.GRDECL *)");

        if (fileNames.isEmpty()) return;

        // Remember the path to next time
        app->setDefaultFileDialogDirectory("INPUT_FILES", QFileInfo(fileNames.last()).absolutePath());
 
        app->openInputEclipseCase("Eclipse Input Files", fileNames);
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
        QString fileName = QFileDialog::getOpenFileName(this, "Open ResInsight Project", defaultDir, "ResInsight project (*.rip)");

        if (fileName.isEmpty()) return;

        // Remember the path to next time
        app->setDefaultFileDialogDirectory("BINARY_GRID", QFileInfo(fileName).absolutePath());

        app->loadProject(fileName);
    }

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotOpenLastUsedProject()
{
    RiaApplication* app = RiaApplication::instance();
    app->loadLastUsedProject();

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotImportWellPathsFromFile()
{
    // Open dialog box to select well path files
    RiaApplication* app = RiaApplication::instance();
    QString defaultDir = app->defaultFileDialogDirectory("WELLPATH_DIR");
    QStringList wellPathFilePaths = QFileDialog::getOpenFileNames(this, "Import Well Paths", defaultDir, "JSON Well Path (*.json);;ASCII Well Paths (*.asc *.asci *.ascii);;Dev Well Paths(*.dev);;All Files (*.*)");

    if (wellPathFilePaths.size() < 1) return;

    // Remember the path to next time
    app->setDefaultFileDialogDirectory("WELLPATH_DIR", QFileInfo(wellPathFilePaths.last()).absolutePath());

    app->addWellPathsToModel(wellPathFilePaths);
    if (app->project()) app->project()->createDisplayModelAndRedrawAllViews();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotMockModel()
{
    RiaApplication* app = RiaApplication::instance();
    app->createMockModel();

    //m_mainViewer->setDefaultView();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotMockResultsModel()
{
    RiaApplication* app = RiaApplication::instance();
    app->createResultsMockModel();

    //m_mainViewer->setDefaultView();
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
void RiuMainWindow::slotInputMockModel()
{
    RiaApplication* app = RiaApplication::instance();
    app->createInputMockModel();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotSetCurrentFrame(int frameIndex)
{
    RiaApplication* app = RiaApplication::instance();
   // app->setTimeStep(frameIndex);
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

QMdiSubWindow* RiuMainWindow::findMdiSubWindow(RiuViewer* viewer)
{
    QList<QMdiSubWindow*> subws = m_mdiArea->subWindowList();
    int i; 
    for (i = 0; i < subws.size(); ++i)
    {
        if (subws[i]->widget() == viewer->layoutWidget())
        {
            return subws[i];
        }
    }

    return NULL;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::removeViewer(RiuViewer* viewer)
{
#if 0
    m_CentralFrame->layout()->removeWidget(viewer->layoutWidget());
#else
    m_mdiArea->removeSubWindow( findMdiSubWindow(viewer));
#endif
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::addViewer(RiuViewer* viewer)
{
#if 0
    m_CentralFrame->layout()->addWidget(viewer->layoutWidget());
#else
    QMdiSubWindow * subWin = m_mdiArea->addSubWindow(viewer->layoutWidget());
    subWin->resize(400, 400);

    if (m_mdiArea->subWindowList().size() == 1)
    {
        // Show first view maximized
        subWin->showMaximized();
    }
    else
    {
        subWin->show();
    }
#endif
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

    caf::PdmUiTreeItem* treeItemRoot = caf::UiTreeItemBuilderPdm::buildViewItems(NULL, -1, m_pdmRoot);
    m_treeModelPdm->setTreeItemRoot(treeItemRoot);

    if (treeItemRoot && m_treeView->selectionModel())
    {
        connect(m_treeView->selectionModel(), SIGNAL(currentChanged ( const QModelIndex & , const QModelIndex & )), SLOT(slotCurrentChanged( const QModelIndex & , const QModelIndex & )));
    }
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
    RimProject * proj = RiaApplication::instance()->project();
    if (!proj) return;

    // Iterate all cases in each oil field
    for (size_t oilFieldIdx = 0; oilFieldIdx < proj->oilFields().size(); oilFieldIdx++)
    {
        RimOilField* oilField = proj->oilFields[oilFieldIdx];
        RimAnalysisModels* analysisModels = oilField ? oilField->analysisModels() : NULL;
        if (analysisModels == NULL) continue;

        for (size_t caseIdx = 0; caseIdx < analysisModels->cases().size(); caseIdx++)
        {
            RimCase* reservoirCase = analysisModels->cases[caseIdx];
            if (reservoirCase == NULL) continue;

            size_t viewIdx;
            for (viewIdx = 0; viewIdx < reservoirCase->reservoirViews().size(); viewIdx++)
            {
                RimReservoirView* riv = reservoirCase->reservoirViews()[viewIdx];

                if (riv &&
                    riv->viewer() &&
                    riv->viewer()->layoutWidget() &&
                    riv->viewer()->layoutWidget()->parent() == subWindow)
                {
                    RimReservoirView* previousActiveReservoirView = RiaApplication::instance()->activeReservoirView();
                    RiaApplication::instance()->setActiveReservoirView(riv);
                    if (previousActiveReservoirView && previousActiveReservoirView != riv)
                    {
                        QModelIndex previousViewModelIndex = m_treeModelPdm->getModelIndexFromPdmObject(previousActiveReservoirView);
                        QModelIndex newViewModelIndex = m_treeModelPdm->getModelIndexFromPdmObject(riv);

                        QModelIndex newSelectionIndex = newViewModelIndex;
                        QModelIndex currentSelectionIndex = m_treeView->selectionModel()->currentIndex();

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
                                    newSelectionIndex = m_treeModelPdm->index(tmp.row(), tmp.column(), newSelectionIndex);
                                }
                            }

                            // Use view model index if anything goes wrong
                            if (!newSelectionIndex.isValid())
                            {
                                newSelectionIndex = newViewModelIndex;
                            }
                        }

                        m_treeView->setCurrentIndex(newSelectionIndex);
                        if (newSelectionIndex != newViewModelIndex)
                        {
                            m_treeView->setExpanded(newViewModelIndex, true);
                        }
                    }

                    slotRefreshViewActions();
                    refreshAnimationActions();
                    refreshDrawStyleActions();
                    break;
                }
            }
        }

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
void RiuMainWindow::slotRefreshDebugActions()
{
    RiaApplication* app = RiaApplication::instance();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotEditPreferences()
{
    RiaApplication* app = RiaApplication::instance();
    RiuPreferencesDialog preferencesDialog(this, app->preferences(), "Preferences");
    if (preferencesDialog.exec() == QDialog::Accepted)
    {
        // Write preferences using QSettings  and apply them to the application
        app->writePreferences();
        app->applyPreferences();
    }
    else
    {
        // Read back currently stored values using QSettings
        app->readPreferences();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::setActiveViewer(RiuViewer* viewer)
{
   QMdiSubWindow * swin = findMdiSubWindow(viewer); 
   if (swin) m_mdiArea->setActiveSubWindow(swin);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotFramerateChanged(double frameRate)
{
    if (RiaApplication::instance()->activeReservoirView() != NULL)
    {
        RiaApplication::instance()->activeReservoirView()->maximumFrameRate.setValueFromUi(QVariant(frameRate));
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
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotCurrentChanged(const QModelIndex & current, const QModelIndex & previous)
{
    RimReservoirView* activeReservoirView = RiaApplication::instance()->activeReservoirView();
    QModelIndex activeViewModelIndex = m_treeModelPdm->getModelIndexFromPdmObject(activeReservoirView);

    QModelIndex tmp = current;

    // Traverse parents until a reservoir view is found
    while (tmp.isValid())
    {
        caf::PdmUiTreeItem* treeItem = m_treeModelPdm->getTreeItemFromIndex(tmp);
        caf::PdmObject* pdmObject = treeItem->dataObject();

        RimReservoirView* rimReservoirView = dynamic_cast<RimReservoirView*>(pdmObject);
        if (rimReservoirView)
        {
            // If current selection is an item within a different reservoir view than active, 
            // show new reservoir view and set this as activate view
            if (rimReservoirView != activeReservoirView)
            {
                RiaApplication::instance()->setActiveReservoirView(rimReservoirView);
                // Set focus in MDI area to this window if it exists
                if (rimReservoirView->viewer())
                {
                    setActiveViewer(rimReservoirView->viewer());
                }
                m_treeView->setCurrentIndex(current);
                refreshDrawStyleActions();
                refreshAnimationActions();
                slotRefreshFileActions();
                slotRefreshEditActions();
                slotRefreshViewActions();

                // The only way to get to this code is by selection change initiated from the project tree view
                // As we are activating an MDI-window, the focus is given to this MDI-window
                // Set focus back to the tree view to be able to continue keyboard tree view navigation
                m_treeView->setFocus();
            }
        }

        // Traverse parents until a reservoir view is found
        tmp = tmp.parent();
    }
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotNewObjectPropertyView()
{
    if (!m_treeModelPdm) return;

    RimUiTreeView* treeView = NULL;
    
    {
        QDockWidget* dockWidget = new QDockWidget("Additional Project Tree " + QString::number(additionalProjectTrees.size() + 1), this);
        dockWidget->setObjectName("dockWidget");
        dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

        treeView = new RimUiTreeView(dockWidget);
        treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);

        // Drag and drop configuration
        m_treeView->setDragEnabled(true);
        m_treeView->viewport()->setAcceptDrops(true);
        m_treeView->setDropIndicatorShown(true);
        m_treeView->setDragDropMode(QAbstractItemView::DragDrop);

        dockWidget->setWidget(treeView);

        addDockWidget(Qt::RightDockWidgetArea, dockWidget);
        additionalProjectTrees.push_back(dockWidget);
    }

    treeView->setModel(m_treeModelPdm);


    {
        QDockWidget* dockWidget = new QDockWidget("Additional Property Editor "  + QString::number(additionalPropertyEditors.size() + 1), this);
        dockWidget->setObjectName("dockWidget");
        dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

        caf::PdmUiPropertyView* propView = new caf::PdmUiPropertyView(dockWidget);
        dockWidget->setWidget(propView);

        addDockWidget(Qt::RightDockWidgetArea, dockWidget);

        connect(treeView, SIGNAL(selectedObjectChanged( caf::PdmObject* )), propView, SLOT(showProperties( caf::PdmObject* )));
        additionalPropertyEditors.push_back(dockWidget);
    }
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

    app->saveSnapshotForAllViews("snapshots");
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
void RiuMainWindow::slotOpenMultipleCases()
{
#if 1
    RiaApplication* app = RiaApplication::instance();
    RiuMultiCaseImportDialog dialog;
    int action = dialog.exec();
    if (action == QDialog::Accepted)
    {
        QStringList gridFileNames = dialog.eclipseCaseFileNames();
        app->addEclipseCases(gridFileNames);
    }

#else  // Code to fast generate a test project
    RiaApplication* app = RiaApplication::instance();

    QStringList gridFileNames;

    if (1)
    {
        gridFileNames += "Result Mock Debug Model With Results";
        gridFileNames += "Result Mock Debug Model With Results";
        gridFileNames += "Result Mock Debug Model With Results";
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
void RiuMainWindow::refreshDrawStyleActions()
{
    bool enable = RiaApplication::instance()->activeReservoirView() != NULL;

    m_drawStyleLinesAction->setEnabled(enable);
    m_drawStyleLinesSolidAction->setEnabled(enable);
    m_drawStyleSurfOnlyAction->setEnabled(enable);

    m_drawStyleToggleFaultsAction->setEnabled(enable);

    if (enable) 
    {
        m_drawStyleToggleFaultsAction->blockSignals(true);
        m_drawStyleToggleFaultsAction->setChecked(   RiaApplication::instance()->activeReservoirView()->meshMode == RimReservoirView::FAULTS_MESH 
                                                  || RiaApplication::instance()->activeReservoirView()->surfaceMode == RimReservoirView::FAULTS);
        m_drawStyleToggleFaultsAction->blockSignals(false);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::storeTreeViewState()
{
    if (m_treeView)
    {
        QString treeViewState;
        m_treeView->storeTreeViewStateToString(treeViewState);

        QModelIndex mi = m_treeView->currentIndex();

        QString encodedModelIndexString;
        RimUiTreeView::encodeStringFromModelIndex(mi, encodedModelIndexString);
        
        RiaApplication::instance()->project()->treeViewState = treeViewState;
        RiaApplication::instance()->project()->currentModelIndexPath = encodedModelIndexString;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::restoreTreeViewState()
{
    if (m_treeView)
    {
        QString stateString = RiaApplication::instance()->project()->treeViewState;
        if (!stateString.isEmpty())
        {
            m_treeView->collapseAll();
            m_treeView->applyTreeViewStateFromString(stateString);
        }

        QString currentIndexString = RiaApplication::instance()->project()->currentModelIndexPath;
        if (!currentIndexString.isEmpty())
        {
            QModelIndex mi = RimUiTreeView::getModelIndexFromString(m_treeView->model(), currentIndexString);
            m_treeView->setCurrentIndex(mi);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::setCurrentObjectInTreeView(caf::PdmObject* object)
{
    if (m_treeView && m_treeModelPdm)
    {
        QModelIndex mi = m_treeModelPdm->getModelIndexFromPdmObject(object);

        if (mi.isValid())
        {
            m_treeView->setCurrentIndex(mi);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotScaleChanged(int scaleValue)
{
    if (RiaApplication::instance()->activeReservoirView())
    {
        RiaApplication::instance()->activeReservoirView()->scaleZ.setValueFromUi(scaleValue);
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
    if (m_treeView && m_treeView->selectionModel())
    {
        QModelIndexList selectedModelIndexes = m_treeView->selectionModel()->selectedIndexes();
        
        caf::PdmObjectGroup group;
        m_treeView->populateObjectGroupFromModelIndexList(selectedModelIndexes, &group);

        std::vector<caf::PdmPointer<RimCase> > typedObjects;
        group.objectsByType(&typedObjects);

        for (size_t i = 0; i < typedObjects.size(); i++)
        {
            cases.push_back(typedObjects[i]);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindow::slotImportWellPathsFromSSIHub()
{
    CVF_ASSERT(m_ssihubInterface);

    RiaApplication* app = RiaApplication::instance();
    if (!app->project())
    {
        return;
    }

    if (!QFile::exists(app->project()->fileName()))
    {
        return;
    }

    QString wellPathsFolderPath;
    QString projectFileName = app->project()->fileName();
    QFileInfo fileInfo(projectFileName);
    wellPathsFolderPath = fileInfo.canonicalPath();
    QString wellPathFolderName = fileInfo.completeBaseName() + "_wellpaths";

    QDir projFolder(wellPathsFolderPath);
    projFolder.mkdir(wellPathFolderName);

    wellPathsFolderPath += "/" + wellPathFolderName;

    m_ssihubInterface->setWebServiceAddress(app->preferences()->ssihubAddress);
    m_ssihubInterface->setJsonDestinationFolder(wellPathsFolderPath);

    double north = cvf::UNDEFINED_DOUBLE;
    double south = cvf::UNDEFINED_DOUBLE;
    double east = cvf::UNDEFINED_DOUBLE;
    double west = cvf::UNDEFINED_DOUBLE;

    app->project()->computeUtmAreaOfInterest(&north, &south, &east, &west);

    if (north != cvf::UNDEFINED_DOUBLE &&
        south != cvf::UNDEFINED_DOUBLE &&
        east != cvf::UNDEFINED_DOUBLE &&
        west != cvf::UNDEFINED_DOUBLE)
    {
        m_ssihubInterface->setRegion(north, south, east, west);
    }

    QStringList wellPaths = m_ssihubInterface->jsonWellPaths();
    if (wellPaths.size() > 0)
    {
        app->addWellPathsToModel(wellPaths);
        if (app->project()) app->project()->createDisplayModelAndRedrawAllViews();
    }
}
