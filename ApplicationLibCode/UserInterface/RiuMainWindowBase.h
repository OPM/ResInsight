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

#pragma once

#include <QByteArray>
#include <QMainWindow>
#include <QPointer>
#include <QString>
#include <QStringList>

#include "cafPdmUiDragDropInterface.h"

#include "ads_globals.h"

#include <memory>
#include <vector>

namespace ads
{
class CDockManager;
class CDockAreaWidget;
class CDockWidget;
} // namespace ads

namespace caf
{
class PdmObject;
class PdmUiTreeView;
class PdmUiItem;
class PdmUiPropertyView;
} // namespace caf

class QAction;
class QUndoView;

class RimViewWindow;

//==================================================================================================
///
//==================================================================================================
class RiuMainWindowBase : public QMainWindow
{
    Q_OBJECT

public:
    static const int DOCKSTATE_VERSION = 4;

    RiuMainWindowBase();
    ~RiuMainWindowBase() override;

    virtual QString mainWindowName() = 0;

    virtual void           initializeViewer( ads::CDockWidget* dockWidget, QWidget* viewer ) {};
    virtual void           setActiveViewer( QString viewerName );
    virtual void           onViewerRemoved() = 0;
    virtual RimViewWindow* activeViewer()    = 0;

    virtual std::vector<RimViewWindow*> viewWindows() = 0;

    void loadWinGeoAndDockToolBarLayout();
    void saveWinGeoAndDockToolBarLayout();
    void showWindow();

    std::vector<caf::PdmUiTreeView*> projectTreeViews();
    caf::PdmUiTreeView*              projectTreeView( int treeId );
    caf::PdmUiTreeView*              getTreeViewWithItem( const caf::PdmUiItem* item );

    void setExpanded( const caf::PdmUiItem* uiItem, bool expanded = true );

    void selectAsCurrentItem( const caf::PdmObject* object, bool allowActiveViewChange = true );
    void toggleItemInSelection( const caf::PdmObject* object, bool allowActiveViewChange = true );

    void setBlockSubWindowActivatedSignal( bool block );
    bool isBlockingSubWindowActivatedSignal() const;

    void setBlockViewSelectionOnSubWindowActivated( bool block );
    bool isBlockingViewSelectionOnSubWindowActivated() const;

    ads::CDockManager* dockManager() const;

    QString dockWidgetStateString() const;
    bool    restoreDockWidgetState( QString dockStateString );
    bool    restoreLastDockWidgetState();

public slots:
    void tileViewWindows();

protected:
    void createTreeViews( int numberOfTrees );
    void setUpCentralDockWidget();

    void restoreTreeViewStates( QString treeStateString, QString treeIndexString );

    ads::CDockAreaWidget* addTabbedWidgets( std::vector<ads::CDockWidget*> widgets,
                                            ads::DockWidgetArea            whereToDock,
                                            ads::CDockAreaWidget*          dockInside = nullptr );

    void addDefaultEntriesToWindowsMenu();

    virtual QStringList defaultDockStateNames() = 0;

    void showEvent( QShowEvent* event ) override;
    void closeEvent( QCloseEvent* event ) override;

    virtual void onCentralWidgetContextMenu( QMenu& menu ) {};

protected slots:
    void slotDockWidgetToggleViewActionTriggered();
    void slotDockViewerVisibilityChanged( bool visible );
    void slotDockViewerClosed();
    void slotRefreshHelpActions();
    void slotHideTabs( bool hideTabs );
    void slotCentralWidgetContextMenu( const QPoint& pos );

    void slotRedo();
    void slotUndo();
    void slotRefreshUndoRedoActions();

    void setDefaultDockLayout();
    void setDockLayout();
    void deleteDockLayout();
    void saveDockLayout();
    void exportDockLayout();

    void maximizeViewWindows();

protected:
    bool m_allowActiveViewChangeFromSelection; // To be used in selectedObjectsChanged() to control
                                               // whether to select the corresponding active view or not

    QAction* m_hideTabsAction;
    QAction* m_tileWindowsAction;
    QAction* m_maximizeWindowsAction;

    QAction*   m_undoAction;
    QAction*   m_redoAction;
    QUndoView* m_undoView;

    QMenu* m_windowMenu;

    QByteArray m_lastDockState;

    std::vector<caf::PdmUiTreeView*>                     m_projectTreeViews;
    std::vector<std::shared_ptr<caf::PdmUiPropertyView>> m_propertyViews;

    QPointer<ads::CDockWidget> m_centralDockWidget;

private:
    QString registryFolderName();

    std::vector<std::unique_ptr<caf::PdmUiDragDropInterface>> m_dragDropInterfaces;

    bool m_blockSubWindowActivation;
    bool m_blockSubWindowProjectTreeSelection;
    bool m_hasBeenVisible;

    ads::CDockManager* m_dockManager;
};
