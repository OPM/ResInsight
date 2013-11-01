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
#include "cvfLibViewing.h"

#include "snipOcclusionQuery.h"

#include "cvfuPartCompoundGenerator.h"
#include "cvfuInputEvents.h"

namespace snip {


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
OcclusionQuery::OcclusionQuery()
{
    m_continousOcclusion = false;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool OcclusionQuery::onInitialize()
{
    PartCompoundGenerator gen;
    //gen.setPartDistribution(Vec3i(50, 50, 10));
    gen.setPartDistribution(Vec3i(5, 5, 10));
    gen.setNumDrawableGeos(1);
    gen.setNumEffects(1);
    gen.useRandomEffectAssignment(false);
    gen.setExtent(Vec3f(3,3,3));
    gen.setOrigin(Vec3f(-1.5f, -1.5f, -1.5f));

    Collection<Part> parts;
    gen.generateSpheres(20, 20, &parts);

    ref<ModelBasicList> myModel = new ModelBasicList;

    size_t i;
    for (i = 0; i < parts.size(); i++)
    {
        myModel->addPart(parts[i].p());
    }

    myModel->updateBoundingBoxesRecursive();

    m_view->rendering(0)->scene()->addModel(myModel.p());

    BoundingBox bb = myModel->boundingBox();
    if (bb.isValid())
    {
        m_camera->fitView(bb, -Vec3d::Z_AXIS, Vec3d::Y_AXIS);
    }

    m_mainRendering = m_view->rendering(0);

    createAllEffects();
    createResultRenderingFromMainRendering();

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OcclusionQuery::onPaintEvent(PostEventAction* postEventAction)
{
    TestSnippet::onPaintEvent(postEventAction);

    if (m_continousOcclusion)
    {
        updateResultRenderingWithOcclusionResults(true, false);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OcclusionQuery::onResizeEvent(int width, int height)
{
    if (m_view.isNull() || m_camera.isNull())
    {
        return;
    }

    m_camera->setViewport(0, 0, width, height);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OcclusionQuery::onKeyPressEvent(KeyEvent* keyEvent)
{
    Key key = keyEvent->key();

    m_continousOcclusion = false;

    if (key == Key_M)
    {
        showOnlyMainRendering();
        Trace::show("Show Main rendering");
    }

    else if (key == Key_R)
    {
        if (!m_continousOcclusion)
        {
            updateResultRenderingWithOcclusionResults(true, true);
        }
        showOnlyResultRendering();
        Trace::show("Show Result rendering");
    }

    else if (key == Key_O)
    {
        Trace::show("Starting per frame OCCLUSION");
        setupForOcclusion();
    }

    else if (key == Key_C)
    {
        Trace::show("Starting continuous OCCLUSION");
        setupForOcclusion();
        updateResultRenderingSetAllOccluded();
        m_continousOcclusion = true;
    }

    keyEvent->setRequestedAction(REDRAW);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<cvf::String> OcclusionQuery::helpText() const
{
    std::vector<String> help;
    help.push_back(String("m - show Main rendering"));
    help.push_back(String("r - show Result rendering"));
    help.push_back(String("o - start per frame occlusion"));
    help.push_back(String("c - start continuous occlusion"));

    return help;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OcclusionQuery::createAllEffects()
{
    {
        ref<Effect> eff = new Effect;
        eff->setRenderState(new Material_FF(Material_FF::GREEN_PLASTIC));

        m_effResultVisible = eff;
    }
    
    {
        ref<Effect> eff = new Effect;
        eff->setRenderState(new Material_FF(Material_FF::RED_PLASTIC));

        m_effResultOccluded = eff;
    }

    {
        ref<Effect> eff = new Effect;
        eff->setRenderState(new Material_FF(Material_FF::PURE_MAGENTA));

        m_effOcclusionPass1 = eff;
    }

    {
        ref<Effect> eff = new Effect;
        eff->setRenderState(new Material_FF(Material_FF::PURE_WHITE));
        eff->setRenderState(new Depth(true, Depth::LEQUAL));

        m_effOcclusionPass2 = eff;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OcclusionQuery::createResultRenderingFromMainRendering()
{
    Scene* srcScene = m_mainRendering->scene();
    CVF_ASSERT(srcScene);

    ref<Scene> newScene = createSingleModelCopyOfScene(srcScene, m_effResultVisible.p());

    ref<Rendering> newRendering = new Rendering;
    newRendering->setCamera(m_camera.p());
    newRendering->setScene(newScene.p());

    deleteAllOpenGLResourcesInRendering(m_openGLContext.p(), m_resultRendering.p());
    m_resultRendering = newRendering;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OcclusionQuery::updateResultRenderingSetAllOccluded()
{
    Collection<Part> resultParts;
    m_resultRendering->scene()->allParts(&resultParts);

    size_t numParts = resultParts.size();

    size_t i;
    for (i = 0; i < numParts; i++)
    {
        Part* resPart = resultParts.at(i);
        resPart->setEffect(m_effResultOccluded.p());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OcclusionQuery::updateResultRenderingWithOcclusionResults(bool updateVisible, bool updateOccluded)
{
    Collection<Part> occlusionParts;
    m_occlusionPass2->scene()->allParts(&occlusionParts);

    Collection<Part> resultParts;
    m_resultRendering->scene()->allParts(&resultParts);

    size_t numParts = occlusionParts.size();
    CVF_ASSERT(numParts == resultParts.size());

    size_t i;
    for (i = 0; i < numParts; i++)
    {
        Part* occPart = occlusionParts.at(i);
        Part* resPart = resultParts.at(i);

        if (occPart->occlusionResultNumFragments() > 0)
        {
            if (updateVisible)
            {
                resPart->setEffect(m_effResultVisible.p());
            }
        }
        else
        {
            if (updateOccluded)
            {
                resPart->setEffect(m_effResultOccluded.p());
            }
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<Scene> OcclusionQuery::createSingleModelCopyOfScene(Scene* srcScene, Effect* overrideEffect)
{
    Collection<Part> srcParts;
    srcScene->allParts(&srcParts);

    ref<ModelBasicList> newModel = new ModelBasicList;

    size_t numSrcParts = srcParts.size();
    size_t i;
    for (i = 0; i < numSrcParts; i++)
    {
        Part* srcPart = srcParts.at(i);
        ref<Part> newPart = new Part;
        newPart->setDrawable(srcPart->drawable(0));
        newPart->setTransform(srcPart->transform());

        if (overrideEffect)
        {
            newPart->setEffect(overrideEffect);
        }
        else
        {
            newPart->setEffect(srcPart->effect());
        }
        

        newModel->addPart(newPart.p());
    }

    newModel->updateBoundingBoxesRecursive();

    ref<Scene> newScene = new Scene;
    newScene->addModel(newModel.p());

    return newScene;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OcclusionQuery::deleteAllOpenGLResourcesInRendering(OpenGLContext* oglContext, Rendering* rendering)
{
    Scene* scene = rendering ? rendering->scene() : NULL;
    int numModels = scene ? scene->modelCount() : 0;

    int i;
    for (i = 0; i < numModels; i++)
    {
        Model* model = scene->model(i);
        model->deleteOrReleaseOpenGLResources(oglContext);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OcclusionQuery::showOnlyMainRendering()
{
    m_view->deleteOrReleaseOpenGLResources(m_openGLContext.p());
    m_view->removeAllRenderings();
    m_view->addRendering(m_mainRendering.p());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OcclusionQuery::showOnlyResultRendering()
{
    m_view->deleteOrReleaseOpenGLResources(m_openGLContext.p());
    m_view->removeAllRenderings();

    if (m_resultRendering.notNull())
    {
        m_view->addRendering(m_resultRendering.p());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OcclusionQuery::setupForOcclusion()
{
    Scene* srcScene = m_mainRendering->scene();
    CVF_ASSERT(srcScene);

    ref<Scene> newScene1 = createSingleModelCopyOfScene(srcScene, m_effOcclusionPass1.p());
    ref<Scene> newScene2 = createSingleModelCopyOfScene(srcScene, m_effOcclusionPass2.p());

    {
        Collection<Part> parts;
        newScene2->allParts(&parts);

        size_t numParts = parts.size();
        size_t i;
        for (i = 0; i < numParts; i++)
        {
            Part* part = parts.at(i);
            part->setOccludee(true);
        }
    }

    deleteAllOpenGLResourcesInRendering(m_openGLContext.p(), m_occlusionPass1.p());
    deleteAllOpenGLResourcesInRendering(m_openGLContext.p(), m_occlusionPass2.p());

    if (m_occlusionPass1.isNull())
    {
        m_occlusionPass1 = new Rendering;
    }

    if (m_occlusionPass2.isNull())
    {
        m_occlusionPass2 = new Rendering;
    }

    m_occlusionPass1->setScene(newScene1.p());
    m_occlusionPass2->setScene(newScene2.p());

    m_occlusionPass1->setCamera(m_camera.p());
    m_occlusionPass2->setCamera(m_camera.p());

    m_occlusionPass1->cullSettings()->enableViewFrustumCulling(false);
    m_occlusionPass1->cullSettings()->enablePixelSizeCulling(false);
    m_occlusionPass2->cullSettings()->enableViewFrustumCulling(false);
    m_occlusionPass2->cullSettings()->enablePixelSizeCulling(false);

    m_occlusionPass2->setClearMode(Viewport::DO_NOT_CLEAR);
    m_occlusionPass2->renderEngine()->enableOcclusionQuery(true);

    m_view->deleteOrReleaseOpenGLResources(m_openGLContext.p());
    m_view->removeAllRenderings();
    m_view->addRendering(m_occlusionPass1.p());
    m_view->addRendering(m_occlusionPass2.p());
}


} // namespace snip

