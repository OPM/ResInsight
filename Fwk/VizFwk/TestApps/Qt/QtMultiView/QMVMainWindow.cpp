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

#include <QtCore/QTimer>
#if QT_VERSION >= 0x050000
#include <QFrame>
#include <QHBoxLayout>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QLabel>
#else
#include <QtGui/QFrame>
#include <QtGui/QHBoxLayout>
#include <QtGui/QAction>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QStatusBar>
#include <QtGui/QLabel>
#endif

using cvf::ref;



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


    m_recycleScenesInWidgetConfigAction = new QAction("Recycle Scenes When Changing Widget Config", this);
    m_recycleScenesInWidgetConfigAction->setCheckable(true);

    m_softwareRenderingWidgetsAction = new QAction("Software Rendering in Widgets", this);
    m_softwareRenderingWidgetsAction->setCheckable(true);
    connect(m_softwareRenderingWidgetsAction, SIGNAL(toggled(bool)), SLOT(slotSoftwareRenderingWidgets(bool)));

    m_configNumWidgets1Action   = new QAction("1 Widget", this);
    m_configNumWidgets2Action   = new QAction("2 Widgets", this);
    m_configNumWidgets4Action   = new QAction("4 Widgets", this);
    m_configNumWidgetsNoneAction= new QAction("No Widgets", this);
    connect(m_configNumWidgets1Action,    SIGNAL(triggered()), SLOT(slotConfigNumWidgets()));
    connect(m_configNumWidgets2Action,    SIGNAL(triggered()), SLOT(slotConfigNumWidgets()));
    connect(m_configNumWidgets4Action,    SIGNAL(triggered()), SLOT(slotConfigNumWidgets()));
    connect(m_configNumWidgetsNoneAction, SIGNAL(triggered()), SLOT(slotConfigNumWidgets()));

    m_createSphereAndBoxSceneAction  = new QAction("Sphere And Box Scene", this);
    m_createSpheresSceneAction       = new QAction("Spheres Scene", this);
    m_createBoxesSceneAction         = new QAction("Boxes Scene", this);
    m_createTrianglesSceneAction     = new QAction("Triangles Scene", this);
    m_allWidgetsDifferentSceneAction = new QAction("All Widgets Show Different Scene", this);
    m_clearSceneAction               = new QAction("Clear Scene", this);
    connect(m_createSphereAndBoxSceneAction,    SIGNAL(triggered()), SLOT(slotCreateSphereAndBoxScene()));
    connect(m_createSpheresSceneAction,         SIGNAL(triggered()), SLOT(slotCreateSpheresScene()));
    connect(m_createBoxesSceneAction,           SIGNAL(triggered()), SLOT(slotCreateBoxesScene()));
    connect(m_createTrianglesSceneAction,       SIGNAL(triggered()), SLOT(slotCreateTrianglesScene()));
    connect(m_allWidgetsDifferentSceneAction,   SIGNAL(triggered()), SLOT(slotAllWidgetsDifferentScene()));
    connect(m_clearSceneAction,                 SIGNAL(triggered()), SLOT(slotClearScene()));

    m_useBufferObjectsAction        = new QAction("Use Buffer Objects", this);
    m_useClientVertexArraysAction   = new QAction("Use Client Vertex Arrays", this);
    connect(m_useBufferObjectsAction,       SIGNAL(triggered()), SLOT(slotUseBufferObjects()));
    connect(m_useClientVertexArraysAction,  SIGNAL(triggered()), SLOT(slotUseClientVertexArrays()));

    m_deleteAllResourcesInResourceManagerAction = new QAction("Delete All Resources In Resource Manager", this);
    connect(m_deleteAllResourcesInResourceManagerAction,    SIGNAL(triggered()), SLOT(slotDeleteAllResourcesInResourceManager()));


    QMenu* widgetsMenu = menuBar()->addMenu("&Widgets");
    widgetsMenu->addAction(m_recycleScenesInWidgetConfigAction);
    widgetsMenu->addSeparator();
    widgetsMenu->addAction(m_softwareRenderingWidgetsAction);
    widgetsMenu->addSeparator();
    widgetsMenu->addAction(m_configNumWidgets1Action);
    widgetsMenu->addAction(m_configNumWidgets2Action);
    widgetsMenu->addAction(m_configNumWidgets4Action);
    widgetsMenu->addAction(m_configNumWidgetsNoneAction);

    QMenu* scenesMenu = menuBar()->addMenu("&Scenes");
    scenesMenu->addAction(m_createSphereAndBoxSceneAction);
    scenesMenu->addAction(m_createSpheresSceneAction);
    scenesMenu->addAction(m_createBoxesSceneAction);
    scenesMenu->addAction(m_createTrianglesSceneAction);
    scenesMenu->addSeparator();
    scenesMenu->addAction(m_allWidgetsDifferentSceneAction);
    scenesMenu->addSeparator();
    scenesMenu->addAction(m_clearSceneAction);

    QMenu* renderingMenu = menuBar()->addMenu("&Rendering");
    renderingMenu->addAction(m_useBufferObjectsAction);
    renderingMenu->addAction(m_useClientVertexArraysAction);

    QMenu* testMenu = menuBar()->addMenu("&Test");
    testMenu->addAction(m_deleteAllResourcesInResourceManagerAction);

    // Must create context group before launching any widgets
    m_contextGroup = new cvf::OpenGLContextGroup;

    createVizWidgets(1, m_softwareRenderingWidgetsAction->isChecked(), false);
    slotCreateSphereAndBoxScene();

    QTimer* timer = new QTimer;
    connect(timer, SIGNAL(timeout()), SLOT(slotUpdateStatusbar()));
    timer->start(250);

    /*
    {
        QWidget* myWidget = new QWidget;
        QGridLayout* layout = new QGridLayout(myWidget);
        
        QLabel* l1 = new QLabel("JALLA", myWidget);
        QLabel* l2 = new QLabel("BALLA", myWidget);
        QLabel* l3 = new QLabel("TRALLA", myWidget);
        layout->addWidget(l1, 0, 0);
        layout->addWidget(l2, 0, 1);
        layout->addWidget(l3, 1, 1);

        QStatusBar* sb = statusBar();
        //sb->addPermanentWidget(new QLabel("JALLA"));
        sb->addPermanentWidget(myWidget);
    }
    */
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
void QMVMainWindow::createVizWidgets(int numWidgets, bool software, bool recycleScenes)
{
    CVF_ASSERT(numWidgets <= MAX_NUM_WIDGETS);

    cvf::Collection<cvf::Scene> sceneCollection;
    if (recycleScenes)
    {
        gatherAllScenes(&sceneCollection);
    }

    deleteAllVizWidgets();

    QWidget* parentWidget = centralWidget();
    QGridLayout* layout = dynamic_cast<QGridLayout*>(parentWidget->layout());
    CVF_ASSERT(layout);

    QGLFormat oglFormat;
    if (software)
    {
        oglFormat.setOption(QGL::IndirectRendering);
    }

    // The context group that all the contexts end up in
    CVF_ASSERT(m_contextGroup.notNull());
    CVF_ASSERT(m_contextGroup->contextCount() == 0);
    
    QMVWidget* shareWidget = NULL;
    
    int i;
    for (i = 0; i < numWidgets; i++)
    {
        QMVWidget* newWidget = NULL;
        if (shareWidget)
        {
            newWidget = new QMVWidget(shareWidget, parentWidget);
        }
        else
        {
            newWidget = new QMVWidget(m_contextGroup.p(), oglFormat, parentWidget);
            shareWidget = newWidget;
        }

        int row = i/2;
        int col = i-2*row;
        layout->addWidget(newWidget, row, col);

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
void QMVMainWindow::deleteAllOpenGLResourcesInAllVizWidgets()
{
    cvf::OpenGLResourceManager* resourceManager = m_contextGroup.notNull() ? m_contextGroup->resourceManager() : NULL;

    // The loop below should not be needed now that we can clean up resources 
    // by calling on the resource manager, but leave it as long as deleteOrReleaseOpenGLResources() is in place
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

            cvf::RenderSequence* renderSeq = vizWidget->renderSequence();
            if (renderSeq)
            {
                renderSeq->deleteOrReleaseOpenGLResources(oglContext);
            }

            CVF_ASSERT(resourceManager);
            resourceManager->deleteAllOpenGLResources(oglContext);
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

    deleteAllOpenGLResourcesInAllVizWidgets();

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
void QMVMainWindow::setSceneInAllVizWidgets(cvf::Scene* scene)
{
    QMVRenderSequenceFactory factory;

    int i;
    for (i = 0; i < MAX_NUM_WIDGETS; i++)
    {
        if (m_vizWidgets[i] != NULL)
        {
            ref<cvf::RenderSequence> renderSeq = factory.createFromScene(scene);
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
                ref<cvf::RenderSequence> renderSeq = factory.createFromScene(scene);
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
    deleteAllOpenGLResourcesInAllVizWidgets();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QMVMainWindow::slotSoftwareRenderingWidgets(bool software)
{
    int currNumWidgets = vizWidgetCount();

    // Just recreate with the same number of widgets
    createVizWidgets(currNumWidgets, software, m_recycleScenesInWidgetConfigAction->isChecked());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QMVMainWindow::slotConfigNumWidgets()
{
    QObject* senderAct = sender();

    bool software = m_softwareRenderingWidgetsAction->isChecked();
    bool recycleScenes = m_recycleScenesInWidgetConfigAction->isChecked();

    if      (senderAct == m_configNumWidgets1Action)    createVizWidgets(1, software, recycleScenes);
    else if (senderAct == m_configNumWidgets2Action)    createVizWidgets(2, software, recycleScenes);
    else if (senderAct == m_configNumWidgets4Action)    createVizWidgets(4, software, recycleScenes);
    else if (senderAct == m_configNumWidgetsNoneAction) createVizWidgets(0, software, recycleScenes);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QMVMainWindow::slotCreateSphereAndBoxScene()
{
    QMVModelFactory modelFactory(!m_softwareRenderingWidgetsAction->isChecked());
    QMVSceneFactory sceneFactory(&modelFactory);

    ref<cvf::Model> model = modelFactory.createSphereAndBox();
    ref<cvf::Scene> scene = sceneFactory.createFromModel(model.p());

    setSceneInAllVizWidgets(scene.p());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QMVMainWindow::slotCreateSpheresScene()
{
    QMVModelFactory modelFactory(!m_softwareRenderingWidgetsAction->isChecked());
    QMVSceneFactory sceneFactory(&modelFactory);

    ref<cvf::Model> model = modelFactory.createSpheres();
    ref<cvf::Scene> scene = sceneFactory.createFromModel(model.p());

    setSceneInAllVizWidgets(scene.p());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QMVMainWindow::slotCreateBoxesScene()
{
    QMVModelFactory modelFactory(!m_softwareRenderingWidgetsAction->isChecked());
    QMVSceneFactory sceneFactory(&modelFactory);

    ref<cvf::Model> model = modelFactory.createBoxes();
    ref<cvf::Scene> scene = sceneFactory.createFromModel(model.p());

    setSceneInAllVizWidgets(scene.p());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QMVMainWindow::slotCreateTrianglesScene()
{
    QMVModelFactory modelFactory(!m_softwareRenderingWidgetsAction->isChecked());
    QMVSceneFactory sceneFactory(&modelFactory);

    ref<cvf::Model> model = modelFactory.createTriangles();
    ref<cvf::Scene> scene = sceneFactory.createFromModel(model.p());

    setSceneInAllVizWidgets(scene.p());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QMVMainWindow::slotAllWidgetsDifferentScene()
{
    QMVModelFactory modelFactory(!m_softwareRenderingWidgetsAction->isChecked());
    QMVSceneFactory sceneFactory(&modelFactory);
    QMVRenderSequenceFactory renderSeqFactory;

    int i;
    for (i = 0; i < MAX_NUM_WIDGETS; i++)
    {
        if (m_vizWidgets[i] != NULL)
        {
            ref<cvf::Scene> scene = sceneFactory.createNumberedScene(i);
            ref<cvf::RenderSequence> renderSeq = renderSeqFactory.createFromScene(scene.p());
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
    if (m_contextGroup.notNull())
    {
        cvf::OpenGLResourceManager* rcMgr = m_contextGroup->resourceManager();
        CVF_ASSERT(rcMgr);

        QMVWidget* vizWidget = m_vizWidgets[0];
        cvf::OpenGLContext* oglContext = vizWidget ? vizWidget->cvfOpenGLContext() : NULL;
        if (oglContext)
        {
            oglContext->makeCurrent();
            rcMgr->deleteAllOpenGLResources(oglContext);
        }
    }

    redrawAllVizWidgets();
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
                QGLFormat oglFormat = vizWidget->format();
                QString hwSw = oglFormat.testOption(QGL::IndirectRendering) ? "sw" : "hw";
                QString viewMsg = QString("V%1(%2) #p=%3 #t=%4    ").arg(i).arg(hwSw).arg(pi.visiblePartsCount).arg((pi.triangleCount));
                msg += viewMsg;
            }
        }
    }

    QStatusBar* sb = statusBar();
    sb->showMessage(msg);
}


//########################################################
#ifndef CVF_USING_CMAKE
#include "qt-generated/moc_QMVMainWindow.cpp"
#endif
//########################################################

