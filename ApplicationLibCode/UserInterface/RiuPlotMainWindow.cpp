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
#include "RiaPreferences.h"
#include "RiaRegressionTestRunner.h"
#include "RiaSummaryTools.h"

#include "RimEnsembleCurveSetCollection.h"
#include "RimMultiPlot.h"
#include "RimProject.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryCurveCollection.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"
#include "RimSummaryPlotFilterTextCurveSetEditor.h"
#include "RimViewWindow.h"
#include "RimWellAllocationPlot.h"
#include "RimWellLogCurveCommonDataSource.h"
#include "RimWellLogPlot.h"
#include "RimWellRftPlot.h"

#include "SummaryPlotCommands/RicSummaryCurveCalculatorDialog.h"
#include "SummaryPlotCommands/RicSummaryPlotEditorDialog.h"

#include "RiuDockWidgetTools.h"
#include "RiuDragDrop.h"
#include "RiuMdiSubWindow.h"
#include "RiuMessagePanel.h"
#include "RiuMultiPlotPage.h"
#include "RiuToolTipMenu.h"
#include "RiuTreeViewEventFilter.h"
#include "RiuWellAllocationPlot.h"

#include "cafCmdFeatureManager.h"
#include "cafPdmObjectHandle.h"
#include "cafPdmUiPropertyView.h"
#include "cafPdmUiToolBarEditor.h"
#include "cafPdmUiTreeView.h"
#include "cafQTreeViewStateSerializer.h"
#include "cafSelectionManager.h"

