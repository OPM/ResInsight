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

#include "RiuMainPlotWindow.h"

#include "RiaApplication.h"
#include "RiaBaseDefs.h"
#include "RiaPreferences.h"

#include "RimProject.h"
#include "RimSummaryPlot.h"
#include "RimTreeViewStateSerializer.h"
#include "RimViewWindow.h"
#include "RimWellLogPlot.h"

#include "RiuDragDrop.h"
#include "RiuMdiSubWindow.h"
#include "RiuSummaryQwtPlot.h"
#include "RiuToolTipMenu.h"
#include "RiuTreeViewEventFilter.h"
#include "RiuWellLogPlot.h"

#include "cafCmdFeatureManager.h"
#include "cafPdmUiPropertyView.h"
#include "cafPdmUiTreeView.h"

#include <QCloseEvent>
#include <QDockWidget>
#include <QLayout>
#include <QMdiSubWindow>
#include <QMenuBar>
#include <QToolBar>
#include <QTreeView>

//==================================================================================================
///
/// \class RiuMainPlotWindow
///
/// Contains our main window
///
//==================================================================================================


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuMainPlotWindow::RiuMainPlotWindow()
    : m_pdmRoot(NULL),
    m_mainViewer(NULL),
    m_windowMenu(NULL),
    m_blockSlotSubWindowActivated(false)
{
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

    m_dragDropInterface = std::unique_ptr<caf::PdmUiDragDropInterface>(new RiuDragDrop());

    initializeGuiNewProjectLoaded();

    // Enabling the line below will activate the undo stack
    // When enableUndoCommandSystem is set false, all commands are executed and deleted immediately
    //caf::CmdExecCommandManager::instance()->enableUndoCommandSystem(true);

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainPlotWindow::initializeGuiNewProjectLoaded()
{
    setPdmRoot(RiaApplication::instance()->project());
    restoreTreeViewState();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainPlotWindow::cleanupGuiBeforeProjectClose()
{
    setPdmRoot(NULL);
    
    if (m_pdmUiPropertyView)
    {
        m_pdmUiPropertyView->showProperties(NULL);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainPlotWindow::closeEvent(QCloseEvent* event)
{
    saveWinGeoAndDockToolBarLayout();

    if (!RiaApplication::instance()->tryCloseMainWindow()) return;

    RiaApplication::instance()->closeProject();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainPlotWindow::createActions()
{
    m_snapshotToFile = new QAction(QIcon(":/SnapShotSave.png"), "Snapshot To File", this);
    m_snapshotToClipboard = new QAction(QIcon(":/SnapShot.png"), "Copy Snapshot To Clipboard", this);
    m_snapshotAllViewsToFile = new QAction(QIcon(":/SnapShotSaveViews.png"), "Snapshot All Views To File", this);

    connect(m_snapshotToFile, SIGNAL(triggered()), SLOT(slotSnapshotToFile()));
    connect(m_snapshotToClipboard, SIGNAL(triggered()), SLOT(slotSnapshotToClipboard()));
    connect(m_snapshotAllViewsToFile, SIGNAL(triggered()), SLOT(slotSnapshotAllViewsToFile()));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainPlotWindow::createMenus()
{
    caf::CmdFeatureManager* cmdFeatureMgr = caf::CmdFeatureManager::instance();

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
    importMenu->addAction(cmdFeatureMgr->action("RicImportInputEclipseCaseOpmFeature"));
    importMenu->addAction(cmdFeatureMgr->action("RicCreateGridCaseGroupFeature"));
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

    fileMenu->addSeparator();
    fileMenu->addAction(cmdFeatureMgr->action("RicSaveProjectFeature"));
    fileMenu->addAction(cmdFeatureMgr->action("RicSaveProjectAsFeature"));

    std::vector<QAction*> recentFileActions = RiaApplication::instance()->recentFileActions();
    for (auto act : recentFileActions)
    {
        fileMenu->addAction(act);
    }

    fileMenu->addSeparator();
    fileMenu->addAction(cmdFeatureMgr->action("RicCloseProjectFeature"));
    fileMenu->addSeparator();
    fileMenu->addAction(cmdFeatureMgr->action("RicExitApplicationFeature"));

    // Edit menu
    QMenu* editMenu = menuBar()->addMenu("&Edit");
    editMenu->addAction(cmdFeatureMgr->action("RicSnapshotViewToClipboardFeature"));

    // View menu
    QMenu* viewMenu = menuBar()->addMenu("&View");
    viewMenu->addAction(cmdFeatureMgr->action("RicViewZoomAllFeature"));

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
void RiuMainPlotWindow::createToolBars()
{
    caf::CmdFeatureManager* cmdFeatureMgr = caf::CmdFeatureManager::instance();
    CVF_ASSERT(cmdFeatureMgr);

    {
        QToolBar* toolbar = addToolBar(tr("Standard"));
        toolbar->setObjectName(toolbar->windowTitle());
        toolbar->addAction(cmdFeatureMgr->action("RicImportEclipseCaseFeature"));
        toolbar->addAction(cmdFeatureMgr->action("RicImportInputEclipseCaseFeature"));
        toolbar->addAction(cmdFeatureMgr->action("RicOpenProjectFeature"));
        toolbar->addAction(cmdFeatureMgr->action("RicSaveProjectFeature"));
    }

    {
        QToolBar* toolbar = addToolBar(tr("Window Management"));
        toolbar->setObjectName(toolbar->windowTitle());
        toolbar->addAction(cmdFeatureMgr->action("RicShowMainWindowFeature"));
        toolbar->addAction(cmdFeatureMgr->action("RicTilePlotWindowsFeature"));
    }

    {
        QToolBar* toolbar = addToolBar(tr("View Snapshots"));
        toolbar->setObjectName(toolbar->windowTitle());
        toolbar->addAction(cmdFeatureMgr->action("RicSnapshotViewToClipboardFeature"));
    }

    {
        QToolBar* toolbar = addToolBar(tr("View"));
        toolbar->setObjectName(toolbar->windowTitle());
        toolbar->addAction(cmdFeatureMgr->action("RicViewZoomAllFeature"));
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------

void RiuMainPlotWindow::createDockPanels()
{
    {
        QDockWidget* dockWidget = new QDockWidget("Plot Object Project Tree", this);
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

        m_projectTreeView->setUiConfigurationName("PlotWindow");
    }
    
    {
        QDockWidget* dockWidget = new QDockWidget("Property Editor", this);
        dockWidget->setObjectName("dockWidget");
        dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

        m_pdmUiPropertyView = new caf::PdmUiPropertyView(dockWidget);
        dockWidget->setWidget(m_pdmUiPropertyView);

        m_pdmUiPropertyView->layout()->setContentsMargins(5,0,0,0);

        addDockWidget(Qt::LeftDockWidgetArea, dockWidget);
    }

    setCorner(Qt::BottomLeftCorner,    Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomRightCorner, Qt::BottomDockWidgetArea);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------

QMdiSubWindow* RiuMainPlotWindow::findMdiSubWindow(QWidget* viewer)
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
QList<QMdiSubWindow*> RiuMainPlotWindow::subWindowList(QMdiArea::WindowOrder order)
{
	return m_mdiArea->subWindowList(order);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainPlotWindow::removeViewer(QWidget* viewer)
{
    m_blockSlotSubWindowActivated = true;
    m_mdiArea->removeSubWindow(findMdiSubWindow(viewer));
    m_blockSlotSubWindowActivated = false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainPlotWindow::addViewer(QWidget* viewer, const RimMdiWindowGeometry& windowsGeometry)
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
        RiuWellLogPlot* wellLogPlot = dynamic_cast<RiuWellLogPlot*>(subWin->widget());
        if (wellLogPlot)
        {
            subWindowSize = QSize(275, m_mdiArea->height());
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
}

//--------------------------------------------------------------------------------------------------
/// This method needs to handle memory deallocation !!!
//--------------------------------------------------------------------------------------------------
void RiuMainPlotWindow::setPdmRoot(caf::PdmObject* pdmRoot)
{
    m_pdmRoot = pdmRoot;

    m_projectTreeView->setPdmItem(pdmRoot);
    // For debug only : m_projectTreeView->treeView()->expandAll();
    m_projectTreeView->setDragDropInterface(m_dragDropInterface.get());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainPlotWindow::slotSubWindowActivated(QMdiSubWindow* subWindow)
{
    if (!subWindow) return;
    if (m_blockSlotSubWindowActivated) return;

    RimProject * proj = RiaApplication::instance()->project();
    if (!proj) return;

    {
        RiuWellLogPlot* wellLogPlotViewer = dynamic_cast<RiuWellLogPlot*>(subWindow->widget());
        if (wellLogPlotViewer)
        {
            RimWellLogPlot* wellLogPlot = wellLogPlotViewer->ownerPlotDefinition();

            if (wellLogPlot != RiaApplication::instance()->activeWellLogPlot())
            {
                RiaApplication::instance()->setActiveWellLogPlot(wellLogPlot);
                projectTreeView()->selectAsCurrentItem(wellLogPlot);
            }
        }
        else
        {
            RiaApplication::instance()->setActiveWellLogPlot(NULL);
        }
    }

    {
        RiuSummaryQwtPlot* summaryPlotViewer = dynamic_cast<RiuSummaryQwtPlot*>(subWindow->widget());
        if (summaryPlotViewer)
        {
            RimSummaryPlot* summaryPlot = summaryPlotViewer->ownerPlotDefinition();

            if (summaryPlot != RiaApplication::instance()->activeSummaryPlot())
            {
                RiaApplication::instance()->setActiveSummaryPlot(summaryPlot);
                projectTreeView()->selectAsCurrentItem(summaryPlot);
            }
        }
        else
        {
            RiaApplication::instance()->setActiveSummaryPlot(NULL);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainPlotWindow::setActiveViewer(QWidget* viewer)
{
   m_blockSlotSubWindowActivated = true;
   
   QMdiSubWindow * swin = findMdiSubWindow(viewer);
   if (swin) m_mdiArea->setActiveSubWindow(swin);
   
   m_blockSlotSubWindowActivated = false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainPlotWindow::slotBuildWindowActions()
{
    m_windowMenu->clear();

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

    m_windowMenu->addAction(caf::CmdFeatureManager::instance()->action("RicTilePlotWindowsFeature"));
    m_windowMenu->addAction(cascadeWindowsAction);
    m_windowMenu->addAction(closeAllSubWindowsAction);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainPlotWindow::selectedObjectsChanged()
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

        // Well log plot

        bool isActiveWellLogPlotChanged = false;
        
        RimWellLogPlot* selectedWellLogPlot = dynamic_cast<RimWellLogPlot*>(firstSelectedObject);

        if (!selectedWellLogPlot)
        {
            firstSelectedObject->firstAncestorOrThisOfType(selectedWellLogPlot);
        }

        if (selectedWellLogPlot)
        {
            if (selectedWellLogPlot->viewer())
            {
                setActiveViewer(selectedWellLogPlot->viewer());

            }
            isActiveWellLogPlotChanged = true;
        }

        if (isActiveWellLogPlotChanged)
        {
            RiaApplication::instance()->setActiveWellLogPlot(selectedWellLogPlot);
        }

        // Summary plot

        bool isActiveSummaryPlotChanged = false;

        RimSummaryPlot* selectedSummaryPlot = dynamic_cast<RimSummaryPlot*>(firstSelectedObject);

        if (!selectedSummaryPlot)
        {
            firstSelectedObject->firstAncestorOrThisOfType(selectedSummaryPlot);
        }

        if (selectedSummaryPlot)
        {
            if (selectedSummaryPlot->viewer())
            {
                setActiveViewer(selectedSummaryPlot->viewer());
            }
            isActiveSummaryPlotChanged = true;
        }

        if (isActiveSummaryPlotChanged)
        {
            RiaApplication::instance()->setActiveSummaryPlot(selectedSummaryPlot);
        }

        if (isActiveWellLogPlotChanged || isActiveSummaryPlotChanged)
        {
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
void RiuMainPlotWindow::slotSnapshotToFile()
{
    RiaApplication* app = RiaApplication::instance();

    app->saveSnapshotPromtpForFilename();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainPlotWindow::slotSnapshotToClipboard()
{
    RiaApplication* app = RiaApplication::instance();

    app->copySnapshotToClipboard();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainPlotWindow::slotSnapshotAllViewsToFile()
{
    RiaApplication* app = RiaApplication::instance();

    // Save images in snapshot catalog relative to project directory
    QString absolutePathToSnapshotDir = app->createAbsolutePathFromProjectRelativePath("snapshots");
    app->saveSnapshotForAllViews(absolutePathToSnapshotDir);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainPlotWindow::hideAllDockWindows()
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
void RiuMainPlotWindow::storeTreeViewState()
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
void RiuMainPlotWindow::restoreTreeViewState()
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
void RiuMainPlotWindow::selectAsCurrentItem(caf::PdmObject* object)
{
    m_projectTreeView->selectAsCurrentItem(object);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainPlotWindow::setDefaultWindowSize()
{
    resize(1000, 810);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainPlotWindow::setExpanded(const caf::PdmUiItem* uiItem, bool expanded)
{
    m_projectTreeView->setExpanded(uiItem, expanded);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainPlotWindow::customMenuRequested(const QPoint& pos)
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
RimMdiWindowGeometry RiuMainPlotWindow::windowGeometryForViewer(QWidget* viewer)
{
    QMdiSubWindow* mdiWindow = findMdiSubWindow(viewer);
    if (mdiWindow)
    {
        return windowGeometryForWidget(mdiWindow);
    }

    RimMdiWindowGeometry geo;
    return geo;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimMdiWindowGeometry RiuMainPlotWindow::windowGeometryForWidget(QWidget* widget)
{
    RimMdiWindowGeometry geo;

    if (widget)
    {
        geo.x = widget->pos().x();
        geo.y = widget->pos().y();
        geo.width = widget->size().width();
        geo.height = widget->size().height();
        geo.isMaximized = widget->isMaximized();
    }

    return geo;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainPlotWindow::tileWindows()
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
bool RiuMainPlotWindow::isAnyMdiSubWindowVisible()
{
    return m_mdiArea->subWindowList().size() > 0;
}

