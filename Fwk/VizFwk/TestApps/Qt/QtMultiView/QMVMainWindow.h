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
#include "cvfCollection.h"
#include "cvfDrawableGeo.h"

#include <QtGlobal>
#if QT_VERSION >= 0x050000
#include <QMainWindow>
#else
#include <QtGui/QMainWindow>
#endif

class QMVWidget;

namespace cvf {
    class View;
    class Scene;
    class OpenGLResourceManager;
    class OpenGLContextGroup;
}



//==================================================================================================
//
// 
//
//==================================================================================================
class QMVMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    QMVMainWindow();
    ~QMVMainWindow();

private:
    int                         vizWidgetCount();
    void                        createVizWidgets(int numWidgets, bool software, bool recycleScenes);
    void                        deleteAllOpenGLResourcesInAllVizWidgets();
    void                        deleteAllVizWidgets();
    void                        setSceneInAllVizWidgets(cvf::Scene* scene);
    void                        spreadScenesAcrossVizWidgets(cvf::Collection<cvf::Scene>* sceneCollection);
    void                        gatherAllScenes(cvf::Collection<cvf::Scene>* sceneCollection);
    void                        redrawAllVizWidgets();
    void                        setRenderModeInAllModels(cvf::DrawableGeo::RenderMode renderMode);

    // Protected overrides
protected:
    virtual void    closeEvent(QCloseEvent* pCE);

private slots:
    void            slotSoftwareRenderingWidgets(bool);
    void            slotConfigNumWidgets();

    void            slotCreateSphereAndBoxScene();
    void            slotCreateSpheresScene();
    void            slotCreateBoxesScene();
    void            slotCreateTrianglesScene();
    void            slotAllWidgetsDifferentScene();
    void            slotClearScene();

    void            slotUseBufferObjects();
    void            slotUseClientVertexArrays();

    void            slotDeleteAllResourcesInResourceManager();

    void            slotUpdateStatusbar();

private:
    static const int                    MAX_NUM_WIDGETS = 4;
    cvf::ref<cvf::OpenGLContextGroup>   m_contextGroup;
    QMVWidget*                          m_vizWidgets[MAX_NUM_WIDGETS];

    QAction*                            m_recycleScenesInWidgetConfigAction;
    QAction*                            m_softwareRenderingWidgetsAction;
    QAction*                            m_configNumWidgets1Action;
    QAction*                            m_configNumWidgets2Action;
    QAction*                            m_configNumWidgets4Action;
    QAction*                            m_configNumWidgetsNoneAction;

    QAction*                            m_createSphereAndBoxSceneAction;
    QAction*                            m_createSpheresSceneAction;
    QAction*                            m_createBoxesSceneAction;
    QAction*                            m_createTrianglesSceneAction;
    QAction*                            m_allWidgetsDifferentSceneAction;
    QAction*                            m_clearSceneAction;

    QAction*                            m_useBufferObjectsAction;
    QAction*                            m_useClientVertexArraysAction;

    QAction*                            m_deleteAllResourcesInResourceManagerAction;
};

