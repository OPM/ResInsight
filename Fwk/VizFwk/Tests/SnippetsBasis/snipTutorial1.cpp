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

#include "snipTutorial1.h"

namespace snip {


//==================================================================================================
//
// MockupQuadModel
//
//==================================================================================================
Tutorial1::MockupQuadModel::MockupQuadModel(cvf::uint numPointsX, cvf::uint numPointsY)
{
    CVF_ASSERT(numPointsX >= 2 && numPointsY >= 2);
    cvf::uint numVertices = numPointsX*numPointsY;
    cvf::uint numQuads = (numPointsX - 1)*(numPointsY - 1);
    vertices.reserve(numVertices);
    quadIndices.reserve(4*numQuads);

    cvf::uint y;
    for (y = 0; y < numPointsY; y++)
    {
        cvf::uint x;
        for (x = 0; x < numPointsX; x++)
        {
            Vec3f v(static_cast<float>(x), static_cast<float>(y), Math::sin(static_cast<float>(x)/2.0f));
            vertices.add(v);

            if (y > 0 && x > 0)
            {
                quadIndices.add(x-1 + (y-1)*numPointsX);
                quadIndices.add(x   + (y-1)*numPointsX);
                quadIndices.add(x   +     y*numPointsX);
                quadIndices.add(x-1 +     y*numPointsX);
            }
        }
    }

    // Create some vertex scalar results
    {
        vertexScalars.resize(numVertices);
        cvf::uint i;
        for (i = 0; i < numVertices; i++)
        {
            const Vec3f& v = vertices[i];
            vertexScalars[i] = v.length();
        }
    }

    // and some element scalar results
    {
        elementScalars.resize(numQuads);
        cvf::uint q;
        for (q = 0; q < numQuads; q++)
        {
            Vec3f centroid(0, 0, 0);
            cvf::uint i;
            for (i = 0; i < 4; i++)
            {
                centroid += vertices[quadIndices[4*q + i]];
            }
            centroid /= 4;
            elementScalars[q] = centroid.length();
        }
    }

    // Finally some vector results
    {
        vectorResults.resize(numVertices);
        cvf::uint i;
        for (i = 0; i < numVertices; i++)
        {
            Vec3f v(0.1f, 0.3f, 3.0f);
            vectorResults[i] = v;
        }
    }
}



//==================================================================================================
//
// Tutorial1
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool Tutorial1::onInitialize()
{
    m_mockupModel = new MockupQuadModel(30, 20);

    m_myModel = new ModelBasicList;

    // Dummy
    {
        GeometryBuilderDrawableGeo builder;
        GeometryUtils::createBox(Vec3f(0,0,0), 2.0, 2.0, 2.0, &builder);
        ref<DrawableGeo> geo = builder.drawableGeo();

        ref<Effect> eff = new Effect;
        eff->setRenderState(new RenderStateMaterial_FF(Color3f(0, 1, 0)));

        ref<Part> part = new Part;
        part->setDrawable(geo.p());
        part->setEffect(eff.p());
        m_myModel->addPart(part.p());
    }


    m_myModel->updateBoundingBoxesRecursive();
    m_renderSequence->firstRendering()->scene()->addModel(m_myModel.p());

    BoundingBox bb = m_myModel->boundingBox();
    m_camera->fitView(bb, Vec3d(0, 0, -1), Vec3d(0, 1, 0));

    return true;
}




} // namespace snip

