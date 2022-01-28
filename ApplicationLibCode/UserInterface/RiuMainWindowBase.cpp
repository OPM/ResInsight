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
#include "RiaVersionInfo.h"

#include "RiuDockWidgetTools.h"
#include "RiuDragDrop.h"
#include "RiuMdiSubWindow.h"

#include "RimProject.h"
#include "RimViewWindow.h"

#include "cafCmdFeatureManager.h"
#include "cafPdmObject.h"
#include "cafPdmUiTreeView.h"
#include "cafQTreeViewStateSerializer.h"

#include <QAction>
#include <QDockWidget>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QSettings>
#include <QTreeView>
#include <QUndoStack>
#include <QUndoView>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuMainWindowBase::RiuMainWindowBase()
    : m_allowActiveViewChangeFromSelection( true )
    , m_showFirstVisibleWindowMaximized( true )
    , m_blockSubWindowActivation( false )
    , m_blockSubWindowProjectTreeSelection( false )
{
    setDockNestingEnabled( true );

    if ( RiaPreferences::current()->useUndoRedo() && RiaApplication::enableDevelopmentFeatures() )
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
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QMdiSubWindow* RiuMainWindowBase::createViewWindow()
{
    RiuMdiSubWindow* subWin =
        new RiuMdiSubWindow( nullptr, Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint | Qt::WindowMaximizeButtonHint );
    subWin->setAttribute( Qt::WA_DeleteOnClose ); // Make sure the contained widget is destroyed when the MDI window
                                                  // is closed

    return subWin;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMdiWindowGeometry RiuMainWindowBase::windowGeometryForViewer( QWidget* viewer )
{
    RiuMdiSubWindow* mdiWindow = dynamic_cast<RiuMdiSubWindow*>( findMdiSubWindow( viewer ) );
    if ( mdiWindow )
    {
        return mdiWindow->windowGeometry();
    }

    RimMdiWindowGeometry geo;
    return geo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindowBase::loadWinGeoAndDockToolBarLayout()
{
    // Company and appname set through QCoreApplication
    QSettings settings;

    QVariant winGeo = settings.value( QString( "%1/winGeometry" ).arg( registryFolderName() ) );
    QVariant layout = settings.value( QString( "%1/dockAndToolBarLayout" ).arg( registryFolderName() ) );

    if ( winGeo.isValid() )
    {
        if ( restoreGeometry( winGeo.toByteArray() ) )
        {
            if ( layout.isValid() )
            {
                restoreState( layout.toByteArray(), 0 );
            }
        }
    }

    restoreDockWidgetVisibilities();
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
    // Company and appname set through QCoreApplication
    QSettings settings;

    QByteArray winGeo = saveGeometry();
    settings.setValue( QString( "%1/winGeometry" ).arg( registryFolderName() ), winGeo );

    QByteArray layout = saveState( 0 );
    settings.setValue( QString( "%1/dockAndToolBarLayout" ).arg( registryFolderName() ), layout );

    settings.setValue( QString( "%1/isMaximized" ).arg( registryFolderName() ), isMaximized() );

    if ( this->isVisible() )
    {
        QVariant dockWindowVisibilities = RiuDockWidgetTools::dockWidgetsVisibility( this );
        QString  key                    = mainWindowDockWidgetSettingsKey( registryFolderName() );

        settings.setValue( key, dockWindowVisibilities );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindowBase::storeDefaultDockWidgetVisibilitiesIfRequired()
{
    QSettings settings;

    QString key = mainWindowDockWidgetSettingsKey( registryFolderName() );

    if ( !settings.contains( key ) )
    {
        QVariant dockWidgetVisibilities = RiuDockWidgetTools::defaultDockWidgetVisibilities();
        settings.setValue( key, dockWidgetVisibilities );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindowBase::restoreDockWidgetVisibilities()
{
    // Company and appname set through QCoreApplication
    QSettings settings;

    QString key = mainWindowDockWidgetSettingsKey( registryFolderName() );

    QVariant dockWindowVisibilities = settings.value( key );
    RiuDockWidgetTools::applyDockWidgetVisibilities( this, dockWindowVisibilities.toMap() );
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
void RiuMainWindowBase::hideAllDockWidgets()
{
    QList<QDockWidget*> dockWidgets = findChildren<QDockWidget*>();

    for ( QDockWidget* dock : dockWidgets )
    {
        if ( dock )
        {
            dock->hide();
        }
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
    if ( tv ) tv->selectAsCurrentItem( object );

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
void RiuMainWindowBase::enableShowFirstVisibleMdiWindowMaximized( bool enable )
{
    m_showFirstVisibleWindowMaximized = enable;
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
void RiuMainWindowBase::removeViewerFromMdiArea( QMdiArea* mdiArea, QWidget* viewer )
{
    bool wasMaximized = viewer && viewer->isMaximized();

    bool removedSubWindowWasActive = false;

    QMdiSubWindow* subWindowBeingClosed = findMdiSubWindow( viewer );
    if ( subWindowBeingClosed )
    {
        if ( subWindowBeingClosed->isActiveWindow() )
        {
            // If we are removing the active window, we will need a new active window
            // Start by making the window inactive so Qt doesn't pick the active window itself
            mdiArea->setActiveSubWindow( nullptr );
            removedSubWindowWasActive = true;
        }
        mdiArea->removeSubWindow( subWindowBeingClosed );

        // These two lines had to be introduced after themes was used
        // Probably related to polish/unpolish of widgets in an MDI setting
        // https://github.com/OPM/ResInsight/issues/6676
        subWindowBeingClosed->hide();
        subWindowBeingClosed->deleteLater();
    }

    QList<QMdiSubWindow*> subWindowList = mdiArea->subWindowList( QMdiArea::ActivationHistoryOrder );
    if ( !subWindowList.empty() )
    {
        if ( removedSubWindowWasActive )
        {
            mdiArea->setActiveSubWindow( nullptr );
            // Make the last activated window the current activated one
            mdiArea->setActiveSubWindow( subWindowList.back() );
        }
        if ( wasMaximized && mdiArea->currentSubWindow() )
        {
            mdiArea->currentSubWindow()->showMaximized();
        }
        else if ( subWindowsAreTiled() )
        {
            tileSubWindows();
        }
    }
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

    auto dockWidget = dynamic_cast<QDockWidget*>( sender()->parent() );
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
//--------------------------------------------------------------------------------------------------
void RiuMainWindowBase::initializeSubWindow( QMdiArea*      mdiArea,
                                             QMdiSubWindow* mdiSubWindow,
                                             const QPoint&  subWindowPos,
                                             const QSize&   subWindowSize )
{
    bool initialStateTiled     = subWindowsAreTiled();
    bool initialStateMaximized = false;

    if ( m_showFirstVisibleWindowMaximized && mdiArea->subWindowList().empty() )
    {
        // Show first 3D view maximized
        initialStateMaximized = true;
    }

    if ( mdiArea->currentSubWindow() && mdiArea->currentSubWindow()->isMaximized() )
    {
        initialStateMaximized = true;
    }

    mdiArea->addSubWindow( mdiSubWindow );

    if ( subWindowPos.x() > -1 )
    {
        mdiSubWindow->move( subWindowPos );
    }
    mdiSubWindow->resize( subWindowSize );

    if ( initialStateMaximized )
    {
        mdiSubWindow->showMaximized();
    }
    else
    {
        mdiSubWindow->showNormal();
        if ( initialStateTiled )
        {
            tileSubWindows();
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
            QModelIndex mi =
                caf::QTreeViewStateSerializer::getModelIndexFromString( tv->treeView()->model(), currentIndexString );
            tv->treeView()->setCurrentIndex( mi );
        }
    }
}
