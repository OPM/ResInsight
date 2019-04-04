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

class QMdiArea;
struct RimMdiWindowGeometry;

namespace caf
{
    class PdmObject;
    class PdmUiTreeView;
    class PdmUiItem;
}


class QMdiSubWindow;


//==================================================================================================
///  
//==================================================================================================
class RiuMainWindowBase : public QMainWindow
{
    Q_OBJECT

public:
    RiuMainWindowBase();

    virtual QString     mainWindowName() = 0;

    virtual void        removeViewer( QWidget* viewer ) = 0;
    virtual void        addViewer(QWidget* viewer, const RimMdiWindowGeometry& windowsGeometry)= 0;
    virtual void        setActiveViewer(QWidget* subWindow) = 0;

    virtual QMdiSubWindow* findMdiSubWindow(QWidget* viewer) = 0;

    RimMdiWindowGeometry windowGeometryForViewer(QWidget* viewer);
    void                loadWinGeoAndDockToolBarLayout();
    void                saveWinGeoAndDockToolBarLayout();
    void                showWindow();

    caf::PdmUiTreeView* projectTreeView() { return m_projectTreeView;}
    void                setExpanded(const caf::PdmUiItem* uiItem, bool expanded = true);

    void                selectAsCurrentItem(const caf::PdmObject* object, bool allowActiveViewChange = true);
    
    void                enableShowFirstVisibleMdiWindowMaximized(bool enable);
 
    virtual void        tileSubWindows() = 0;
    virtual void        storeSubWindowTiling(bool tiled) = 0;
    virtual void        clearWindowTiling() = 0;
    virtual bool        subWindowsAreTiled() const = 0;

protected slots:
    void                slotDockWidgetToggleViewActionTriggered();
    void			    addViewerToMdiArea(QMdiArea* mdiArea, QWidget* viewer, const QPoint& subWindowPos, const QSize& subWindowSize);

protected:
    caf::PdmUiTreeView* m_projectTreeView;
    bool                m_allowActiveViewChangeFromSelection; // To be used in selectedObjectsChanged() to control 
                                                                         // whether to select the corresponding active view or not
private:
    QString             registryFolderName();

private:
    bool                m_showFirstVisibleWindowMaximized;
};
