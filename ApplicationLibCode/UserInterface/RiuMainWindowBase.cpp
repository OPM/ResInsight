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

#include "RiuMainWindowBase.h"

#include "RiaApplication.h"
#include "RiaDefines.h"
#include "RiaPreferences.h"
#include "RiaPreferencesSystem.h"
#include "RiaRegressionTestRunner.h"
#include "RiaVersionInfo.h"

#include "RimProject.h"
#include "RimViewWindow.h"

#include "RiuDockWidgetTools.h"
#include "RiuDragDrop.h"
#include "RiuGuiTheme.h"
#include "RiuInterfaceToViewWindow.h"
#include "RiuMainWindowTools.h"
#include "RiuViewer.h"

#include "cafCmdFeatureManager.h"
#include "cafPdmObject.h"
#include "cafPdmUiTreeView.h"
#include "cafQTreeViewStateSerializer.h"

#include "DockAreaTitleBar.h"
#include "DockAreaWidget.h"
#include "DockManager.h"
#include "DockWidget.h"

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QInputDialog>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QSettings>
#include <QTextEdit>
#include <QTreeView>
#include <QUndoStack>
#include <QUndoView>
#include <QUuid>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuMainWindowBase::RiuMainWindowBase()
    : m_allowActiveViewChangeFromSelection( true )
    , m_blockSubWindowActivation( false )
    , m_blockSubWindowProjectTreeSelection( false )
    , m_hasBeenVisible( false )
    , m_windowMenu( nullptr )
{
    ads::CDockManager::setAutoHideConfigFlags( ads::CDockManager::DefaultAutoHideConfig );
    m_dockManager = new ads::CDockManager( this );
    m_dockManager->setStyleSheet( "" );

    if ( RiaPreferences::current()->useUndoRedo() && RiaPreferencesSystem::current()->isFeatureEnabled( "undo-redo-view" ) )
    {
        m_undoView = new QUndoView( this );
    }
    else
    {
        m_undoView = nullptr;
    }

    m_undoAction = new QAction( QIcon( ":/undo.png" ), tr( "Undo" ), this );
    m_undoAction->setShortcut( QKeySequence::Undo );
    connect( m_undoAction, SIGNAL( triggered() ), SLOT( slotUndo() ) );

    m_redoAction = new QAction( QIcon( ":/redo.png" ), tr( "Redo" ), this );
    m_redoAction->setShortcut( QKeySequence::Redo );
    connect( m_redoAction, SIGNAL( triggered() ), SLOT( slotRedo() ) );

    m_hideTabsAction = new QAction( QIcon( ":/HideTabs.svg" ), "Show/Hide Tabs", this );
    m_hideTabsAction->setToolTip( "Show/Hide Tabs in Main Views" );
    m_hideTabsAction->setCheckable( true );
    connect( m_hideTabsAction, SIGNAL( toggled( bool ) ), SLOT( slotHideTabs( bool ) ) );

#ifdef Q_OS_WIN
    if ( RiaPreferences::current()->guiTheme() == RiaDefines::ThemeEnum::DARK )
    {
        RiuMainWindowTools::setDarkTitleBarWindows( this );
    }
#endif
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuMainWindowBase::~RiuMainWindowBase()
{
    for ( auto v : m_projectTreeViews )
    {
        v->setPdmItem( nullptr );
        delete v;
    }

    m_projectTreeViews.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ads::CDockManager* RiuMainWindowBase::dockManager() const
{
    return m_dockManager;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuMainWindowBase::dockWidgetStateString() const
{
    return QString::fromLatin1( m_dockManager->saveState( DOCKSTATE_VERSION ).toBase64() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuMainWindowBase::restoreDockWidgetState( QString dockStateString )
{
    if ( dockStateString.isEmpty() ) return false;
    QByteArray dockState = QByteArray::fromBase64( dockStateString.toLatin1() );
    return m_dockManager->restoreState( dockState, DOCKSTATE_VERSION );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuMainWindowBase::restoreLastDockWidgetState()
{
    if ( m_lastDockState.isEmpty() ) return false;
    return m_dockManager->restoreState( m_lastDockState, DOCKSTATE_VERSION );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindowBase::setActiveViewer( QString viewerName )
{
    for ( auto view : viewWindows() )
    {
        if ( !view->dockWidget() ) continue;
        auto dockName = view->dockWidget()->objectName();
        if ( view->dockWidget() && view->dockWidget()->objectName() == viewerName )
        {
            if ( !view->isActiveViewer() )
            {
                view->setAsActiveViewer( !isBlockingViewSelectionOnSubWindowActivated() );
            }
            if ( !view->dockWidget()->isCurrentTab() )
            {
                view->dockWidget()->setAsCurrentTab();
            }

            break;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindowBase::loadWinGeoAndDockToolBarLayout()
{
    // Company and appname set through QCoreApplication
    QSettings settings;

    QVariant winGeo        = settings.value( QString( "%1/winGeometry" ).arg( registryFolderName() ) );
    QVariant toolbarLayout = settings.value( QString( "%1/toolBarLayout" ).arg( registryFolderName() ) );

    if ( winGeo.isValid() )
    {
        if ( restoreGeometry( winGeo.toByteArray() ) )
        {
            if ( toolbarLayout.isValid() )
            {
                restoreState( toolbarLayout.toByteArray(), 1 );
            }
        }
    }

    if ( !RiaRegressionTestRunner::instance()->isRunningRegressionTests() )
    {
        // Performance of m_dockManager->restoreState() is very bad is degrading as more and more regression tests are
        // launched. Disable restore of state for regression test.

        bool     dockingOk = false;
        QVariant dockState = settings.value( QString( "%1/dockLayout" ).arg( registryFolderName() ) );

        if ( dockState.isValid() )
        {
            dockingOk = m_dockManager->restoreState( dockState.toByteArray(), DOCKSTATE_VERSION );
        }

        if ( !dockingOk )
        {
            m_dockManager->restoreState( RiuDockWidgetTools::defaultDockState( defaultDockStateNames()[0] ), DOCKSTATE_VERSION );
        }
    }

    settings.beginGroup( registryFolderName() );
    m_dockManager->loadPerspectives( settings );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString mainWindowDockWidgetSettingsKey( const QString& settingsFolderName )
{
    QString key = settingsFolderName + "/dockWindowVisibilies";

    return key;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindowBase::saveWinGeoAndDockToolBarLayout()
{
    // Do not save the state if the window never has been visible. This will write out a state that is not valid.
    // https://github.com/OPM/ResInsight/issues/9935
    if ( !m_hasBeenVisible ) return;

    // Company and appname set through QCoreApplication
    QSettings settings;

    QByteArray winGeo = saveGeometry();
    settings.setValue( QString( "%1/winGeometry" ).arg( registryFolderName() ), winGeo );

    QByteArray layout = saveState( 1 );
    settings.setValue( QString( "%1/toolBarLayout" ).arg( registryFolderName() ), layout );

    settings.setValue( QString( "%1/isMaximized" ).arg( registryFolderName() ), isMaximized() );

    settings.setValue( QString( "%1/dockLayout" ).arg( registryFolderName() ), m_dockManager->saveState( DOCKSTATE_VERSION ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindowBase::showWindow()
{
    // Company and appname set through QCoreApplication
    QSettings settings;

    showNormal();

    QVariant isMax = settings.value( QString( "%1/isMaximized" ).arg( registryFolderName() ), false );
    if ( isMax.toBool() )
    {
        showMaximized();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuMainWindowBase::registryFolderName()
{
    QString versionName( STRPRODUCTVER );
    QString regFolder = QString( "%1_Qt%2/%3" ).arg( versionName ).arg( QT_VERSION_STR ).arg( mainWindowName() );
    return regFolder;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindowBase::selectAsCurrentItem( const caf::PdmObject* object, bool allowActiveViewChange )
{
    m_allowActiveViewChangeFromSelection = allowActiveViewChange;

    auto tv = getTreeViewWithItem( object );
    if ( tv )
    {
        tv->selectAsCurrentItem( object );
        if ( auto dw = m_dockManager->findDockWidget( tv->objectName() ) )
        {
            dw->setAsCurrentTab();
        }
    }

    m_allowActiveViewChangeFromSelection = true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindowBase::toggleItemInSelection( const caf::PdmObject* object, bool allowActiveViewChange )
{
    auto tv = getTreeViewWithItem( object );
    if ( !tv ) return;

    m_allowActiveViewChangeFromSelection = allowActiveViewChange;
    std::vector<caf::PdmUiItem*> currentSelection;
    tv->selectedUiItems( currentSelection );
    std::vector<const caf::PdmUiItem*> updatedSelection;

    bool alreadySelected = false;
    for ( caf::PdmUiItem* uiItem : currentSelection )
    {
        if ( object == uiItem )
        {
            alreadySelected = true;
        }
        else
        {
            updatedSelection.push_back( uiItem );
        }
    }
    if ( !alreadySelected )
    {
        updatedSelection.push_back( object );
    }
    tv->selectItems( updatedSelection );
    m_allowActiveViewChangeFromSelection = true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindowBase::setBlockSubWindowActivatedSignal( bool block )
{
    m_blockSubWindowActivation = block;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuMainWindowBase::isBlockingSubWindowActivatedSignal() const
{
    return m_blockSubWindowActivation;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindowBase::setBlockViewSelectionOnSubWindowActivated( bool block )
{
    m_blockSubWindowProjectTreeSelection = block;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuMainWindowBase::isBlockingViewSelectionOnSubWindowActivated() const
{
    return m_blockSubWindowProjectTreeSelection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindowBase::setExpanded( const caf::PdmUiItem* uiItem, bool expanded )
{
    caf::PdmUiTreeView* tv = getTreeViewWithItem( uiItem );
    if ( tv ) tv->setExpanded( uiItem, expanded );
}

//--------------------------------------------------------------------------------------------------
///
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindowBase::slotDockWidgetToggleViewActionTriggered()
{
    if ( !sender() ) return;

    auto dockWidget = dynamic_cast<ads::CDockWidget*>( sender()->parent() );
    if ( dockWidget )
    {
        if ( dockWidget->isVisible() )
        {
            // Raise the dock widget to make it visible if the widget is part of a tab widget
            dockWidget->raise();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindowBase::slotCentralWidgetContextMenu( const QPoint& pos )
{
    QMenu menu( this );
    onCentralWidgetContextMenu( menu );
    if ( !menu.isEmpty() && m_centralDockWidget->widget() != nullptr )
    {
        menu.exec( m_centralDockWidget->widget()->mapToGlobal( pos ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindowBase::slotDockViewerVisibilityChanged( bool visible )
{
    if ( !visible ) return;

    if ( auto dockWidget = dynamic_cast<ads::CDockWidget*>( sender() ) )
    {
        for ( auto view : viewWindows() )
        {
            if ( view->dockWidget() == dockWidget )
            {
                view->setAsActiveViewer();
                if ( !isBlockingViewSelectionOnSubWindowActivated() )
                {
                    selectAsCurrentItem( view );
                }
                break;
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindowBase::slotDockViewerClosed()
{
    if ( auto dockWidget = dynamic_cast<ads::CDockWidget*>( sender() ) )
    {
        auto           mainWidget = dockWidget->widget();
        RimViewWindow* viewWindow = RiuInterfaceToViewWindow::viewWindowFromWidget( mainWidget );
        if ( !viewWindow )
        {
            RiuViewer* viewer = mainWidget->findChild<RiuViewer*>();
            if ( viewer )
            {
                viewWindow = viewer->ownerViewWindow();
            }
        }

        if ( viewWindow )
        {
            viewWindow->setShowWindow( false );
            viewWindow->removeWindowFromDock();
            viewWindow->updateConnectedEditors();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindowBase::slotRefreshHelpActions()
{
    caf::CmdFeatureManager::instance()->action( "RicSearchHelpFeature" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindowBase::slotRedo()
{
    if ( m_undoView && m_undoView->stack() && m_undoView->stack()->canRedo() )
    {
        m_undoView->stack()->redo();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindowBase::slotUndo()
{
    if ( m_undoView && m_undoView->stack() && m_undoView->stack()->canUndo() )
    {
        m_undoView->stack()->undo();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindowBase::slotRefreshUndoRedoActions()
{
    if ( !m_undoView ) return;

    m_redoAction->setDisabled( !m_undoView->stack()->canRedo() );
    m_undoAction->setDisabled( !m_undoView->stack()->canUndo() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindowBase::createTreeViews( int numberOfTrees )
{
    CVF_ASSERT( m_projectTreeViews.empty() );

    for ( int i = 0; i < numberOfTrees; i++ )
    {
        auto                         tv                = new caf::PdmUiTreeView();
        caf::PdmUiDragDropInterface* dragDropInterface = new RiuDragDrop( tv );
        tv->setDragDropInterface( dragDropInterface );
        m_projectTreeViews.push_back( tv );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindowBase::setUpCentralDockWidget()
{
    m_centralDockWidget = RiuDockWidgetTools::createDockWidget( "", RiuDockWidgetTools::centralScreenName(), this );
    QLabel* label       = new QLabel();
    label->setAutoFillBackground( true );
    label->setStyleSheet( "QLabel { background-color: darkgrey; }" );
    label->setContextMenuPolicy( Qt::CustomContextMenu );
    connect( label, SIGNAL( customContextMenuRequested( const QPoint& ) ), SLOT( slotCentralWidgetContextMenu( const QPoint& ) ) );
    m_centralDockWidget->setWidget( label );
    m_centralDockWidget->setFeature( ads::CDockWidget::NoTab, true );
    dockManager()->setCentralWidget( m_centralDockWidget );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmUiTreeView* RiuMainWindowBase::projectTreeView( int treeId )
{
    CVF_ASSERT( treeId >= 0 );
    CVF_ASSERT( treeId < (int)m_projectTreeViews.size() );

    return m_projectTreeViews[treeId];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmUiTreeView* RiuMainWindowBase::getTreeViewWithItem( const caf::PdmUiItem* uiItem )
{
    for ( auto tv : m_projectTreeViews )
    {
        QModelIndex qmi = tv->findModelIndex( uiItem );
        if ( qmi.isValid() )
        {
            return tv;
        }
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<caf::PdmUiTreeView*> RiuMainWindowBase::projectTreeViews()
{
    return m_projectTreeViews;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindowBase::restoreTreeViewStates( QString treeStateString, QString treeIndexeString )
{
    QStringList treeStates  = treeStateString.split( RiaDefines::stringListSeparator() );
    QStringList treeIndexes = treeIndexeString.split( RiaDefines::stringListSeparator() );

    const int nTreeViews = (int)projectTreeViews().size();

    if ( treeStates.size() < nTreeViews ) return;
    if ( treeIndexes.size() < nTreeViews ) return;

    for ( int treeId = 0; treeId < nTreeViews; treeId++ )
    {
        auto tv = projectTreeView( treeId );

        QString stateString = treeStates[treeId];
        if ( !stateString.isEmpty() )
        {
            tv->treeView()->collapseAll();
            caf::QTreeViewStateSerializer::applyTreeViewStateFromString( tv->treeView(), stateString );
        }

        QString currentIndexString = treeIndexes[treeId];
        if ( !currentIndexString.isEmpty() )
        {
            QModelIndex mi = caf::QTreeViewStateSerializer::getModelIndexFromString( tv->treeView()->model(), currentIndexString );
            tv->treeView()->setCurrentIndex( mi );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ads::CDockAreaWidget* RiuMainWindowBase::addTabbedWidgets( std::vector<ads::CDockWidget*> widgets,
                                                           ads::DockWidgetArea            whereToDock,
                                                           ads::CDockAreaWidget*          dockInside )
{
    ads::CDockAreaWidget* areaToDockIn = nullptr;

    for ( auto widget : widgets )
    {
        if ( areaToDockIn )
        {
            m_dockManager->addDockWidgetTabToArea( widget, areaToDockIn );
        }
        else
        {
            areaToDockIn = m_dockManager->addDockWidget( whereToDock, widget, dockInside );
        }
    }
    return areaToDockIn;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindowBase::setDefaultDockLayout()
{
    QAction* action = dynamic_cast<QAction*>( sender() );
    if ( action )
    {
        QString layoutName = action->text();
        RiuDockWidgetTools::setDockLayout( this, layoutName );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindowBase::setDockLayout()
{
    QAction* action = dynamic_cast<QAction*>( sender() );
    if ( action )
    {
        QString layoutName = action->text();
        RiuDockWidgetTools::setDockLayout( this, layoutName );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindowBase::deleteDockLayout()
{
    QAction* action = dynamic_cast<QAction*>( sender() );
    if ( action )
    {
        QString name = action->text();

        QString questionStr = "Are you sure you want to delete the window layout \"" + name + "\"?";
        auto reply = QMessageBox::question( this, "Delete Window Layout", questionStr, QMessageBox::Yes | QMessageBox::No, QMessageBox::No );
        if ( reply == QMessageBox::Yes )
        {
            dockManager()->removePerspective( name );
            QSettings settings;
            settings.beginGroup( registryFolderName() );
            dockManager()->savePerspectives( settings );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindowBase::exportDockLayout()
{
    QClipboard* clipboard = QApplication::clipboard();
    if ( clipboard )
    {
        QByteArray state = dockManager()->saveState( DOCKSTATE_VERSION );

        QString exportStr;
        int     i = 0;

        exportStr = "static const char stateData[] = {\n";

        for ( unsigned char c : state )
        {
            if ( i > 0 )
            {
                if ( i % 25 == 0 )
                    exportStr += ",\n";
                else
                    exportStr += ", ";
            }
            exportStr += QString( "'\\x%1'" ).arg( c, 2, 16, QChar( '0' ) );
            i++;
        }
        exportStr += "\n};\n";

        clipboard->setText( exportStr );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindowBase::saveDockLayout()
{
    bool    ok   = false;
    QString name = QInputDialog::getText( this, "Save Window Layout", "Give the window layout a name:", QLineEdit::Normal, "", &ok );
    if ( ok && !name.isEmpty() )
    {
        dockManager()->addPerspective( name );
        QSettings settings;
        settings.beginGroup( registryFolderName() );
        dockManager()->savePerspectives( settings );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindowBase::addDefaultEntriesToWindowsMenu()
{
    QMenu* dockWindowsMenu = m_windowMenu->addMenu( QIcon( ":/window-management.svg" ), "Windows" );

    auto dockMap = dockManager()->dockWidgetsMap();
    auto keys    = dockMap.keys();
    keys.sort();
    for ( auto& key : keys )
    {
        if ( key == RiuDockWidgetTools::centralScreenName() ) continue;
        auto dock = dockMap[key];
        dockWindowsMenu->addAction( dock->toggleViewAction() );
    }
    m_windowMenu->addSeparator();

    QAction* saveLayoutAction = m_windowMenu->addAction( "Save Window Layout..." );
    connect( saveLayoutAction, SIGNAL( triggered() ), this, SLOT( saveDockLayout() ) );

    QStringList defaultNames = defaultDockStateNames();
    QStringList names        = dockManager()->perspectiveNames();

    if ( defaultNames.size() + names.size() > 0 )
    {
        QMenu* layoutsMenu      = m_windowMenu->addMenu( "Use Window Layout" );
        QMenu* deleteLayoutMenu = nullptr;
        if ( !names.empty() ) deleteLayoutMenu = m_windowMenu->addMenu( "Delete Window Layout" );

        for ( auto& defLayout : defaultNames )
        {
            QAction* defLayoutAction = layoutsMenu->addAction( defLayout );
            connect( defLayoutAction, SIGNAL( triggered() ), this, SLOT( setDefaultDockLayout() ) );
        }

        if ( !defaultNames.empty() ) layoutsMenu->addSeparator();

        for ( auto& layout : names )
        {
            QAction* chooseLayoutAction = layoutsMenu->addAction( layout );
            connect( chooseLayoutAction, SIGNAL( triggered() ), this, SLOT( setDockLayout() ) );
            QAction* deleteLayoutAction = deleteLayoutMenu->addAction( layout );
            connect( deleteLayoutAction, SIGNAL( triggered() ), this, SLOT( deleteDockLayout() ) );
        }
    }

    if ( RiaApplication::enableDevelopmentFeatures() )
    {
        QAction* exportLayoutAction = m_windowMenu->addAction( "Export Layout to Clipboard" );
        connect( exportLayoutAction, SIGNAL( triggered() ), this, SLOT( exportDockLayout() ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindowBase::showEvent( QShowEvent* event )
{
    m_hasBeenVisible = true;

    QMainWindow::showEvent( event );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindowBase::closeEvent( QCloseEvent* event )
{
    saveWinGeoAndDockToolBarLayout();

    for ( auto view : viewWindows() )
    {
        view->removeWindowFromDock();
    }

    QMainWindow::closeEvent( event );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindowBase::slotHideTabs( bool hideTabs )
{
    std::set<ads::CDockAreaWidget*> areasToToggle;
    std::set<ads::CDockAreaWidget*> areasToNotToggle;

    for ( auto [name, widget] : dockManager()->dockWidgetsMap().asKeyValueRange() )
    {
        // ignore the dummy central widget
        if ( name == RiuDockWidgetTools::centralScreenName() ) continue;

        // is the name a valid uuid, if so, it is a regular viewer
        QUuid dockId = QUuid( name );
        if ( dockId.isNull() )
        {
            // should not toggle tabs for other docked windows
            areasToNotToggle.insert( widget->dockAreaWidget() );
            if ( areasToToggle.contains( widget->dockAreaWidget() ) )
            {
                areasToToggle.erase( widget->dockAreaWidget() );
            }
        }
        else
        {
            if ( !areasToNotToggle.contains( widget->dockAreaWidget() ) )
            {
                areasToToggle.insert( widget->dockAreaWidget() );
            }
        }
    }

    // toggle visibility for tab widgets in viewer-only areas
    for ( auto area : areasToToggle )
    {
        if ( !area || !area->titleBar() ) continue;
        if ( hideTabs )
        {
            area->titleBar()->hide();
        }
        else
        {
            area->titleBar()->show();
        }
    }
}
