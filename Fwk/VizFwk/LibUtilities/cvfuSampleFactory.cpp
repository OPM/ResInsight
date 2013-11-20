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


#include "cvfBase.h"
#include "cvfuSampleFactory.h"
#include "cvfCamera.h"
#include "cvfScene.h"
#include "cvfPrimitiveSetIndexedUShort.h"
#include "cvfArray.h"
#include "cvfModelBasicList.h"
#include "cvfSampler.h"
#include "cvfDrawableGeo.h"
#include "cvfEffect.h"
#include "cvfShaderProgram.h"
#include "cvfShaderSourceProvider.h"
#include "cvfGeometryBuilderDrawableGeo.h"
#include "cvfGeometryUtils.h"
#include "cvfUniform.h"
#include "cvfShaderProgramGenerator.h"
#include "cvfRenderStateTextureBindings.h"

namespace cvfu {

using cvf::ref;
using cvf::Vec3f;
using cvf::Vec2f;
using cvf::Vec3d;


//==================================================================================================
///
/// \class cvfu::SampleFactory
/// \ingroup Utilities
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<cvf::Part> SampleFactory::createTexturedQuad(cvf::Texture* texture, float aspectRatio)
{
    ref<cvf::Sampler> sampler = new cvf::Sampler;
    sampler->setWrapMode(cvf::Sampler::CLAMP_TO_EDGE);
    sampler->setMinFilter(cvf::Sampler::NEAREST);
    sampler->setMagFilter(cvf::Sampler::NEAREST);

    return createTexturedQuad(texture, sampler.p(), aspectRatio);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Part> SampleFactory::createTexturedQuad(cvf::Texture* texture, cvf::Sampler* sampler, float aspectRatio)
{
    CVF_ASSERT(texture);
    CVF_ASSERT(sampler);

    ref<cvf::PrimitiveSetIndexedUShort> quad = new cvf::PrimitiveSetIndexedUShort(cvf::PT_TRIANGLES);

    ref<cvf::UShortArray> triIndices = new cvf::UShortArray;
    triIndices->reserve(6);

    triIndices->add(0);
    triIndices->add(1);
    triIndices->add(2);
    triIndices->add(0);
    triIndices->add(2);
    triIndices->add(3);

    quad->setIndices(triIndices.p());

    ref<cvf::Vec3fArray> quadVerts = new cvf::Vec3fArray;
    quadVerts->reserve(4);

    float scaleX = 1.0f;
    float scaleY = 1.0f;

    if (aspectRatio > 1.0)
    {
        scaleY /= aspectRatio;
    }

    if (aspectRatio < 1.0)
    {
        scaleX *= aspectRatio;
    }

    quadVerts->add(Vec3f(0,0,0));
    quadVerts->add(Vec3f(1*scaleX,0,0));
    quadVerts->add(Vec3f(1*scaleX,1*scaleY,0));
    quadVerts->add(Vec3f(0,1*scaleY,0));

    ref<cvf::Vec2fArray> textureCoords = new cvf::Vec2fArray;
    textureCoords->reserve(4);

    textureCoords->add(Vec2f(0,0));
    textureCoords->add(Vec2f(1,0));
    textureCoords->add(Vec2f(1,1));
    textureCoords->add(Vec2f(0,1));

    ref<cvf::DrawableGeo> geo1 = new cvf::DrawableGeo;
    geo1->addPrimitiveSet(quad.p());
    geo1->setVertexArray(quadVerts.p());
    geo1->setTextureCoordArray(textureCoords.p());
    geo1->computeNormals();

    ref<cvf::Part> part1 = new cvf::Part;
    part1->setDrawable(geo1.p());

    ref<cvf::Effect> eff1 = new cvf::Effect;
    part1->setEffect(eff1.p());

    // Setup the texture binding render state
    ref<cvf::RenderStateTextureBindings> textureBindings = new cvf::RenderStateTextureBindings;
    textureBindings->addBinding(texture, sampler, "u_texture2D");
    eff1->setRenderState(textureBindings.p());

    cvf::ShaderProgramGenerator gen("TexturedQuad", cvf::ShaderSourceProvider::instance());
    gen.addVertexCode(cvf::ShaderSourceRepository::vs_Standard);
    gen.addFragmentCode(cvf::ShaderSourceRepository::src_Texture);
    gen.addFragmentCode(cvf::ShaderSourceRepository::fs_Unlit);
    ref<cvf::ShaderProgram> prog = gen.generate();
    eff1->setShaderProgram(prog.p());

    part1->updateBoundingBox();

    return part1;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Part> SampleFactory::createUnlitSphere(double radius, cvf::Color3f color)
{
    cvf::GeometryBuilderDrawableGeo builder;
    cvf::GeometryUtils::createSphere(radius, 10, 10, &builder);
    ref<cvf::DrawableGeo> geo = builder.drawableGeo();
    geo->computeNormals();

    ref<cvf::Part> part = new cvf::Part;
    part->setDrawable(geo.p());

    cvf::ShaderProgramGenerator gen("UnlitSphere", cvf::ShaderSourceProvider::instance());
    gen.addVertexCode(cvf::ShaderSourceRepository::vs_Minimal);
    gen.addFragmentCode(cvf::ShaderSourceRepository::src_Color);
    gen.addFragmentCode(cvf::ShaderSourceRepository::fs_Unlit);
    ref<cvf::ShaderProgram> prog = gen.generate();

    ref<cvf::Effect> eff = new cvf::Effect;
    eff->setShaderProgram(prog.p());
    eff->setUniform(new cvf::UniformFloat("u_color", cvf::Color4f(color)));

    part->setEffect(eff.p());

    return part;
}

} // namespace cvfu
