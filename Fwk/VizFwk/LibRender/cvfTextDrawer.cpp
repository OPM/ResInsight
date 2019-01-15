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
#include "cvfTextDrawer.h"
#include "cvfFont.h"
#include "cvfGlyph.h"
#include "cvfTextureImage.h"
#include "cvfOpenGL.h"
#include "cvfOpenGLResourceManager.h"
#include "cvfShaderProgram.h"
#include "cvfUniform.h"
#include "cvfCamera.h"
#include "cvfViewport.h"
#include "cvfBoundingBox.h"
#include "cvfShaderProgramGenerator.h"
#include "cvfShaderSourceProvider.h"
#include "cvfMatrixState.h"
#include "cvfRenderStateDepth.h"
#include "cvfRenderStateBlending.h"
#include "cvfRenderStatePolygonOffset.h"
#include "cvfOpenGLCapabilities.h"

#ifndef CVF_OPENGL_ES
#include "cvfRenderState_FF.h"
#endif

namespace cvf {


//==================================================================================================
///
/// \class cvf::TextDrawer
/// \ingroup Render
///
/// Contains text which can be rendered with a given font
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TextDrawer::TextDrawer(Font* font)
:   m_font(font),
    m_drawBackground(false),
    m_drawBorder(false),
    m_textColor(Color3::GRAY),
    m_backgroundColor(1.0f, 1.0f, 0.8f),
    m_borderColor(Color3::DARK_GRAY),
    m_useDepthBuffer(false),
    m_verticalAlignment(0)      // BASELINE
{
    CVF_ASSERT(font);
    CVF_ASSERT(!font->isEmpty());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TextDrawer::~TextDrawer()
{
}


// --------------------------------------------------------------------------------------------------
/// Add text to be drawn
// --------------------------------------------------------------------------------------------------
void TextDrawer::addText(const String& text, const Vec2f& pos, const Vec2f& dir)
{
    CVF_ASSERT(!text.isEmpty());
    CVF_ASSERT(!pos.isUndefined());

    m_texts.push_back(text);
    m_positions.push_back(Vec3f(pos));
    m_directions.push_back(Vec3f(dir));
}


//--------------------------------------------------------------------------------------------------
/// Add text to be drawn
/// 
/// Note: The Z coordinate needs to correspond with the orthographic projection that is setup
/// to render the text. So the range should be <1..-1> with 1 being closest to the near plane.
//--------------------------------------------------------------------------------------------------
void TextDrawer::addText(const String& text, const Vec3f& pos, const Vec3f& dir)
{
    CVF_ASSERT(!text.isEmpty());
    CVF_ASSERT(!pos.isUndefined());

    m_texts.push_back(text);
    m_positions.push_back(pos);
    m_directions.push_back(dir);
}


//--------------------------------------------------------------------------------------------------
/// Remove all texts in the drawer
//--------------------------------------------------------------------------------------------------
void TextDrawer::removeAllTexts()
{
    m_texts.clear();
    m_positions.clear();
    m_directions.clear();
}


//--------------------------------------------------------------------------------------------------
/// Set color of the text to be rendered
//--------------------------------------------------------------------------------------------------
void TextDrawer::setTextColor(const Color3f& color)
{
    m_textColor = color;
}


//--------------------------------------------------------------------------------------------------
/// Set color of the text to be rendered
//--------------------------------------------------------------------------------------------------
void TextDrawer::setBackgroundColor(const Color3f& color)
{
    m_backgroundColor = color;
}


//--------------------------------------------------------------------------------------------------
/// Set color of the text to be rendered
//--------------------------------------------------------------------------------------------------
void TextDrawer::setBorderColor(const Color3f& color)
{
    m_borderColor = color;
}


//--------------------------------------------------------------------------------------------------
/// Set vertical alignment for horizontal text
//--------------------------------------------------------------------------------------------------
void TextDrawer::setVerticalAlignment(TextDrawer::Alignment alignment)
{
    CVF_ASSERT(m_font.notNull());
    CVF_ASSERT(!m_font->isEmpty());

    m_verticalAlignment = calculateVerticalAlignmentOffset(*m_font, alignment);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void TextDrawer::setDrawBackground(bool drawBackground)
{
    m_drawBackground = drawBackground;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void TextDrawer::setDrawBorder(bool drawBorder)
{
    m_drawBorder = drawBorder;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void TextDrawer::setUseDepthBuffer(bool useDepthBuffer)
{
    m_useDepthBuffer = useDepthBuffer;
}


//--------------------------------------------------------------------------------------------------
/// Returns the color used to draw the text
//--------------------------------------------------------------------------------------------------
Color3f TextDrawer::textColor() const
{
    return m_textColor;
}


//--------------------------------------------------------------------------------------------------
/// Returns the color of the background
//--------------------------------------------------------------------------------------------------
Color3f TextDrawer::backgroundColor() const
{
    return m_backgroundColor;
}


//--------------------------------------------------------------------------------------------------
/// Returns the color of the border.
//--------------------------------------------------------------------------------------------------
Color3f TextDrawer::borderColor() const
{
    return m_borderColor;
}


//--------------------------------------------------------------------------------------------------
/// Returns true if the background will be drawn
//--------------------------------------------------------------------------------------------------
bool TextDrawer::drawBackground() const
{
    return m_drawBackground;
}


//--------------------------------------------------------------------------------------------------
/// Returns true if the border will be drawn
//--------------------------------------------------------------------------------------------------
bool TextDrawer::drawBorder() const
{
    return m_drawBorder;
}


//--------------------------------------------------------------------------------------------------
/// Draw text based using OpenGL shader programs
//--------------------------------------------------------------------------------------------------
void TextDrawer::render(OpenGLContext* oglContext, const MatrixState& matrixState)
{
    doRender2d(oglContext, matrixState, false);
}


//--------------------------------------------------------------------------------------------------
/// Draw text based using OpenGL shader programs
//--------------------------------------------------------------------------------------------------
void TextDrawer::renderSoftware(OpenGLContext* oglContext, const MatrixState& matrixState)
{
    doRender2d(oglContext, matrixState, true);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void TextDrawer::doRender2d(OpenGLContext* oglContext, const MatrixState& matrixState, bool softwareRendering)
{
    CVF_CALLSITE_OPENGL(oglContext);
    CVF_ASSERT(m_positions.size() == m_texts.size() && m_positions.size() == m_directions.size());

    static const ushort connects[]     = { 0, 1, 2, 0, 2, 3 };
    static const ushort lineConnects[] = { 0, 1, 1, 2, 2, 3, 3, 0 };

    float vertexArray[12];
    float textureCoords[8];                 // Will be updated for each glyph 

    std::array<float*, 4> vertices = { &vertexArray[0], &vertexArray[3], &vertexArray[6], &vertexArray[9] };
    for (size_t i = 0; i < vertices.size(); ++i)
    {
        vertices[i][2] = 0.0f;
    }

    // Prepare 2D pixel exact projection to draw texts
    Camera projCam;
    projCam.setViewport(matrixState.viewportPosition().x(), matrixState.viewportPosition().y(), matrixState.viewportSize().x(), matrixState.viewportSize().y());
    projCam.setProjectionAsPixelExact2D();
    projCam.setViewMatrix(Mat4d::IDENTITY);

    MatrixState projMatrixState(projCam);

    // Turn off depth test
    RenderStateDepth depth(m_useDepthBuffer, RenderStateDepth::LESS, m_useDepthBuffer);
    depth.applyOpenGL(oglContext);

    // Setup viewport
    projCam.viewport()->applyOpenGL(oglContext, Viewport::DO_NOT_CLEAR);

    if (softwareRendering)
    {
        if (ShaderProgram::supportedOpenGL(oglContext))
        {
            ShaderProgram::useNoProgram(oglContext);
        }

#ifndef CVF_OPENGL_ES
        RenderStateMaterial_FF mat;
        mat.enableColorMaterial(true);
        mat.applyOpenGL(oglContext);

        RenderStateLighting_FF light(false);
        light.applyOpenGL(oglContext);

        // The active texture may be set to something different than unit 0 if we end up using
        // software rendering here, BUT the context actually has higher capabilities
        // Must be set before any texture related OpenGL calls
        if (oglContext->capabilities()->supportsOpenGL2())
        {
            glActiveTexture(GL_TEXTURE0);
        }

        // Will get turned on during rendering of text, but must be off for background and border
        glDisable(GL_TEXTURE_2D);

        projCam.applyOpenGL();
#endif
    }
    else
    {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glEnableVertexAttribArray(ShaderProgram::VERTEX);
        glVertexAttribPointer(ShaderProgram::VERTEX, 3, GL_FLOAT, GL_FALSE, 0, vertexArray);
    }

    // Use a fixed line spacing
    float lineSpacing = m_font->lineSpacing();
    Vec2f offset(0, 0);
    
    // Render background and border
    // -------------------------------------------------------------------------
    if (m_drawBackground || m_drawBorder)
    {
        if (m_useDepthBuffer)
        {
            RenderStatePolygonOffset polygonOffset;
            polygonOffset.configurePolygonPositiveOffset();
            polygonOffset.applyOpenGL(oglContext);
        }

        ref<ShaderProgram> backgroundShader;

        if (!softwareRendering)
        {
            backgroundShader = oglContext->resourceManager()->getLinkedUnlitColorShaderProgram(oglContext);
            if (backgroundShader->useProgram(oglContext))
            {
                backgroundShader->clearUniformApplyTracking();
                backgroundShader->applyFixedUniforms(oglContext, projMatrixState);
            }
        }

        // Figure out margin
        ref<Glyph> glyph = m_font->getGlyph(L'A');
        float glyphMarginX = cvf::Math::floor(0.5f * static_cast<float>(glyph->width()));
        float glyphMarginY = cvf::Math::floor(0.5f * static_cast<float>(glyph->height()));
        size_t numTexts = m_texts.size();
        for (size_t i = 0; i < numTexts; i++)
        {
            Vec3f pos  = m_positions[i];

            String text = m_texts[i];
            Vec2f textExtent(m_font->textExtent(text));
            std::array<Vec3f, 4> corners = textCorners(*glyph, cvf::Vec2f::ZERO, textExtent, m_verticalAlignment, m_directions[i], glyphMarginX, glyphMarginY);

            for (size_t v = 0; v < vertices.size(); ++v)
            {
                for (int c = 0; c < 3; ++c)
                {
                    vertices[v][c] = pos[c] + corners[v][c];
                }
            }

            if (m_drawBackground)
            {
                if (softwareRendering)
                {
#ifndef CVF_OPENGL_ES
                    glEnable(GL_COLOR_MATERIAL);
                    glDisable(GL_TEXTURE_2D);
                    glColor3fv(m_backgroundColor.ptr());
                    glBegin(GL_TRIANGLE_FAN);
                    for (size_t v = 0; v < vertices.size(); ++v)
                    {
                        glVertex3fv(vertices[v]);
                    }
                    glEnd();
#endif
                }
                else
                {
                    UniformFloat backgroundColor("u_color", Color4f(m_backgroundColor));
                    backgroundShader->applyUniform(oglContext, backgroundColor);

                    // Draw background
                    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, connects);
                }
            }

            if (m_drawBorder)
            {
                if (softwareRendering)
                {
#ifndef CVF_OPENGL_ES
                    glColor3fv(m_borderColor.ptr());
                    glBegin(GL_LINE_LOOP);
                    for (size_t v = 0; v < vertices.size(); ++v)
                    {
                        glVertex3fv(vertices[v]);
                    }
                    glEnd();
#endif
                }
                else
                {
                    UniformFloat borderColor("u_color", Color4f(m_borderColor));
                    backgroundShader->applyUniform(oglContext, borderColor);

                    // Draw border
                    glDrawElements(GL_LINES, 8, GL_UNSIGNED_SHORT, lineConnects);
                }
            }
        }

        if (m_useDepthBuffer)
        {
            RenderStatePolygonOffset resetPolygonOffset;
            resetPolygonOffset.applyOpenGL(oglContext);
        }
    }

    // Render texts
    // -------------------------------------------------------------------------
    if (softwareRendering)
    {
#ifndef CVF_OPENGL_ES
        glEnable(GL_TEXTURE_2D);
        glColor3fv(m_textColor.ptr());
#endif
    }
    else
    {
        ref<ShaderProgram> textShader = oglContext->resourceManager()->getLinkedTextShaderProgram(oglContext);
        if (textShader->useProgram(oglContext))
        {
            textShader->clearUniformApplyTracking();
            textShader->applyFixedUniforms(oglContext, projMatrixState);

            UniformFloat uniformColor("u_color", m_textColor);
            textShader->applyUniform(oglContext, uniformColor);

            UniformInt uniformTexture("u_texture2D", 0);
            textShader->applyUniform(oglContext, uniformTexture);
        }

        glEnableVertexAttribArray(ShaderProgram::TEX_COORD_2F_0);
        glVertexAttribPointer(ShaderProgram::TEX_COORD_2F_0, 2, GL_FLOAT, GL_FALSE, 0, textureCoords);

        // Setup texture, Note: Each glyph will do additional setup
        glActiveTexture(GL_TEXTURE0);
    }

    RenderStateBlending blending;
    blending.configureTransparencyBlending();
    blending.applyOpenGL(oglContext);

    wchar_t character;
    size_t numCharacters;
    size_t numTexts = m_texts.size();

    size_t i, j;
    for (i = 0; i < numTexts; i++)
    {
        Vec3f pos     = m_positions[i];
        Vec3f textDir = m_directions[i];
        String text   = m_texts[i];
        
        // Need to round off to integer positions to avoid buggy text drawing on iPad2
        pos.x() = cvf::Math::floor(pos.x());
        pos.y() = cvf::Math::floor(pos.y());

        // Cursor incrementor
        Vec2f cursor(Vec2f::ZERO);
        cursor += offset;

        std::vector<cvf::String> lines = text.split("\n");

        for (size_t lineIdx = lines.size(); lineIdx-- > 0; )
        {
            String line = lines[lineIdx];

            numCharacters = line.size();

            for (j = 0; j < numCharacters; j++)
            {
                character = line[j];
                ref<Glyph> glyph = m_font->getGlyph(character);
                cvf::Vec2f glyphExtent(static_cast<float>(glyph->width()), static_cast<float>(glyph->height()));
                std::array<Vec3f, 4> corners = textCorners(*glyph, cursor, glyphExtent, m_verticalAlignment, textDir);

                for (size_t v = 0; v < vertices.size(); ++v)
                {
                    for (int c = 0; c < 3; ++c)
                    {
                        vertices[v][c] = pos[c] + corners[v][c];
                    }
                }
                if (textDir.dot(cvf::Vec3f::X_AXIS) < 0.9 && textDir.dot(cvf::Vec3f::Y_AXIS) < 0.9)
                {
                    glyph->setMinFilter(Glyph::LINEAR);
                    glyph->setMagFilter(Glyph::LINEAR);
                }
                else
                {
                    glyph->setMinFilter(Glyph::NEAREST);
                    glyph->setMagFilter(Glyph::NEAREST);
                }
                
                glyph->setupAndBindTexture(oglContext, softwareRendering);

                // Get texture coordinates
                const FloatArray* textureCoordinates = glyph->textureCoordinates();
                CVF_ASSERT(textureCoordinates);
                CVF_ASSERT(textureCoordinates->size() == 8);
                const float* textureCoordinatesPtr = textureCoordinates->ptr();
                CVF_ASSERT(textureCoordinatesPtr);
                int t;
                for (t = 0; t < 8; t++)
                {
                    textureCoords[t] = textureCoordinatesPtr[t];
                }

                if (softwareRendering)
                {
#ifndef CVF_OPENGL_ES
                    glBegin(GL_TRIANGLES);

                    // First triangle in quad
                    glTexCoord2fv(&textureCoordinatesPtr[0]);
                    glVertex3fv(vertices[0]);
                    glTexCoord2fv(&textureCoordinatesPtr[2]);
                    glVertex3fv(vertices[1]);
                    glTexCoord2fv(&textureCoordinatesPtr[4]);
                    glVertex3fv(vertices[2]);

                    // Second triangle in quad
                    glTexCoord2fv(&textureCoordinatesPtr[0]);
                    glVertex3fv(vertices[0]);
                    glTexCoord2fv(&textureCoordinatesPtr[4]);
                    glVertex3fv(vertices[2]);
                    glTexCoord2fv(&textureCoordinatesPtr[6]);
                    glVertex3fv(vertices[3]);

                    glEnd();
#endif
                }
                else
                {
                    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, connects);
                }

                // Jump to the next character in the string, if any
                if (j < (numCharacters - 1))
                {
                    float advance = static_cast<float>(m_font->advance(character, text[j + 1]));
                    cursor.x() += advance;
                }
            }

            // CR
            cursor.x() =  offset.x();
            cursor.y() += lineSpacing;
        }
    }

    if (softwareRendering)
    {
#ifndef CVF_OPENGL_ES
        // apply the projection matrix
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(matrixState.projectionMatrix().ptr());

        // apply the view matrix
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(matrixState.viewMatrix().ptr());

        RenderStateMaterial_FF mat;
        mat.applyOpenGL(oglContext);

        RenderStateLighting_FF light;
        light.applyOpenGL(oglContext);
#endif
    }
    else    
    {
        glDisableVertexAttribArray(ShaderProgram::TEX_COORD_2F_0);
        glDisableVertexAttribArray(ShaderProgram::VERTEX);

        ShaderProgram::useNoProgram(oglContext);
    }

    // Reset render states
    RenderStateBlending resetBlending;
    resetBlending.applyOpenGL(oglContext);

    // Turn off depth test
    RenderStateDepth resetDepth;
    resetDepth.applyOpenGL(oglContext);

    CVF_CHECK_OGL(oglContext);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool TextDrawer::pickText(const Vec3f& pickCoord2d, const String& text, const Vec3f& pos, Font* font)
{
    // Figure out margin
    ref<Glyph> glyph = font->getGlyph(L'A');
    float charHeight = static_cast<float>(glyph->height());
    float charWidth  = static_cast<float>(glyph->width());

    float offsetX = cvf::Math::floor(charWidth/2.0f);
    float offsetY = cvf::Math::floor(charHeight/2.0f);

    Vec2ui textExtent = font->textExtent(text);

    Vec3f min = pos;
    Vec3f max = Vec3f(min.x() + static_cast<float>(textExtent.x()) + offsetX*2.0f, min.y() + static_cast<float>(textExtent.y()) + offsetY*2.0f, 0.0f);

    if (pickCoord2d.x() > min.x() && pickCoord2d.x() < max.x() &&
        pickCoord2d.y() > min.y() && pickCoord2d.y() < max.y())
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
short TextDrawer::calculateVerticalAlignmentOffset(Font& font, Alignment alignment)
{
    switch (alignment)
    {
        case TextDrawer::TOP:
        {
            // Character assumed to reach all the way up
            ref<Glyph> glyph_top = font.getGlyph(L'`');
            return static_cast<short>(-glyph_top->horizontalBearingY());
        }

        case TextDrawer::CENTER:
        {
            // Center around A
            ref<Glyph> glyph_top = font.getGlyph(L'A');
            return static_cast<short>(-((glyph_top->horizontalBearingY() + 1) >> 1));
        }

        case TextDrawer::BASELINE:
        {
            return 0;
        }

        case TextDrawer::BOTTOM:
        {
            // Character assumed to reach all the way down
            ref<Glyph> glyph_bottom = font.getGlyph(L'g');
            return static_cast<short>(static_cast<short>(glyph_bottom->height()) + glyph_bottom->horizontalBearingY());
        }

        default:
        {
            CVF_FAIL_MSG("Unsupported alignment type");
            break;
        }
    }
    return 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::array<cvf::Vec3f, 4> TextDrawer::textCorners(const Glyph& glyph, const Vec2f& textStart, const Vec2f& textExtent, short verticalAlignment, const Vec3f& textDirection, float marginX, float marginY)
{
    Vec3f tangent = textDirection;
    if (tangent.x() < 0.0f) tangent *= -1.0f;
    Vec2f tan2d(tangent.x(), tangent.y());
    Vec3f normal(-tan2d.perpendicularVector(), tangent.z());

    float x1 = textStart.x() + static_cast<float>(glyph.horizontalBearingX()) - marginY;
    float y1 = textStart.y() + static_cast<float>(glyph.horizontalBearingY()) - static_cast<float>(glyph.height()) + static_cast<float>(verticalAlignment) - marginY;

    float x2 = x1 + textExtent.x() + 2.0f * marginX;
    float y2 = y1 + textExtent.y() + 2.0f * marginY;

    // Lower left corner
    Vec3f c1 = tangent * x1 + normal * y1;
    // Lower right corner
    Vec3f c2 = tangent * x2 + normal * y1;
    // Upper right corner
    Vec3f c3 = tangent * x2 + normal * y2;
    // Upper left corner
    Vec3f c4 = tangent * x1 + normal * y2;


    std::array<cvf::Vec3f, 4> retArr = { c1, c2, c3, c4 };
    return retArr;
}

}  // namespace cvf
