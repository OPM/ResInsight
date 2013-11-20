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

#include "cvfuInputEvents.h"

#include "snipHiddenLines.h"

namespace snip {


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool HiddenLines::onInitialize()
{
    ref<ModelBasicList> myModel = new ModelBasicList;
    m_renderSequence->firstRendering()->scene()->addModel(myModel.p());
    m_renderSequence->firstRendering()->addOverlayItem(new OverlayAxisCross(m_camera.p(), new FixedAtlasFont(FixedAtlasFont::STANDARD)));


    {
        GeometryBuilderDrawableGeo builder;
        GeometryUtils::createSphere(2, 10, 10, &builder);
        ref<DrawableGeo> geo = builder.drawableGeo();
        addAsHiddenLine(geo.p());
    }

    {
        BoxGenerator gen;
        gen.setCenterAndExtent(Vec3d(1, 1, 0), Vec3d(2, 2, 2));
        gen.setSubdivisions(12, 8, 4);

        GeometryBuilderDrawableGeo builder;
        gen.generate(&builder);

        ref<DrawableGeo> geo = builder.drawableGeo();
        addAsHiddenLine(geo.p());
    }

    {
        BoxGenerator gen;
        gen.setCenterAndExtent(Vec3d(0, 0, 0), Vec3d(1, 1, 1));
        gen.setSubdivisions(3, 3, 3);

        GeometryBuilderDrawableGeo builder;
        gen.generate(&builder);

        ref<DrawableGeo> geo = builder.drawableGeo();

        ref<Effect> eff = new Effect;
        eff->setRenderState(new cvf::RenderStateMaterial_FF(Color3::RED));

        ref<Part> part = new Part;
        part->setDrawable(geo.p());
        part->setEffect(eff.p());

        myModel->addPart(part.p());
    }



    myModel->updateBoundingBoxesRecursive();
    BoundingBox bb = myModel->boundingBox();
    if (bb.isValid())
    {
        m_camera->fitView(bb, Vec3d::Y_AXIS, Vec3d::Z_AXIS);
    }

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void HiddenLines::addAsHiddenLine(DrawableGeo* geo)
{
    ModelBasicList* model = dynamic_cast<ModelBasicList*>(m_renderSequence->firstRendering()->scene()->model(0));

    {
        ref<Effect> eff = new Effect;
        eff->setRenderState(new cvf::RenderStateMaterial_FF(Color3::MAGENTA));
        eff->setRenderState(new cvf::RenderStateLighting_FF(false));
        eff->setRenderState(new cvf::RenderStateColorMask(false));

        cvf::RenderStatePolygonOffset* polyOffset = new cvf::RenderStatePolygonOffset;
        polyOffset->configurePolygonPositiveOffset();
        eff->setRenderState(polyOffset);

        ref<Part> part = new Part;
        part->setPriority(1);
        part->setDrawable(geo);
        part->setEffect(eff.p());

        model->addPart(part.p());
    }

    {
        ref<Effect> eff = new Effect;
        eff->setRenderState(new cvf::RenderStateMaterial_FF(Color3::YELLOW));
        eff->setRenderState(new cvf::RenderStatePolygonMode(cvf::RenderStatePolygonMode::LINE));
        eff->setRenderState(new cvf::RenderStateDepth(true, cvf::RenderStateDepth::LEQUAL));
        eff->setRenderState(new cvf::RenderStateLighting_FF(false));

        ref<Part> part = new Part;
        part->setPriority(2);
        part->setDrawable(geo);
        part->setEffect(eff.p());

        model->addPart(part.p());
    }
}


} // namespace snip

