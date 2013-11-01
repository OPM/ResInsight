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

#include "snipPartMerger.h"

#include "cvfuPartCompoundGenerator.h"

namespace snip {


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool PartMerger::onInitialize()
{
    PartCompoundGenerator gen;
    gen.setPartDistribution(Vec3i(10, 10, 1));
    gen.setNumEffects(8);
    gen.useRandomEffectAssignment(false);
    gen.setExtent(Vec3f(3,3,3));
    gen.setOrigin(Vec3f(-1.5f, -1.5f, -1.5f));
    gen.setNumDrawableGeos(10);

    Collection<Part> parts;
    gen.generateSpheres(7, 7, &parts);

    gen.setPartDistribution(Vec3i(5, 1, 1));
    gen.generateBoxes(&parts);


    ref<ModelBasicTree> myModel = new ModelBasicTree;
    ModelBasicTreeNode* root =  new ModelBasicTreeNode;

    size_t i = 0;
    myModel->setRoot(root);
    root->addPart(parts[i++].p());

    ModelBasicTreeNode* c11 =  new ModelBasicTreeNode;
    root->addChild(c11);
    c11->addPart(parts[i++].p());

    ModelBasicTreeNode* c12 =  new ModelBasicTreeNode;
    root->addChild(c12);
    c12->addPart(parts[i++].p());
    c12->addPart(parts[i++].p());
    c12->addPart(parts[i++].p());
    c12->addPart(parts[i++].p());

    ModelBasicTreeNode* c21 =  new ModelBasicTreeNode;
    c11->addChild(c21);

    size_t j;
    for (j = i; j < parts.size(); j++)
    {
        c21->addPart(parts[j].p());
    }

    myModel->updateBoundingBoxesRecursive();

//     ModelStatistics stats;
//     stats = myModel->computeStatistics();
//     stats.showLog("before merge");
// 
    myModel->mergeParts(2, 200000);

//     stats = myModel->computeStatistics();
//     stats.showLog("after merge");

    myModel->updateBoundingBoxesRecursive();
    m_renderSequence->rendering(0)->scene()->addModel(myModel.p());

    BoundingBox bb = myModel->boundingBox();
    if (bb.isValid())
    {
        m_camera->fitView(bb, -Vec3d::Z_AXIS, Vec3d::Y_AXIS);
    }

    return true;
}


} // namespace snip

