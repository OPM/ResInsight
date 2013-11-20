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

#include "cvfuImageJpeg.h"
#include "cvfuImageTga.h"

#include "snipTexturing.h"

namespace snip {


static const char sg_scrTwoTexures[] = 
"uniform sampler2D myTextureSampler1;                                                   \n"
"uniform sampler2D myTextureSampler2;                                                   \n"
"                                                                                       \n"
"varying vec2 v_texCoord;                                                               \n"
"                                                                                       \n"
"vec4 srcFragment()                                                                     \n"
"{                                                                                      \n"
"    return texture2D(myTextureSampler1, v_texCoord)                                    \n"
"           + texture2D(myTextureSampler2, v_texCoord);                                 \n"
"}                                                                                      \n";



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool Texturing::onInitialize()
{
    ref<ModelBasicList> myModel = new ModelBasicList;

    bool canUseShaders = ShaderProgram::supportedOpenGL(m_openGLContext.p());

    if (canUseShaders)
    {
        ref<Part> spherePart = createMultiTexturedSpherePart();
        myModel->addPart(spherePart.p());

        ref<Part> quadPart = createSingleTexturedQuadPart(Vec3f(1.5, 1.5, 0), 2, 1);
        myModel->addPart(quadPart.p());
    }

    {
        ref<TextureImage> imgA = cvfu::ImageTga::loadImage(m_testDataDir + "TestTexture1_alpha.tga");
        ref<TextureImage> img = createImageWithConstantAlpha(*imgA, 255);

        if (canUseShaders)
        {
            myModel->addPart(createQuadPartWithTexture(Vec3f(-1.5, -3, 0), 1, 1, img.p()).p());
            myModel->addPart(createQuadPartWithTexture(Vec3f(-1.5, -4, 0), 1, 1, imgA.p()).p());
        }

        myModel->addPart(createQuadPartWithTextureFixedFunction(Vec3f(0,    -3, 0), 1, 1, img.p(),  RenderStateTextureMapping_FF::DECAL).p());
        myModel->addPart(createQuadPartWithTextureFixedFunction(Vec3f(0,    -4, 0), 1, 1, imgA.p(), RenderStateTextureMapping_FF::DECAL).p());
        myModel->addPart(createQuadPartWithTextureFixedFunction(Vec3f(1.1f, -3, 0), 1, 1, img.p(),  RenderStateTextureMapping_FF::MODULATE).p());
        myModel->addPart(createQuadPartWithTextureFixedFunction(Vec3f(1.1f, -4, 0), 1, 1, imgA.p(), RenderStateTextureMapping_FF::MODULATE).p());

    }

    myModel->updateBoundingBoxesRecursive();

    m_renderSequence->firstRendering()->scene()->addModel(myModel.p());
    m_renderSequence->firstRendering()->addOverlayItem(new cvf::OverlayAxisCross(m_camera.p(), new cvf::FixedAtlasFont(cvf::FixedAtlasFont::STANDARD)));

    BoundingBox bb = m_renderSequence->boundingBox();
    if (bb.isValid())
    {
        m_camera->fitView(bb, -Vec3d::Z_AXIS, Vec3d::Y_AXIS);
        m_trackball->setRotationPoint(bb.center());
    }

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<Part> Texturing::createMultiTexturedSpherePart()
{
    ref<DrawableGeo> geo1 = new DrawableGeo;

    {
        GeometryBuilderFaceList builder;
        GeometryUtils::createSphere(2.0f, 40, 40, &builder);

        ref<Vec3fArray> vertices = builder.vertices();
        ref<UIntArray> faceList = builder.faceList();
        geo1->setVertexArray(vertices.p());
        geo1->setFromFaceList(*faceList);
        geo1->computeNormals();

        {
            ref<Vec2fArray> textureCoordinates = new Vec2fArray;
            textureCoordinates->resize(vertices->size());
            for (size_t i = 0; i < vertices->size(); i++)
            {
                textureCoordinates->set(i, Vec2f(vertices->get(i).x(), vertices->get(i).y()));
            }
            geo1->setTextureCoordArray(textureCoordinates.p());
        }
    }

    ref<Part> part1 = new Part;
    part1->setDrawable(geo1.p());

    ref<Effect> eff1 = new Effect;
    eff1->setRenderState(new RenderStateMaterial_FF(Color3f(1, 1, 1)));

    ref<RenderStateTextureBindings> textureBindings = new RenderStateTextureBindings;

    {
        // Setup texturing
        ref<TextureImage> textureImage = create4x4ColoredImage();
        ref<Texture> texture = new Texture(textureImage.p());
        ref<Sampler> sampler = new Sampler;

        // Configure the sampler
        sampler->setMinFilter(Sampler::LINEAR);
        sampler->setMagFilter(Sampler::NEAREST);
        sampler->setWrapModeS(Sampler::REPEAT);
        sampler->setWrapModeT(Sampler::REPEAT);

        // Setup the texture binding render state
        textureBindings->addBinding(texture.p(), sampler.p(), "myTextureSampler1");
    }

    {
        // Setup second texture
        ref<TextureImage> textureImage = create32x32CheckersBoardImage();
        ref<Texture> texture = new Texture(textureImage.p());
        ref<Sampler> sampler = new Sampler;

        // Configure the sampler
        sampler->setMinFilter(Sampler::LINEAR);
        sampler->setMagFilter(Sampler::LINEAR);
        sampler->setWrapModeS(Sampler::REPEAT);
        sampler->setWrapModeT(Sampler::REPEAT);

        // Setup the texture binding render state
        textureBindings->addBinding(texture.p(), sampler.p(), "myTextureSampler2");
    }

    eff1->setRenderState(textureBindings.p());

    // Setup shader
    bool useShaderProgram = true;

    if (useShaderProgram)
    {
        ShaderProgramGenerator gen("MultiTexture", ShaderSourceProvider::instance());
        gen.addVertexCode(cvf::ShaderSourceRepository::vs_Standard);
        gen.addFragmentCode(sg_scrTwoTexures);
        gen.addFragmentCode(ShaderSourceRepository::light_SimpleHeadlight);
        gen.addFragmentCode(ShaderSourceRepository::fs_Standard);
        ref<ShaderProgram> prog = gen.generate();

        eff1->setShaderProgram(prog.p());
    }

    part1->setEffect(eff1.p());

    return part1;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<Part> Texturing::createSingleTexturedQuadPart(const Vec3f& llCornerPos, float width, float height)
{
    ref<DrawableGeo> geo  = createXYPlaneQuadGeoWithTexCoords(llCornerPos, width, height);

    ref<Effect> eff = new Effect;

    {
        ref<TextureImage> textureImage = create4x4ColoredImage();
        //cvfu::ImageJpeg::saveImage(*textureImage, "Sig.jpg");

        ref<Texture> texture = new Texture(textureImage.p());
        ref<Sampler> sampler = new Sampler;
        sampler->setMinFilter(Sampler::LINEAR);
        sampler->setMagFilter(Sampler::NEAREST);
        sampler->setWrapModeS(Sampler::CLAMP_TO_EDGE);
        sampler->setWrapModeT(Sampler::CLAMP_TO_EDGE);

        ref<RenderStateTextureBindings> textureBindings = new RenderStateTextureBindings;
        textureBindings->addBinding(texture.p(), sampler.p(), "u_texture2D");

        eff->setRenderState(textureBindings.p());
    }

    bool useShaderProgram = true;
    if (useShaderProgram)
    {
        ShaderProgramGenerator gen("Texturing", ShaderSourceProvider::instance());
        gen.addVertexCode(cvf::ShaderSourceRepository::vs_Standard);
        gen.addFragmentCode(ShaderSourceRepository::src_Texture);
        gen.addFragmentCode(ShaderSourceRepository::light_SimpleHeadlight);
        gen.addFragmentCode(ShaderSourceRepository::fs_Standard);
        ref<ShaderProgram> prog = gen.generate();

        eff->setShaderProgram(prog.p());
    }

    eff->setRenderState(new RenderStateMaterial_FF(Color3f(1, 1, 1)));

    ref<Part> part = new Part;
    part->setDrawable(geo.p());
    part->setEffect(eff.p());

    return part;
}


//--------------------------------------------------------------------------------------------------
/// Create a simple 4x4 texture with 16 unique colors
//--------------------------------------------------------------------------------------------------
ref<TextureImage> Texturing::create4x4ColoredImage()
{
    ref<TextureImage> textureImage = new TextureImage;

    {
        ref<Color4ubArray> textureData = new Color4ubArray;
        textureData->reserve(16);
        textureData->add(Color4ub(Color3::RED));
        textureData->add(Color4ub(Color3::GREEN));
        textureData->add(Color4ub(Color3::BLUE));
        textureData->add(Color4ub(Color3::YELLOW));
        textureData->add(Color4ub(Color3::CYAN));
        textureData->add(Color4ub(Color3::MAGENTA));
        textureData->add(Color4ub(Color3::INDIGO));
        textureData->add(Color4ub(Color3::OLIVE));
        textureData->add(Color4ub(Color3::LIGHT_GRAY));
        textureData->add(Color4ub(Color3::BROWN));
        textureData->add(Color4ub(Color3::CRIMSON));
        textureData->add(Color4ub(Color3::DARK_BLUE));
        textureData->add(Color4ub(Color3::DARK_CYAN));
        textureData->add(Color4ub(Color3::DARK_GREEN));
        textureData->add(Color4ub(Color3::DARK_MAGENTA));
        textureData->add(Color4ub(Color3::DARK_ORANGE));
        textureImage->setData(textureData->ptr()->ptr(), 4, 4);
    }

    CVF_ASSERT(textureImage->pixel(1, 0) == Color4ub(Color3::GREEN));
    CVF_ASSERT(textureImage->pixel(2, 0) == Color4ub(Color3::BLUE));

    return textureImage;
}


//--------------------------------------------------------------------------------------------------
/// Create a checkers board texture
//--------------------------------------------------------------------------------------------------
ref<TextureImage> Texturing::create32x32CheckersBoardImage()
{
    ref<TextureImage> textureImage = new TextureImage;

    {
        ref<Color4ubArray> textureData = new Color4ubArray;

        int dim = 32;
        textureData->reserve(dim*dim);
        int x;
        for (x = 0; x < dim; x++)
        {
            int y;
            for (y = 0; y < dim; y++)
            {
                Color4ub col;
                if ((y%2 == 0 && x%2 == 1) || (y%2 == 1 && x%2 == 0))
                {
                    col = Color4ub(Color3::BLACK);
                }
                else
                {
                    col  = Color4ub(Color3::WHITE);
                }

                textureData->add(col);
            }
        }

        textureImage->setData(textureData->ptr()->ptr(), dim, dim);
    }

    return textureImage;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<DrawableGeo> Texturing::createXYPlaneQuadGeoWithTexCoords( const Vec3f& llCornerPos, float width, float height )
{
    ref<Vec3fArray> vertices = new Vec3fArray;
    ref<Vec2fArray> texCoords = new Vec2fArray;
    vertices->reserve(4);
    texCoords->reserve(4);

    vertices->add(llCornerPos + Vec3f(0,     0,      0));  texCoords->add(Vec2f(0, 0));
    vertices->add(llCornerPos + Vec3f(width, 0,      0));  texCoords->add(Vec2f(1, 0));
    vertices->add(llCornerPos + Vec3f(width, height, 0));  texCoords->add(Vec2f(1, 1));
    vertices->add(llCornerPos + Vec3f(0,     height, 0));  texCoords->add(Vec2f(0, 1));

    ref<DrawableGeo> geo = new DrawableGeo;
    geo->setVertexArray(vertices.p());
    geo->setTextureCoordArray(texCoords.p());

    ref<UIntArray> indices = new UIntArray;
    indices->reserve(6);
    indices->add(0);  indices->add(1);  indices->add(2);
    indices->add(0);  indices->add(2);  indices->add(3);
    ref<PrimitiveSetIndexedUInt> primSet = new PrimitiveSetIndexedUInt(PT_TRIANGLES);
    primSet->setIndices(indices.p());
    geo->addPrimitiveSet(primSet.p());

    geo->computeNormals();

    return geo;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<Part> Texturing::createQuadPartWithTexture(const Vec3f& llCornerPos, float width, float height, TextureImage* textureImg)
{
    ref<DrawableGeo> geo =  createXYPlaneQuadGeoWithTexCoords(llCornerPos, width, height);

    ref<Texture> texture = new Texture(textureImg);
    ref<Sampler> sampler = new Sampler;
    sampler->setMinFilter(Sampler::LINEAR);
    sampler->setMagFilter(Sampler::LINEAR);
    sampler->setWrapMode(Sampler::CLAMP_TO_EDGE);

    ref<RenderStateTextureBindings> textureBindings = new RenderStateTextureBindings;
    textureBindings->addBinding(texture.p(), sampler.p(), "u_texture2D");

    ShaderProgramGenerator gen("Texturing", ShaderSourceProvider::instance());
    gen.addVertexCode(cvf::ShaderSourceRepository::vs_Standard);
    gen.addFragmentCode(ShaderSourceRepository::src_Texture);
    gen.addFragmentCode(ShaderSourceRepository::light_SimpleHeadlight);
    gen.addFragmentCode(ShaderSourceRepository::fs_Standard);
    ref<ShaderProgram> prog = gen.generate();

    ref<Effect> eff = new Effect;
    eff->setShaderProgram(prog.p());
    eff->setRenderState(textureBindings.p());
    
    ref<Part> part = new Part;
    part->setDrawable(geo.p());
    part->setEffect(eff.p());

    return part;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<Part> Texturing::createQuadPartWithTextureFixedFunction(const Vec3f& llCornerPos, float width, float height, TextureImage* textureImg, RenderStateTextureMapping_FF::TextureFunction texFunc)
{
    ref<DrawableGeo> geo =  createXYPlaneQuadGeoWithTexCoords(llCornerPos, width, height);

    ref<Texture2D_FF> texture = new Texture2D_FF(textureImg);
    texture->setMinFilter(Texture2D_FF::LINEAR);
    texture->setMagFilter(Texture2D_FF::LINEAR);
    texture->setWrapMode(Texture2D_FF::CLAMP);

    ref<RenderStateTextureMapping_FF> textureMapping = new RenderStateTextureMapping_FF(texture.p());
    textureMapping->setTextureFunction(texFunc);

    ref<Effect> eff = new Effect;
    eff->setRenderState(textureMapping.p());
    eff->setRenderState(new RenderStateMaterial_FF(Color3f::fromByteColor(200, 200, 0)));

    ref<Part> part = new Part;
    part->setDrawable(geo.p());
    part->setEffect(eff.p());

    return part;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<TextureImage> Texturing::createImageWithConstantAlpha(const TextureImage& sourceImage, ubyte newAlphaValue)
{
    ref<TextureImage> img = new TextureImage;
    img->allocate(sourceImage.width(), sourceImage.height());

    for (cvf::uint y = 0; y < sourceImage.height(); y++)
    {
        for (cvf::uint x = 0; x < sourceImage.width(); x++)
        {
            Color4ub clr = sourceImage.pixel(x, y);
            clr.a() = newAlphaValue;
            img->setPixel(x, y, clr);
        }
    }

    return img;
}


} // namespace snip

