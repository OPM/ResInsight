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

#include <QFrame>
#include <QHBoxLayout>
#include <QAction>
#include <QMenu>
#include <QMenuBar>



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QMMainWindow::QMMainWindow()
{
    QFrame* mainFrame = new QFrame;
    QHBoxLayout* frameLayout = new QHBoxLayout(mainFrame);
    setCentralWidget(mainFrame);

    m_contextGroup = new cvf::OpenGLContextGroup;

    // Pass pointer to ourselves to get notified when the vizWidget is ready for use and we can load our default scene
    m_vizWidget = new QMWidget(m_contextGroup.p(), mainFrame, this);

    m_vizWidget->setFocus();
    frameLayout->addWidget(m_vizWidget);

    QMenu* menu = menuBar()->addMenu("&Scenes");
    menu->addAction("Default Scene", this, SLOT(slotCreateDefaultScene()));
    menu->addSeparator();
    menu->addAction("Clear Scene", this, SLOT(slotClearScene()));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QMMainWindow::handleVizWidgetIsOpenGLReady()
{
    slotCreateDefaultScene();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QMMainWindow::slotCreateDefaultScene()
{
    cvf::ref<cvf::ModelBasicList> model = new cvf::ModelBasicList;

    const bool useShaders = true;

    CVF_ASSERT(m_contextGroup->isContextGroupInitialized());
    cvf::ShaderProgramGenerator spGen("SimpleHeadlight", cvf::ShaderSourceProvider::instance());
    spGen.configureStandardHeadlightColor();
    cvf::ref<cvf::ShaderProgram> shaderProg = spGen.generate();

    {
        cvf::GeometryBuilderDrawableGeo builder;
        cvf::GeometryUtils::createSphere(2, 10, 10, &builder);

        cvf::ref<cvf::Effect> eff = new cvf::Effect;
        if (useShaders)
        {
            eff->setShaderProgram(shaderProg.p());
            eff->setUniform(new cvf::UniformFloat("u_color", cvf::Color4f(cvf::Color3::GREEN)));
        }
        else
        {
            eff->setRenderState(new cvf::RenderStateMaterial_FF(cvf::Color3::BLUE));
        }

        cvf::ref<cvf::Part> part = new cvf::Part;
        part->setName("MySphere");
        part->setDrawable(0, builder.drawableGeo().p());
        part->setEffect(eff.p());

        model->addPart(part.p());
    }

    {
        cvf::GeometryBuilderDrawableGeo builder;
        cvf::GeometryUtils::createBox(cvf::Vec3f(5, 0, 0), 2, 3, 4, &builder);

        cvf::ref<cvf::Effect> eff = new cvf::Effect;
        if (useShaders)
        {
            eff->setShaderProgram(shaderProg.p());
            eff->setUniform(new cvf::UniformFloat("u_color", cvf::Color4f(cvf::Color3::YELLOW)));
        }
        else
        {
            eff->setRenderState(new cvf::RenderStateMaterial_FF(cvf::Color3::RED));
        }

        cvf::ref<cvf::Part> part = new cvf::Part;
        part->setName("MyBox");
        part->setDrawable(0, builder.drawableGeo().p());
        part->setEffect(eff.p());

        model->addPart(part.p());
    }

    model->updateBoundingBoxesRecursive();

    cvf::ref<cvf::Scene> scene = new cvf::Scene;
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


