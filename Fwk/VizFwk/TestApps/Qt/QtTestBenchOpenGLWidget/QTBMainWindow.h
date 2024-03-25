//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################

#pragma once

#include "cvfBase.h"
#include "cvfObject.h"

#include "QTBVizWidget.h"

#include <QMainWindow>
#include <QPointer>



//==================================================================================================
//
// 
//
//==================================================================================================
class QTBMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    QTBMainWindow();
    ~QTBMainWindow();

    void    handleVizWidgetIsOpenGLReady(QTBVizWidget* vizWidget);

private:
    enum NewWidgetPlacement
    {
        AddToCentralWidget,
        AsNewTopLevelWidget,
        InDockWidget_1,
        InDockWidget_2
    };

private:
    void                        addVizWidget(NewWidgetPlacement newWidgetPlacement);
    void                        populateAllValidVizWidgetsWithTestScene(bool allWidgetsShareSameScene);
    void                        removeStaleEntriesFromVizWidgetArr();
    void                        assignViewTitlesToVizWidgetsInArr();
    std::vector<cvf::String>    vizWidgetNames();
    void                        decideNewActiveVizWidgetAndHighlight();
    void                        highlightActiveVizWidget();
    QTBVizWidget*               getActiveVizWidget();
    void                        saveWinGeoAndDockToolBarLayout();
    void                        loadWinGeoAndDockToolBarLayout();

    virtual void                closeEvent(QCloseEvent* event);

private slots:
    void    slotAddVizWidget();
    void    slotDeleteVizWidget();
    void    slotReparentVizWidgetIntoCentralWidget();
    void    slotReparentVizWidgetAsTopLevel();
    void    slotMoveVizWidgetToDockWidget_1();
    void    slotMoveVizWidgetToDockWidget_2();
    void    slotReparentAllVizWidgetsIntoCentralWidget();
    void    slotRefreshVizWidgetArr();

    void    slotAboutToShowActivateMenu();
    void    slotSomeVizWidgetActivated();

    void    slotCreateTestScene();
    void    slotClearScene();

    void    slotSaveLayout();
    void    slotRestoreLayout();
    void    slotGrabNativeHandle();

private:
    cvf::ref<cvf::OpenGLContextGroup>           m_commonContextGroup;
    std::vector<QPointer<QTBVizWidget>>         m_vizWidgetArr;
    cvf::String                                 m_activeVizWidgetName;

    QPointer<QDockWidget>                       m_dockWidget_1;
    QPointer<QDockWidget>                       m_dockWidget_2;

    QPointer<QAction>                           m_setWorkingVizWidgetFirstAction;
    QPointer<QAction>                           m_setWorkingVizWidgetLastAction;
};

