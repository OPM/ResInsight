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
#include "cvfOverlayNavigationCube.h"
#include "cvfOpenGL.h"
#include "cvfOpenGLResourceManager.h"
#include "cvfGeometryBuilderDrawableGeo.h"
#include "cvfGeometryUtils.h"
#include "cvfViewport.h"
#include "cvfCamera.h"
#include "cvfTextDrawer.h"
#include "cvfFont.h"
#include "cvfShaderProgram.h"
#include "cvfUniform.h"
#include "cvfMatrixState.h"
#include "cvfDrawableVectors.h"
#include "cvfGeometryBuilderTriangles.h"
#include "cvfArrowGenerator.h"
#include "cvfBufferObjectManaged.h"
#include "cvfDrawableText.h"
#include "cvfTextureImage.h"
#include "cvfPrimitiveSet.h"
#include "cvfPrimitiveSetIndexedUShort.h"
#include "cvfShaderProgramGenerator.h"
#include "cvfShaderSourceProvider.h"
#include "cvfRay.h"
#include "cvfRenderStateDepth.h"
#include "cvfTexture.h"
#include "cvfSampler.h"
#include "cvfRenderStateTextureBindings.h"
#include "cvfRect.h"

#ifndef CVF_OPENGL_ES
#include "cvfRenderState_FF.h"
#include "cvfTexture2D_FF.h"
#endif


