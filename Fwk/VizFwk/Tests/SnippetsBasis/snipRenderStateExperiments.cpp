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
#include "cvfLibGeometry.h"
#include "cvfLibRender.h"
#include "cvfLibViewing.h"

#include "cvfuInputEvents.h"

#include "snipRenderStateExperiments.h"

namespace snip {


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RenderStateExperiments::onInitialize()
{
    ref<RenderStateMaterial_FF> matRed = new RenderStateMaterial_FF(RenderStateMaterial_FF::RED_RUBBER);
    ref<RenderStateMaterial_FF> matYellow = new RenderStateMaterial_FF(RenderStateMaterial_FF::YELLOW_PLASTIC);

    ref<Effect> effRed = new Effect;
    ref<Effect> effYellow = new Effect;

    effRed->setRenderState(matRed.p());
    effYellow->setRenderState(matYellow.p());

    ref<ModelBasicList> myModel = new ModelBasicList;

    int numPartsX = 30;
    int numPartsY = 30;
    int x;
    for (x = 0; x < numPartsX; x++)
    {
        int y;
        for (y = 0; y < numPartsY; y++)
        {
            ref<Part> part = createBoxPart(Vec3f(2.0f*static_cast<float>(x), 2.0f*static_cast<float>(y), 0.0f));
            //ref<Part> part = createTrianglePart(Vec3f(2.0f*x, 2.0f*y, 0));

            ref<Effect> useEff = (y % 2 == 0) ? effRed : effYellow;
            part->setEffect(useEff.p());

            myModel->addPart(part.p());
        }
    }


    bool addDisc = true;
    if (addDisc)
    {
        ref<RenderStateDepth> depth = new RenderStateDepth;
        depth->setFunction(RenderStateDepth::ALWAYS);
        
        ref<RenderStateMaterial_FF> mat = new RenderStateMaterial_FF;
        mat->setAmbientAndDiffuse(Color3f(0, 0, 0.8f));

        ref<RenderStateLighting_FF> lighting = new RenderStateLighting_FF;
        lighting->enableTwoSided(true);

        ref<Effect> eff = new Effect;
        eff->setRenderState(depth.p());
        eff->setRenderState(mat.p());
        eff->setRenderState(lighting.p());


        m_xyDiscPart = createXYDiscPart(10);
        m_xyDiscPart->setEffect(eff.p());

        myModel->addPart(m_xyDiscPart.p());
    }


    myModel->updateBoundingBoxesRecursive();
    m_renderSequence->firstRendering()->scene()->addModel(myModel.p());

    BoundingBox bb = m_renderSequence->boundingBox();
    if (bb.isValid())
    {
        m_camera->fitView(bb, -Vec3d::Z_AXIS, Vec3d::X_AXIS);
    }

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<Part> RenderStateExperiments::createTrianglePart(Vec3f centPos)
{
    ref<Vec3fArray> vertices = new Vec3fArray;
    vertices->reserve(3);
    vertices->add(centPos + Vec3f(-0.5f, -0.5, 0));
    vertices->add(centPos + Vec3f( 0.5f, -0.5, 0));
    vertices->add(centPos + Vec3f( 0.0f,  0.5, 0));

    UIntArray faceList;
    faceList.reserve(4);
    faceList.add(3);
    faceList.add(0);
    faceList.add(1);
    faceList.add(2);

    ref<DrawableGeo> geo = new DrawableGeo;
    geo->setVertexArray(vertices.p());
    geo->setFromFaceList(faceList);
    geo->computeNormals();

    ref<Part> part = new Part;
    part->setDrawable(geo.p());
    part->updateBoundingBox();

    return part;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<Part> RenderStateExperiments::createBoxPart(Vec3f centPos)
{
    ref<DrawableGeo> geo = createBoxGeo(centPos);

    ref<Part> part = new Part;
    part->setDrawable(geo.p());
    part->updateBoundingBox();

    return part;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<DrawableGeo> RenderStateExperiments::createBoxGeo(Vec3f centPos)
{
    GeometryBuilderFaceList builder;
    GeometryUtils::createBox(centPos, 1.0, 1.0, 1.0, &builder);

    ref<Vec3fArray> vertices = builder.vertices();
    ref<UIntArray> faceList = builder.faceList();

    ref<DrawableGeo> geo = new DrawableGeo;
    geo->setVertexArray(vertices.p());
    geo->setFromFaceList(*faceList);
    geo->computeNormals();

    return geo;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<Part> RenderStateExperiments::createXYDiscPart(double radius)
{
    GeometryBuilderFaceList builder;
    GeometryUtils::createDisc(radius, 50, &builder);

    ref<Vec3fArray> vertices = builder.vertices();
    ref<UIntArray> faceList = builder.faceList();

    ref<DrawableGeo> geo = new DrawableGeo;
    geo->setVertexArray(vertices.p());
    geo->setFromFaceList(*faceList);
    geo->computeNormals();

    ref<Part> part = new Part;
    part->setDrawable(geo.p());
    part->updateBoundingBox();

    return part;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderStateExperiments::addRenderingWithEdges()
{
    Scene* srcScene = m_renderSequence->rendering(0)->scene();
    CVF_ASSERT(srcScene);

    Collection<Part> srcParts;
    srcScene->allParts(&srcParts);

    ref<ModelBasicList> newModel = new ModelBasicList;

    ref<RenderStatePolygonOffset> polyOffset = new RenderStatePolygonOffset;
    polyOffset->configureLineNegativeOffset();

    ref<Effect> lineEffect = new Effect;
    lineEffect->setRenderState(new RenderStatePolygonMode(RenderStatePolygonMode::LINE));
    lineEffect->setRenderState(polyOffset.p());
    lineEffect->setRenderState(new RenderStateDepth(true, RenderStateDepth::LEQUAL));

    ref<RenderStateMaterial_FF> lineMat = new RenderStateMaterial_FF;
    lineMat->setAmbientAndDiffuse(Color3f::MAGENTA);
    lineEffect->setRenderState(lineMat.p());
    lineEffect->setRenderState(new RenderStateLighting_FF(false));

    size_t numSrcParts = srcParts.size();
    size_t i;
    for (i = 0; i < numSrcParts; i++)
    {
        Part* srcPart = srcParts.at(i);
        ref<Part> newPart = new Part;
        newPart->setDrawable(srcPart->drawable(0));
        newPart->setEffect(lineEffect.p());

        newModel->addPart(newPart.p());
    }

    newModel->updateBoundingBoxesRecursive();

    ref<Scene> newScene = new Scene;
    newScene->addModel(newModel.p());

    ref<Rendering> newRendering = new Rendering;
    newRendering->setCamera(m_camera.p());
    newRendering->setScene(newScene.p());
    newRendering->setClearMode(Viewport::DO_NOT_CLEAR);

    m_renderSequence->addRendering(newRendering.p());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderStateExperiments::deleteAllRenderingsExceptFirst()
{
    ref<Rendering> firstRendering = m_renderSequence->rendering(0);

    m_renderSequence->removeAllRenderings();
    m_renderSequence->addRendering(firstRendering.p());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderStateExperiments::onResizeEvent(int width, int height)
{
    if (m_renderSequence.isNull() || m_camera.isNull())
    {
        return;
    }

    m_camera->setViewport(0, 0, width, height);

//     BoundingBox bb = m_renderSequence->boundingBox();
//     if (bb.isValid())
//     {
//         m_camera->setClipPlanesFromBoundingBox(bb);
//     }

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderStateExperiments::onKeyPressEvent(KeyEvent* keyEvent)
{
    Key key = keyEvent->key();
    char character = keyEvent->character();

    if (key == Key_D)
    {
        bool depthTestOn = (character == 'd') ? true : false;
    
        CVF_ASSERT(m_xyDiscPart->effect());
        
        RenderStateDepth* depth = static_cast<RenderStateDepth*>(m_xyDiscPart->effect()->renderStateOfType(RenderState::DEPTH));
        if (depth)
        {
            depth->setFunction(depthTestOn ? RenderStateDepth::LESS : RenderStateDepth::ALWAYS);
        }
    }

    else if (key == Key_E)
    {
        bool edgesOn = (character == 'e') ? true : false;

        if (edgesOn)    addRenderingWithEdges();
        else            deleteAllRenderingsExceptFirst();
    }

    else if (key == Key_S)
    {
        int numRenderings = m_renderSequence->renderingCount();
        if (numRenderings < 1) return;


		RenderQueueSorterBasic* newSorter = NULL;
		RenderQueueSorterBasic* currSorter = dynamic_cast<RenderQueueSorterBasic*>(m_renderSequence->rendering(0)->renderQueueSorter());
		if (currSorter) 
		{
			RenderQueueSorterBasic::SortStrategy currStrategy = currSorter->strategy();
			if		(currStrategy == RenderQueueSorterBasic::MINIMAL)		newSorter = new RenderQueueSorterBasic(RenderQueueSorterBasic::EFFECT_ONLY);  
			else if (currStrategy == RenderQueueSorterBasic::EFFECT_ONLY)	newSorter = new RenderQueueSorterBasic(RenderQueueSorterBasic::STANDARD);  
			else if (currStrategy == RenderQueueSorterBasic::STANDARD)		newSorter = NULL;
		}
		else
		{
			// No current sorter, start with minimal
			newSorter = new RenderQueueSorterBasic(RenderQueueSorterBasic::MINIMAL);
		}

		if (newSorter)
		{
			RenderQueueSorterBasic::SortStrategy newStrategy = newSorter->strategy();
			if		(newStrategy == RenderQueueSorterBasic::MINIMAL)		Trace::show("Setting sorter to: MINIMAL");
			else if (newStrategy == RenderQueueSorterBasic::EFFECT_ONLY)	Trace::show("Setting sorter to: EFFECT_ONLY");
			else if (newStrategy == RenderQueueSorterBasic::STANDARD)		Trace::show("Setting sorter to: STANDARD");
		}
		else
		{
			Trace::show("Setting sorter to: NONE/NULL");
		}

        int i;
        for (i = 0; i < numRenderings; i++)
        {
            Rendering* rendering = m_renderSequence->rendering(i);
            rendering->setRenderQueueSorter(newSorter);
        }
    }

    keyEvent->setRequestedAction(REDRAW);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<cvf::String> RenderStateExperiments::helpText() const
{
    std::vector<String> help;
    help.push_back(String("d/D - to toggle between depth func LESS and ALWAYS"));
    help.push_back(String("e/E - to toggle between edges on and edges off"));
    help.push_back(String("s   - cycle render queue sorter"));

    return help;
}


} // namespace snip

