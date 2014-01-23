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

#include "snipEnableMasks.h"

#include "cvfuInputEvents.h"
#include "cvfuPartCompoundGenerator.h"

namespace snip {


const unsigned int redBit   = 0x00000001;
const unsigned int greenBit = 0x00000002;
const unsigned int blueBit  = 0x00000004;
const unsigned int yellowBit= 0x00000008;


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool EnableMasks::onInitialize()
{
    PartCompoundGenerator gen;
    gen.setPartDistribution(Vec3i(5, 5, 5));
    gen.setNumEffects(1);
    gen.useRandomEffectAssignment(false);
    gen.setExtent(Vec3f(3,3,3));
    gen.setOrigin(Vec3f(-1.5f, -1.5f, -1.5f));

    Collection<Part> parts;
    gen.generateSpheres(20, 20, &parts);


    ref<Effect> e1 = new Effect;
    ref<Effect> e2 = new Effect;
    ref<Effect> e3 = new Effect;
    ref<Effect> e4 = new Effect;
    e1->setRenderState(new RenderStateMaterial_FF(RenderStateMaterial_FF::PURE_RED));
    e2->setRenderState(new RenderStateMaterial_FF(RenderStateMaterial_FF::PURE_GREEN));
    e3->setRenderState(new RenderStateMaterial_FF(RenderStateMaterial_FF::PURE_BLUE));
    e4->setRenderState(new RenderStateMaterial_FF(RenderStateMaterial_FF::PURE_YELLOW));

    Collection<Effect> effs;
    effs.push_back(e1.p());
    effs.push_back(e2.p());
    effs.push_back(e3.p());
    effs.push_back(e4.p());
    size_t numEffects = effs.size();

    ref<ModelBasicList> myModel = new ModelBasicList;

    size_t i;
    for (i = 0; i < parts.size(); i++)
    {
        Part* part = parts[i].p();

        size_t effectIdx = i % numEffects;
        Effect* e = effs.at(effectIdx);
        part->setEffect(e);

        unsigned int mask = 0xffffffff;
        if      (effectIdx == 0) mask = redBit;
        else if (effectIdx == 1) mask = greenBit;
        else if (effectIdx == 2) mask = blueBit;
        else if (effectIdx == 3) mask = yellowBit;

        part->setEnableMask(mask);

        myModel->addPart(part);
    }

    myModel->updateBoundingBoxesRecursive();

    m_renderSequence->rendering(0)->scene()->addModel(myModel.p());

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
void EnableMasks::onKeyPressEvent(KeyEvent* keyEvent)
{
    Rendering* rendering = m_renderSequence->firstRendering();
    Scene* scene = rendering ? rendering->scene() : NULL;
    if (!scene) return;

    Model* model = scene->modelCount() > 0 ? scene->model(0) : NULL;
    if (!model) return;

    unsigned int renderingEnableMask = rendering->enableMask();
    unsigned int modelEnableMask = model->partEnableMask();

    Key key = keyEvent->key();
    switch (key)
    {
        case Key_1: modelEnableMask = toggleBit(modelEnableMask, redBit);     break;
        case Key_2: modelEnableMask = toggleBit(modelEnableMask, greenBit);   break;
        case Key_3: modelEnableMask = toggleBit(modelEnableMask, blueBit);    break;
        case Key_4: modelEnableMask = toggleBit(modelEnableMask, yellowBit);  break;
        case Key_5: modelEnableMask = 0xffffffff;                             break;

        case Key_6: renderingEnableMask = toggleBit(renderingEnableMask, redBit);     break;
        case Key_7: renderingEnableMask = toggleBit(renderingEnableMask, greenBit);   break;
        case Key_8: renderingEnableMask = toggleBit(renderingEnableMask, blueBit);    break;
        case Key_9: renderingEnableMask = toggleBit(renderingEnableMask, yellowBit);  break;
        case Key_0: renderingEnableMask = 0xffffffff;                                 break;

        default: break;
    }

    Trace::show("Enable masks set to: modelMask=0x%08x  renderingMask=0x%08x", modelEnableMask, renderingEnableMask);

    model->setPartEnableMask(modelEnableMask);
    rendering->setEnableMask(renderingEnableMask);

    keyEvent->setRequestedAction(REDRAW);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
unsigned int EnableMasks::toggleBit(unsigned int bitfield, unsigned int toggleBit)
{
    if (bitfield & toggleBit)
    {
        bitfield &= ~toggleBit;
    }
    else
    {
        bitfield |= toggleBit;
    }

    return bitfield;
}


} // namespace snip

