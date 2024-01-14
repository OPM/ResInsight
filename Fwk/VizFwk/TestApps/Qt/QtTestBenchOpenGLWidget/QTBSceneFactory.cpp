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

#include "QTBSceneFactory.h"




//==================================================================================================
//
// 
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QTBSceneFactory::QTBSceneFactory(bool useShaders)
:   m_useShaders(useShaders)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Scene> QTBSceneFactory::createTestScene(const cvf::OpenGLCapabilities& capabilities) const
{
    cvf::ref<cvf::ModelBasicList> model = new cvf::ModelBasicList;

    cvf::ShaderProgramGenerator spGen("MyStandardHeadlight", cvf::ShaderSourceProvider::instance());
    spGen.configureStandardHeadlightColor();
    cvf::ref<cvf::ShaderProgram> shaderProg = spGen.generate();

    {
        cvf::GeometryBuilderDrawableGeo builder;
        cvf::GeometryUtils::createSphere(2, 10, 10, &builder);

        cvf::ref<cvf::Effect> eff = new cvf::Effect;
        if (m_useShaders)
        {
            eff->setShaderProgram(shaderProg.p());
            eff->setUniform(new cvf::UniformFloat("u_color", cvf::Color4f(cvf::Color3::GREEN)));
        }
        else
        {
            eff->setRenderState(new cvf::RenderStateMaterial_FF(cvf::Color3::BLUE));
        }

        cvf::ref<cvf::Part> part = new cvf::Part;
        part->setName("MySphere");
        part->setDrawable(0, builder.drawableGeo().p());
        part->setEffect(eff.p());

        model->addPart(part.p());
    }

    {
        cvf::GeometryBuilderDrawableGeo builder;
        cvf::GeometryUtils::createBox(cvf::Vec3f(5, 0, 0), 2, 3, 4, &builder);

        cvf::ref<cvf::Effect> eff = new cvf::Effect;
        if (m_useShaders)
        {
            eff->setShaderProgram(shaderProg.p());
            eff->setUniform(new cvf::UniformFloat("u_color", cvf::Color4f(cvf::Color3::YELLOW)));
        }
        else
        {
            eff->setRenderState(new cvf::RenderStateMaterial_FF(cvf::Color3::RED));
        }

        cvf::ref<cvf::Part> part = new cvf::Part;
        part->setName("MyBox");
        part->setDrawable(0, builder.drawableGeo().p());
        part->setEffect(eff.p());

        model->addPart(part.p());
    }


    model->addPart(createTexturedPart(capabilities).p());

    model->addPart(createDrawableTextPart(capabilities).p());

    model->updateBoundingBoxesRecursive();

    cvf::ref<cvf::Scene> scene = new cvf::Scene;
    scene->addModel(model.p());

    return scene;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> QTBSceneFactory::createQuadGeoWithTexCoords(const cvf::Vec3f& origin, const cvf::Vec3f& u, const cvf::Vec3f& v)
{
    cvf::ref<cvf::Vec3fArray> vertices = new cvf::Vec3fArray(4);
    vertices->set(0, origin);
    vertices->set(1, origin + u);
    vertices->set(2, origin + u + v);
    vertices->set(3, origin + v);

    cvf::ref<cvf::Vec2fArray> texCoords = new cvf::Vec2fArray(4);
    texCoords->set(0, cvf::Vec2f(0, 0));
    texCoords->set(1, cvf::Vec2f(1, 0));
    texCoords->set(2, cvf::Vec2f(1, 1));
    texCoords->set(3, cvf::Vec2f(0, 1));

    const cvf::uint conns[6] = { 0, 1, 2, 0, 2, 3};
    cvf::ref<cvf::UIntArray> indices = new cvf::UIntArray(conns, 6);

    cvf::ref<cvf::PrimitiveSetIndexedUInt> primSet = new cvf::PrimitiveSetIndexedUInt(cvf::PT_TRIANGLES);
    primSet->setIndices(indices.p());

    cvf::ref<cvf::DrawableGeo> geo = new cvf::DrawableGeo;
    geo->setVertexArray(vertices.p());
    geo->setTextureCoordArray(texCoords.p());
    geo->addPrimitiveSet(primSet.p());

    geo->computeNormals();

    return geo;

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::TextureImage> QTBSceneFactory::create4x4ColoredImage()
{
    // Create a simple 4x4 texture with 16 unique colors
    cvf::ref<cvf::TextureImage> textureImage = new cvf::TextureImage;

    {
        cvf::ref<cvf::Color4ubArray> textureData = new cvf::Color4ubArray;
        textureData->reserve(16);
        textureData->add(cvf::Color4ub(cvf::Color3::RED));
        textureData->add(cvf::Color4ub(cvf::Color3::GREEN));
        textureData->add(cvf::Color4ub(cvf::Color3::BLUE));
        textureData->add(cvf::Color4ub(cvf::Color3::YELLOW));
        textureData->add(cvf::Color4ub(cvf::Color3::CYAN));
        textureData->add(cvf::Color4ub(cvf::Color3::MAGENTA));
        textureData->add(cvf::Color4ub(cvf::Color3::INDIGO));
        textureData->add(cvf::Color4ub(cvf::Color3::OLIVE));
        textureData->add(cvf::Color4ub(cvf::Color3::LIGHT_GRAY));
        textureData->add(cvf::Color4ub(cvf::Color3::BROWN));
        textureData->add(cvf::Color4ub(cvf::Color3::CRIMSON));
        textureData->add(cvf::Color4ub(cvf::Color3::DARK_BLUE));
        textureData->add(cvf::Color4ub(cvf::Color3::DARK_CYAN));
        textureData->add(cvf::Color4ub(cvf::Color3::DARK_GREEN));
        textureData->add(cvf::Color4ub(cvf::Color3::DARK_MAGENTA));
        textureData->add(cvf::Color4ub(cvf::Color3::DARK_ORANGE));
        textureImage->setData(textureData->ptr()->ptr(), 4, 4);
    }

    CVF_ASSERT(textureImage->pixel(1, 0) == cvf::Color4ub(cvf::Color3::GREEN));
    CVF_ASSERT(textureImage->pixel(2, 0) == cvf::Color4ub(cvf::Color3::BLUE));

    return textureImage;

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Part> QTBSceneFactory::createTexturedPart(const cvf::OpenGLCapabilities& capabilities) const
{
    cvf::ref<cvf::DrawableGeo> geo = createQuadGeoWithTexCoords(cvf::Vec3f(2, 2, 0), cvf::Vec3f(2, 0, 0), cvf::Vec3f(0, 3, 0));
    
    cvf::ref<cvf::TextureImage> textureImage = create4x4ColoredImage();

    cvf::ref<cvf::Effect> eff = new cvf::Effect;

    if (m_useShaders)
    {
        cvf::ref<cvf::Texture> texture = new cvf::Texture(textureImage.p());

        cvf::ref<cvf::Sampler> sampler = new cvf::Sampler;
        sampler->setMinFilter(cvf::Sampler::LINEAR);
        sampler->setMagFilter(cvf::Sampler::NEAREST);
        sampler->setWrapModeS(cvf::Sampler::REPEAT);
        sampler->setWrapModeT(cvf::Sampler::REPEAT);

        cvf::ref<cvf::RenderStateTextureBindings> textureBindings = new cvf::RenderStateTextureBindings;
        textureBindings->addBinding(texture.p(), sampler.p(), "u_texture2D");

        eff->setRenderState(textureBindings.p());

        cvf::ShaderProgramGenerator spGen("MyTexturedStandardHeadlight", cvf::ShaderSourceProvider::instance());
        spGen.configureStandardHeadlightTexture();
        cvf::ref<cvf::ShaderProgram> shaderProg = spGen.generate();
        eff->setShaderProgram(shaderProg.p());
    }
    else
    {
        cvf::ref<cvf::Texture2D_FF> texture = new cvf::Texture2D_FF(textureImage.p());
        texture->setMinFilter(cvf::Texture2D_FF::LINEAR);
        texture->setMagFilter(cvf::Texture2D_FF::NEAREST);
        texture->setWrapMode(cvf::Texture2D_FF::REPEAT);

        cvf::ref<cvf::RenderStateTextureMapping_FF> textureMapping = new cvf::RenderStateTextureMapping_FF(texture.p());

        eff->setRenderState(textureMapping.p());
    }
   
    cvf::ref<cvf::Part> part = new cvf::Part;
    part->setName("MyTexturedQuad");
    part->setDrawable(0, geo.p());
    part->setEffect(eff.p());

    return part;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Part> QTBSceneFactory::createDrawableTextPart(const cvf::OpenGLCapabilities& capabilities) const
{
    static int sl_callCount = 0;
    cvf::String textStr = cvf::String("Test text string %1").arg(sl_callCount++);

    cvf::ref<cvf::DrawableText> drawableText = new cvf::DrawableText;
    drawableText->setFont(new cvf::FixedAtlasFont(cvf::FixedAtlasFont::LARGE));
    drawableText->setTextColor(cvf::Color3::RED);

    drawableText->addText(textStr, cvf::Vec3f(2, 2, 0));
    drawableText->setCheckPosVisible(false);

    cvf::ref<cvf::RenderStateBlending> blending = new cvf::RenderStateBlending;
    blending->configureTransparencyBlending();

    cvf::ref<cvf::Effect> eff = new cvf::Effect;
    eff->setRenderState(blending.p());

    if (m_useShaders)
    {
        cvf::ShaderProgramGenerator spGen("MyTextShaderProgram", cvf::ShaderSourceProvider::instance());
        spGen.addVertexCode(cvf::ShaderSourceRepository::vs_MinimalTexture);
        spGen.addFragmentCode(cvf::ShaderSourceRepository::fs_Text);
        cvf::ref<cvf::ShaderProgram> shaderProg = spGen.generate();
        shaderProg->disableUniformTrackingForUniform("u_texture2D");
        shaderProg->disableUniformTrackingForUniform("u_color");
        eff->setShaderProgram(shaderProg.p());
    }

    cvf::ref<cvf::Part> part = new cvf::Part;
    part->setDrawable(drawableText.p());
    part->setEffect(eff.p());

    return part;
}
