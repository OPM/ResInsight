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

#include "snipRenderPriority.h"

namespace snip {


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RenderPriority::onInitialize()
{
    ref<Effect> myEff = new Effect;
    myEff->setRenderState(new RenderStateMaterial_FF(RenderStateMaterial_FF::PURE_YELLOW));
    myEff->setRenderState(new RenderStateDepth(false, RenderStateDepth::LESS, false));

    // Note order in which we create effects.
    // By default this will control the drawing order since by default the snippet has an EFFECT_ONLY sorter that on the pointers
    ref<Effect> e2 = new Effect;
    e2->setRenderState(new RenderStateMaterial_FF(RenderStateMaterial_FF::PURE_GREEN));
    e2->setRenderState(new RenderStateDepth(false, RenderStateDepth::LESS, false));

    ref<Effect> e0 = new Effect;
    e0->setRenderState(new RenderStateMaterial_FF(RenderStateMaterial_FF::PURE_MAGENTA));
    e0->setRenderState(new RenderStateDepth(false, RenderStateDepth::LESS, false));

    ref<Effect> e3 = new Effect;
    e3->setRenderState(new RenderStateMaterial_FF(RenderStateMaterial_FF::PURE_BLUE));
    e3->setRenderState(new RenderStateDepth(false, RenderStateDepth::LESS, false));

    ref<Effect> e1 = new Effect;
    e1->setRenderState(new RenderStateMaterial_FF(RenderStateMaterial_FF::PURE_RED));
    e1->setRenderState(new RenderStateDepth(false, RenderStateDepth::LESS, false));


    ref<ModelBasicList> myModel = new ModelBasicList;

    {
        GeometryBuilderDrawableGeo builder;
        GeometryUtils::createSphere(1, 10, 10, &builder);
        ref<Part> spherePart = new Part;
        spherePart->setDrawable(builder.drawableGeo().p());
        spherePart->setEffect(myEff.p());
        myModel->addPart(spherePart.p());
    }


    {
        GeometryBuilderDrawableGeo builder;
        GeometryUtils::createPatch(Vec3f(0, 0, 0), Vec3f::X_AXIS*3, Vec3f::Y_AXIS*3, 1, 1, &builder);
        ref<Part> part = new Part;
        part->setDrawable(builder.drawableGeo().p());
        part->setEffect(e0.p());
        myModel->addPart(part.p());
    }

    {
        GeometryBuilderDrawableGeo builder;
        GeometryUtils::createPatch(Vec3f(1, 1, 1), Vec3f::X_AXIS*3, Vec3f::Y_AXIS*3, 1, 1, &builder);
        ref<Part> part = new Part;
        part->setDrawable(builder.drawableGeo().p());
        part->setEffect(e1.p());
        myModel->addPart(part.p());
    }

    {
        GeometryBuilderDrawableGeo builder;
        GeometryUtils::createPatch(Vec3f(2, 2, 2), Vec3f::X_AXIS*3, Vec3f::Y_AXIS*3, 1, 1, &builder);
        ref<Part> part = new Part;
        part->setDrawable(builder.drawableGeo().p());
        part->setEffect(e2.p());
        myModel->addPart(part.p());
    }

    {
        GeometryBuilderDrawableGeo builder;
        GeometryUtils::createPatch(Vec3f(3, 3, 3), Vec3f::X_AXIS*3, Vec3f::Y_AXIS*3, 1, 1, &builder);
        ref<Part> part = new Part;
        part->setDrawable(builder.drawableGeo().p());
        part->setEffect(e3.p());
        myModel->addPart(part.p());
    }


    {
        GeometryBuilderDrawableGeo builder;
        GeometryUtils::createSphere(1, 10, 10, &builder);
        ref<Part> spherePart = new Part;
        spherePart->setDrawable(builder.drawableGeo().p());
        spherePart->setEffect(myEff.p());
        myModel->addPart(spherePart.p());
    }


    myModel->updateBoundingBoxesRecursive();

    m_renderSequence->rendering(0)->scene()->addModel(myModel.p());

    BoundingBox bb = myModel->boundingBox();
    if (bb.isValid())
    {
        m_camera->fitView(bb, -Vec3d::Z_AXIS, Vec3d::Y_AXIS);
    }

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderPriority::onKeyPressEvent(KeyEvent* keyEvent)
{
    Rendering* rendering = m_renderSequence->firstRendering();
    ModelBasicList* model = dynamic_cast<ModelBasicList*>(rendering->scene()->model(0));

    int partCount = static_cast<int>(model->partCount());

    Key key = keyEvent->key();
    if (key == Key_R) 
    {
        Trace::show("Set all priorities to 0");
        int i;
        for (i = 0; i < partCount; i++)
        {
            Part* part = model->part(i);
            part->setPriority(0);
        }
    }
    
    else if (key == Key_I)
    {
        Trace::show("Set increasing priority");
        int i;
        for (i = 0; i < partCount; i++)
        {
            Part* part = model->part(i);
            part->setPriority(i);
        }
    }
        
    else if (key == Key_D)
    {
        Trace::show("Set decreasing priority");
        int i;
        for (i = 0; i < partCount; i++)
        {
            Part* part = model->part(i);
            part->setPriority(partCount - i - 1);
        }
    }

    else if (key == Key_S)
    {
		RenderQueueSorterBasic* newSorter = NULL;
		RenderQueueSorterBasic* currSorter = dynamic_cast<RenderQueueSorterBasic*>(rendering->renderQueueSorter());
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

        rendering->setRenderQueueSorter(newSorter);
    }

    keyEvent->setRequestedAction(REDRAW);
}


} // namespace snip