namespace cvf {


//==================================================================================================
///
/// \class cvf::OverlayNavigationCube
/// \ingroup Render
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// Constructor
//--------------------------------------------------------------------------------------------------
OverlayNavigationCube::OverlayNavigationCube(Camera* camera, Font* font)
:   m_camera(camera),
    m_font(font),
    m_xLabel("x"),
    m_yLabel("y"),
    m_zLabel("z"),
    m_textColor(Color3::BLACK),
    m_size(120, 120),
    m_homeViewDirection(-Vec3f::Z_AXIS),
    m_homeUp(Vec3f::Y_AXIS),
    m_hightlightItem(NCI_NONE),
    m_upVector(Vec3d::Z_AXIS),
    m_frontVector(-Vec3d::Y_AXIS),
    m_xFaceColor(Color3::RED),
    m_yFaceColor(Color3::GREEN),
    m_zFaceColor(Color3::BLUE),
    m_itemHighlightColor(Color3::GRAY),
    m_2dItemsColor(Color3::WHITE)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
OverlayNavigationCube::~OverlayNavigationCube()
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OverlayNavigationCube::setAxisLabels(const String& xLabel, const String& yLabel, const String& zLabel)
{
    // Clipping of axis label text is depends on m_size and
    // z-part of axisMatrix.setTranslation(Vec3d(0, 0, -4.4)) defined in OverlayNavigationCube::render()
    CVF_ASSERT (xLabel.size() < 5 && yLabel.size() < 5 && zLabel.size() < 5);

    m_xLabel = xLabel;
    m_yLabel = yLabel;
    m_zLabel = zLabel;
}


//--------------------------------------------------------------------------------------------------
/// Set color of the axis labels
//--------------------------------------------------------------------------------------------------
void OverlayNavigationCube::setAxisLabelsColor(const Color3f& color)
{
    m_textColor = color;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec2ui OverlayNavigationCube::sizeHint()
{
    return m_size;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OverlayNavigationCube::setSize(const Vec2ui& size)
{
    m_size = size;
}


//--------------------------------------------------------------------------------------------------
/// Hardware rendering using shader programs
//--------------------------------------------------------------------------------------------------
void OverlayNavigationCube::render(OpenGLContext* oglContext, const Vec2i& position, const Vec2ui& size)
{
    render(oglContext, position, size, false);
}


//--------------------------------------------------------------------------------------------------
/// Software rendering 
//--------------------------------------------------------------------------------------------------
void OverlayNavigationCube::renderSoftware(OpenGLContext* oglContext, const Vec2i& position, const Vec2ui& size)
{
    render(oglContext, position, size, true);
}


//--------------------------------------------------------------------------------------------------
/// Set up camera/viewport and render
//--------------------------------------------------------------------------------------------------
void OverlayNavigationCube::render(OpenGLContext* oglContext, const Vec2i& position, const Vec2ui& size, bool software)
{
    if (size.x() <= 0 || size.y() <= 0)
    {
        return;
    }

    if (software && ShaderProgram::supportedOpenGL(oglContext))
    {
        ShaderProgram::useNoProgram(oglContext);
    }

    if (m_axis.isNull()) 
    {
        createAxisGeometry(software);
    }

    if (m_cubeGeos.size() == 0)
    {
        createCubeGeos();

        if (!software)
        {
            // Create the shader for the cube geometry
            ShaderProgramGenerator gen("CubeGeoShader", ShaderSourceProvider::instance());
            gen.configureStandardHeadlightColor();
            m_cubeGeoShader = gen.generate();
            m_cubeGeoShader->linkProgram(oglContext);

            {
                ShaderProgramGenerator gen("CubeGeoTextureShader", ShaderSourceProvider::instance());

                gen.addVertexCode(cvf::ShaderSourceRepository::vs_Standard);
                gen.addFragmentCode(cvf::ShaderSourceRepository::src_Texture);
                gen.addFragmentCode(cvf::ShaderSourceRepository::light_Headlight);
                gen.addFragmentCode(cvf::ShaderSourceRepository::fs_Standard);

                m_cubeGeoTextureShader = gen.generate();

                m_cubeGeoTextureShader->setDefaultUniform(new cvf::UniformFloat("u_specularIntensity", 0.1f));
                m_cubeGeoTextureShader->setDefaultUniform(new cvf::UniformFloat("u_ambientIntensity",  0.2f));
                m_cubeGeoTextureShader->setDefaultUniform(new cvf::UniformFloat("u_emissiveColor",    cvf::Vec3f(0.0f, 0.0f, 0.0f)));
                m_cubeGeoTextureShader->setDefaultUniform(new cvf::UniformFloat("u_ecLightPosition",  cvf::Vec3f(0.5f, 5.0f, 7.0f)));

                m_cubeGeoTextureShader->linkProgram(oglContext);
            }
        }
    }

    if (m_2dGeos.size() == 0)
    {
        create2dGeos();
    }

    // Setup camera
    Camera cam;
    configureLocalCamera(&cam, position, size);

    // Setup viewport
    cam.viewport()->applyOpenGL(oglContext, Viewport::CLEAR_DEPTH);
    cam.applyOpenGL();


    // Do the actual rendering
    MatrixState matrixState(cam);
    if (software)
    {
        renderAxisImmediateMode(oglContext, matrixState);
    }
    else
    {
        renderAxis(oglContext, matrixState);
    }

    renderCubeGeos(oglContext, software, matrixState);
    renderAxisLabels(oglContext, software, matrixState);

    render2dItems(oglContext, position, size, software);
}


//--------------------------------------------------------------------------------------------------
/// Draw the axis
//--------------------------------------------------------------------------------------------------
void OverlayNavigationCube::renderAxis(OpenGLContext* oglContext, const MatrixState& matrixState)
{
    CVF_ASSERT(m_axis.notNull());

    OpenGLResourceManager* resourceManager = oglContext->resourceManager();
    ref<ShaderProgram> vectorProgram = resourceManager->getLinkedVectorDrawerShaderProgram(oglContext);

    if (vectorProgram->useProgram(oglContext))
    {
        vectorProgram->clearUniformApplyTracking();
        vectorProgram->applyFixedUniforms(oglContext, matrixState);
    }

    // Draw X, Y and Z vectors
    m_axis->render(oglContext, vectorProgram.p(), matrixState);
}


//--------------------------------------------------------------------------------------------------
/// Draw the axis using immediate mode OpenGL
//--------------------------------------------------------------------------------------------------
void OverlayNavigationCube::renderAxisImmediateMode(OpenGLContext* oglContext, const MatrixState& matrixState)
{
#ifdef CVF_OPENGL_ES
    CVF_FAIL_MSG("Not supported on OpenGL ES");
#else
    m_axis->renderImmediateMode(oglContext, matrixState);  
#endif // CVF_OPENGL_ES
}


//--------------------------------------------------------------------------------------------------
/// Create the geometry used to draw the axis (vector arrows) and the two triangles
//--------------------------------------------------------------------------------------------------
void OverlayNavigationCube::createAxisGeometry(bool software)
{
    CVF_ASSERT(m_axis.isNull());

    // Axis colors
    ref<Color3fArray> colorArray = new Color3fArray;
    colorArray->resize(3);
    colorArray->set(0, Color3::RED);                // X axis
    colorArray->set(1, Color3::GREEN);              // Y axis
    colorArray->set(2, Color3::BLUE);               // Z axis

    // Positions of the vectors - All in origo
    Vec3f cp[8];
    navCubeCornerPoints(cp);

    ref<cvf::Vec3fArray> vertexArray = new Vec3fArray;
    vertexArray->resize(3);
    vertexArray->set(0, cp[0]);                     // X axis
    vertexArray->set(1, cp[0]);                     // Y axis
    vertexArray->set(2, cp[0]);                     // Z axis

    // Direction & magnitude of the vectors
    float arrowLength = 0.8f;

    ref<cvf::Vec3fArray> vectorArray = new Vec3fArray;
    vectorArray->resize(3);
    vectorArray->set(0, arrowLength*Vec3f::X_AXIS); 
    vectorArray->set(1, arrowLength*Vec3f::Y_AXIS);
    vectorArray->set(2, arrowLength*Vec3f::Z_AXIS);

    // Create the arrow glyph for the vector drawer
    GeometryBuilderTriangles arrowBuilder;
    ArrowGenerator gen;
    gen.setShaftRelativeRadius(0.020f);
    gen.setHeadRelativeRadius(0.05f);
    gen.setHeadRelativeLength(0.1f);
    gen.setNumSlices(30);
    gen.generate(&arrowBuilder);

    if (software)
    {
        m_axis = new DrawableVectors();
    }
    else
    {
        m_axis = new DrawableVectors("u_transformationMatrix", "u_color");
    }

    m_axis->setVectors(vertexArray.p(), vectorArray.p());
    m_axis->setColors(colorArray.p());
    m_axis->setGlyph(arrowBuilder.trianglesUShort().p(), arrowBuilder.vertices().p());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OverlayNavigationCube::updateTextureBindings(OpenGLContext* oglContext, bool software)
{
    m_faceTextureBindings.clear();

    for (size_t i = 0; i < 6; i++)
    {
        NavCubeFace face = static_cast<NavCubeFace>(i);
        std::map<NavCubeFace, ref<TextureImage> >::iterator it = m_faceTextures.find(face);

        if (it != m_faceTextures.end())
        {
            if (software)
            {
#ifndef CVF_OPENGL_ES
                // Use fixed function texture setup
                ref<Texture2D_FF> texture = new Texture2D_FF(it->second.p());
                texture->setWrapMode(Texture2D_FF::CLAMP);
                texture->setMinFilter(Texture2D_FF::NEAREST);
                texture->setMagFilter(Texture2D_FF::NEAREST);
                texture->setupTexture(oglContext);
                texture->setupTextureParams(oglContext);

                ref<RenderStateTextureMapping_FF> textureMapping = new RenderStateTextureMapping_FF(texture.p());
                textureMapping->setTextureFunction(RenderStateTextureMapping_FF::MODULATE);

                m_faceTextureBindings[face] = textureMapping;
#else
                CVF_FAIL_MSG("Not supported on OpenGL ES");
#endif

            }
            else
            {
                ref<cvf::Texture> texture = new cvf::Texture(it->second.p());
                ref<cvf::Sampler> sampler = new cvf::Sampler;
                texture->enableMipmapGeneration(true);
                sampler->setWrapMode(Sampler::CLAMP_TO_EDGE);
                sampler->setMinFilter(Sampler::LINEAR_MIPMAP_LINEAR);
                sampler->setMagFilter(Sampler::NEAREST);

                ref<cvf::RenderStateTextureBindings> texBind = new cvf::RenderStateTextureBindings;
                texBind->addBinding(texture.p(), sampler.p(), "u_texture2D");

                texBind->setupTextures(oglContext);

                m_faceTextureBindings[face] = texBind;
            }
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OverlayNavigationCube::renderCubeGeos(OpenGLContext* oglContext, bool software, const MatrixState& matrixState)
{
    CVF_UNUSED(software);

    if (m_faceTextureBindings.size() != m_faceTextures.size())
    {
        updateTextureBindings(oglContext, software);
    }

    if (software)
    {
#ifndef CVF_OPENGL_ES
        RenderStateMaterial_FF mat;
        mat.enableColorMaterial(true);
        mat.applyOpenGL(oglContext);

        RenderStateLighting_FF light;
        light.applyOpenGL(oglContext);
#endif
    }

    for (size_t i = 0; i < 6; i++)
    {
        NavCubeFace face = static_cast<NavCubeFace>(i);

        std::map<NavCubeFace, ref<TextureImage> >::iterator it = m_faceTextures.find(m_cubeGeoFace[i]);
        ShaderProgram* shader = NULL;
        bool hasTexture = it != m_faceTextures.end();

        if (hasTexture)
        {
            RenderState* textureBinding = m_faceTextureBindings[face].p();
            CVF_ASSERT(textureBinding);
            textureBinding->applyOpenGL(oglContext);
        }

        if (!software)
        {
            shader = hasTexture ? m_cubeGeoTextureShader.p() : m_cubeGeoShader.p();

            if (shader->useProgram(oglContext))
            {
                shader->applyFixedUniforms(oglContext, matrixState);
            }
        }

        Color3f faceColor = Color3f(Color3::WHITE);

        if (!hasTexture)
        {
            switch (face)
            {
                case NCF_X_POS:
                case NCF_X_NEG:     faceColor = m_xFaceColor; break;
                case NCF_Y_POS:
                case NCF_Y_NEG:     faceColor = m_yFaceColor; break;
                case NCF_Z_POS:
                case NCF_Z_NEG:     faceColor = m_zFaceColor; break;
            }
        }

        for (size_t i  = 0; i < m_cubeGeos.size(); ++i)
        {
            if (m_cubeGeoFace[i] == face)
            {
                Color3f renderFaceColor = faceColor;
                cvf::Vec3f emissiveColor = cvf::Vec3f(0.0f, 0.0f, 0.0f);

                if (m_cubeItemType[i] == m_hightlightItem)
                {
                    renderFaceColor = m_itemHighlightColor;
                    emissiveColor = cvf::Vec3f(-0.25f, -0.25f, -0.25f);
                }

                if (software)
                {
#ifdef CVF_OPENGL_ES
                    CVF_FAIL_MSG("Not supported on OpenGL ES");
#else
                    glColor3fv(renderFaceColor.ptr());
                    m_cubeGeos[i]->renderImmediateMode(oglContext, matrixState);
#endif
                }
                else
                {
                    if (hasTexture)
                    {
                        UniformFloat uniform("u_emissiveColor", emissiveColor);
                        shader->applyUniform(oglContext, uniform);
                    }
                    else
                    {
                        UniformFloat uniform("u_color", Color4f(renderFaceColor));
                        shader->applyUniform(oglContext, uniform);
                    }

                    m_cubeGeos[i]->render(oglContext, m_cubeGeoShader.p(), matrixState);
                }
            }
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OverlayNavigationCube::render2dItems(OpenGLContext* oglContext, const Vec2i& position, const Vec2ui& size, bool software)
{
    Camera cam;
    cam.setViewport(position.x(), position.y(), size.x(), size.y());
    cam.setProjectionAsUnitOrtho();

    // Setup viewport
    cam.viewport()->applyOpenGL(oglContext, Viewport::CLEAR_DEPTH);
    cam.applyOpenGL();

    RenderStateDepth depth(false);
    depth.applyOpenGL(oglContext);

    MatrixState matrixState(cam);

    if (software)
    {
#ifdef CVF_OPENGL_ES
        CVF_FAIL_MSG("Not supported on OpenGL ES");
#else
        RenderStateLighting_FF light(false);
        light.applyOpenGL(oglContext);
        glColor3fv(m_hightlightItem == NCI_HOME ? m_itemHighlightColor.ptr() : m_2dItemsColor.ptr());

        m_homeGeo->renderImmediateMode(oglContext, matrixState);
#endif
    }
    else
    {
        if (m_cubeGeoShader->useProgram(oglContext))
        {
            m_cubeGeoShader->applyFixedUniforms(oglContext, matrixState);
        }

        UniformFloat colorUniform("u_color", Color4f(m_hightlightItem == NCI_HOME ? m_itemHighlightColor : m_2dItemsColor));
        m_cubeGeoShader->applyUniform(oglContext, colorUniform);
        m_homeGeo->render(oglContext, m_cubeGeoShader.p(), matrixState);
    }

    if (isFaceAlignedViewPoint())
    {
        for (size_t i  = 0; i < m_2dGeos.size(); ++i)
        {
            Color3f renderFaceColor = Color3f(1,1,1);

            if (m_2dItemType[i] == m_hightlightItem)
            {
                renderFaceColor = m_itemHighlightColor;
            }

            if (software)
            {
#ifdef CVF_OPENGL_ES
                CVF_FAIL_MSG("Not supported on OpenGL ES");
#else
                glColor3fv(renderFaceColor.ptr());
                m_2dGeos[i]->renderImmediateMode(oglContext, matrixState);
#endif
            }
            else
            {
                UniformFloat colorUniform("u_color", Color4f(renderFaceColor));
                m_cubeGeoShader->applyUniform(oglContext, colorUniform);

                m_2dGeos[i]->render(oglContext, m_cubeGeoShader.p(), matrixState);
            }
        }
    }

    RenderStateDepth resetDepth;
    resetDepth.applyOpenGL(oglContext);

    if (software)
    {
#ifdef CVF_OPENGL_ES
        CVF_FAIL_MSG("Not supported on OpenGL ES");
#else
        RenderStateLighting_FF resetLight;
        resetLight.applyOpenGL(oglContext);
#endif
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OverlayNavigationCube::create2dGeos()
{
    // "Home" aka. House geometry
    {
        m_homeGeo = new DrawableGeo;

        ref<Vec3fArray> vertexArray = new Vec3fArray(12);
        vertexArray->set(0, Vec3f(-0.97f, 0.86f, 0.0f));
        vertexArray->set(1, Vec3f(-0.68f, 0.86f, 0.0f));
        vertexArray->set(2, Vec3f(-0.825f, 1.0f, 0.0f));
        vertexArray->set(3, Vec3f(-0.825f, 1.0f, 0.0f));

        vertexArray->set(4, Vec3f(-0.9f, 0.76f, 0.0f));
        vertexArray->set(5, Vec3f(-0.75f, 0.76f, 0.0f));
        vertexArray->set(6, Vec3f(-0.75f, 0.86f, 0.0f));
        vertexArray->set(7, Vec3f(-0.9f, 0.86f, 0.0f));

        vertexArray->set(8, Vec3f(-0.77f, 0.86f, 0.0f));
        vertexArray->set(9, Vec3f(-0.75f, 0.86f, 0.0f));
        vertexArray->set(10, Vec3f(-0.75f, 1.0f, 0.0f));
        vertexArray->set(11, Vec3f(-0.77f, 1.0f, 0.0f));

        for (size_t i = 0; i < vertexArray->size(); ++i)
        {
            Vec3f v = vertexArray->get(i);
            v.x() = 0.5f + v.x()/2.0f;
            v.y() = 0.5f + v.y()/2.0f;
            vertexArray->set(i, v);
        }

        m_homeGeo->setVertexArray(vertexArray.p());

        ref<cvf::UShortArray> indices = new cvf::UShortArray(18);
        indices->set(0, 0);  indices->set(1, 1); indices->set(2, 2);
        indices->set(3, 0);  indices->set(4, 2); indices->set(5, 3);

        indices->set(6, 4);  indices->set(7, 5); indices->set(8, 6);
        indices->set(9, 4);  indices->set(10, 6); indices->set(11, 7);

        indices->set(12, 8);  indices->set(13, 9); indices->set(14, 10);
        indices->set(15, 8);  indices->set(16, 10); indices->set(17, 11);
        
        ref<cvf::PrimitiveSetIndexedUShort> primSet = new cvf::PrimitiveSetIndexedUShort(cvf::PT_TRIANGLES);
        primSet->setIndices(indices.p());
        m_homeGeo->addPrimitiveSet(primSet.p());
        m_homeGeo->computeNormals();
    }

    m_2dGeos.push_back(create2dArrow(Vec3f(-0.7f, 0,0), Vec3f(-0.9f, 0,0)).p());    m_2dItemType.push_back(NCI_ARROW_LEFT);
    m_2dGeos.push_back(create2dArrow(Vec3f(0.7f,  0,0), Vec3f(0.9f,  0,0)).p());    m_2dItemType.push_back(NCI_ARROW_RIGHT);
    m_2dGeos.push_back(create2dArrow(Vec3f(0, -0.7f,0), Vec3f(0, -0.9f,0)).p());    m_2dItemType.push_back(NCI_ARROW_BOTTOM);
    m_2dGeos.push_back(create2dArrow(Vec3f(0,  0.7f,0), Vec3f(0,  0.9f,0)).p());    m_2dItemType.push_back(NCI_ARROW_TOP);

    // Rotate arrows
    m_2dGeos.push_back(create2dArrow(Vec3f(0.75f, 0.65f, 0.0f), Vec3f(0.87f,0.49f, 0.0f)).p());    m_2dItemType.push_back(NCI_ROTATE_CW);
    m_2dGeos.push_back(create2dArrow(Vec3f(0.71f, 0.70f, 0.0f), Vec3f(0.59f,0.86f, 0.0f)).p());    m_2dItemType.push_back(NCI_ROTATE_CCW);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<DrawableGeo> OverlayNavigationCube::create2dArrow(const Vec3f& start, const Vec3f& end)
{
    float fWidth = 0.042f;
    float fArrowWidth = 0.12f;
    float fBaseRelLength = 0.50f;
    float fLength = (end - start).length();

    Vec3f vUp = Vec3f(0,0,1);
    Vec3f vDir = (end - start);
    Vec3f vRight = vUp^vDir;
    vDir.normalize();
    vRight.normalize();

    Vec3f vBaseBL = start + vDir*fLength*fBaseRelLength + vRight*fWidth;
    Vec3f vBaseBR = start + vRight*fWidth;
    Vec3f vBaseTL = start + vDir*fLength*fBaseRelLength - vRight*fWidth;
    Vec3f vBaseTR = start - vRight*fWidth;

    Vec3f vArrowB = start + vDir*fLength*fBaseRelLength + vRight*fArrowWidth;
    Vec3f vArrowT = start + vDir*fLength*fBaseRelLength - vRight*fArrowWidth;

    ref<DrawableGeo> geo = new DrawableGeo;

    ref<Vec3fArray> vertexArray = new Vec3fArray(7);

    vertexArray->set(0, vBaseBL);
    vertexArray->set(1, vBaseBR);
    vertexArray->set(2, vBaseTR);
    vertexArray->set(3, vBaseTL);
                        
    vertexArray->set(4, vArrowT);
    vertexArray->set(5, end);
    vertexArray->set(6, vArrowB);

    for (size_t i = 0; i < vertexArray->size(); ++i)
    {
        Vec3f v = vertexArray->get(i);
        v.x() = 0.5f + v.x()/2.0f;
        v.y() = 0.5f + v.y()/2.0f;
        vertexArray->set(i, v);
    }

    geo->setVertexArray(vertexArray.p());

    ref<cvf::UShortArray> indices = new cvf::UShortArray(9);
    indices->set(0, 0);  indices->set(1, 1); indices->set(2, 2);
    indices->set(3, 0);  indices->set(4, 2); indices->set(5, 3);
    indices->set(6, 4);  indices->set(7, 5); indices->set(8, 6);

    ref<cvf::PrimitiveSetIndexedUShort> primSet = new cvf::PrimitiveSetIndexedUShort(cvf::PT_TRIANGLES);
    primSet->setIndices(indices.p());
    geo->addPrimitiveSet(primSet.p());
    geo->computeNormals();

    m_2dGeos.push_back(geo.p());
    m_2dItemType.push_back(NCI_HOME);

    return geo;
}


//--------------------------------------------------------------------------------------------------
/// Draw the axis labels
//--------------------------------------------------------------------------------------------------
void OverlayNavigationCube::renderAxisLabels(OpenGLContext* oglContext, bool software, const MatrixState& matrixState)
{
    if (m_xLabel.isEmpty() && m_yLabel.isEmpty() && m_zLabel.isEmpty())
    {
        return;
    }

    float fBoxLength = 0.65f;

    // Multiply with 1.08 will slightly pull the labels away from the corresponding arrow head
    Vec3f xPos(0.5f, -fBoxLength/2.0f, -fBoxLength/2.0f);
    Vec3f yPos(-fBoxLength/2.0f, 0.5f, -fBoxLength/2.0f);
    Vec3f zPos(-fBoxLength/2.0f, -fBoxLength/2.0f, 0.5f);

    DrawableText drawableText;
    drawableText.setFont(m_font.p());
    drawableText.setCheckPosVisible(false);
    drawableText.setUseDepthBuffer(true);
    drawableText.setDrawBorder(false);
    drawableText.setDrawBackground(false);
    drawableText.setVerticalAlignment(TextDrawer::CENTER);
    drawableText.setTextColor(m_textColor);

    if (!m_xLabel.isEmpty()) drawableText.addText(m_xLabel, xPos);
    if (!m_yLabel.isEmpty()) drawableText.addText(m_yLabel, yPos);
    if (!m_zLabel.isEmpty()) drawableText.addText(m_zLabel, zPos);

    // Do the actual rendering
    // -----------------------------------------------
    if (software)
    {
        drawableText.renderSoftware(oglContext, matrixState);
    }
    else
    {
        ref<ShaderProgram> textShader = oglContext->resourceManager()->getLinkedTextShaderProgram(oglContext);
        drawableText.render(oglContext, textShader.p(), matrixState);
    }
}


//--------------------------------------------------------------------------------------------------
///											  Face (with local indices):
///				   7---------6                 4                    3  
///				  /|	    /|	   |z   		|---|----------|---|
///				 / |	   / |	   |  / y  	    |TL |   TOP    |TR |
///				4---------5  |     | /   		|---|----------|---|
///				|  3------|--2	   *---x		|   |          |   |
///				| /		  | /	       			| L |  CENTER  | R |
///				|/        |/	      			|   |          |   |
///				0---------1						|---|----------|---|
///											    |BL |  BOTTOM  |BR |
///												|---|----------|---|
///											   1                    2
///
///	Items:
///		Faces:
///		+X : VT_NCI_X_POS : RIGHT	: 0 2 6 5 
///		-X : VT_NCI_X_NEG : LEFT	: 3 0 4 7
///		+Y : VT_NCI_Y_POS : BACK	: 2 3 7 6
///		-Y : VT_NCI_Y_NEG : FRONT	: 0 1 5 4
///		+Z : VT_NCI_Z_POS : TOP		: 4 5 6 7
///		-Z : VT_NCI_Z_NEG : BOTTOM	: 3 2 1 0
///
///		Corners: 
///		0 : VT_NCI_CORNER_XN_YN_ZN
///		1 : VT_NCI_CORNER_XP_YN_ZN
///		2 : VT_NCI_CORNER_XP_YP_ZN
///		3 : VT_NCI_CORNER_XN_YP_ZN
///		4 : VT_NCI_CORNER_XN_YN_ZP
///		5 : VT_NCI_CORNER_XP_YN_ZP
///		6 : VT_NCI_CORNER_XP_YP_ZP
///		7 : VT_NCI_CORNER_XN_YP_ZP
///
///		Edges:
///		01: VT_NCI_EDGE_YN_ZN
///		12: VT_NCI_EDGE_XP_ZN
///		23: VT_NCI_EDGE_YP_ZN
///		03: VT_NCI_EDGE_XN_ZN
///		45: VT_NCI_EDGE_YN_ZP
///		56: VT_NCI_EDGE_XP_ZP
///		67: VT_NCI_EDGE_YP_ZP
///		47: VT_NCI_EDGE_XN_ZP
///		04: VT_NCI_EDGE_XN_YN
///		15: VT_NCI_EDGE_XP_YN
///		26: VT_NCI_EDGE_XP_YP
///		37: VT_NCI_EDGE_XN_YP
//--------------------------------------------------------------------------------------------------
void OverlayNavigationCube::createCubeGeos()
{
    Vec3f cp[8];
    navCubeCornerPoints(cp);

    m_cubeGeos.clear();

    createCubeFaceGeos(NCF_Y_NEG, cp[0], cp[1], cp[5], cp[4]);
    createCubeFaceGeos(NCF_Y_POS, cp[2], cp[3], cp[7], cp[6]);
                                                                                                             
    createCubeFaceGeos(NCF_Z_POS, cp[4], cp[5], cp[6], cp[7]);
    createCubeFaceGeos(NCF_Z_NEG, cp[3], cp[2], cp[1], cp[0]);
                                                            
    createCubeFaceGeos(NCF_X_NEG, cp[3], cp[0], cp[4], cp[7]);
    createCubeFaceGeos(NCF_X_POS, cp[1], cp[2], cp[6], cp[5]);
}


//--------------------------------------------------------------------------------------------------
///											  Face (with local indices):
///			               4                    3  
///			   |z   		|---|----------|---|
///			   |  / y  	    |TL |   TOP    |TR |
///			   | /   		|---|----------|---|
///			   *---x		|   |          |   |
///			       			| L |  CENTER  | R |
///			      			|   |          |   |
///							|---|----------|---|
///						    |BL |  BOTTOM  |BR |
///			    			|---|----------|---|
///                        1                    2
//--------------------------------------------------------------------------------------------------
void OverlayNavigationCube::createCubeFaceGeos(NavCubeFace face, Vec3f p1, Vec3f p2, Vec3f p3, Vec3f p4)
{
    Vec2f t1(0,0);
    Vec2f t2(1,0);
    Vec2f t3(1,1);
    Vec2f t4(0,1);

    float fCornerFactor = 0.175f;
    float fOneMinusCF   = 1.0f - fCornerFactor;
    Vec3f p12 = p1 + (p2 - p1)*fCornerFactor;   Vec2f t12(fCornerFactor,    0);
    Vec3f p14 = p1 + (p4 - p1)*fCornerFactor;   Vec2f t14(0,                fCornerFactor);
    Vec3f pi1 = p1 + (p12 - p1) + (p14 - p1);   Vec2f ti1(fCornerFactor,    fCornerFactor);

    Vec3f p21 = p2 + (p1 - p2)*fCornerFactor;   Vec2f t21(fOneMinusCF,      0);
    Vec3f p23 = p2 + (p3 - p2)*fCornerFactor;   Vec2f t23(1.0,              fCornerFactor);
    Vec3f pi2 = p2 + (p21 - p2) + (p23 - p2);   Vec2f ti2(fOneMinusCF,      fCornerFactor);

    Vec3f p32 = p3 + (p2 - p3)*fCornerFactor;   Vec2f t32(1.0,              fOneMinusCF);
    Vec3f p34 = p3 + (p4 - p3)*fCornerFactor;   Vec2f t34(fOneMinusCF,      1.0);
    Vec3f pi3 = p3 + (p32 - p3) + (p34 - p3);   Vec2f ti3(fOneMinusCF,      fOneMinusCF);

    Vec3f p41 = p4 + (p1 - p4)*fCornerFactor;   Vec2f t41(0,                fOneMinusCF);
    Vec3f p43 = p4 + (p3 - p4)*fCornerFactor;   Vec2f t43(fCornerFactor,    1.0);
    Vec3f pi4 = p4 + (p41 - p4) + (p43 - p4);   Vec2f ti4(fCornerFactor,    fOneMinusCF);

    // Bottom left
    m_cubeItemType.push_back(navCubeItem(face, NCFI_BOTTOM_LEFT));
    m_cubeGeoFace.push_back(face);
    m_cubeGeos.push_back(createQuadGeo(p1, p12, pi1, p14, t1, t12, ti1, t14).p());

    // Bottom right
    m_cubeItemType.push_back(navCubeItem(face, NCFI_BOTTOM_RIGHT));
    m_cubeGeoFace.push_back(face);
    m_cubeGeos.push_back(createQuadGeo(p2, p23, pi2, p21, t2, t23, ti2, t21).p());

    // Top right
    m_cubeItemType.push_back(navCubeItem(face, NCFI_TOP_RIGHT));
    m_cubeGeoFace.push_back(face);
    m_cubeGeos.push_back(createQuadGeo(p3, p34, pi3, p32, t3, t34, ti3, t32).p());

    // Top left
    m_cubeItemType.push_back(navCubeItem(face, NCFI_TOP_LEFT));
    m_cubeGeoFace.push_back(face);
    m_cubeGeos.push_back(createQuadGeo(p4, p41, pi4, p43, t4, t41, ti4, t43).p());

    // Bottom
    m_cubeItemType.push_back(navCubeItem(face, NCFI_BOTTOM));
    m_cubeGeoFace.push_back(face);
    m_cubeGeos.push_back(createQuadGeo(p12, p21, pi2, pi1, t12, t21, ti2, ti1).p());

    // Top
    m_cubeItemType.push_back(navCubeItem(face, NCFI_TOP));
    m_cubeGeoFace.push_back(face);
    m_cubeGeos.push_back(createQuadGeo(p34, p43, pi4, pi3, t34, t43, ti4, ti3).p());

    // Right
    m_cubeItemType.push_back(navCubeItem(face, NCFI_RIGHT));
    m_cubeGeoFace.push_back(face);
    m_cubeGeos.push_back(createQuadGeo(p23, p32, pi3, pi2, t23, t32, ti3, ti2).p());

    // Left
    m_cubeItemType.push_back(navCubeItem(face, NCFI_LEFT));
    m_cubeGeoFace.push_back(face);
    m_cubeGeos.push_back(createQuadGeo(p41, p14, pi1, pi4, t41, t14, ti1, ti4).p());

    // Inner part
    m_cubeItemType.push_back(navCubeItem(face, NCFI_CENTER));
    m_cubeGeoFace.push_back(face);
    m_cubeGeos.push_back(createQuadGeo(pi1, pi2, pi3, pi4, ti1, ti2, ti3, ti4).p());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<DrawableGeo> OverlayNavigationCube::createQuadGeo(const Vec3f& v1, const Vec3f& v2, const Vec3f& v3, const Vec3f& v4, const Vec2f& t1, const Vec2f& t2, const Vec2f& t3, const Vec2f& t4)
{
    ref<DrawableGeo> geo = new DrawableGeo;

    ref<Vec3fArray> vertexArray = new Vec3fArray(4);
    vertexArray->set(0, v1);
    vertexArray->set(1, v2);
    vertexArray->set(2, v3);
    vertexArray->set(3, v4);

    ref<Vec2fArray> textureCoordArray = new Vec2fArray(4);
    textureCoordArray->set(0, t1);
    textureCoordArray->set(1, t2);
    textureCoordArray->set(2, t3);
    textureCoordArray->set(3, t4);

    geo->setVertexArray(vertexArray.p());
    geo->setTextureCoordArray(textureCoordArray.p());

    ref<cvf::UShortArray> indices = new cvf::UShortArray(6);
    indices->set(0, 0);
    indices->set(1, 1);
    indices->set(2, 2);
    indices->set(3, 0);
    indices->set(4, 2);
    indices->set(5, 3);

    ref<cvf::PrimitiveSetIndexedUShort> primSet = new cvf::PrimitiveSetIndexedUShort(cvf::PT_TRIANGLES);
    primSet->setIndices(indices.p());
    geo->addPrimitiveSet(primSet.p());
    geo->computeNormals();

    return geo;
}


//--------------------------------------------------------------------------------------------------
///   		   7---------6                
///   		  /|	    /|	   |z   	
///   		 / |	   / |	   |  / y  	
///   		4---------5  |     | /   	
///   		|  3------|--2	   *---x	
///   		| /		  | /	       	
///   		|/        |/	      	
///   		0---------1             		
//--------------------------------------------------------------------------------------------------
void OverlayNavigationCube::navCubeCornerPoints(Vec3f points[8])
{
    float fBoxLength = 0.65f;

    Vec3f min(-fBoxLength/2.0f, -fBoxLength/2.0f, -fBoxLength/2.0f);
    Vec3f max(fBoxLength/2.0f, fBoxLength/2.0f, fBoxLength/2.0f);

    points[0].set(min.x(), min.y(), min.z());
    points[1].set(max.x(), min.y(), min.z());
    points[2].set(max.x(), max.y(), min.z());
    points[3].set(min.x(), max.y(), min.z());
    points[4].set(min.x(), min.y(), max.z());
    points[5].set(max.x(), min.y(), max.z());
    points[6].set(max.x(), max.y(), max.z());
    points[7].set(min.x(), max.y(), max.z());
}


//--------------------------------------------------------------------------------------------------
/// Convert face + faceItem to NavCubeItem
//--------------------------------------------------------------------------------------------------
OverlayNavigationCube::NavCubeItem OverlayNavigationCube::navCubeItem(NavCubeFace face, NavCubeFaceItem faceItem) const
{
	NavCubeItem item = NCI_NONE;

	switch(face)
	{
		case NCF_X_POS:
		{
			switch(faceItem)
			{
				case NCFI_CENTER:		item = NCI_FACE_X_POS; break;
				case NCFI_TOP:			item = NCI_EDGE_XP_ZP; break;
				case NCFI_BOTTOM:		item = NCI_EDGE_XP_ZN; break;
				case NCFI_LEFT:			item = NCI_EDGE_XP_YN; break;
				case NCFI_RIGHT:		item = NCI_EDGE_XP_YP; break;
				case NCFI_TOP_LEFT:		item = NCI_CORNER_XP_YN_ZP; break;
				case NCFI_TOP_RIGHT:	item = NCI_CORNER_XP_YP_ZP; break;
				case NCFI_BOTTOM_LEFT:	item = NCI_CORNER_XP_YN_ZN; break;
				case NCFI_BOTTOM_RIGHT:	item = NCI_CORNER_XP_YP_ZN; break;
				case NCFI_NONE:			item = NCI_NONE; break;
			}
			break;
		}
		case NCF_X_NEG:
		{
			switch(faceItem)
			{
				case NCFI_CENTER:		item = NCI_FACE_X_NEG; break;
				case NCFI_TOP:			item = NCI_EDGE_XN_ZP; break;
				case NCFI_BOTTOM:		item = NCI_EDGE_XN_ZN; break;
				case NCFI_LEFT:			item = NCI_EDGE_XN_YP; break;
				case NCFI_RIGHT:		item = NCI_EDGE_XN_YN; break;
				case NCFI_TOP_LEFT:		item = NCI_CORNER_XN_YP_ZP; break;
				case NCFI_TOP_RIGHT:	item = NCI_CORNER_XN_YN_ZP; break;
				case NCFI_BOTTOM_LEFT:	item = NCI_CORNER_XN_YP_ZN; break;
				case NCFI_BOTTOM_RIGHT:	item = NCI_CORNER_XN_YN_ZN; break;
				case NCFI_NONE:			item = NCI_NONE; break;
			}
			break;
		}
		case NCF_Y_POS:
		{
			switch(faceItem)
			{
				case NCFI_CENTER:		item = NCI_FACE_Y_POS; break;
				case NCFI_TOP:			item = NCI_EDGE_YP_ZP; break;
				case NCFI_BOTTOM:		item = NCI_EDGE_YP_ZN; break;
				case NCFI_LEFT:			item = NCI_EDGE_XP_YP; break;
				case NCFI_RIGHT:		item = NCI_EDGE_XN_YP; break;
				case NCFI_TOP_LEFT:		item = NCI_CORNER_XP_YP_ZP; break;
				case NCFI_TOP_RIGHT:	item = NCI_CORNER_XN_YP_ZP; break;
				case NCFI_BOTTOM_LEFT:	item = NCI_CORNER_XP_YP_ZN; break;
				case NCFI_BOTTOM_RIGHT:	item = NCI_CORNER_XN_YP_ZN; break;
				case NCFI_NONE:			item = NCI_NONE; break;
			}
			break;
		}
		case NCF_Y_NEG:
		{
			switch(faceItem)
			{
				case NCFI_CENTER:		item = NCI_FACE_Y_NEG; break;
				case NCFI_TOP:			item = NCI_EDGE_YN_ZP; break;
				case NCFI_BOTTOM:		item = NCI_EDGE_YN_ZN; break;
				case NCFI_LEFT:			item = NCI_EDGE_XN_YN; break;
				case NCFI_RIGHT:		item = NCI_EDGE_XP_YN; break;
				case NCFI_TOP_LEFT:		item = NCI_CORNER_XN_YN_ZP; break;
				case NCFI_TOP_RIGHT:	item = NCI_CORNER_XP_YN_ZP; break;
				case NCFI_BOTTOM_LEFT:	item = NCI_CORNER_XN_YN_ZN; break;
				case NCFI_BOTTOM_RIGHT:	item = NCI_CORNER_XP_YN_ZN; break;
				case NCFI_NONE:			item = NCI_NONE; break;
			}
			break;
		}
		case NCF_Z_POS:
		{
			switch(faceItem)
			{
				case NCFI_CENTER:		item = NCI_FACE_Z_POS; break;
				case NCFI_TOP:			item = NCI_EDGE_YP_ZP; break;
				case NCFI_BOTTOM:		item = NCI_EDGE_YN_ZP; break;
				case NCFI_LEFT:			item = NCI_EDGE_XN_ZP; break;
				case NCFI_RIGHT:		item = NCI_EDGE_XP_ZP; break;
				case NCFI_TOP_LEFT:		item = NCI_CORNER_XN_YP_ZP; break;
				case NCFI_TOP_RIGHT:	item = NCI_CORNER_XP_YP_ZP; break;
				case NCFI_BOTTOM_LEFT:	item = NCI_CORNER_XN_YN_ZP; break;
				case NCFI_BOTTOM_RIGHT:	item = NCI_CORNER_XP_YN_ZP; break;
				case NCFI_NONE:			item = NCI_NONE; break;
			}
			break;
		}
		case NCF_Z_NEG:
		{
			switch(faceItem)
			{
				case NCFI_CENTER:		item = NCI_FACE_Z_NEG; break;
				case NCFI_TOP:			item = NCI_EDGE_YN_ZN; break;
				case NCFI_BOTTOM:		item = NCI_EDGE_YP_ZN; break;
				case NCFI_LEFT:			item = NCI_EDGE_XN_ZN; break;
				case NCFI_RIGHT:		item = NCI_EDGE_XP_ZN; break;
				case NCFI_TOP_LEFT:		item = NCI_CORNER_XN_YN_ZN; break;
				case NCFI_TOP_RIGHT:	item = NCI_CORNER_XP_YN_ZN; break;
				case NCFI_BOTTOM_LEFT:	item = NCI_CORNER_XN_YP_ZN; break;
				case NCFI_BOTTOM_RIGHT:	item = NCI_CORNER_XP_YP_ZN; break;
				case NCFI_NONE:			item = NCI_NONE; break;
			}
			break;
		}
	}

	return item;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool OverlayNavigationCube::pick(int winCoordX, int winCoordY, const Vec2i& position, const Vec2ui& size)
{
    return pickItem(winCoordX, winCoordY, position, size) != cvf::UNDEFINED_SIZE_T;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool OverlayNavigationCube::updateHighlight(int winCoordX, int winCoordY, const Vec2i& position, const Vec2ui& size)
{
    // Early out
    if (winCoordX < position.x() || winCoordX > (position.x() + static_cast<int>(size.x())) ||
        winCoordY < position.y() || winCoordY > (position.y() + static_cast<int>(size.y())))
    {
        bool redraw = m_hightlightItem != NCI_NONE;
        m_hightlightItem = NCI_NONE;
        return redraw;
    }

    NavCubeItem item2d = pick2dItem(winCoordX, winCoordY, position, size);
    if (item2d != NCI_NONE)
    {
        bool redraw = m_hightlightItem != item2d;
        m_hightlightItem = item2d;
        return redraw;
    }

    size_t itemIndex = pickItem(winCoordX, winCoordY, position, size);

    bool redraw = false;

    if (itemIndex == cvf::UNDEFINED_SIZE_T)
    {
        if (m_hightlightItem != NCI_NONE)
        {
            m_hightlightItem = NCI_NONE;
            redraw = true;
        }
    }
    else
    {
        if (m_hightlightItem != m_cubeItemType[itemIndex])
        {
            m_hightlightItem = m_cubeItemType[itemIndex];
            redraw = true;
        }
    }

    return redraw;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool OverlayNavigationCube::processSelection(int winCoordX, int winCoordY, const Vec2i& position, const Vec2ui& size, Vec3d* viewDir, Vec3d* up)
{
    *viewDir = Vec3d::UNDEFINED;
    *up = Vec3d::UNDEFINED;

    NavCubeItem faceItem = pick2dItem(winCoordX, winCoordY, position, size);

    if (faceItem == NCI_NONE)
    {
        size_t minIndex = pickItem(winCoordX, winCoordY, position, size);

        if (minIndex == cvf::UNDEFINED_SIZE_T)
        {
            return false;
        }

        faceItem = m_cubeItemType[minIndex];
    }

    if (faceItem == NCI_NONE)
    {
        return false;
    }

    viewConfigurationFromNavCubeItem(faceItem, viewDir, up);

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t OverlayNavigationCube::pickItem(int winCoordX, int winCoordY, const Vec2i& position, const Vec2ui& size) const
{
    Camera cam;
    configureLocalCamera(&cam, position, size);

    ref<cvf::Ray> ray = cam.rayFromWindowCoordinates(winCoordX, winCoordY);

    double minDistSq = cvf::UNDEFINED_DOUBLE_THRESHOLD;
    size_t minIndex = cvf::UNDEFINED_SIZE_T;

    for (size_t i = 0; i < m_cubeGeos.size(); ++i)
    {
        Vec3d intersectionPoint;
        ref<HitDetail> detail;
        if (m_cubeGeos[i]->rayIntersectCreateDetail(*ray, &intersectionPoint, &detail))
        {
            double distSq = ray->origin().pointDistanceSquared(intersectionPoint);

            if (distSq < minDistSq)
            {
                minDistSq = distSq;
                minIndex = i;
            }
        }
    }

    return minIndex;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
OverlayNavigationCube::NavCubeItem OverlayNavigationCube::pick2dItem(int winCoordX, int winCoordY, const Vec2i& position, const Vec2ui& size) const
{
    Vec2f vpOrigin;
    vpOrigin.x() = static_cast<float>(position.x()) + static_cast<float>(size.x())*0.5f;
    vpOrigin.y() = static_cast<float>(position.y()) + static_cast<float>(size.y())*0.5f;

    Vec2f relCoord; 
    relCoord.x() = (static_cast<float>(winCoordX) - vpOrigin.x())/(static_cast<float>(size.x())*0.5f);
    relCoord.y() = (static_cast<float>(winCoordY) - vpOrigin.y())/(static_cast<float>(size.y())*0.5f);

    // Check for home
    Rectf home(-0.97f, 0.76f, 0.21f, 0.24f);
    if (home.contains(relCoord)) return NCI_HOME;

    if (isFaceAlignedViewPoint())
    {
        float fEnd = 0.9f;
        float fStart = 0.7f;
        float fWidth = 0.12f;

        Rectf leftArrow(-fEnd, -fWidth, 0.2f, 2*fWidth);
        Rectf rightArrow(fStart, -fWidth, 0.2f, 2*fWidth);
        Rectf topArrow(-fWidth, fStart, 2*fWidth, 0.2f);
        Rectf bottomArrow(-fWidth, -fEnd, 2*fWidth, 0.2f);
        Rectf rotateCW(0.75f, 0.49f, 0.12f, 0.16f);
        Rectf rotateCCW(0.59f, 0.70f, 0.12f, 0.16f);

        if (leftArrow.contains(relCoord))		 return NCI_ARROW_LEFT;
        else if (rightArrow.contains(relCoord))  return NCI_ARROW_RIGHT;
        else if (topArrow.contains(relCoord))	 return NCI_ARROW_TOP;
        else if (bottomArrow.contains(relCoord)) return NCI_ARROW_BOTTOM;
        else if (rotateCW.contains(relCoord))	 return NCI_ROTATE_CW;
        else if (rotateCCW.contains(relCoord))	 return NCI_ROTATE_CCW;
    }

    return NCI_NONE;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OverlayNavigationCube::configureLocalCamera(Camera* camera, const Vec2i& position, const Vec2ui& size) const
{
    // Position the camera far enough away to make the axis and the text fit within the viewport
    Mat4d axisMatrix = m_camera->viewMatrix();
    axisMatrix.setTranslation(Vec3d(0, 0, -2.0));

    // Setup camera
    camera->setProjectionAsPerspective(40.0, 0.05, 100.0);
    camera->setViewMatrix(axisMatrix);
    camera->setViewport(position.x(), position.y(), size.x(), size.y());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OverlayNavigationCube::viewConfigurationFromNavCubeItem(NavCubeItem item, Vec3d* viewDir, Vec3d* up)
{
	// Handle Home and Rotate specially, as they do not fall into the simple "view from" category
	if (item == NCI_HOME)
	{
        *viewDir = m_homeViewDirection;
        *up = m_homeUp;
		return;
	}
	else if ((item == NCI_ROTATE_CW) || (item == NCI_ROTATE_CCW))
	{
        *viewDir = m_camera->direction();
        *up = m_camera->up();

        Mat4d mat = Mat4d::fromRotation(*viewDir, Math::toRadians(item == NCI_ROTATE_CW ? -90.0 : 90.0));
        up->transformVector(mat);

        return;
	}

	// Determine the view from point based on the VTNavCubeItem
	Vec3d viewFrom;

	switch(item)
	{
		case NCI_ARROW_LEFT:
		case NCI_ARROW_RIGHT:
		case NCI_ARROW_TOP:
		case NCI_ARROW_BOTTOM:
		{
            Vec3d currentViewDir = m_camera->direction();
            Vec3d currentUp = m_camera->up();
			Vec3d rightVec = currentViewDir^currentUp;

			if		(item == NCI_ARROW_LEFT)	viewFrom = -rightVec;
			else if (item == NCI_ARROW_RIGHT)	viewFrom = rightVec;
			else if (item == NCI_ARROW_TOP)		viewFrom = currentUp;
			else if (item == NCI_ARROW_BOTTOM)	viewFrom = -currentUp;
			break;
		}
		case NCI_CORNER_XN_YN_ZN:	viewFrom = Vec3d(-1, -1, -1); break;
		case NCI_CORNER_XP_YN_ZN:	viewFrom = Vec3d( 1, -1, -1); break;
		case NCI_CORNER_XP_YP_ZN: 	viewFrom = Vec3d( 1,  1, -1); break;
		case NCI_CORNER_XN_YP_ZN: 	viewFrom = Vec3d(-1,  1, -1); break;
		case NCI_CORNER_XN_YN_ZP: 	viewFrom = Vec3d(-1, -1,  1); break;
		case NCI_CORNER_XP_YN_ZP: 	viewFrom = Vec3d( 1, -1,  1); break;
		case NCI_CORNER_XP_YP_ZP: 	viewFrom = Vec3d( 1,  1,  1); break;
		case NCI_CORNER_XN_YP_ZP: 	viewFrom = Vec3d(-1,  1,  1); break;
		case NCI_EDGE_YN_ZN:		viewFrom = Vec3d( 0, -1, -1); break;
		case NCI_EDGE_XP_ZN: 		viewFrom = Vec3d( 1,  0, -1); break;
		case NCI_EDGE_YP_ZN: 		viewFrom = Vec3d( 0,  1, -1); break;
		case NCI_EDGE_XN_ZN: 		viewFrom = Vec3d(-1,  0, -1); break;
		case NCI_EDGE_YN_ZP: 		viewFrom = Vec3d( 0, -1,  1); break;
		case NCI_EDGE_XP_ZP: 		viewFrom = Vec3d( 1,  0,  1); break;
		case NCI_EDGE_YP_ZP: 		viewFrom = Vec3d( 0,  1,  1); break;
		case NCI_EDGE_XN_ZP: 		viewFrom = Vec3d(-1,  0,  1); break;
		case NCI_EDGE_XN_YN: 		viewFrom = Vec3d(-1, -1,  0); break;
		case NCI_EDGE_XP_YN: 		viewFrom = Vec3d( 1, -1,  0); break;
		case NCI_EDGE_XP_YP: 		viewFrom = Vec3d( 1,  1,  0); break;
		case NCI_EDGE_XN_YP: 		viewFrom = Vec3d(-1,  1,  0); break;
		case NCI_FACE_X_POS: 		viewFrom = Vec3d( 1,  0,  0); break;
		case NCI_FACE_X_NEG: 		viewFrom = Vec3d(-1,  0,  0); break;
		case NCI_FACE_Y_POS: 		viewFrom = Vec3d( 0,  1,  0); break;
		case NCI_FACE_Y_NEG: 		viewFrom = Vec3d( 0, -1,  0); break;
		case NCI_FACE_Z_POS: 		viewFrom = Vec3d( 0,  0,  1); break;
		case NCI_FACE_Z_NEG: 		viewFrom = Vec3d( 0,  0, -1); break;
        case NCI_NONE:
        case NCI_HOME:
        case NCI_ROTATE_CW:
        case NCI_ROTATE_CCW:
		default:					CVF_ASSERT(0); break;
	}

    *viewDir = Vec3d::ZERO - viewFrom;

	// Find the new up vector
	*up = computeNewUpVector(viewFrom, m_camera->up());
}


//--------------------------------------------------------------------------------------------------
/// Find the new up vector
//--------------------------------------------------------------------------------------------------
Vec3d OverlayNavigationCube::computeNewUpVector(const Vec3d& viewFrom, const Vec3d currentUp) const
{
	Vec3d upVector = currentUp;
	upVector.normalize();

	// Snap to axis before rotate, give priority to Z axis if equal
	upVector = snapToAxis(upVector, &Vec3d::Z_AXIS);
	
	Vec3d currentUpVectorSnapped = upVector;
	Vec3d viewDir = -viewFrom;

	// New approach:
	Vec3d currentViewDir = m_camera->direction();
	Vec3d rotAxis;

	if (vectorsParallelFuzzy(currentViewDir, viewDir))
	{
		// The current and new dirs are parallel, just use the up vector as it is perpendicular to the view dir
		rotAxis = currentUp;
	}
	else
	{
		rotAxis = currentViewDir^viewDir;
	}

	rotAxis.normalize();

    // Guard acos against out-of-domain input
    const double dotProduct = Math::clamp(currentViewDir*viewDir, -1.0, 1.0);
    const double angle = Math::acos(dotProduct);
    Mat4d rotMat = Mat4d::fromRotation(rotAxis, angle);
	upVector.transformVector(rotMat);

	// Snap to closest axis
	if (cvf::Math::abs(upVector*currentUpVectorSnapped) > 0.01) 
	{
		upVector = currentUpVectorSnapped;
	}
	else
	{
		upVector = snapToAxis(upVector, &currentUpVectorSnapped);
	}

	if (vectorsParallelFuzzy(upVector, viewDir))
	{
		// The found up vector and view dir are parallel, select another axis based on the current up vector
		if (vectorsParallelFuzzy(Vec3d::Z_AXIS, viewDir))
		{
			if (cvf::Math::abs(currentUp.y()) >= cvf::Math::abs(currentUp.x())) upVector = (currentUp.y() >= 0.0f) ? Vec3d::Y_AXIS : -Vec3d::Y_AXIS;
			else												                upVector = (currentUp.x() >= 0.0f) ? Vec3d::X_AXIS : -Vec3d::X_AXIS;
		}
		else if (vectorsParallelFuzzy(Vec3d::Y_AXIS, viewDir))
		{
			if (cvf::Math::abs(currentUp.x()) >= cvf::Math::abs(currentUp.z())) upVector = (currentUp.x() >= 0.0f) ? Vec3d::X_AXIS : -Vec3d::X_AXIS;
			else												                upVector = (currentUp.z() >= 0.0f) ? Vec3d::Z_AXIS : -Vec3d::Z_AXIS;
		}
		else
		{
			if (cvf::Math::abs(currentUp.y()) >= cvf::Math::abs(currentUp.z())) upVector = (currentUp.y() >= 0.0f) ? Vec3d::Y_AXIS : -Vec3d::Y_AXIS;
			else												                upVector = (currentUp.z() >= 0.0f) ? Vec3d::Z_AXIS : -Vec3d::Z_AXIS;
		}
	}

	return upVector;
}


//--------------------------------------------------------------------------------------------------
/// Static
//--------------------------------------------------------------------------------------------------
Vec3d OverlayNavigationCube::snapToAxis(const Vec3d& vector, const Vec3d* pPreferIfEqual) 
{
    // Snap to closest axis
    int closestAxis = findClosestAxis(vector);

    if (pPreferIfEqual)
    {
        int closestPreferAxis = findClosestAxis(*pPreferIfEqual);

        if (closestAxis != closestPreferAxis)
        {
            if (cvf::Math::abs(cvf::Math::abs(vector[closestAxis]) - cvf::Math::abs(vector[closestPreferAxis])) < 0.01)
            {
                closestAxis = closestPreferAxis;
            }
        }
    }

    Vec3d snapVector = vector;

    if (closestAxis == 0) snapVector = vector.x() >= 0.0f ? Vec3d::X_AXIS : -Vec3d::X_AXIS;
    if (closestAxis == 1) snapVector = vector.y() >= 0.0f ? Vec3d::Y_AXIS : -Vec3d::Y_AXIS;
    if (closestAxis == 2) snapVector = vector.z() >= 0.0f ? Vec3d::Z_AXIS : -Vec3d::Z_AXIS;

    return snapVector;
}


//--------------------------------------------------------------------------------------------------
/// Static
//--------------------------------------------------------------------------------------------------
bool OverlayNavigationCube::vectorsParallelFuzzy(Vec3d v1, Vec3d v2) 
{
    v1.normalize();
    v2.normalize();
    if (cvf::Math::abs(v1*v2) < 0.999f) return false;

    return true;
}


//--------------------------------------------------------------------------------------------------
/// Static
//--------------------------------------------------------------------------------------------------
int OverlayNavigationCube::findClosestAxis(const Vec3d& vector) 
{
    int retAxis = 0;
    double largest = cvf::Math::abs(vector.x());

    if (cvf::Math::abs(vector.y()) > largest)
    {
        largest = cvf::Math::abs(vector.y());
        retAxis = 1;
    }

    if (cvf::Math::abs(vector.z()) > largest)
    {
        retAxis = 2;
    }

    return retAxis;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OverlayNavigationCube::setFaceTexture(NavCubeFace face, TextureImage* texture)
{
    m_faceTextures[face] = texture;
    m_faceTextureBindings.clear();
}


//--------------------------------------------------------------------------------------------------
/// Check if the current view dir is aligned with a face (principal axis) 
//--------------------------------------------------------------------------------------------------
bool OverlayNavigationCube::isFaceAlignedViewPoint() const
{
    Vec3d viewDir   = m_camera->direction().getNormalized();
    Vec3d upVector  = m_camera->up().getNormalized();

	// First check up vector
	float fThreshold = 0.999f;	
	if ((Math::abs(upVector*Vec3d::X_AXIS) < fThreshold) &&
		(Math::abs(upVector*Vec3d::Y_AXIS) < fThreshold) &&
		(Math::abs(upVector*Vec3d::Z_AXIS) < fThreshold))
	{
		return false;
	}

	if      (viewDir*Vec3d::X_AXIS > fThreshold)  return true;
	else if (viewDir*Vec3d::X_AXIS < -fThreshold) return true;
	else if (viewDir*Vec3d::Y_AXIS > fThreshold)  return true;
	else if (viewDir*Vec3d::Y_AXIS < -fThreshold) return true;
	else if (viewDir*Vec3d::Z_AXIS > fThreshold)  return true;
	else if (viewDir*Vec3d::Z_AXIS < -fThreshold) return true;

    return false;
}


//--------------------------------------------------------------------------------------------------
/// Set the "home" camera angle, which is used when the user presses the house 2d item
//--------------------------------------------------------------------------------------------------
void OverlayNavigationCube::setHome(const Vec3d& viewDirection, const Vec3d& up)
{
    m_homeViewDirection = viewDirection;
    m_homeUp = up;
}

} // namespace cvf

