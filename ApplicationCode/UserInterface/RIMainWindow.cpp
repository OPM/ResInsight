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

#include "RIStdInclude.h"
#include "RIBaseDefs.h"

#include "RIApplication.h"
#include "RIMainWindow.h"
#include "RIViewer.h"
#include "RIResultInfoPanel.h"
#include "RIProcessMonitor.h"
#include "RIPreferences.h"
#include "RIPreferencesDialog.h"

#include "RigEclipseCase.h"
#include "RigReservoirCellResults.h"
#include "RimReservoir.h"
#include "RimUiTreeModelPdm.h"

#include "cvfqtBasicAboutDialog.h"
#include "cafUtils.h"

#include "cafFrameAnimationControl.h"
#include "cafAnimationToolBar.h"

#include "cafPdmUiPropertyView.h"
#include "RimUiTreeView.h"
#include "RiuMultiCaseImportDialog.h"



//==================================================================================================
///
/// \class RIMainWindow
///
/// Contains our main window
///
//==================================================================================================


RIMainWindow* RIMainWindow::sm_mainWindowInstance = NULL;

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RIMainWindow::RIMainWindow()
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
    setPdmRoot(RIApplication::instance()->project());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RIMainWindow* RIMainWindow::instance()
{
    return sm_mainWindowInstance;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RIMainWindow::initializeGuiNewProjectLoaded()
{
    slotRefreshFileActions();
    slotRefreshEditActions();
    refreshAnimationActions();
    setPdmRoot(RIApplication::instance()->project());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RIMainWindow::cleanupGuiBeforeProjectClose()
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
/// Lightweight refresh of GUI (mainly toolbar actions). Called during idle processing
//--------------------------------------------------------------------------------------------------
void RIMainWindow::refreshGuiLightweight()
{
    refreshToolbars();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RIMainWindow::refreshToolbars()
{
    slotRefreshFileActions();
    slotRefreshViewActions();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RIMainWindow::closeEvent(QCloseEvent* event)
{
    if (!RIApplication::instance()->closeProject(true))
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
void RIMainWindow::createActions()
{
    // File actions
    m_openAction                = new QAction(QIcon(":/AppLogo48x48.png"), "&Open Eclipse Case", this);
    m_openInputEclipseFileAction= new QAction(QIcon(":/EclipseInput48x48.png"), "&Open Input Eclipse Case", this);
    m_openMultipleEclipseCasesAction = new QAction(QIcon(":/CreateGridCaseGroup16x16.png"), "&Create Grid Case Group from Files", this);
    
    m_openProjectAction         = new QAction(style()->standardIcon(QStyle::SP_DirOpenIcon), "&Open Project", this);
    m_openLastUsedProjectAction = new QAction("Open &Last Used Project", this);

    m_mockModelAction           = new QAction("&Mock Model", this);
    m_mockResultsModelAction    = new QAction("Mock Model With &Results", this);
    m_mockLargeResultsModelAction = new QAction("Large Mock Model", this);
    m_mockInputModelAction      = new QAction("Input Mock Model", this);

    m_snapshotToFile            = new QAction("Snapshot To File", this);
    m_snapshotToClipboard       = new QAction("Snapshot To Clipboard", this);
    m_snapshotAllViewsToFile    = new QAction("Snapshot All Views To File", this);

    m_saveProjectAction         = new QAction(QIcon(":/Save.png"), "&Save Project", this);
    m_saveProjectAsAction       = new QAction(QIcon(":/Save.png"), "Save Project &As", this);

    m_closeAction               = new QAction("&Close", this);
    m_exitAction		        = new QAction("E&xit", this);

    connect(m_openAction,	            SIGNAL(triggered()), SLOT(slotOpenBinaryGridFiles()));
    connect(m_openInputEclipseFileAction,SIGNAL(triggered()), SLOT(slotOpenInputFiles()));
    connect(m_openMultipleEclipseCasesAction,SIGNAL(triggered()), SLOT(slotOpenMultipleCases()));
    connect(m_openProjectAction,	    SIGNAL(triggered()), SLOT(slotOpenProject()));
    connect(m_openLastUsedProjectAction,SIGNAL(triggered()), SLOT(slotOpenLastUsedProject()));
    
    connect(m_mockModelAction,	        SIGNAL(triggered()), SLOT(slotMockModel()));
    connect(m_mockResultsModelAction,	SIGNAL(triggered()), SLOT(slotMockResultsModel()));
    connect(m_mockLargeResultsModelAction,	SIGNAL(triggered()), SLOT(slotMockLargeResultsModel()));
    connect(m_mockInputModelAction,	    SIGNAL(triggered()), SLOT(slotInputMockModel()));

    connect(m_snapshotToFile,	        SIGNAL(triggered()), SLOT(slotSnapshotToFile()));
    connect(m_snapshotToClipboard,	    SIGNAL(triggered()), SLOT(slotSnapshotToClipboard()));
    connect(m_snapshotAllViewsToFile,   SIGNAL(triggered()), SLOT(slotSnapshotAllViewsToFile()));
    
    connect(m_saveProjectAction,	    SIGNAL(triggered()), SLOT(slotSaveProject()));
    connect(m_saveProjectAsAction,	    SIGNAL(triggered()), SLOT(slotSaveProjectAs()));

    connect(m_closeAction,	            SIGNAL(triggered()), SLOT(slotCloseProject()));

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
    m_newPropertyView = new QAction("New Property View", this);
    connect(m_newPropertyView, SIGNAL(triggered()), SLOT(slotNewObjectPropertyView()));

    // Help actions
    m_aboutAction = new QAction("&About", this);    
    connect(m_aboutAction, SIGNAL(triggered()), SLOT(slotAbout()));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RIMainWindow::createMenus()
{
    // File menu
    QMenu* fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction(m_openAction);
    fileMenu->addAction(m_openInputEclipseFileAction);
    fileMenu->addAction(m_openMultipleEclipseCasesAction);
    fileMenu->addAction(m_openProjectAction);
    fileMenu->addAction(m_openLastUsedProjectAction);

    fileMenu->addSeparator();
    fileMenu->addAction(m_saveProjectAction);
    fileMenu->addAction(m_saveProjectAsAction);

    fileMenu->addSeparator();
    fileMenu->addAction(m_closeAction);
    fileMenu->addSeparator();
    fileMenu->addAction(m_exitAction);

    connect(fileMenu, SIGNAL(aboutToShow()), SLOT(slotRefreshFileActions()));

    // Edit menu
    QMenu* editMenu = menuBar()->addMenu("&Edit");
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
    viewMenu->addSeparator();
    viewMenu->addAction(m_newPropertyView);

    connect(viewMenu, SIGNAL(aboutToShow()), SLOT(slotRefreshViewActions()));

    // Debug menu
    QMenu* debugMenu = menuBar()->addMenu("&Debug");
    debugMenu->addAction(m_mockModelAction);
    debugMenu->addAction(m_mockResultsModelAction);
    debugMenu->addAction(m_mockLargeResultsModelAction);
    debugMenu->addAction(m_mockInputModelAction);

    debugMenu->addSeparator();
    debugMenu->addAction(m_snapshotToClipboard);
    debugMenu->addAction(m_snapshotToFile);
    debugMenu->addAction(m_snapshotAllViewsToFile);

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
void RIMainWindow::createToolBars()
{

    m_standardToolBar = addToolBar(tr("Standard"));
    m_standardToolBar->setObjectName(m_standardToolBar->windowTitle());

    m_standardToolBar->addAction(m_openAction);
    m_standardToolBar->addAction(m_openInputEclipseFileAction);
    m_standardToolBar->addAction(m_openProjectAction);
    //m_standardToolBar->addAction(m_openLastUsedProjectAction);
    m_standardToolBar->addAction(m_saveProjectAction);

    // Create animation toolbar
    m_animationToolBar = new caf::AnimationToolBar("Animation", this);
    addToolBar(m_animationToolBar);
    //connect(m_animationToolBar, SIGNAL(signalFrameRateChanged(double)), SLOT(slotFramerateChanged(double)));

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

    refreshAnimationActions();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------

void RIMainWindow::createDockPanels()
{
    {
        QDockWidget* dockWidget = new QDockWidget("Project", this);
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
        QDockWidget* dockPanel = new QDockWidget("Result Info", this);
        dockPanel->setObjectName("dockResultInfoPanel");
        dockPanel->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
        m_resultInfoPanel = new RIResultInfoPanel(dockPanel);
        dockPanel->setWidget(m_resultInfoPanel);

        addDockWidget(Qt::BottomDockWidgetArea, dockPanel);
    }

    {
        QDockWidget* dockPanel = new QDockWidget("Process Monitor", this);
        dockPanel->setObjectName("dockProcessMonitor");
        dockPanel->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
        m_processMonitor = new RIProcessMonitor(dockPanel);
        dockPanel->setWidget(m_processMonitor);

        addDockWidget(Qt::BottomDockWidgetArea, dockPanel);
    }

    {
        QDockWidget* dockWidget = new QDockWidget("Properties", this);
        dockWidget->setObjectName("dockWidget");
        dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

        m_pdmUiPropertyView = new caf::PdmUiPropertyView(dockWidget);
        dockWidget->setWidget(m_pdmUiPropertyView);

        connect(m_treeView, SIGNAL(selectedObjectChanged( caf::PdmObject* )), m_pdmUiPropertyView, SLOT(showProperties( caf::PdmObject* )));

        addDockWidget(Qt::LeftDockWidgetArea, dockWidget);
    }

    setCorner(Qt::BottomLeftCorner,	Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomRightCorner, Qt::BottomDockWidgetArea);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RIMainWindow::saveWinGeoAndDockToolBarLayout()
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
void RIMainWindow::loadWinGeoAndDockToolBarLayout()
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
void RIMainWindow::setResultInfo(const QString& info) const
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
void RIMainWindow::slotRefreshFileActions()
{
    RIApplication* app = RIApplication::instance();

    bool projectExists = true;
    m_saveProjectAction->setEnabled(projectExists);
    m_saveProjectAsAction->setEnabled(projectExists);
    m_closeAction->setEnabled(projectExists);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RIMainWindow::slotRefreshEditActions()
{
//     RIApplication* app = RIApplication::instance();
//     RISceneManager* proj = app->project();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RIMainWindow::slotRefreshViewActions()
{
//     RIApplication* app = RIApplication::instance();
//     RISceneManager* proj = app->project();

    bool enabled = true;
    m_viewFromNorth->setEnabled(enabled);
    m_viewFromSouth->setEnabled(enabled);
    m_viewFromEast->setEnabled(enabled);
    m_viewFromWest->setEnabled(enabled);
    m_viewFromAbove->setEnabled(enabled);
    m_viewFromBelow->setEnabled(enabled);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RIMainWindow::refreshAnimationActions()
{
    caf::FrameAnimationControl* ac = NULL;
    if (RIApplication::instance()->activeReservoirView() && RIApplication::instance()->activeReservoirView()->viewer())
    {
        ac = RIApplication::instance()->activeReservoirView()->viewer()->animationControl();
    }

    m_animationToolBar->connectAnimationControl(ac);

    QStringList timeStepStrings;
    int currentTimeStepIndex = 0;

    RIApplication* app = RIApplication::instance();

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
                currentTimeStepIndex = RIApplication::instance()->activeReservoirView()->currentTimeStep();
            }
            else
            {
                timeStepStrings.push_back(tr("Static Property"));
            }
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
void RIMainWindow::slotAbout()
{
    cvfqt::BasicAboutDialog dlg(this);

    dlg.setApplicationName(RI_APPLICATION_NAME);
    dlg.setApplicationVersion(RIApplication::getVersionStringApp(true));
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
void RIMainWindow::slotOpenBinaryGridFiles()
{
    if (checkForDocumentModifications())
    {
        RIApplication* app = RIApplication::instance();

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
void RIMainWindow::slotOpenInputFiles()
{
    if (checkForDocumentModifications())
    {
        RIApplication* app = RIApplication::instance();
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
void RIMainWindow::slotOpenProject()
{
    if (checkForDocumentModifications())
    {
        RIApplication* app = RIApplication::instance();
        QString defaultDir = app->defaultFileDialogDirectory("BINARY_GRID");
        QString fileName = QFileDialog::getOpenFileName(this, "Open ResInsight Project", defaultDir, "ResInsight project (*.rip)");

        if (fileName.isEmpty()) return;

        // Remember the path to next time
        app->setDefaultFileDialogDirectory("BINARY_GRID", QFileInfo(fileName).absolutePath());

        app->loadProject(fileName);
    }

    //m_mainViewer->setDefaultView();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RIMainWindow::slotOpenLastUsedProject()
{
    RIApplication* app = RIApplication::instance();
    app->loadLastUsedProject();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RIMainWindow::slotMockModel()
{
    RIApplication* app = RIApplication::instance();
    app->createMockModel();

    //m_mainViewer->setDefaultView();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RIMainWindow::slotMockResultsModel()
{
    RIApplication* app = RIApplication::instance();
    app->createResultsMockModel();

    //m_mainViewer->setDefaultView();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RIMainWindow::slotMockLargeResultsModel()
{
    RIApplication* app = RIApplication::instance();
    app->createLargeResultsMockModel();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RIMainWindow::slotInputMockModel()
{
    RIApplication* app = RIApplication::instance();
    app->createInputMockModel();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RIMainWindow::slotSetCurrentFrame(int frameIndex)
{
    RIApplication* app = RIApplication::instance();
   // app->setTimeStep(frameIndex);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RIMainWindow::checkForDocumentModifications()
{
    RIApplication* app = RIApplication::instance();
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
void RIMainWindow::slotCloseProject()
{
    RIApplication* app = RIApplication::instance();
    bool ret = app->closeProject(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------

QMdiSubWindow* RIMainWindow::findMdiSubWindow(RIViewer* viewer)
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
void RIMainWindow::removeViewer(RIViewer* viewer)
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
void RIMainWindow::addViewer(RIViewer* viewer)
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
void RIMainWindow::slotSaveProject()
{
    RIApplication* app = RIApplication::instance();

    app->saveProject();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RIMainWindow::slotSaveProjectAs()
{
    RIApplication* app = RIApplication::instance();

    app->saveProjectPromptForFileName();
}


//--------------------------------------------------------------------------------------------------
/// This method needs to handle memory deallocation !!!
//--------------------------------------------------------------------------------------------------
void RIMainWindow::setPdmRoot(caf::PdmObject* pdmRoot)
{
    m_pdmRoot = pdmRoot;

    caf::PdmUiTreeItem* treeItemRoot = caf::UiTreeItemBuilderPdm::buildViewItems(NULL, -1, m_pdmRoot);
    m_treeModelPdm->setRoot(treeItemRoot);

    m_treeView->expandAll();

    if (treeItemRoot && m_treeView->selectionModel())
    {
        connect(m_treeView->selectionModel(), SIGNAL(currentChanged ( const QModelIndex & , const QModelIndex & )), SLOT(slotCurrentChanged( const QModelIndex & , const QModelIndex & )));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RIMainWindow::slotViewFromNorth()
{
    if (RIApplication::instance()->activeReservoirView() &&  RIApplication::instance()->activeReservoirView()->viewer())
    {
        RIApplication::instance()->activeReservoirView()->viewer()->setView(cvf::Vec3d(0,-1,0), cvf::Vec3d(0,0,1));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RIMainWindow::slotViewFromSouth()
{
    if (RIApplication::instance()->activeReservoirView() &&  RIApplication::instance()->activeReservoirView()->viewer())
    {
        RIApplication::instance()->activeReservoirView()->viewer()->setView(cvf::Vec3d(0,1,0), cvf::Vec3d(0,0,1));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RIMainWindow::slotViewFromEast()
{
    if (RIApplication::instance()->activeReservoirView() &&  RIApplication::instance()->activeReservoirView()->viewer())
    {
        RIApplication::instance()->activeReservoirView()->viewer()->setView(cvf::Vec3d(-1,0,0), cvf::Vec3d(0,0,1));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RIMainWindow::slotViewFromWest()
{
    if (RIApplication::instance()->activeReservoirView() &&  RIApplication::instance()->activeReservoirView()->viewer())
    {
        RIApplication::instance()->activeReservoirView()->viewer()->setView(cvf::Vec3d(1,0,0), cvf::Vec3d(0,0,1));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RIMainWindow::slotViewFromAbove()
{
    if (RIApplication::instance()->activeReservoirView() &&  RIApplication::instance()->activeReservoirView()->viewer())
    {
        RIApplication::instance()->activeReservoirView()->viewer()->setView(cvf::Vec3d(0,0,-1), cvf::Vec3d(0,1,0));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RIMainWindow::slotViewFromBelow()
{
    if (RIApplication::instance()->activeReservoirView() &&  RIApplication::instance()->activeReservoirView()->viewer())
    {
        RIApplication::instance()->activeReservoirView()->viewer()->setView(cvf::Vec3d(0,0,1), cvf::Vec3d(0,1,0));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RIMainWindow::slotZoomAll()
{
    if (RIApplication::instance()->activeReservoirView() &&  RIApplication::instance()->activeReservoirView()->viewer())
    {
        RIApplication::instance()->activeReservoirView()->viewer()->zoomAll();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RIMainWindow::slotSubWindowActivated(QMdiSubWindow* subWindow)
{
    RimProject * proj = RIApplication::instance()->project();
    if (!proj) return;

    size_t i;
    for (i = 0; i < proj->reservoirs().size(); ++i)
    {
        RimReservoir* ri = proj->reservoirs()[i];
        if (!ri) continue;

        size_t j;
        for (j = 0; j < ri->reservoirViews().size(); j++)
        {
            RimReservoirView* riv = ri->reservoirViews()[j];

            if (riv &&
                riv->viewer() &&
                riv->viewer()->layoutWidget() &&
                riv->viewer()->layoutWidget()->parent() == subWindow)
            {
                RimReservoirView* previousActiveReservoirView = RIApplication::instance()->activeReservoirView();
                RIApplication::instance()->setActiveReservoirView(riv);
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
                    //m_treeView->setExpanded(previousViewModelIndex, false);
                    if (newSelectionIndex != newViewModelIndex)
                    {
                        m_treeView->setExpanded(newViewModelIndex, true);
                    }
                }

                refreshAnimationActions();
                break;
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RIMainWindow::slotUseShaders(bool enable)
{
    RIApplication::instance()->setUseShaders(enable);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RIMainWindow::slotShowPerformanceInfo(bool enable)
{
    RIApplication::instance()->setShowPerformanceInfo(enable);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RIMainWindow::slotRefreshDebugActions()
{
    RIApplication* app = RIApplication::instance();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RIMainWindow::slotEditPreferences()
{
    RIApplication* app = RIApplication::instance();
    RIPreferencesDialog preferencesDialog(this, app->preferences(), "Preferences");
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
void RIMainWindow::setActiveViewer(RIViewer* viewer)
{
   QMdiSubWindow * swin = findMdiSubWindow(viewer); 
   if (swin) m_mdiArea->setActiveSubWindow(swin);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RIMainWindow::slotFramerateChanged(double frameRate)
{
    if (RIApplication::instance()->activeReservoirView() != NULL)
    {
        RIApplication::instance()->activeReservoirView()->maximumFrameRate.setValueFromUi(QVariant(frameRate));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RIProcessMonitor* RIMainWindow::processMonitor()
{
    return m_processMonitor;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RIMainWindow::slotBuildWindowActions()
{
    m_windowMenu->clear();

    QList<QDockWidget*> dockWidgets = findChildren<QDockWidget*>();
    foreach (QDockWidget* dock, dockWidgets)
    {
        if (dock)
        {
            m_windowMenu->addAction(dock->toggleViewAction());
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RIMainWindow::slotCurrentChanged(const QModelIndex & current, const QModelIndex & previous)
{
    RimReservoirView* activeReservoirView = RIApplication::instance()->activeReservoirView();
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
                RIApplication::instance()->setActiveReservoirView(rimReservoirView);

                // Set focus in MDI area to this window if it exists
                if (rimReservoirView->viewer())
                {
                    setActiveViewer(rimReservoirView->viewer());
                }
                m_treeView->setCurrentIndex(current);

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
void RIMainWindow::slotNewObjectPropertyView()
{
    if (!m_treeModelPdm) return;

    RimUiTreeView* treeView = NULL;
    
    {
        QDockWidget* dockWidget = new QDockWidget("Project", this);
        dockWidget->setObjectName("dockWidget");
        dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

        treeView = new RimUiTreeView(dockWidget);
        dockWidget->setWidget(treeView);

        addDockWidget(Qt::RightDockWidgetArea, dockWidget);
    }

    treeView->setModel(m_treeModelPdm);
    treeView->expandAll();


    {
        QDockWidget* dockWidget = new QDockWidget("Properties", this);
        dockWidget->setObjectName("dockWidget");
        dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

        caf::PdmUiPropertyView* propView = new caf::PdmUiPropertyView(dockWidget);
        dockWidget->setWidget(propView);

        addDockWidget(Qt::RightDockWidgetArea, dockWidget);

        connect(treeView, SIGNAL(selectedObjectChanged( caf::PdmObject* )), propView, SLOT(showProperties( caf::PdmObject* )));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RIMainWindow::slotSnapshotToFile()
{
    RIApplication* app = RIApplication::instance();

    app->saveSnapshotPromtpForFilename();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RIMainWindow::slotSnapshotToClipboard()
{
    RIApplication* app = RIApplication::instance();

    app->copySnapshotToClipboard();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RIMainWindow::slotSnapshotAllViewsToFile()
{
    RIApplication* app = RIApplication::instance();

    app->saveSnapshotForAllViews("snapshots");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RIMainWindow::hideAllDockWindows()
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
void RIMainWindow::slotOpenMultipleCases()
{
    RIApplication* app = RIApplication::instance();

    RiuMultiCaseImportDialog dialog;
    int action = dialog.exec();
    if (action == QDialog::Accepted)
    {
        QStringList gridFileNames = dialog.eclipseCaseFileNames();
        app->addEclipseCases(gridFileNames);
    }
}
