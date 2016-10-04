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

#include "RiuMainWindowBase.h"

#include "cafPdmUiDragDropInterface.h"

#include <QMdiArea>

#include <memory>

class QMdiSubWindow;

class RiuViewer;

struct RimMdiWindowGeometry;

namespace caf
{
     class PdmUiTreeView;
     class PdmObject;
     class PdmUiPropertyView;
     class PdmUiItem;
}

//==================================================================================================
//
// 
//
//==================================================================================================
class RiuMainPlotWindow : public RiuMainWindowBase
{
    Q_OBJECT

public:
    RiuMainPlotWindow();
    
    virtual QString mainWindowName()            { return "RiuMainPlotWindow";  }
    
    void            initializeGuiNewProjectLoaded();
    void            cleanupGuiBeforeProjectClose();

    void            removeViewer( QWidget* viewer );
    void            addViewer(QWidget* viewer, const RimMdiWindowGeometry& windowsGeometry);
    void            setActiveViewer(QWidget* subWindow);

    caf::PdmUiTreeView* projectTreeView() { return m_projectTreeView;}

    void            hideAllDockWindows();

    void            selectAsCurrentItem(caf::PdmObject* object);

    void            setDefaultWindowSize();

    void            setExpanded(const caf::PdmUiItem* uiItem, bool expanded);

    RimMdiWindowGeometry    windowGeometryForViewer(QWidget* viewer);
    RimMdiWindowGeometry    windowGeometryForWidget(QWidget* widget);

    void            tileWindows();
    bool            isAnyMdiSubWindowVisible();
    QMdiSubWindow*  findMdiSubWindow(QWidget* viewer);
	QList<QMdiSubWindow*> subWindowList(QMdiArea::WindowOrder order);

protected:
    virtual void    closeEvent(QCloseEvent* event);

private:
    void            createActions();
    void            createMenus();
    void            createToolBars();
    void            createDockPanels();

    void            storeTreeViewState();
    void            restoreTreeViewState();

private:
    QByteArray                m_initialDockAndToolbarLayout;    // Initial dock window and toolbar layout, used to reset GUI

private:
    QAction*            m_snapshotToFile;
    QAction*            m_snapshotToClipboard;
    QAction*            m_snapshotAllViewsToFile;

    QMdiArea*           m_mdiArea;
    RiuViewer*          m_mainViewer;
    
    QMenu*              m_windowMenu;


// Menu and action slots
private slots:

    friend class RiuMdiSubWindow;

    void    slotSnapshotToFile();
    void    slotSnapshotToClipboard();
    void    slotSnapshotAllViewsToFile();

    void    slotBuildWindowActions();

    void    slotSubWindowActivated(QMdiSubWindow* subWindow);

    void    selectedObjectsChanged();
    void    customMenuRequested(const QPoint& pos);

public:
    void setPdmRoot(caf::PdmObject* pdmRoot);

private:
    caf::PdmUiTreeView*            m_projectTreeView;
    
    std::unique_ptr<caf::PdmUiDragDropInterface> m_dragDropInterface;
    
    caf::PdmObject*             m_pdmRoot;
    caf::PdmUiPropertyView*     m_pdmUiPropertyView;

    bool                        m_blockSlotSubWindowActivated;
};
