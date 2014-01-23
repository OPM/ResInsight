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
#include "cvfShaderSourceProvider.h"

#include "cvfuInputEvents.h"
#include "cvfuImageJpeg.h"
#include "cvfuWavefrontObjImport.h"
#include "cvfuSampleFactory.h"

#include "snipCubeMapping.h"

namespace snip {



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool CubeMapping::onInitialize()
{
    ref<ModelBasicList> myModel = new ModelBasicList;

    m_modelTransformMatrix = new Transform;
    m_lastAnimUpdateTimeStamp = 0;

    ref<cvf::Texture> cubeMapTexture = loadCubeMapTexture();
    cubeMapTexture->enableMipmapGeneration(true);
    ref<cvf::Sampler> sampler = new cvf::Sampler;
    sampler->setWrapMode(Sampler::CLAMP_TO_EDGE);
    sampler->setMinFilter(Sampler::LINEAR_MIPMAP_LINEAR);
    sampler->setMagFilter(Sampler::LINEAR);
    ref<cvf::RenderStateTextureBindings> texBind = new cvf::RenderStateTextureBindings;
    texBind->addBinding(cubeMapTexture.p(), sampler.p(), "u_cubeMap");

    // The 'main' geometry
    {
        ref<DrawableGeo> geo;

//         // Box
//         GeometryBuilderDrawableGeo builder;
//         BoxGenerator gen;
//         gen.setMinMax(Vec3d(-1, -1, -1), Vec3d(1, 1, 1));
//         gen.generate(&builder);
//         geo = builder.drawableGeo();

//         // Sphere
//         GeometryBuilderDrawableGeo builder;
//         GeometryUtils::createSphere(2.0, 50, 50, &builder);
//         geo = builder.drawableGeo();

        WavefrontObjImport imp;
        //imp.readFile(m_testDataDir + "dragon_10k.obj");
        imp.readFile(m_testDataDir + "teapot.obj");
        GeometryBuilderDrawableGeo builder;
        imp.buildGeometry(&builder);
        geo = builder.drawableGeo();
        geo->weldVertices(0.00001);
        geo->computeNormals();

        ref<Part> part = new Part;
        part->setDrawable(geo.p());

//         ref<ShaderProgram> prog = new ShaderProgram;
//         ref<Shader> vs = cvf::ShaderFactory::instance()->createFromFile(Shader::VERTEX_SHADER,  "CubeMapping_Vert");
//         ref<Shader> fs = cvf::ShaderFactory::instance()->createFromFile(Shader::FRAGMENT_SHADER,"CubeMapping_Frag");
//         prog->addShader(vs.p());
//         prog->addShader(fs.p());
//         eff->setUniform(new UniformFloat("ambientIntensity", 0.2f));
//         eff->setUniform(new UniformFloat("color", Color3f(Color3::YELLOW)));
//         eff->setUniform(new UniformFloat("reflectivity", 0.8f));

        ShaderProgramGenerator gen("CubeMapping", ShaderSourceProvider::instance());
        gen.addVertexCode(ShaderSourceRepository::vs_Standard);
        gen.addFragmentCode(ShaderSourceRepository::src_Color);
        gen.addFragmentCodeFromFile("CubeMapping_Light");
        gen.addFragmentCode(ShaderSourceRepository::fs_Standard);
        ref<ShaderProgram> prog = gen.generate();

        // Link and show log to see any warnings
        prog->linkProgram(m_openGLContext.p());
        Trace::show(prog->programInfoLog(m_openGLContext.p()));

        ref<Effect> eff = new Effect;
        eff->setRenderState(texBind.p());
        eff->setShaderProgram(prog.p());

        // Doesn't work well with saturated colors
        eff->setUniform(new UniformFloat("u_color", Color4f(Color3::OLIVE)));
        eff->setUniform(new UniformFloat("u_ambientIntensity", 0.2f));
        eff->setUniform(new UniformFloat("u_reflectivity", 0.8f));

        part->setEffect(eff.p());

        part->setTransform(m_modelTransformMatrix.p());

        myModel->addPart(part.p());
    }

    // Create a point light to approximately coincide with the sun in the cube map
    // Register the light dynamic uniform set with the rendering
    m_light = new PointLight;
    m_light->setPosition(Vec3d(-10.0, 10.0, 13.0));
    m_renderSequence->firstRendering()->addGlobalDynamicUniformSet(m_light.p());

    // Hook a marker into the model and
    ref<Part> lightMarkerPart = SampleFactory::createUnlitSphere(0.5, Color3f::YELLOW);
    lightMarkerPart->setTransform(m_light->markerTransform());
    myModel->addPart(lightMarkerPart.p());


    m_renderSequence->firstRendering()->scene()->addModel(myModel.p());

    // For debugging the cube map texture images
    // Does not combine with other drawing
    //addDebugCubeWithCubeMapImages();


    myModel->updateBoundingBoxesRecursive();
    BoundingBox bb = myModel->boundingBox();
    if (bb.isValid())
    {
        //m_camera->setProjectionAsPerspective(40, 0.01, 100.0);
        m_camera->fitView(bb, -Vec3d::Z_AXIS, Vec3d::Y_AXIS);
    }

    m_renderSequence->firstRendering()->addOverlayItem(new OverlayAxisCross(m_camera.p(), new FixedAtlasFont(FixedAtlasFont::STANDARD)));

    // Add skybox after setting camera
    {
        const double skyBoxSize = 100.0;
        GeometryBuilderDrawableGeo builder;
        BoxGenerator boxGen;
        boxGen.setMinMax(skyBoxSize*Vec3d(-0.5, -0.5, -0.5), skyBoxSize*Vec3d(0.5, 0.5, 0.5));
        boxGen.generate(&builder);
        ref<DrawableGeo> geo = builder.drawableGeo();;

        ref<Part> part = new Part;
        part->setDrawable(geo.p());

        ShaderProgramGenerator gen("CubeMappingSkyBox", ShaderSourceProvider::instance());
        gen.addVertexCodeFromFile("CubeMappingSkyBox_Vert");
        gen.addFragmentCodeFromFile("CubeMappingSkyBox_Frag");
        ref<ShaderProgram> prog = gen.generate();

        // Link and show log to see any warnings
        prog->linkProgram(m_openGLContext.p());
        Trace::show(prog->programInfoLog(m_openGLContext.p()));

        ref<Effect> eff = new Effect;
        eff->setRenderState(texBind.p());
        eff->setShaderProgram(prog.p());

        part->setEffect(eff.p());

        myModel->addPart(part.p());
        myModel->updateBoundingBoxesRecursive();
    }

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void CubeMapping::loadCubeMapImages(ref<TextureImage> cubeImages[6]) const
{
//     cubeImages[Texture::TEXTURE_CUBE_MAP_POSITIVE_X] = cvfu::ImageJpeg::loadImage(m_testDataDir + "CubeMaps/TestQMap_rt.jpg");
//     cubeImages[Texture::TEXTURE_CUBE_MAP_NEGATIVE_X] = cvfu::ImageJpeg::loadImage(m_testDataDir + "CubeMaps/TestQMap_lf.jpg");
//     cubeImages[Texture::TEXTURE_CUBE_MAP_POSITIVE_Y] = cvfu::ImageJpeg::loadImage(m_testDataDir + "CubeMaps/TestQMap_up.jpg");
//     cubeImages[Texture::TEXTURE_CUBE_MAP_NEGATIVE_Y] = cvfu::ImageJpeg::loadImage(m_testDataDir + "CubeMaps/TestQMap_dn.jpg");
//     cubeImages[Texture::TEXTURE_CUBE_MAP_POSITIVE_Z] = cvfu::ImageJpeg::loadImage(m_testDataDir + "CubeMaps/TestQMap_ft.jpg");
//     cubeImages[Texture::TEXTURE_CUBE_MAP_NEGATIVE_Z] = cvfu::ImageJpeg::loadImage(m_testDataDir + "CubeMaps/TestQMap_bk.jpg");

    cubeImages[Texture::TEXTURE_CUBE_MAP_POSITIVE_X] = cvfu::ImageJpeg::loadImage(m_testDataDir + "CubeMaps/GreenBook_pos_x.jpg");
    cubeImages[Texture::TEXTURE_CUBE_MAP_NEGATIVE_X] = cvfu::ImageJpeg::loadImage(m_testDataDir + "CubeMaps/GreenBook_neg_x.jpg");
    cubeImages[Texture::TEXTURE_CUBE_MAP_POSITIVE_Y] = cvfu::ImageJpeg::loadImage(m_testDataDir + "CubeMaps/GreenBook_pos_y.jpg");
    cubeImages[Texture::TEXTURE_CUBE_MAP_NEGATIVE_Y] = cvfu::ImageJpeg::loadImage(m_testDataDir + "CubeMaps/GreenBook_neg_y.jpg");
    cubeImages[Texture::TEXTURE_CUBE_MAP_POSITIVE_Z] = cvfu::ImageJpeg::loadImage(m_testDataDir + "CubeMaps/GreenBook_pos_z.jpg");
    cubeImages[Texture::TEXTURE_CUBE_MAP_NEGATIVE_Z] = cvfu::ImageJpeg::loadImage(m_testDataDir + "CubeMaps/GreenBook_neg_z.jpg");
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<Texture> CubeMapping::loadCubeMapTexture() const
{
    ref<TextureImage> cubeImages[6];
    loadCubeMapImages(cubeImages);

    ref<Texture> tex = new Texture(Texture::TEXTURE_CUBE_MAP, Texture::RGBA);
    tex->setCubeMapImage(Texture::TEXTURE_CUBE_MAP_POSITIVE_X, cubeImages[Texture::TEXTURE_CUBE_MAP_POSITIVE_X].p());
    tex->setCubeMapImage(Texture::TEXTURE_CUBE_MAP_NEGATIVE_X, cubeImages[Texture::TEXTURE_CUBE_MAP_NEGATIVE_X].p());
    tex->setCubeMapImage(Texture::TEXTURE_CUBE_MAP_POSITIVE_Y, cubeImages[Texture::TEXTURE_CUBE_MAP_POSITIVE_Y].p());
    tex->setCubeMapImage(Texture::TEXTURE_CUBE_MAP_NEGATIVE_Y, cubeImages[Texture::TEXTURE_CUBE_MAP_NEGATIVE_Y].p());
    tex->setCubeMapImage(Texture::TEXTURE_CUBE_MAP_POSITIVE_Z, cubeImages[Texture::TEXTURE_CUBE_MAP_POSITIVE_Z].p());
    tex->setCubeMapImage(Texture::TEXTURE_CUBE_MAP_NEGATIVE_Z, cubeImages[Texture::TEXTURE_CUBE_MAP_NEGATIVE_Z].p());

    return tex;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void CubeMapping::addDebugCubeWithCubeMapImages()
{
    ref<TextureImage> cubeImages[6];
    loadCubeMapImages(cubeImages);

    ModelBasicList* myModel = dynamic_cast<ModelBasicList*>(m_renderSequence->firstRendering()->scene()->model(0));
    CVF_ASSERT(myModel);

    //        (-1,1,-1)*---------*(1,1,-1)                
    //                /|        /|            |y    
    //               / |       / |            |     
    //              *---------*  |            |     
    //              |  *------|--*(1,-1,-1)   *-----x   
    //              | /       | /            /  
    //              |/        |/            /z      
    //     (-1,-1,1)*---------*(1,-1,1)                     
    //            

    {
        // PosX/Right
        ref<DrawableGeo> geo = createTexturedQuad(Vec3f(1, 1, 1), -2.0f*Vec3f::Z_AXIS, -2.0f*Vec3f::Y_AXIS);
        myModel->addPart( createTexturedPart(geo.p(), cubeImages[Texture::TEXTURE_CUBE_MAP_POSITIVE_X].p()).p() );
    }

    {
        // NegX/Left
        ref<DrawableGeo> geo = createTexturedQuad(Vec3f(-1, 1, -1), 2.0f*Vec3f::Z_AXIS, -2.0f*Vec3f::Y_AXIS);
        myModel->addPart( createTexturedPart(geo.p(), cubeImages[Texture::TEXTURE_CUBE_MAP_NEGATIVE_X].p()).p() );
    }

    {
        // PosY/Top
        ref<DrawableGeo> geo = createTexturedQuad(Vec3f(-1, 1, -1), 2.0f*Vec3f::X_AXIS, 2.0f*Vec3f::Z_AXIS);
        myModel->addPart( createTexturedPart(geo.p(), cubeImages[Texture::TEXTURE_CUBE_MAP_POSITIVE_Y].p()).p() );
    }

    {
        // NegY/Bottom
        ref<DrawableGeo> geo = createTexturedQuad(Vec3f(-1, -1, 1), 2.0f*Vec3f::X_AXIS, -2.0f*Vec3f::Z_AXIS);
        myModel->addPart( createTexturedPart(geo.p(), cubeImages[Texture::TEXTURE_CUBE_MAP_NEGATIVE_Y].p()).p() );
    }

    {
        // PosZ/Front
        ref<DrawableGeo> geo = createTexturedQuad(Vec3f(-1, 1, 1), 2.0f*Vec3f::X_AXIS, -2.0f*Vec3f::Y_AXIS);
        myModel->addPart( createTexturedPart(geo.p(), cubeImages[Texture::TEXTURE_CUBE_MAP_POSITIVE_Z].p()).p() );
    }

    {
        // NegZ/Back
        ref<DrawableGeo> geo = createTexturedQuad(Vec3f(1, 1, -1), -2.0f*Vec3f::X_AXIS, -2.0f*Vec3f::Y_AXIS);
        myModel->addPart( createTexturedPart(geo.p(), cubeImages[Texture::TEXTURE_CUBE_MAP_NEGATIVE_Z].p()).p() );
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<DrawableGeo> CubeMapping::createTexturedQuad(const Vec3f& origin, const Vec3f& u, const Vec3f& v)
{
    ref<Vec3fArray> vertices = new Vec3fArray(4);
    vertices->set(0, origin);
    vertices->set(1, origin + u);
    vertices->set(2, origin + u + v);
    vertices->set(3, origin + v);

    ref<Vec2fArray> texCoords = new Vec2fArray(4);
    texCoords->set(0, Vec2f(0, 0));
    texCoords->set(1, Vec2f(1, 0));
    texCoords->set(2, Vec2f(1, 1));
    texCoords->set(3, Vec2f(0, 1));

    const cvf::uint conns[6] = { 0, 1, 2, 0, 2, 3};
    ref<UIntArray> indices = new UIntArray(conns, 6);

    ref<PrimitiveSetIndexedUInt> primSet = new PrimitiveSetIndexedUInt(PT_TRIANGLES);
    primSet->setIndices(indices.p());

    ref<DrawableGeo> geo = new DrawableGeo;
    geo->setVertexArray(vertices.p());
    geo->setTextureCoordArray(texCoords.p());
    geo->addPrimitiveSet(primSet.p());

    geo->computeNormals();

    return geo;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<Part> CubeMapping::createTexturedPart(DrawableGeo* geo, TextureImage* texImg)
{
    ref<Part> part = new Part;
    part->setDrawable(geo);

    ref<Effect> eff = new Effect;
    eff->setRenderState(new RenderStateMaterial_FF(Color3::WHITE));

    ref<cvf::RenderStateLighting_FF> lighting = new cvf::RenderStateLighting_FF;
    lighting->enableTwoSided(true);
    eff->setRenderState(lighting.p());

    ref<cvf::Texture> texture = new cvf::Texture(texImg);
    ref<cvf::Sampler> sampler = new cvf::Sampler;

    ref<cvf::RenderStateTextureBindings> texBind = new cvf::RenderStateTextureBindings;
    texBind->addBinding(texture.p(), sampler.p(), "dummy");
    eff->setRenderState(texBind.p());

    part->setEffect(eff.p());

    return part;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void CubeMapping::onUpdateAnimation(double animationTime, PostEventAction* postEventAction)
{
    TestSnippet::onUpdateAnimation(animationTime, postEventAction);

    if (Math::abs(animationTime - m_lastAnimUpdateTimeStamp) > 0.05)
    {
        Mat4d m = m_modelTransformMatrix->worldTransform();
        Mat4d r = Mat4d::fromRotation(Vec3d::Y_AXIS, Math::toRadians(1.0));
        m_modelTransformMatrix->setLocalTransform(m*r);
        m_lastAnimUpdateTimeStamp = animationTime;

        if (postEventAction)
        {
            *postEventAction = REDRAW;
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void CubeMapping::onKeyPressEvent(KeyEvent* keyEvent)
{
    keyEvent->setRequestedAction(REDRAW);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<cvf::String> CubeMapping::helpText() const
{
    std::vector<cvf::String> helpText;

    return helpText;
}


} // namespace snip
