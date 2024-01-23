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

#include "cvfLibCore.h"
#include "cvfLibRender.h"
#include "cvfLibGeometry.h"
#include "cvfLibViewing.h"

#include "QMVMainWindow.h"
#include "QMVWidget.h"
#include "QMVFactory.h"

#include <QTimer>
#include <QFrame>
#include <QHBoxLayout>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QLabel>



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QMVMainWindow::QMVMainWindow()
{
    memset(m_vizWidgets, 0, sizeof(m_vizWidgets));

    QFrame* mainFrame = new QFrame;
    QGridLayout* frameLayout = new QGridLayout;
    mainFrame->setLayout(frameLayout);
    setCentralWidget(mainFrame);


    QMenu* widgetsMenu = menuBar()->addMenu("&Widgets");

    m_createWidgetsAsFloatingDialogsAction = new QAction("Create Widgets as Floating Dialogs", this);
    m_createWidgetsAsFloatingDialogsAction->setCheckable(true);

    m_recycleScenesInWidgetConfigAction = new QAction("Recycle Scenes When Changing Widget Config", this);
    m_recycleScenesInWidgetConfigAction->setCheckable(true);

    m_configNumWidgets1Action = new QAction("1 Widget", this);
    m_configNumWidgets2Action = new QAction("2 Widgets", this);
    m_configNumWidgets4Action = new QAction("4 Widgets", this);
    m_configNumWidgetsNoneAction = new QAction("No Widgets", this);
    connect(m_configNumWidgets1Action, SIGNAL(triggered()), SLOT(slotConfigNumVizWidgets()));
    connect(m_configNumWidgets2Action, SIGNAL(triggered()), SLOT(slotConfigNumVizWidgets()));
    connect(m_configNumWidgets4Action, SIGNAL(triggered()), SLOT(slotConfigNumVizWidgets()));
    connect(m_configNumWidgetsNoneAction, SIGNAL(triggered()), SLOT(slotConfigNumVizWidgets()));

    widgetsMenu->addAction(m_createWidgetsAsFloatingDialogsAction);
    widgetsMenu->addSeparator();
    widgetsMenu->addAction(m_recycleScenesInWidgetConfigAction);
    widgetsMenu->addSeparator();
    widgetsMenu->addAction(m_configNumWidgets1Action);
    widgetsMenu->addAction(m_configNumWidgets2Action);
    widgetsMenu->addAction(m_configNumWidgets4Action);
    widgetsMenu->addAction(m_configNumWidgetsNoneAction);
    widgetsMenu->addSeparator();
    widgetsMenu->addAction("Delete First Widget", this, SLOT(slotDeleteFirstVizWidget()));
    widgetsMenu->addAction("Delete Second Widget", this, SLOT(slotDeleteSecondVizWidget()));


    QMenu* scenesMenu = menuBar()->addMenu("&Scenes");

    m_useShadersAction = new QAction("Use Shaders When Creating Scenes", this);
    m_useShadersAction->setCheckable(true);
    m_useShadersAction->setChecked(true);

    scenesMenu->addAction(m_useShadersAction);
    scenesMenu->addSeparator();
    scenesMenu->addAction("Sphere And Box Scene", this, SLOT(slotCreateSphereAndBoxScene()));
    scenesMenu->addAction("Spheres Scene", this, SLOT(slotCreateSpheresScene()));
    scenesMenu->addAction("Boxes Scene", this, SLOT(slotCreateBoxesScene()));
    scenesMenu->addAction("Triangles Scene", this, SLOT(slotCreateTrianglesScene()));
    scenesMenu->addSeparator();
    scenesMenu->addAction("All Widgets Show Different Scene", this, SLOT(slotAllWidgetsDifferentScene()));
    scenesMenu->addSeparator();
    scenesMenu->addAction("Clear Scene", this, SLOT(slotClearScene()));


    QMenu* renderingMenu = menuBar()->addMenu("&Rendering");
    renderingMenu->addAction("Use Buffer Objects", this, SLOT(slotUseBufferObjects()));
    renderingMenu->addAction("Use Client Vertex Arrays", this, SLOT(slotUseClientVertexArrays()));


    QMenu* testMenu = menuBar()->addMenu("&Test");
    testMenu->addAction("Delete All Resources In Resource Manager", this, SLOT(slotDeleteAllResourcesInResourceManager()));
    testMenu->addAction("Delete or Release OpenGL Resources in All Widgets", this, SLOT(slotDeleteOrReleaseOpenGLResourcesInAllVizWidgets()));


    // Must create context group before launching any widgets
    m_contextGroup = new cvf::OpenGLContextGroup;

    createVizWidgets(1, false);

    QTimer* timer = new QTimer;
    connect(timer, SIGNAL(timeout()), SLOT(slotUpdateStatusbar()));
    timer->start(250);
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QMVMainWindow::~QMVMainWindow()
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int QMVMainWindow::vizWidgetCount()
{
    int count = 0;

    int i;
    for (i = 0; i < MAX_NUM_WIDGETS; i++)
    {
        if (m_vizWidgets[i])
        {
            count++;
        }
    }

    return count;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QMVMainWindow::createVizWidgets(int numWidgets, bool recycleScenes)
{
    CVF_ASSERT(numWidgets <= MAX_NUM_WIDGETS);

    cvf::Collection<cvf::Scene> sceneCollection;
    if (recycleScenes)
    {
        gatherAllScenes(&sceneCollection);
    }

    deleteAllVizWidgets();

    // Note that creating the widgets as floating dialogs will only work if the 
    // Qt::AA_ShareOpenGLContexts has been set using QApplication::setAttribute(Qt::AA_ShareOpenGLContexts) before Application object is constructed
    const bool createAsDialogs = m_createWidgetsAsFloatingDialogsAction->isChecked();

    QWidget* parentWidget = centralWidget();

    // The context group that all the contexts end up in
    CVF_ASSERT(m_contextGroup.notNull());
    CVF_ASSERT(m_contextGroup->contextCount() == 0);
    
    int i;
    for (i = 0; i < numWidgets; i++)
    {
        QMVWidget* newWidget = NULL;

        if (createAsDialogs)
        {
            newWidget = new QMVWidget(m_contextGroup.p(), i, parentWidget, Qt::Dialog);
            newWidget->resize(600, 400);
            newWidget->show();
        }
        else
        {
            newWidget = new QMVWidget(m_contextGroup.p(), i, parentWidget);
            QGridLayout* layout = parentWidget ? dynamic_cast<QGridLayout*>(parentWidget->layout()) : NULL;
            if (layout)
            {
                int row = i/2;
                int col = i-2*row;
                layout->addWidget(newWidget, row, col);
            }
        }

        m_vizWidgets[i] = newWidget;
    }

    if (recycleScenes)
    {
        spreadScenesAcrossVizWidgets(&sceneCollection);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QMVMainWindow::deleteOrReleaseOpenGLResourcesInAllVizWidgets()
{
    // Will be set to one of the OpenGL contexts so we can use it in final evict call
    cvf::OpenGLContext* someOglContext = NULL;

    // Loops over all the widgets and deletes/releases the OpenGL resources for each of them
    int i;
    for (i = 0; i < MAX_NUM_WIDGETS; i++)
    {
        QMVWidget* vizWidget = m_vizWidgets[i];
        if (vizWidget)
        {
            vizWidget->makeCurrent();
            cvf::OpenGLContext* oglContext = vizWidget->cvfOpenGLContext();
            CVF_ASSERT(oglContext);
            CVF_ASSERT(oglContext->isCurrent());
            someOglContext = oglContext;

            cvf::RenderSequence* renderSeq = vizWidget->renderSequence();
            if (renderSeq)
            {
                renderSeq->deleteOrReleaseOpenGLResources(oglContext);
            }
        }
    }

    cvf::OpenGLResourceManager* resourceManager = m_contextGroup.notNull() ? m_contextGroup->resourceManager() : NULL;
    if (resourceManager && someOglContext)
    {
        resourceManager->evictOrphanedOpenGLResources(someOglContext);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QMVMainWindow::deleteAllOpenGLResourcesInResourceManager()
{
    if (m_contextGroup.notNull())
    {
        cvf::OpenGLResourceManager* rcMgr = m_contextGroup->resourceManager();
        CVF_ASSERT(rcMgr);

        if (m_contextGroup->contextCount() > 0)
        {
            // Grab any context in the group
            cvf::OpenGLContext* oglContext = m_contextGroup->context(0);
            oglContext->makeCurrent();
            rcMgr->deleteAllOpenGLResources(oglContext);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QMVMainWindow::deleteAllVizWidgets()
{
    QWidget* parentWidget = centralWidget();
    QLayout* layout = parentWidget->layout();

    // Should not be needed, but left for experimentation
    //deleteOrReleaseOpenGLResourcesInAllVizWidgets();
    //deleteAllOpenGLResourcesInResourceManager();

    int i;
    for (i = 0; i < MAX_NUM_WIDGETS; i++)
    {
        if (m_vizWidgets[i])
        {
            layout->removeWidget(m_vizWidgets[i]);
            delete m_vizWidgets[i];
            m_vizWidgets[i] = NULL;
        }
    }

    CVF_ASSERT(m_contextGroup.isNull() || m_contextGroup->contextCount() == 0);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QMVMainWindow::deleteVizWidgetAt(int index)
{
    QWidget* parentWidget = centralWidget();
    QLayout* layout = parentWidget->layout();

    if (m_vizWidgets[index])
    {
        layout->removeWidget(m_vizWidgets[index]);
        delete m_vizWidgets[index];
        m_vizWidgets[index] = NULL;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QMVMainWindow::setSceneInAllVizWidgets(cvf::Scene* scene)
{
    QMVRenderSequenceFactory factory;

    int i;
    for (i = 0; i < MAX_NUM_WIDGETS; i++)
    {
        if (m_vizWidgets[i] != NULL)
        {
            cvf::ref<cvf::RenderSequence> renderSeq = factory.createFromScene(scene);
            m_vizWidgets[i]->setRenderSequence(renderSeq.p());
        }
    }

    redrawAllVizWidgets();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QMVMainWindow::spreadScenesAcrossVizWidgets(cvf::Collection<cvf::Scene>* sceneCollection)
{
    QMVRenderSequenceFactory factory;

    cvf::uint i;
    for (i = 0; i < static_cast<cvf::uint>(MAX_NUM_WIDGETS); i++)
    {
        QMVWidget* vizWidget = m_vizWidgets[i];
        if (vizWidget)
        {
            cvf::Scene* scene = (sceneCollection->size() > i) ? sceneCollection->at(i) : NULL;
            if (scene)
            {
                cvf::ref<cvf::RenderSequence> renderSeq = factory.createFromScene(scene);
                vizWidget->setRenderSequence(renderSeq.p());
            }
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QMVMainWindow::gatherAllScenes(cvf::Collection<cvf::Scene>* sceneCollection)
{
    int i;
    for (i = 0; i < MAX_NUM_WIDGETS; i++)
    {
        if (m_vizWidgets[i] != NULL)
        {
            cvf::RenderSequence* renderSeq = m_vizWidgets[i]->renderSequence();
            cvf::Rendering* rendering = renderSeq ? renderSeq->firstRendering() : NULL;
            cvf::Scene* scene = rendering ? rendering->scene() : NULL;
            if (scene)
            {
                sceneCollection->push_back(scene);
            }
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QMVMainWindow::redrawAllVizWidgets()
{
    int i;
    for (i = 0; i < MAX_NUM_WIDGETS; i++)
    {
        if (m_vizWidgets[i] != NULL)
        {
            m_vizWidgets[i]->update();
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QMVMainWindow::setRenderModeInAllModels(cvf::DrawableGeo::RenderMode renderMode)
{
    int i;
    for (i = 0; i < MAX_NUM_WIDGETS; i++)
    {
        if (m_vizWidgets[i] != NULL)
        {
            cvf::RenderSequence* renderSeq = m_vizWidgets[i]->renderSequence();
            cvf::Rendering* rendering = renderSeq ? renderSeq->firstRendering() : NULL;
            cvf::Scene* scene = rendering ? rendering->scene() : NULL;
            if (scene)
            {
                cvf::Collection<cvf::Part> allParts;
                scene->allParts(&allParts);

                size_t numParts = allParts.size();
                size_t partIdx;
                for (partIdx = 0; partIdx < numParts; partIdx++)
                {
                    cvf::Part* part = allParts.at(partIdx);

                    cvf::uint lod;
                    for (lod = 0; lod < cvf::Part::MAX_NUM_LOD_LEVELS; lod++)
                    {
                        cvf::DrawableGeo* drawableGeo = dynamic_cast<cvf::DrawableGeo*>(part->drawable(lod));
                        if (drawableGeo)
                        {
                            drawableGeo->setRenderMode(renderMode);
                        }
                    }
                }
            }
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QMVMainWindow::closeEvent(QCloseEvent*)
{
    // Should not be needed any more, but left for experimentation
    //deleteOrReleaseOpenGLResourcesInAllVizWidgets();
    //deleteAllOpenGLResourcesInResourceManager();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QMVMainWindow::slotConfigNumVizWidgets()
{
    QObject* senderAct = sender();

    bool recycleScenes = m_recycleScenesInWidgetConfigAction->isChecked();

    if      (senderAct == m_configNumWidgets1Action)    createVizWidgets(1, recycleScenes);
    else if (senderAct == m_configNumWidgets2Action)    createVizWidgets(2, recycleScenes);
    else if (senderAct == m_configNumWidgets4Action)    createVizWidgets(4, recycleScenes);
    else if (senderAct == m_configNumWidgetsNoneAction) createVizWidgets(0, recycleScenes);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QMVMainWindow::slotDeleteFirstVizWidget()
{
    deleteVizWidgetAt(0);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QMVMainWindow::slotDeleteSecondVizWidget()
{
    deleteVizWidgetAt(1);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QMVMainWindow::slotCreateSphereAndBoxScene()
{
    // Without initialization we're not getting the correct capabilities
    CVF_ASSERT(m_contextGroup->isContextGroupInitialized());

    QMVModelFactory modelFactory(m_useShadersAction->isChecked(), *m_contextGroup->capabilities());
    QMVSceneFactory sceneFactory(&modelFactory);

    cvf::ref<cvf::Model> model = modelFactory.createSphereAndBox();
    cvf::ref<cvf::Scene> scene = sceneFactory.createFromModel(model.p());

    setSceneInAllVizWidgets(scene.p());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QMVMainWindow::slotCreateSpheresScene()
{
    // Without initialization we're not getting the correct capabilities
    CVF_ASSERT(m_contextGroup->isContextGroupInitialized());

    QMVModelFactory modelFactory(m_useShadersAction->isChecked(), *m_contextGroup->capabilities());
    QMVSceneFactory sceneFactory(&modelFactory);

    cvf::ref<cvf::Model> model = modelFactory.createSpheres();
    cvf::ref<cvf::Scene> scene = sceneFactory.createFromModel(model.p());

    setSceneInAllVizWidgets(scene.p());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QMVMainWindow::slotCreateBoxesScene()
{
    // Without initialization we're not getting the correct capabilities
    CVF_ASSERT(m_contextGroup->isContextGroupInitialized());

    QMVModelFactory modelFactory(m_useShadersAction->isChecked(), *m_contextGroup->capabilities());
    QMVSceneFactory sceneFactory(&modelFactory);

    cvf::ref<cvf::Model> model = modelFactory.createBoxes();
    cvf::ref<cvf::Scene> scene = sceneFactory.createFromModel(model.p());

    setSceneInAllVizWidgets(scene.p());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QMVMainWindow::slotCreateTrianglesScene()
{
    // Without initialization we're not getting the correct capabilities
    CVF_ASSERT(m_contextGroup->isContextGroupInitialized());

    QMVModelFactory modelFactory(m_useShadersAction->isChecked(), *m_contextGroup->capabilities());
    QMVSceneFactory sceneFactory(&modelFactory);

    cvf::ref<cvf::Model> model = modelFactory.createTriangles();
    cvf::ref<cvf::Scene> scene = sceneFactory.createFromModel(model.p());

    setSceneInAllVizWidgets(scene.p());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QMVMainWindow::slotAllWidgetsDifferentScene()
{
    // Without initialization we're not getting the correct capabilities
    CVF_ASSERT(m_contextGroup->isContextGroupInitialized());

    QMVModelFactory modelFactory(m_useShadersAction->isChecked(), *m_contextGroup->capabilities());
    QMVSceneFactory sceneFactory(&modelFactory);
    QMVRenderSequenceFactory renderSeqFactory;

    int i;
    for (i = 0; i < MAX_NUM_WIDGETS; i++)
    {
        if (m_vizWidgets[i] != NULL)
        {
            cvf::ref<cvf::Scene> scene = sceneFactory.createNumberedScene(i);
            cvf::ref<cvf::RenderSequence> renderSeq = renderSeqFactory.createFromScene(scene.p());
            m_vizWidgets[i]->setRenderSequence(renderSeq.p());
        }
    }

    redrawAllVizWidgets();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QMVMainWindow::slotClearScene()
{
    int i;
    for (i = 0; i < MAX_NUM_WIDGETS; i++)
    {
        if (m_vizWidgets[i] != NULL)
        {
            m_vizWidgets[i]->setRenderSequence(NULL);
        }
    }

    redrawAllVizWidgets();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QMVMainWindow::slotUseBufferObjects()
{
    setRenderModeInAllModels(cvf::DrawableGeo::BUFFER_OBJECT);
    redrawAllVizWidgets();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QMVMainWindow::slotUseClientVertexArrays()
{
    setRenderModeInAllModels(cvf::DrawableGeo::VERTEX_ARRAY);
    redrawAllVizWidgets();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QMVMainWindow::slotDeleteAllResourcesInResourceManager()
{
    deleteAllOpenGLResourcesInResourceManager();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QMVMainWindow::slotDeleteOrReleaseOpenGLResourcesInAllVizWidgets()
{
    deleteOrReleaseOpenGLResourcesInAllVizWidgets();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QMVMainWindow::slotUpdateStatusbar()
{
    cvf::OpenGLResourceManager* resourceManager = m_contextGroup.notNull() ? m_contextGroup->resourceManager() : NULL;

    QString msg = "N/A  ";
    if (resourceManager)
    {
        cvf::uint boCount = resourceManager->bufferObjectCount();
        double boMemUsageMB = static_cast<double>(resourceManager->bufferObjectMemoryUsage())/(1024.0*1024.0);
        msg = QString("#bo=%1 (MB=%2)  |  ").arg(boCount).arg(boMemUsageMB, 0, 'f', 3);
    }

    int i;
    for (i = 0; i < MAX_NUM_WIDGETS; i++)
    {
        QMVWidget* vizWidget = m_vizWidgets[i];
        if (vizWidget)
        {
            cvf::RenderSequence* renderSeq = vizWidget->renderSequence();
            if (renderSeq)
            {
                cvf::PerformanceInfo pi = renderSeq->performanceInfo();
                QString viewMsg = QString("V%1 #p=%2 #t=%3    ").arg(i).arg(pi.visiblePartsCount).arg((pi.triangleCount));
                msg += viewMsg;
            }
        }
    }

    if (m_contextGroup.notNull())
    {
        const cvf::OpenGLInfo info = m_contextGroup->info();
        msg += QString("  |  ") + QString::fromStdString(info.renderer().toStdString());
    }

    QStatusBar* sb = statusBar();
    sb->showMessage(msg);
}


