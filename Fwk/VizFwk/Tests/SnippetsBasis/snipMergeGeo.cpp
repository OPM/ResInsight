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

#include "snipMergeGeo.h"

namespace snip {


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool MergeGeo::onInitialize()
{
    ref<ModelBasicList> myModel = new ModelBasicList;

    // Tri strip
    // -------------------------------------------------------------------------
    ref<DrawableGeo> geo1 = new DrawableGeo;

    ref<PrimitiveSetIndexedUInt> triStrip = new PrimitiveSetIndexedUInt(PT_TRIANGLE_STRIP);

    ref<UIntArray> triStripIndices = new UIntArray;
    triStripIndices->reserve(6);

    triStripIndices->add(0);
    triStripIndices->add(1);
    triStripIndices->add(2);
    triStripIndices->add(3);
    triStripIndices->add(4);
    triStripIndices->add(5);

    triStrip->setIndices(triStripIndices.p());

    ref<Vec3fArray> triStripVertices = new Vec3fArray;
    triStripVertices->reserve(6);

    triStripVertices->add(Vec3f(0,1,0));
    triStripVertices->add(Vec3f(0,0,0));
    triStripVertices->add(Vec3f(1,1,0));
    triStripVertices->add(Vec3f(2.5,0,0));
    triStripVertices->add(Vec3f(3,1,0));
    triStripVertices->add(Vec3f(3.5,-0.5,0));

    geo1->addPrimitiveSet(triStrip.p());
    geo1->setVertexArray(triStripVertices.p());
    geo1->computeNormals();


    // Tri fan
    // -------------------------------------------------------------------------
    ref<DrawableGeo> geo2 = new DrawableGeo;

    ref<PrimitiveSetIndexedUInt> triFan = new PrimitiveSetIndexedUInt(PT_TRIANGLE_FAN);

    ref<UIntArray> triFanIndices = new UIntArray;
    triFanIndices->reserve(5);

    triFanIndices->add(0);
    triFanIndices->add(1);
    triFanIndices->add(2);
    triFanIndices->add(3);
    triFanIndices->add(4);

    triFan->setIndices(triFanIndices.p());

    ref<Vec3fArray> triFanVertices = new Vec3fArray;
    triFanVertices->reserve(5);

    // Note that the example in the RedBook on page 36 (1 ed) leads to triangles with inward normal!
    triFanVertices->add(Vec3f(4,0,0));
    triFanVertices->add(Vec3f(5,-1.5,0));
    triFanVertices->add(Vec3f(5.5,0,0));
    triFanVertices->add(Vec3f(5,1.5,0));
    triFanVertices->add(Vec3f(4,2,0));

    geo2->addPrimitiveSet(triFan.p());
    geo2->setVertexArray(triFanVertices.p());
    geo2->computeNormals();

    Collection<DrawableGeo> myCollection;
    myCollection.push_back(geo1.p());
    myCollection.push_back(geo2.p());

    ref<DrawableGeo> geo4 = new DrawableGeo;
    geo4->mergeInto(myCollection);

    ref<UIntArray> faceList = geo4->toFaceList();
    
    const Vec3fArray* vertexArray = geo4->vertexArray();
    ref<Vec3fArray> vertices = new Vec3fArray(*vertexArray);

    ref<DrawableGeo> geo5 = new DrawableGeo;
    geo5->setVertexArray(vertices.p());
    geo5->setFromFaceList(*faceList);
    geo5->computeNormals();


    ref<Part> part5 = new Part;
    part5->setDrawable(geo5.p());

    ref<Effect> eff5 = new Effect;
    eff5->setRenderState(new RenderStateMaterial_FF(Color3f(0, 0, 1)));

    part5->setEffect(eff5.p());
    myModel->addPart(part5.p());



    // Finally, update the model and and add it to the scene
    // -------------------------------------------------------------------------
    myModel->updateBoundingBoxesRecursive();

    m_renderSequence->rendering(0)->scene()->addModel(myModel.p());

//    m_camera->setFromLookAt(Vec3d(3,0,10), Vec3d(3,0,0), Vec3d(0,1,0));
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
std::vector<String> MergeGeo::helpText() const
{
    std::vector<String> defaultHelpText;
    defaultHelpText.push_back("Snippet used to show result of a merged geometry.\nReset camera position by pressing 'A'.");

    return defaultHelpText;
}


} // namespace snip

