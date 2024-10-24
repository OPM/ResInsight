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

#include <QMainWindow>
#include <QPointer>

class QMVWidget;

namespace cvf {
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
    void                        createVizWidgets(int numWidgets, bool recycleScenes);
    void                        deleteAllOpenGLResourcesInResourceManager();
    void                        deleteOrReleaseOpenGLResourcesInAllVizWidgets();
    void                        deleteAllVizWidgets();
    void                        deleteVizWidgetAt(int index);
    void                        setSceneInAllVizWidgets(cvf::Scene* scene);
    void                        spreadScenesAcrossVizWidgets(cvf::Collection<cvf::Scene>* sceneCollection);
    void                        gatherAllScenes(cvf::Collection<cvf::Scene>* sceneCollection);
    void                        redrawAllVizWidgets();
    void                        setRenderModeInAllModels(cvf::DrawableGeo::RenderMode renderMode);

protected:
    virtual void    closeEvent(QCloseEvent* pCE);

private slots:
    void            slotConfigNumVizWidgets();
    void            slotDeleteFirstVizWidget();
    void            slotDeleteSecondVizWidget();

    void            slotCreateSphereAndBoxScene();
    void            slotCreateSpheresScene();
    void            slotCreateBoxesScene();
    void            slotCreateTrianglesScene();
    void            slotAllWidgetsDifferentScene();
    void            slotClearScene();

    void            slotUseBufferObjects();
    void            slotUseClientVertexArrays();

    void            slotDeleteAllResourcesInResourceManager();
    void            slotDeleteOrReleaseOpenGLResourcesInAllVizWidgets();

    void            slotUpdateStatusbar();

private:
    static const int                    MAX_NUM_WIDGETS = 4;
    cvf::ref<cvf::OpenGLContextGroup>   m_contextGroup;
    QPointer<QMVWidget>                 m_vizWidgets[MAX_NUM_WIDGETS];

    QAction*                            m_createWidgetsAsFloatingDialogsAction;
    QAction*                            m_recycleScenesInWidgetConfigAction;
    QAction*                            m_configNumWidgets1Action;
    QAction*                            m_configNumWidgets2Action;
    QAction*                            m_configNumWidgets4Action;
    QAction*                            m_configNumWidgetsNoneAction;

    QAction*                            m_useShadersAction;
};

