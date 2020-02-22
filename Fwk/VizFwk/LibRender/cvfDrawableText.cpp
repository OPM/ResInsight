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
#include "cvfDrawableText.h"
#include "cvfOpenGL.h"
#include "cvfOpenGLResourceManager.h"
#include "cvfBufferObjectManaged.h"
#include "cvfDrawableGeo.h"
#include "cvfQuat.h"
#include "cvfUniform.h"
#include "cvfRay.h"
#include "cvfFont.h"
#include "cvfGlyph.h"
#include "cvfCamera.h"
#include "cvfShaderProgram.h"
#include "cvfViewport.h"
#include "cvfGeometryUtils.h"
#include "cvfMatrixState.h"
#include "cvfRenderStatePoint.h"
#include "cvfRenderStateDepth.h"
#include "cvfRenderStatePolygonOffset.h"
#include "cvfRenderStateBlending.h"

#ifndef CVF_OPENGL_ES 
#include "cvfRenderState_FF.h"
#endif

namespace cvf {


//==================================================================================================
///
/// \class cvf::DrawableText
/// \ingroup Render
///
/// DrawableText implements drawing of text strings in 2D space.
/// 
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// Constructor
//--------------------------------------------------------------------------------------------------
DrawableText::DrawableText()
:   m_verticalAlignment(TextDrawer::BASELINE),
    m_textColor(Color3::BLACK),
    m_backgroundColor(1.0f, 1.0f, 0.8f),
    m_borderColor(Color3::BLACK),
    m_drawBackground(true),
    m_drawBorder(true),
    m_checkPosVisible(true),
    m_useDepthBuffer(false)
{
}


//--------------------------------------------------------------------------------------------------
/// Destructor
//--------------------------------------------------------------------------------------------------
DrawableText::~DrawableText()
{
}


//--------------------------------------------------------------------------------------------------
/// Set font to be used for drawing text
//--------------------------------------------------------------------------------------------------
void DrawableText::setFont(Font* font)
{
    m_font = font;
}


//--------------------------------------------------------------------------------------------------
/// Get font used to draw text
//--------------------------------------------------------------------------------------------------
ref<Font> DrawableText::font() const
{
    return m_font;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void DrawableText::setVerticalAlignment(TextDrawer::Alignment alignment)
{
    m_verticalAlignment = alignment;
}


//--------------------------------------------------------------------------------------------------
/// Set color of the text to be rendered
//--------------------------------------------------------------------------------------------------
void DrawableText::setTextColor(const Color3f& color)
{
    m_textColor = color;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void DrawableText::setBackgroundColor(const Color3f& color)
{
    m_backgroundColor = color;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void DrawableText::setBorderColor(const Color3f& color)
{
    m_borderColor = color;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void DrawableText::setDrawBackground(bool drawBackground)
{
    m_drawBackground = drawBackground;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void DrawableText::setDrawBorder(bool drawBorder)
{
    m_drawBorder = drawBorder;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void DrawableText::setCheckPosVisible(bool checkpos)
{
    m_checkPosVisible = checkpos;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void DrawableText::setUseDepthBuffer(bool useDepthBuffer)
{
    m_useDepthBuffer = useDepthBuffer;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void DrawableText::addText(const String& text, const Vec3f& position, const Vec3f& direction)
{
    m_positions.push_back(position);
    m_texts.push_back(text);
    m_directions.push_back(direction);
    m_boundingBox.add(position);
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t DrawableText::numberOfTexts() const
{
    return m_texts.size();
}

//--------------------------------------------------------------------------------------------------
/// Main shader based rendering path for the geometry 
//--------------------------------------------------------------------------------------------------
void DrawableText::render(OpenGLContext* oglContext, ShaderProgram* shaderProgram, const MatrixState& matrixState)
{
    renderText(oglContext, shaderProgram, matrixState);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void DrawableText::renderSoftware(OpenGLContext* oglContext, const MatrixState& matrixState)
{
    renderText(oglContext, NULL, matrixState);
}


//--------------------------------------------------------------------------------------------------
/// Get total number of vertices for all texts
//--------------------------------------------------------------------------------------------------
size_t DrawableText::vertexCount() const
{
    return faceCount() * 4; // Four vertices per face/quad 
}


//--------------------------------------------------------------------------------------------------
/// Get total number of triangles for all texts
//--------------------------------------------------------------------------------------------------
size_t DrawableText::triangleCount() const
{
    size_t numCharacters = 0;

    size_t numTexts = m_texts.size();
    size_t i;
    for (i = 0; i < numTexts; i++)
    {
        numCharacters += m_texts[i].size();
    }

    return faceCount() * 2;   // Two triangles per face/quad
}


//--------------------------------------------------------------------------------------------------
/// Get total number of faces for all texts
//--------------------------------------------------------------------------------------------------
size_t DrawableText::faceCount() const
{
    size_t numCharacters = 0;

    size_t numTexts = m_texts.size();
    size_t i;
    for (i = 0; i < numTexts; i++)
    {
        numCharacters += m_texts[i].size();
    }

    return numCharacters;   // One face/quad per character/glyph
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
BoundingBox DrawableText::boundingBox() const
{
    return m_boundingBox;
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox DrawableText::textBoundingBox(const String& text, const Vec3f& position, const Vec3f& direction /*= Vec3f::X_AXIS*/)
{
    ref<Glyph> glyph = m_font->getGlyph(L'A');
    Vec2f      textExtent(m_font->textExtent(text));
    short      verticalAlignment = TextDrawer::calculateVerticalAlignmentOffset(*m_font, m_verticalAlignment);
    float      glyphMarginX      = 0.25f * static_cast<float>(glyph->width());
    float      glyphMarginY      = 0.25f * static_cast<float>(glyph->height());

    BoundingBox textBox;
    std::array<Vec3f, 4> corners = TextDrawer::textCorners(*glyph, cvf::Vec2f::ZERO, textExtent, verticalAlignment, direction, glyphMarginX, glyphMarginY);
    for (size_t i = 0; i < corners.size(); i++)
    {
        const Vec3f& corner = corners[i];
        textBox.add(position + corner);
    }
    return textBox;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool DrawableText::rayIntersectCreateDetail(const Ray& ray, Vec3d* intersectionPoint, ref<HitDetail>* hitDetail) const
{
    CVF_UNUSED(ray);
    CVF_UNUSED(intersectionPoint);
    CVF_UNUSED(hitDetail);

    return false;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void DrawableText::renderText(OpenGLContext* oglContext, ShaderProgram* shaderProgram, const MatrixState& matrixState)
{
    CVF_ASSERT(m_font.notNull());
    CVF_ASSERT(!m_font->isEmpty());
    CVF_ASSERT(oglContext);
    CVF_ASSERT(!shaderProgram || ShaderProgram::supportedOpenGL(oglContext));

    if (m_texts.size() == 0) return;
    CVF_ASSERT(m_positions.size() == m_texts.size() && m_positions.size() == m_directions.size());

    if (m_checkPosVisible)
    {
        if (shaderProgram)
        {
            ref<ShaderProgram> nudgeShader = oglContext->resourceManager()->getLinkedNudgeShaderProgram(oglContext);
            nudgeShader->useProgram(oglContext);
            nudgeShader->clearUniformApplyTracking();
            nudgeShader->applyFixedUniforms(oglContext, matrixState);

            RenderStatePoint point(RenderStatePoint::PROGRAM_SIZE);
            point.applyOpenGL(oglContext);
        }
        else
        {
            if (ShaderProgram::supportedOpenGL(oglContext))
            {
                ShaderProgram::useNoProgram(oglContext);
            }

#ifndef CVF_OPENGL_ES
            RenderStateMaterial_FF mat;
            mat.enableColorMaterial(true);

            RenderStateLighting_FF noLight(false);
            noLight.applyOpenGL(oglContext);
#endif
        }

        RenderStateDepth visibleCheckDepthRS(true, RenderStateDepth::LEQUAL, false);
        visibleCheckDepthRS.applyOpenGL(oglContext);

        RenderStatePolygonOffset po;
        po.enablePointMode(true);
        po.setFactor(-3.0f);
        po.setUnits(-3.0f);
        po.applyOpenGL(oglContext);

        RenderStateBlending addBlend;
        addBlend.enableBlending(true);
        addBlend.setFunction(RenderStateBlending::ONE, RenderStateBlending::ONE);
        addBlend.setEquation(RenderStateBlending::FUNC_ADD);
        addBlend.applyOpenGL(oglContext);
    }

    Mat4d modelViewProjectionMatrix = Mat4d(matrixState.modelViewProjectionMatrix());
    std::vector<Vec3f> projCoords;
    std::vector<String> textsToDraw;            // Text strings to be drawn
    std::vector<Vec3f> directions;
    size_t pos;
    for (pos = 0; pos < m_positions.size(); pos++)
    {
        Vec3d proj;
        GeometryUtils::project(modelViewProjectionMatrix, matrixState.viewportPosition(), matrixState.viewportSize(), Vec3d(m_positions[pos]), &proj);
        CVF_ASSERT(!proj.isUndefined());
        if (!m_checkPosVisible || labelAnchorVisible(oglContext, proj, m_positions[pos], shaderProgram == NULL))
        {
            // Note: Need to adjust for the current viewport, as the coords returned from project are in global windows coordinates
            projCoords.push_back(Vec3f(static_cast<float>(proj.x() - matrixState.viewportPosition().x()), static_cast<float>(proj.y() - matrixState.viewportPosition().y()), static_cast<float>(1.0 - 2.0*proj.z())));  // Map z into 1 .. -1
            textsToDraw.push_back(m_texts[pos]);
            directions.push_back(m_directions[pos]);
        }
    }

    if (m_checkPosVisible)
    {
        RenderStatePolygonOffset resetPO;
        resetPO.applyOpenGL(oglContext);

        RenderStateBlending resetBlend;
        resetBlend.applyOpenGL(oglContext);

#ifndef CVF_OPENGL_ES
        if (!shaderProgram)
        {
            RenderStateMaterial_FF mat;
            mat.applyOpenGL(oglContext);

            RenderStateLighting_FF light;
            light.applyOpenGL(oglContext);
        }
#endif
    }

    TextDrawer drawer(m_font.p());
    drawer.setVerticalAlignment(m_verticalAlignment);
    drawer.setTextColor(m_textColor);
    drawer.setBackgroundColor(m_backgroundColor);
    drawer.setBorderColor(m_borderColor);
    drawer.setDrawBorder(m_drawBorder);
    drawer.setDrawBackground(m_drawBackground);
    drawer.setUseDepthBuffer(m_useDepthBuffer);

    size_t i;
    for (i = 0; i < textsToDraw.size(); i++)
    {
        Vec3f pos = projCoords[i];
        drawer.addText(textsToDraw[i], pos, directions[i]);
    }

    if (shaderProgram)
    {
        drawer.render(oglContext, matrixState);

        // Keep current program in use when we exit
        shaderProgram->useProgram(oglContext);
    }
    else
    {
        drawer.renderSoftware(oglContext, matrixState);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool DrawableText::labelAnchorVisible(OpenGLContext* oglContext, const Vec3d winCoord, const Vec3f& worldCoord, bool softwareRendering) const
{
    CVF_CALLSITE_OPENGL(oglContext);
    CVF_UNUSED(worldCoord);

    Vec2i coord(static_cast<GLint>(winCoord.x()), static_cast<GLint>(winCoord.y()));

    // Read out current z buffer contents
    GLubyte bufferBefore[9*4];
    glReadPixels(coord.x() - 1, coord.y() - 1, 3, 3, GL_RGBA, GL_UNSIGNED_BYTE, bufferBefore);

    if (softwareRendering)
    {
#ifndef CVF_OPENGL_ES
        glPointSize(3);
        glColor3f(0.02f, 0.02f, 0.02f);
        glBegin(GL_POINTS);
        glVertex3fv(worldCoord.ptr());
        glEnd();
#endif
    }
    else
    {
        glEnableVertexAttribArray(ShaderProgram::VERTEX);
        glVertexAttribPointer(ShaderProgram::VERTEX, 3, GL_FLOAT, GL_FALSE, 0, worldCoord.ptr());

        glDrawArrays(GL_POINTS, 0, 1);
    }

    // Now compare with previous z buffer contents to see if marker is visible
    GLubyte bufferAfter[9*4];
    glReadPixels(coord.x() - 1, coord.y() - 1, 3, 3, GL_RGBA, GL_UNSIGNED_BYTE, bufferAfter);

    size_t j;
    for (j = 0; j < 9*4; j++)
    {
        if (bufferBefore[j] != bufferAfter[j]) return true;
    }

    return false;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool DrawableText::rayIntersect(const Ray& ray, const Camera& camera, Vec3d* intersectionPoint)
{
    Vec3d pickCoord2d;
    camera.project(ray.origin(), &pickCoord2d);

    size_t i;
    for (i = 0; i < m_positions.size(); i++)
    {
        Vec3d proj;
        camera.project(Vec3d(m_positions[i]), &proj);

        Vec3f posTextDrawer = Vec3f(static_cast<float>(proj.x() - camera.viewport()->x()), static_cast<float>(proj.y() - camera.viewport()->y()), static_cast<float>(1.0 - 2.0*proj.z()));  // Map z into 1 .. -1

        if (TextDrawer::pickText(Vec3f(pickCoord2d), m_texts[i], posTextDrawer, m_font.p()))
        {
            if (m_useDepthBuffer)
            {
                *intersectionPoint = Vec3d(m_positions[i]);
            }
            else
            {
                *intersectionPoint = ray.origin();
            }

            return true;
        }
    }

    return false;
}

} // namespace cvf
