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

#include <QMainWindow>

#include "cafPdmUiDragDropInterface.h"

#include "DockManager.h"

#include <memory>
#include <vector>

class QMdiArea;
struct RimMdiWindowGeometry;

class RiuMdiArea;

namespace caf
{
class PdmObject;
class PdmUiTreeView;
class PdmUiItem;
} // namespace caf

class QAction;
class QMdiArea;
class QMdiSubWindow;
class QUndoView;

//==================================================================================================
///
//==================================================================================================
class RiuMainWindowBase : public QMainWindow
{
    Q_OBJECT

public:
    RiuMainWindowBase();
    ~RiuMainWindowBase() override;

    virtual QString mainWindowName() = 0;

    QMdiSubWindow* createViewWindow();

    virtual void removeViewer( QWidget* viewer )                                 = 0;
    virtual void initializeViewer( QMdiSubWindow*              viewWindow,
                                   QWidget*                    viewWidget,
                                   const RimMdiWindowGeometry& windowsGeometry ) = 0;
    virtual void setActiveViewer( QWidget* subWindow )                           = 0;

    virtual QMdiSubWindow* findMdiSubWindow( QWidget* viewer ) = 0;

    RimMdiWindowGeometry windowGeometryForViewer( QWidget* viewer );
    void                 loadWinGeoAndDockToolBarLayout();
    void                 saveWinGeoAndDockToolBarLayout();
    void                 showWindow();

    std::vector<caf::PdmUiTreeView*> projectTreeViews();
    caf::PdmUiTreeView*              projectTreeView( int treeId );
    caf::PdmUiTreeView*              getTreeViewWithItem( const caf::PdmUiItem* item );

    void setExpanded( const caf::PdmUiItem* uiItem, bool expanded = true );

    void selectAsCurrentItem( const caf::PdmObject* object, bool allowActiveViewChange = true );
    void toggleItemInSelection( const caf::PdmObject* object, bool allowActiveViewChange = true );

    void enableShowFirstVisibleMdiWindowMaximized( bool enable );

    virtual void tileSubWindows()                   = 0;
    virtual void storeSubWindowTiling( bool tiled ) = 0;
    virtual void clearWindowTiling()                = 0;
    virtual bool subWindowsAreTiled() const         = 0;

    void setBlockSubWindowActivatedSignal( bool block );
    bool isBlockingSubWindowActivatedSignal() const;

    void setBlockViewSelectionOnSubWindowActivated( bool block );
    bool isBlockingViewSelectionOnSubWindowActivated() const;

    ads::CDockManager* dockManager() const;

protected:
    void createTreeViews( int numberOfTrees );
    void removeViewerFromMdiArea( QMdiArea* mdiArea, QWidget* viewer );
    void initializeSubWindow( QMdiArea*      mdiArea,
                              QMdiSubWindow* mdiSubWindow,
                              const QPoint&  subWindowPos,
                              const QSize&   subWindowSize );

    void restoreTreeViewStates( QString treeStateString, QString treeIndexString );

    ads::CDockAreaWidget* addTabbedWidgets( std::vector<ads::CDockWidget*> widgets,
                                            ads::DockWidgetArea            whereToDock,
                                            ads::CDockAreaWidget*          dockInside = nullptr );

    void addDefaultEntriesToWindowsMenu();

    virtual QStringList defaultDockStateNames() = 0;

    virtual QAction* tileSubWindowsAction() = 0;

protected slots:
    void slotDockWidgetToggleViewActionTriggered();
    void slotRefreshHelpActions();

    void slotRedo();
    void slotUndo();
    void slotRefreshUndoRedoActions();

    void setDefaultDockLayout();
    void setDockLayout();
    void deleteDockLayout();
    void saveDockLayout();
    void exportDockLayout();

protected:
    bool m_allowActiveViewChangeFromSelection; // To be used in selectedObjectsChanged() to control
                                               // whether to select the corresponding active view or not

    QAction*   m_undoAction;
    QAction*   m_redoAction;
    QUndoView* m_undoView;

    RiuMdiArea* m_mdiArea;
    QMenu*      m_windowMenu;

private:
    QString registryFolderName();

private:
    std::vector<caf::PdmUiTreeView*>                          m_projectTreeViews;
    std::vector<std::unique_ptr<caf::PdmUiDragDropInterface>> m_dragDropInterfaces;

    bool m_showFirstVisibleWindowMaximized;
    bool m_blockSubWindowActivation;
    bool m_blockSubWindowProjectTreeSelection;

    ads::CDockManager* m_dockManager;
};
