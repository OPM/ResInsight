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
#include "cvfLibViewing.h"
#include "cvfLibRender.h"

#include "snipMinimalModel.h"

#include "cvfuInputEvents.h"
#include "cvfuPartCompoundGenerator.h"

namespace snip {


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool MinimalModel::onInitialize()
{
    PartCompoundGenerator gen;
    gen.setPartDistribution(Vec3i(5, 5, 5));
    gen.setNumEffects(8);
    gen.useRandomEffectAssignment(false);
    gen.setExtent(Vec3f(3,3,3));
    gen.setOrigin(Vec3f(-1.5f, -1.5f, -1.5f));

    Collection<Part> parts;
    gen.generateSpheres(20, 20, &parts);

    m_model = new ModelBasicList;

    cvf::ShaderSourceProvider::instance();

    ShaderProgramGenerator spGen("SimpleHeadlight", cvf::ShaderSourceProvider::instance());
    spGen.configureStandardHeadlightColor();
    m_shaderProg  = spGen.generate();

    m_useShaders = false;

    size_t i;
    for (i = 0; i < parts.size(); i++)
    {
        Part* part = parts[i].p();
        ref<Effect> eff = part->effect();

        eff->setShaderProgram(m_useShaders ? m_shaderProg.p() : NULL);

        m_model->addPart(parts[i].p());
    }

    m_model->updateBoundingBoxesRecursive();

    m_renderSequence->firstRendering()->scene()->addModel(m_model.p());

    //m_renderSequence->firstRendering()->setOverlayAxisCross(new OverlayAxisCross(new FixedAtlasFont(FixedAtlasFont::STANDARD)));

    BoundingBox bb = m_model->boundingBox();
    if (bb.isValid())
    {
        m_camera->fitView(bb, Vec3d::Y_AXIS, Vec3d::Z_AXIS);
    }

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void MinimalModel::onKeyPressEvent(KeyEvent* keyEvent)
{
    if (keyEvent->character() == 's')
    {
        m_useShaders = true;
    }
    if (keyEvent->character() == 'f')
    {
        m_useShaders = false;
    }

    Collection<Part> partCollection;
    m_model->allParts(&partCollection);

    size_t i;
    for (i = 0; i < partCollection.size(); i++)
    {
        ref<Part> part = partCollection[i];
        CVF_ASSERT(part.notNull());

        part->effect()->setShaderProgram(m_useShaders ? m_shaderProg.p() : NULL);
    }

    keyEvent->setRequestedAction(REDRAW);
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<cvf::String> MinimalModel::helpText() const
{
    std::vector<cvf::String> helpText;

    helpText.push_back("'s' to use a shader program for rendering");
    helpText.push_back("'f' to use fixed function pipeline for rendering");

    return helpText;
}


} // namespace snip