#include <QCloseEvent>
#include <QDockWidget>
#include <QLayout>
#include <QMdiSubWindow>
#include <QMenuBar>
#include <QToolBar>
#include <QTreeView>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotMainWindow::RiuPlotMainWindow()
    : m_activePlotViewWindow( nullptr )
    , m_windowMenu( nullptr )
{
    m_mdiArea = new RiuMdiArea;
    connect( m_mdiArea, SIGNAL( subWindowActivated( QMdiSubWindow* ) ), SLOT( slotSubWindowActivated( QMdiSubWindow* ) ) );
    setCentralWidget( m_mdiArea );

    createMenus();
    createToolBars();
    createDockPanels();

    // Store the layout so we can offer reset option
    m_initialDockAndToolbarLayout = saveState( 0 );

    m_dragDropInterface = std::unique_ptr<caf::PdmUiDragDropInterface>( new RiuDragDrop() );

    // Enabling the line below will activate the undo stack
    // When enableUndoCommandSystem is set false, all commands are executed and deleted immediately
    // caf::CmdExecCommandManager::instance()->enableUndoCommandSystem(true);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotMainWindow::~RiuPlotMainWindow()
{
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
void RiuPlotMainWindow::initializeGuiNewProjectLoaded()
{
    setPdmRoot( RimProject::current() );
    restoreTreeViewState();

    if ( m_pdmUiPropertyView && m_pdmUiPropertyView->currentObject() )
    {
        m_pdmUiPropertyView->currentObject()->uiCapability()->updateConnectedEditors();
    }

    {
        auto* obj = RiaSummaryTools::summaryCaseMainCollection();
        if ( obj )
        {
            setExpanded( obj );
        }
    }

    {
        auto* obj = RiaSummaryTools::summaryPlotCollection();
        if ( obj )
        {
            setExpanded( obj );
        }
    }

    if ( subWindowsAreTiled() )
    {
        tileSubWindows();
    }

    if ( m_activePlotViewWindow && m_activePlotViewWindow->viewWidget() &&
         !RiaRegressionTestRunner::instance()->isRunningRegressionTests() )
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
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotMainWindow::cleanupGuiBeforeProjectClose()
{
    setPdmRoot( nullptr );

    if ( m_pdmUiPropertyView )
    {
        m_pdmUiPropertyView->showProperties( nullptr );
    }

    cleanUpTemporaryWidgets();

    m_wellLogPlotToolBarEditor->clear();
    m_summaryPlotToolBarEditor->clear();
    m_multiPlotToolBarEditor->clear();

    setWindowTitle( "Plots - ResInsight" );
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
    this->saveWinGeoAndDockToolBarLayout();

    RiaGuiApplication* app = RiaGuiApplication::instance();

    if ( app->isMain3dWindowVisible() )
    {
        event->ignore();
        this->hide();
        return;
    }

    if ( !app->askUserToSaveModifiedProject() )
    {
        event->ignore();
        return;
    }

    this->hideAllDockWidgets();
    app->closeMainWindowIfOpenButHidden();
    app->closeProject();
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

    // File menu
    QMenu* fileMenu = new RiuToolTipMenu( menuBar() );
    fileMenu->setTitle( "&File" );

    menuBar()->addMenu( fileMenu );

    fileMenu->addAction( cmdFeatureMgr->action( "RicOpenProjectFeature" ) );
    fileMenu->addAction( cmdFeatureMgr->action( "RicOpenLastUsedFileFeature" ) );
    fileMenu->addSeparator();

    QMenu* importMenu = fileMenu->addMenu( "&Import" );

    QMenu* importEclipseMenu = importMenu->addMenu( QIcon( ":/Case24x24.png" ), "Eclipse Cases" );
    importEclipseMenu->addAction( cmdFeatureMgr->action( "RicImportEclipseCaseFeature" ) );
    importEclipseMenu->addAction( cmdFeatureMgr->action( "RicImportEclipseCasesFeature" ) );
    importEclipseMenu->addAction( cmdFeatureMgr->action( "RicImportInputEclipseCaseFeature" ) );
    importEclipseMenu->addAction( cmdFeatureMgr->action( "RicCreateGridCaseGroupFeature" ) );
    importEclipseMenu->addAction( cmdFeatureMgr->action( "RicCreateGridCaseGroupFromFilesFeature" ) );

#ifdef USE_ODB_API
    importMenu->addSeparator();
    QMenu* importGeoMechMenu = importMenu->addMenu( QIcon( ":/GeoMechCase24x24.png" ), "Geo Mechanical Cases" );
    importGeoMechMenu->addAction( cmdFeatureMgr->action( "RicImportGeoMechCaseFeature" ) );
    importGeoMechMenu->addAction( cmdFeatureMgr->action( "RicImportElementPropertyFeature" ) );
#endif

    importMenu->addSeparator();
    QMenu* importSummaryMenu = importMenu->addMenu( QIcon( ":/SummaryCase.svg" ), "Summary Cases" );
    importSummaryMenu->addAction( cmdFeatureMgr->action( "RicImportSummaryCaseFeature" ) );
    importSummaryMenu->addAction( cmdFeatureMgr->action( "RicImportSummaryCasesFeature" ) );
    importSummaryMenu->addAction( cmdFeatureMgr->action( "RicImportSummaryGroupFeature" ) );
    importSummaryMenu->addAction( cmdFeatureMgr->action( "RicImportEnsembleFeature" ) );

    importMenu->addSeparator();
    QMenu* importWellMenu = importMenu->addMenu( QIcon( ":/Well.svg" ), "Well Data" );
    importWellMenu->addAction( cmdFeatureMgr->action( "RicWellPathsImportFileFeature" ) );
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
    exportMenu->addAction( cmdFeatureMgr->action( "RicSnapshotViewToPdfFeature" ) );
    exportMenu->addAction( cmdFeatureMgr->action( "RicSnapshotAllPlotsToFileFeature" ) );
    exportMenu->addAction( cmdFeatureMgr->action( "RicSaveEclipseInputActiveVisibleCellsFeature" ) );

    fileMenu->addSeparator();
    fileMenu->addAction( cmdFeatureMgr->action( "RicSaveProjectFeature" ) );
    fileMenu->addAction( cmdFeatureMgr->action( "RicSaveProjectAsFeature" ) );

    std::vector<QAction*> recentFileActions = RiaGuiApplication::instance()->recentFileActions();
    for ( auto act : recentFileActions )
    {
        fileMenu->addAction( act );
    }

    fileMenu->addSeparator();
    fileMenu->addAction( cmdFeatureMgr->action( "RicCloseProjectFeature" ) );
    fileMenu->addSeparator();
    fileMenu->addAction( cmdFeatureMgr->action( "RicExitApplicationFeature" ) );

    // Edit menu
    QMenu* editMenu = menuBar()->addMenu( "&Edit" );
    editMenu->addAction( cmdFeatureMgr->action( "RicSnapshotViewToClipboardFeature" ) );
    editMenu->addAction( cmdFeatureMgr->action( "RicSnapshotViewToFileFeature" ) );
    editMenu->addAction( cmdFeatureMgr->action( "RicSnapshotViewToPdfFeature" ) );
    editMenu->addSeparator();
    editMenu->addAction( cmdFeatureMgr->action( "RicEditPreferencesFeature" ) );

    // View menu
    QMenu* viewMenu = menuBar()->addMenu( "&View" );
    viewMenu->addAction( cmdFeatureMgr->action( "RicViewZoomAllFeature" ) );

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
    helpMenu->addAction( cmdFeatureMgr->action( "RicSearchHelpFeature" ) );

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
        commandIds << "RicImportGeneralDataFeature";
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
    toolbarNames << "Standard"
                 << "Window Management"
                 << "View Snapshots"
                 << "View";

    for ( QString toolbarName : toolbarNames )
    {
        QToolBar* toolbar = addToolBar( toolbarName );
        toolbar->setObjectName( toolbar->windowTitle() );

        QStringList toolbarCommands = toolbarCommandIds( toolbarName );
        for ( QString s : toolbarCommands )
        {
            toolbar->addAction( cmdFeatureMgr->action( s ) );
        }
    }

    m_wellLogPlotToolBarEditor = new caf::PdmUiToolBarEditor( "Well Log Plot", this );
    m_wellLogPlotToolBarEditor->hide();

    m_summaryPlotToolBarEditor = new caf::PdmUiToolBarEditor( "Summary Plot", this );
    m_summaryPlotToolBarEditor->hide();

    m_multiPlotToolBarEditor = new caf::PdmUiToolBarEditor( "Multi Plot", this );
    m_multiPlotToolBarEditor->hide();
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
    {
        QDockWidget* dockWidget = new QDockWidget( "Plot Project Tree", this );
        dockWidget->setObjectName( RiuDockWidgetTools::plotMainWindowProjectTreeName() );
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

        m_projectTreeView->setUiConfigurationName( "PlotWindow" );
    }

    {
        QDockWidget* dockWidget = new QDockWidget( "Property Editor", this );
        dockWidget->setObjectName( RiuDockWidgetTools::plotMainWindowPropertyEditorName() );
        dockWidget->setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );

        m_pdmUiPropertyView = new caf::PdmUiPropertyView( dockWidget );
        dockWidget->setWidget( m_pdmUiPropertyView );

        addDockWidget( Qt::LeftDockWidgetArea, dockWidget );
    }

    {
        QDockWidget* dockWidget = new QDockWidget( "Messages", this );
        dockWidget->setObjectName( RiuDockWidgetTools::plotMainWindowMessagesName() );
        m_messagePanel = new RiuMessagePanel( dockWidget );
        dockWidget->setWidget( m_messagePanel );
        addDockWidget( Qt::BottomDockWidgetArea, dockWidget );
        dockWidget->hide();
    }

    setCorner( Qt::BottomLeftCorner, Qt::LeftDockWidgetArea );
    setCorner( Qt::BottomRightCorner, Qt::BottomDockWidgetArea );

    QList<QDockWidget*> dockWidgets = findChildren<QDockWidget*>();
    for ( QDockWidget* dock : dockWidgets )
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
        std::vector<caf::PdmFieldHandle*> toolBarFields = { plotWindow->pagePreviewField(),
                                                            plotWindow->columnCountField(),
                                                            plotWindow->rowsPerPageField() };
        m_multiPlotToolBarEditor->setFields( toolBarFields );
        m_multiPlotToolBarEditor->updateUi();
        m_multiPlotToolBarEditor->show();
    }
    else
    {
        m_multiPlotToolBarEditor->clear();
        m_multiPlotToolBarEditor->hide();
    }
    refreshToolbars();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotMainWindow::updateSummaryPlotToolBar( bool forceUpdateUi )
{
    RimSummaryPlot* summaryPlot = dynamic_cast<RimSummaryPlot*>( m_activePlotViewWindow.p() );
    RimMultiPlot*   multiPlot   = dynamic_cast<RimMultiPlot*>( m_activePlotViewWindow.p() );
    if ( multiPlot )
    {
        summaryPlot = caf::SelectionManager::instance()->selectedItemOfType<RimSummaryPlot>();
    }

    if ( summaryPlot )
    {
        std::vector<caf::PdmFieldHandle*> toolBarFields = summaryPlot->fieldsToShowInToolbar();

        QString keyword;

        if ( !m_summaryPlotToolBarEditor->isEditorDataValid( toolBarFields ) )
        {
            keyword = m_summaryPlotToolBarEditor->keywordForFocusWidget();

            m_summaryPlotToolBarEditor->setFields( toolBarFields );
        }

        m_summaryPlotToolBarEditor->updateUi( caf::PdmUiToolBarEditor::uiEditorConfigName() );
        m_summaryPlotToolBarEditor->show();
        m_summaryPlotToolBarEditor->setFocusWidgetFromKeyword( keyword );
    }
    else
    {
        m_summaryPlotToolBarEditor->clear();
        m_summaryPlotToolBarEditor->hide();
    }

    refreshToolbars();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotMainWindow::setFocusToLineEditInSummaryToolBar()
{
    if ( m_summaryPlotToolBarEditor )
    {
        m_summaryPlotToolBarEditor->setFocusWidgetFromKeyword(
            RimSummaryPlotFilterTextCurveSetEditor::curveFilterFieldKeyword() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicSummaryPlotEditorDialog* RiuPlotMainWindow::summaryCurveCreatorDialog()
{
    if ( m_summaryCurveCreatorDialog.isNull() )
    {
        m_summaryCurveCreatorDialog = new RicSummaryPlotEditorDialog( this );
    }

    return m_summaryCurveCreatorDialog;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicSummaryCurveCalculatorDialog* RiuPlotMainWindow::summaryCurveCalculatorDialog()
{
    if ( m_summaryCurveCalculatorDialog.isNull() )
    {
        m_summaryCurveCalculatorDialog = new RicSummaryCurveCalculatorDialog( this );
    }

    return m_summaryCurveCalculatorDialog;
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
/// This method needs to handle memory deallocation !!!
//--------------------------------------------------------------------------------------------------
void RiuPlotMainWindow::setPdmRoot( caf::PdmObject* pdmRoot )
{
    m_projectTreeView->setPdmItem( pdmRoot );
    // For debug only : m_projectTreeView->treeView()->expandAll();
    m_projectTreeView->setDragDropInterface( m_dragDropInterface.get() );
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
        std::vector<caf::PdmUiItem*> currentSelection;
        m_projectTreeView->selectedUiItems( currentSelection );
        bool childSelected = false;
        for ( caf::PdmUiItem* uiItem : currentSelection )
        {
            caf::PdmObject* pdmObject = dynamic_cast<caf::PdmObject*>( uiItem );
            if ( pdmObject )
            {
                std::vector<RimViewWindow*> ancestralViews;
                pdmObject->allAncestorsOrThisOfType( ancestralViews );
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

    updateWellLogPlotToolBar();
    updateSummaryPlotToolBar();
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

    {
        caf::CmdFeatureManager* cmdFeatureMgr = caf::CmdFeatureManager::instance();
        m_windowMenu->addAction( cmdFeatureMgr->action( "RicShowMainWindowFeature" ) );
        m_windowMenu->addSeparator();
    }

    QList<QDockWidget*> dockWidgets = findChildren<QDockWidget*>();
    for ( QDockWidget* dock : dockWidgets )
    {
        m_windowMenu->addAction( dock->toggleViewAction() );
    }

    m_windowMenu->addSeparator();
    QAction* cascadeWindowsAction = new QAction( "Cascade Windows", this );
    connect( cascadeWindowsAction, SIGNAL( triggered() ), m_mdiArea, SLOT( cascadeSubWindows() ) );

    QAction* closeAllSubWindowsAction = new QAction( "Close All Windows", this );
    connect( closeAllSubWindowsAction, SIGNAL( triggered() ), m_mdiArea, SLOT( closeAllSubWindows() ) );

    m_windowMenu->addAction( caf::CmdFeatureManager::instance()->action( "RicTilePlotWindowsFeature" ) );
    m_windowMenu->addAction( cascadeWindowsAction );
    m_windowMenu->addAction( closeAllSubWindowsAction );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotMainWindow::selectedObjectsChanged()
{
    std::vector<caf::PdmUiItem*> uiItems;
    m_projectTreeView->selectedUiItems( uiItems );

    caf::PdmObjectHandle* firstSelectedObject = nullptr;

    if ( uiItems.size() == 1 )
    {
        firstSelectedObject = dynamic_cast<caf::PdmObjectHandle*>( uiItems[0] );
    }

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

        RimViewWindow* selectedWindow = dynamic_cast<RimViewWindow*>( firstSelectedObject );
        if ( !selectedWindow )
        {
            firstSelectedObject->firstAncestorOrThisOfType( selectedWindow );
        }

        // If we cant find the view window as an MDI sub window, we search higher in the
        // project tree to find a possible parent view window that has.
        if ( selectedWindow && !findMdiSubWindow( selectedWindow->viewWidget() ) )
        {
            if ( selectedWindow->parentField() && selectedWindow->parentField()->ownerObject() )
            {
                selectedWindow->parentField()->ownerObject()->firstAncestorOrThisOfType( selectedWindow );
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
                RimSummaryPlot* summaryPlot = nullptr;
                firstSelectedObject->firstAncestorOrThisOfType( summaryPlot );
                if ( summaryPlot )
                {
                    updateSummaryPlotToolBar();
                }
            }

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
void RiuPlotMainWindow::restoreTreeViewState()
{
    if ( m_projectTreeView )
    {
        QString stateString = RimProject::current()->plotWindowTreeViewState;
        if ( !stateString.isEmpty() )
        {
            m_projectTreeView->treeView()->collapseAll();
            caf::QTreeViewStateSerializer::applyTreeViewStateFromString( m_projectTreeView->treeView(), stateString );
        }

        QString currentIndexString = RimProject::current()->plotWindowCurrentModelIndexPath;
        if ( !currentIndexString.isEmpty() )
        {
            QModelIndex mi =
                caf::QTreeViewStateSerializer::getModelIndexFromString( m_projectTreeView->treeView()->model(),
                                                                        currentIndexString );
            m_projectTreeView->treeView()->setCurrentIndex( mi );
        }
    }
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
void RiuPlotMainWindow::tileSubWindows()
{
    QMdiArea::WindowOrder currentActivationOrder = m_mdiArea->activationOrder();

    std::list<QMdiSubWindow*> windowList;
    for ( QMdiSubWindow* subWindow : m_mdiArea->subWindowList( currentActivationOrder ) )
    {
        windowList.push_back( subWindow );
    }

    // Perform stable sort of list so we first sort by window position but retain activation order
    // for windows with the same position.
    windowList.sort( []( const QMdiSubWindow* lhs, const QMdiSubWindow* rhs ) {
        if ( lhs->frameGeometry().topLeft().ry() == rhs->frameGeometry().topLeft().ry() )
        {
            return lhs->frameGeometry().topLeft().rx() < rhs->frameGeometry().topLeft().rx();
        }
        return lhs->frameGeometry().topLeft().ry() < rhs->frameGeometry().topLeft().ry();
    } );

    // Based on workaround described here
    // https://forum.qt.io/topic/50053/qmdiarea-tilesubwindows-always-places-widgets-in-activationhistoryorder-in-subwindowview-mode

    bool prevActivationBlock = isBlockingSubWindowActivatedSignal();
    // Force activation order so they end up in the order of the loop.
    m_mdiArea->setActivationOrder( QMdiArea::ActivationHistoryOrder );
    QMdiSubWindow* a = m_mdiArea->activeSubWindow();

    setBlockSubWindowActivatedSignal( true );
    // Activate in reverse order
    for ( auto it = windowList.rbegin(); it != windowList.rend(); ++it )
    {
        m_mdiArea->setActiveSubWindow( *it );
    }

    m_mdiArea->tileSubWindows();
    // Set back the original activation order to avoid messing with the standard ordering
    m_mdiArea->setActivationOrder( currentActivationOrder );
    m_mdiArea->setActiveSubWindow( a );
    setBlockSubWindowActivatedSignal( prevActivationBlock );

    storeSubWindowTiling( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotMainWindow::storeSubWindowTiling( bool tiled )
{
    RimProject::current()->setSubWindowsTiledInPlotWindow( tiled );
    refreshToolbars();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotMainWindow::clearWindowTiling()
{
    setBlockSubWindowActivatedSignal( true );
    QMdiArea::WindowOrder currentActivationOrder = m_mdiArea->activationOrder();

    for ( QMdiSubWindow* subWindow : m_mdiArea->subWindowList( currentActivationOrder ) )
    {
        subWindow->hide();
        subWindow->showNormal();
    }
    storeSubWindowTiling( false );
    setBlockSubWindowActivatedSignal( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuPlotMainWindow::subWindowsAreTiled() const
{
    if ( RimProject::current() )
    {
        return RimProject::current()->subWindowsTiledPlotWindow();
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuPlotMainWindow::isAnyMdiSubWindowVisible()
{
    return m_mdiArea->subWindowList().size() > 0;
}
