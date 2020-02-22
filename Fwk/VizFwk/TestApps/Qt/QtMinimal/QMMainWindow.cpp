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

#include "QMMainWindow.h"
#include "QMWidget.h"
#if QT_VERSION >= 0x050000
#include <QFrame>
#include <QHBoxLayout>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#else
#include <QtGui/QFrame>
#include <QtGui/QHBoxLayout>
#include <QtGui/QAction>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#endif
using cvf::ref;



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QMMainWindow::QMMainWindow()
{
    QFrame* mainFrame = new QFrame;
    QHBoxLayout* frameLayout = new QHBoxLayout(mainFrame);
    setCentralWidget(mainFrame);

    m_contextGroup = new cvf::OpenGLContextGroup;
    m_vizWidget = new QMWidget(m_contextGroup.p(), mainFrame);

    m_vizWidget->setFocus();
    frameLayout->addWidget(m_vizWidget);


    m_createDefaultSceneAction  = new QAction("Default Scene", this);
    m_clearSceneAction          = new QAction("Clear Scene", this);
    connect(m_createDefaultSceneAction, SIGNAL(triggered()), SLOT(slotCreateDefaultScene()));
    connect(m_clearSceneAction,         SIGNAL(triggered()), SLOT(slotClearScene()));

    QMenu* menu = menuBar()->addMenu("&Scenes");
    menu->addAction(m_createDefaultSceneAction);
    menu->addSeparator();
    menu->addAction(m_clearSceneAction);

    slotCreateDefaultScene();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QMMainWindow::slotCreateDefaultScene()
{
    ref<cvf::ModelBasicList> model = new cvf::ModelBasicList;

    {
        cvf::GeometryBuilderDrawableGeo builder;
        cvf::GeometryUtils::createSphere(2, 10, 10, &builder);

        ref<cvf::Effect> eff = new cvf::Effect;
        eff->setRenderState(new cvf::RenderStateMaterial_FF(cvf::Color3::BLUE));

        ref<cvf::Part> part = new cvf::Part;
        part->setName("MySphere");
        part->setDrawable(0, builder.drawableGeo().p());
        part->setEffect(eff.p());

        model->addPart(part.p());
    }

    {
        cvf::GeometryBuilderDrawableGeo builder;
        cvf::GeometryUtils::createBox(cvf::Vec3f(5, 0, 0), 2, 3, 4, &builder);

        ref<cvf::Effect> eff = new cvf::Effect;
        eff->setRenderState(new cvf::RenderStateMaterial_FF(cvf::Color3::RED));

        ref<cvf::Part> part = new cvf::Part;
        part->setName("MyBox");
        part->setDrawable(0, builder.drawableGeo().p());
        part->setEffect(eff.p());

        model->addPart(part.p());
    }

    model->updateBoundingBoxesRecursive();

    ref<cvf::Scene> scene = new cvf::Scene;
    scene->addModel(model.p());

    CVF_ASSERT(m_vizWidget);
    m_vizWidget->setScene(scene.p());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QMMainWindow::slotClearScene()
{
    CVF_ASSERT(m_vizWidget);
    m_vizWidget->setScene(NULL);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QMMainWindow::closeEvent(QCloseEvent* /*event*/)
{
    CVF_ASSERT(m_contextGroup.notNull());
    CVF_ASSERT(m_vizWidget);

    // Shut down the CeeViz OpenGL context contained in the widget
    // Deletes all OpenGL resources and removes context from context group
    m_vizWidget->cvfShutdownOpenGLContext();
}


#ifndef CVF_USING_CMAKE
//########################################################
#include "qt-generated/moc_QMMainWindow.cpp"
//########################################################
#endif
