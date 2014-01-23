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

#include "cvfVertexAttribute.h"
#include "snipVertexAttributes.h"

namespace snip {


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool VertexAttributes::onInitialize()
{
    ref<Vec3fArray> vertices = new Vec3fArray;
    {
        vertices->reserve(4);
        vertices->add(Vec3f(0, 0, 0));
        vertices->add(Vec3f(2, 0, 0));
        vertices->add(Vec3f(2, 1, 0));
        vertices->add(Vec3f(0, 1, 0));
    }

    ref<PrimitiveSetIndexedUInt> primSet = new PrimitiveSetIndexedUInt(PT_TRIANGLES);
    {
        ref<UIntArray> indices = new UIntArray;
        indices->reserve(6);
        indices->add(0);  indices->add(1);  indices->add(2);
        indices->add(0);  indices->add(2);  indices->add(3);

        primSet->setIndices(indices.p());
    }

    ref<VertexAttribute> yDisplacementAttribArray;
    {
        ref<FloatArray> yDisplacement = new FloatArray;
        yDisplacement->reserve(4);
        yDisplacement->add(0);
        yDisplacement->add(0.4f);
        yDisplacement->add(-0.4f);
        yDisplacement->add(0);
        yDisplacementAttribArray = new FloatVertexAttribute("a_yDisplacement", yDisplacement.p());
    }

    ref<VertexAttribute> colorAttribArray;
    {
        ref<Color3ubArray> colors = new Color3ubArray;
        colors->reserve(4);
        colors->add(Color3ub::RED);
        colors->add(Color3ub::CYAN);
        colors->add(Color3ub::BLUE);
        colors->add(Color3ub::GREEN);
        colorAttribArray = new Color3ubVertexAttributePure("a_color", colors.p());
    }

    ref<DrawableGeo> geo = new DrawableGeo;
    geo->setVertexArray(vertices.p());
    geo->setVertexAttribute(yDisplacementAttribArray.p());
    geo->setVertexAttribute(colorAttribArray.p());
    geo->addPrimitiveSet(primSet.p());
    geo->computeNormals();

    // We can now use BOs for attributes
    geo->setRenderMode(DrawableGeo::BUFFER_OBJECT);

    
    ShaderProgramGenerator gen("MyShaderProg", ShaderSourceProvider::instance());
    gen.addVertexCodeFromFile("VertexAttributes_Vert");
    gen.addFragmentCodeFromFile("VertexAttributes_Frag");

    ref<Effect> eff = new Effect;
    eff->setShaderProgram(gen.generate().p());

    ref<Part> part = new Part;
    part->setDrawable(geo.p());
    part->setEffect(eff.p());


    ref<ModelBasicList> myModel = new ModelBasicList;
    myModel->addPart(part.p());

    myModel->updateBoundingBoxesRecursive();
    m_renderSequence->firstRendering()->scene()->addModel(myModel.p());

    BoundingBox bb = myModel->boundingBox();
    if (bb.isValid())
    {
        m_camera->fitView(bb, -Vec3d::Z_AXIS, Vec3d::Y_AXIS);
    }

    return true;
}



} // namespace snip

