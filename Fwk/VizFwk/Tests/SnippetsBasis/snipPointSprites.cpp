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

#include "cvfuInputEvents.h"
#include "cvfuSnippetPropertyPublisher.h"

#include "snipPointSprites.h"
#include "cvfuImageJpeg.h"

namespace snip {


/*
#define STRINGIFY(A) #A

const char *vertexShader = STRINGIFY(
uniform float pointRadius;  // point size in world space
uniform float pointScale;   // scale to calculate size in pixels
uniform mat4 cvfu_modelViewProjectionMatrix;
uniform mat4 cvfu_modelViewMatrix;
attribute vec4 cvfa_vertex;
varying vec3 v_ecPosition;
varying vec2 v_texCoord;
void main()
{
    // calculate window-space point size
    v_ecPosition = (cvfu_modelViewMatrix * cvfa_vertex).xyz;
    //vec3 posEye = vec3(cvfu_modelViewMatrix * vec4(cvfa_vertex.xyz, 1.0));
    float dist = length(v_ecPosition);
    gl_PointSize = 2*pointRadius * (pointScale / dist);

    gl_Position = cvfu_modelViewProjectionMatrix*cvfa_vertex;
}
);



// pixel shader for rendering points as shaded spheres
const char *spherePixelShader = STRINGIFY(
varying vec3 v_ecPosition;

//const vec3 lightDir = vec3(0.577, 0.577, 0.577);
void main()
{
    const vec3  ecLightPosition = vec3(0.5, 5.0, 7.0);
    vec3 lightDir = normalize(ecLightPosition - v_ecPosition);

    // calculate normal from texture coordinates
    vec3 N;
    N.xy = gl_PointCoord.xy*vec2(2.0, -2.0) + vec2(-1.0, 1.0);
    float mag = dot(N.xy, N.xy);
    if (mag > 1) discard;   // kill pixels outside circle
    N.z = sqrt(1-mag);

    // calculate lighting
    float diffuse = max(0.0, dot(lightDir, N));

    vec4 color = srcFragment();
    gl_FragColor = color*diffuse;

    //gl_FragDepth = gl_FragCoord.z - 15.0*(1-mag);
}
);
*/


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PointSprites::PointSprites()
{
    m_usePerspective = true;
    
    // Must remove distance scaling in shader in order for this to work
    //m_usePerspective = false;
    
    m_fovScale = 1.0;
    m_nearPlane = 0.01;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool PointSprites::onInitialize()
{
    ref<ModelBasicList> model = new ModelBasicList;

    bool useShaders = true;

    {
        GeometryBuilderDrawableGeo builder;
        GeometryUtils::createSphere(1, 10, 10, &builder);
        ref<DrawableGeo> geo = builder.drawableGeo();

        ref<Effect> eff = new Effect;

        if (useShaders)
        {
            cvf::ShaderProgramGenerator gen("SimpleHeadlight", cvf::ShaderSourceProvider::instance());
            gen.configureStandardHeadlightColor();
            ref<ShaderProgram> prog = gen.generate();
            eff->setShaderProgram(prog.p());
            eff->setUniform(new UniformFloat("u_color", Color4f(Color3::YELLOW)));
        }
        else
        {
            eff->setRenderState(new RenderStateMaterial_FF(RenderStateMaterial_FF::PURE_YELLOW));
        }

        ref<Part> part = new Part;
        part->setDrawable(geo.p());
        part->setEffect(eff.p());

        model->addPart(part.p());
    }

    {
        ref<Vec3fArray> vertices = new Vec3fArray;
        vertices->reserve(10);
        vertices->add(Vec3f(0, 0, 0));
        vertices->add(Vec3f(-3, 0, 0));
        vertices->add(Vec3f(3, 0, 0));

        ref<UIntArray> indices = new UIntArray(vertices->size());
        indices->setConsecutive(0);

        ref<PrimitiveSetIndexedUInt> primSet = new PrimitiveSetIndexedUInt(PT_POINTS);
        primSet->setIndices(indices.p());

        ref<DrawableGeo> geo = new DrawableGeo;
        geo->setVertexArray(vertices.p());
        geo->addPrimitiveSet(primSet.p());

        ref<Effect> eff = new Effect;

        if (useShaders)
        {
            bool useTextureSprite = true;

            cvf::ShaderProgramGenerator gen("PointSprites", cvf::ShaderSourceProvider::instance());
            gen.addVertexCode(ShaderSourceRepository::vs_DistanceScaledPoints);
            
            if (useTextureSprite)  gen.addFragmentCode(ShaderSourceRepository::src_TextureFromPointCoord);
            else                   gen.addFragmentCode(ShaderSourceRepository::src_Color);
            gen.addFragmentCode(ShaderSourceRepository::fs_CenterLitSpherePoints);

            ref<cvf::ShaderProgram> prog = gen.generate();
            eff->setShaderProgram(prog.p());
            eff->setUniform(new UniformFloat("u_pointRadius", 1.0f));

            if (useTextureSprite)
            {
                ref<Texture> tex = createTexture();
                ref<Sampler> sampler = new Sampler;
                sampler->setMinFilter(Sampler::LINEAR);
                sampler->setMagFilter(Sampler::NEAREST);
                sampler->setWrapModeS(Sampler::REPEAT);
                sampler->setWrapModeT(Sampler::REPEAT);
                ref<RenderStateTextureBindings> texBind = new RenderStateTextureBindings(tex.p(), sampler.p(), "u_texture2D");
                eff->setRenderState(texBind.p());
            }
            else
            {
                eff->setUniform(new UniformFloat("u_color", Color4f(Color3::RED)));
            }

            ref<RenderStatePoint> point = new RenderStatePoint(RenderStatePoint::PROGRAM_SIZE);
            point->enablePointSprite(true);
            eff->setRenderState(point.p());
        }
        else
        {
            eff->setRenderState(new RenderStateLighting_FF(false));
            eff->setRenderState(new RenderStateMaterial_FF(RenderStateMaterial_FF::PURE_MAGENTA));

            ref<RenderStatePoint> point = new RenderStatePoint(RenderStatePoint::FIXED_SIZE);
            point->enablePointSprite(true);
            point->setSize(600.0f);
            eff->setRenderState(point.p());
        }

        ref<Part> part = new Part;
        part->setDrawable(geo.p());
        part->setEffect(eff.p());

        model->addPart(part.p());
    }

    model->updateBoundingBoxesRecursive();
    m_renderSequence->firstRendering()->scene()->addModel(model.p());

    BoundingBox bb = model->boundingBox();
    if (bb.isValid())
    {
        m_camera->fitView(bb, -Vec3d::Z_AXIS, Vec3d::Y_AXIS);
        if (m_usePerspective)
        {
            m_camera->setProjectionAsPerspective(m_fovScale*40.0, m_nearPlane, m_camera->farPlane());
        }
        else
        {
            m_camera->setProjectionAsOrtho(m_fovScale*bb.extent().length(), m_nearPlane, m_camera->farPlane());
        }
    }

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<Texture> PointSprites::createTexture()
{
    ref<TextureImage> textureImage = new TextureImage;

    // Create a simple 4x4 texture
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

//     {
//         textureImage = cvfu::ImageJpeg::loadImage(m_testDataDir + "Turbine.jpg");
//     }

    ref<Texture> texture = new Texture(textureImage.p());

    return texture;
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PointSprites::onPaintEvent(PostEventAction* postEventAction)
{
    /*
    double fov = m_camera->fieldOfViewYDeg();
    double windowHeight = m_camera->viewport()->height();
    //double pointScale = windowHeight/tan(fov*0.5*PI_D/180.0);
    double pointScale = windowHeight/(2*Math::tan(0.5*Math::toRadians(fov)));
    //double pixelsPerRadian = windowHeight/Math::toRadians(fov);
    //pointScale = pixelsPerRadian;

    if (m_uniPointRadius.notNull()) m_uniPointRadius->set(1.0f);
    if (m_uniPointScale.notNull())  m_uniPointScale->set(static_cast<float>(pointScale));
    */

    TestSnippet::onPaintEvent(postEventAction);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PointSprites::onKeyPressEvent(KeyEvent* keyEvent)
{
    char character = keyEvent->character();
    if (character == '+' || character == L'-')
    {
        if (character == '+')   m_fovScale += 0.1;
        else                    m_fovScale -= 0.1;
        m_fovScale = Math::clamp(m_fovScale, 0.1, 4.0);
    }

    if (character == ',' || character == L'.')
    {
        if (character == ',')   m_nearPlane += 0.5;
        else                    m_nearPlane -= 0.5;
        m_nearPlane = Math::clamp(m_nearPlane, 0.0001, 200.0);
    }

    if (m_usePerspective)
    {
        double fovDeg = m_fovScale*40.0;
        m_camera->setProjectionAsPerspective(fovDeg, m_nearPlane, m_camera->farPlane());
        Trace::show("Perspective: FOV set to %.1f   NEAR set to %.1f", fovDeg, m_nearPlane);
    }
    else
    {
        BoundingBox bb = m_renderSequence->boundingBox();
        double height = m_fovScale*bb.extent().length();
        m_camera->setProjectionAsOrtho(height, m_nearPlane, m_camera->farPlane());
        Trace::show("Ortho: HEIGHT set to %.1f   NEAR set to %.1f", height, m_nearPlane);
    }

    keyEvent->setRequestedAction(REDRAW);
}





} // namespace snip

