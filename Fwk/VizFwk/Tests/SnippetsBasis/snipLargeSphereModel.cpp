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

#include "snipLargeSphereModel.h"

namespace snip {


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool LargeSphereModel::onInitialize()
{
    ref<DrawableGeo> geo1 = new DrawableGeo;
    {
        GeometryBuilderFaceList builder;
        GeometryUtils::createSphere(2, 2000, 2000, &builder);

        ref<Vec3fArray> vertices = builder.vertices();
        ref<UIntArray> faceList = builder.faceList();

        geo1->setVertexArray(vertices.p());
        geo1->setFromFaceList(*faceList);
        geo1->computeNormals();
    }

    ref<Part> part1 = new Part;
    part1->setDrawable(geo1.p());

    ref<Effect> eff1 = new Effect;
    eff1->setRenderState(new RenderStateMaterial_FF(Color3f(1, 0, 0)));

    part1->setEffect(eff1.p());

    ref<ModelBasicList> myModel = new ModelBasicList;
    myModel->addPart(part1.p());
    myModel->updateBoundingBoxesRecursive();

    m_renderSequence->rendering(0)->scene()->addModel(myModel.p());

    BoundingBox bb = myModel->boundingBox();
    if (bb.isValid())
    {
        m_camera->fitView(bb, Vec3d::Y_AXIS, Vec3d::Z_AXIS);
    }

    return true;
}


} // namespace snip

