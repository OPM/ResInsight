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
#include "cvfOverlayTextBox.h"
#include "cvfDrawableText.h"
#include "cvfMatrixState.h"
#include "cvfCamera.h"
#include "cvfShaderProgram.h"
#include "cvfOpenGL.h"
#include "cvfViewport.h"
#include "cvfOpenGLResourceManager.h"
#include "cvfUniform.h"
#include "cvfRenderStateDepth.h"
#include "cvfFont.h"
#include "cvfGlyph.h"
#include "cvfRenderStateLine.h"

#ifndef CVF_OPENGL_ES
#include "cvfRenderState_FF.h"
#endif

namespace cvf {

//==================================================================================================
///
/// \class cvf::OverlayTextBox
/// \ingroup Render
///
/// An view overlay item capable of showing a text with optionally a border and a background
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// Constructor. The specified font is used to draw all the text
//--------------------------------------------------------------------------------------------------
OverlayTextBox::OverlayTextBox(Font* font)
:   m_size(200, 50),
    m_font(font),
    m_drawBackground(true),
    m_drawBorder(true),
    m_textColor(Color3::WHITE),
    m_backgroundColor(0.2f, 0.2f, 1.0f),
    m_borderColor(0.6f, 0.6f, 1.0f)
{
    m_textDrawer = new TextDrawer(font);
    m_textDrawer->setVerticalAlignment(TextDrawer::BASELINE);
    m_textDrawer->setDrawBackground(false);
    m_textDrawer->setDrawBorder(false);
}


//--------------------------------------------------------------------------------------------------
/// Destructor
//--------------------------------------------------------------------------------------------------
OverlayTextBox::~OverlayTextBox()
{
}


//--------------------------------------------------------------------------------------------------
/// Returns the wanted size in pixels
//--------------------------------------------------------------------------------------------------
cvf::Vec2ui OverlayTextBox::sizeHint()
{
    return m_size;
}


//--------------------------------------------------------------------------------------------------
/// Render using Shaders
//--------------------------------------------------------------------------------------------------
void OverlayTextBox::render(OpenGLContext* oglContext, const Vec2i& position, const Vec2ui& size)
{
    render(oglContext, position, size, false);
}


//--------------------------------------------------------------------------------------------------
/// Render using Fixed Function
//--------------------------------------------------------------------------------------------------
void OverlayTextBox::renderSoftware(OpenGLContext* oglContext, const Vec2i& position, const Vec2ui& size)
{
    render(oglContext, position, size, true);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OverlayTextBox::render(OpenGLContext* oglContext, const Vec2i& position, const Vec2ui& size, bool software)
{
    Mat4d ident;
    MatrixState matrixState(position, size, ident, ident);
    Vec2f textPos(0.0f, static_cast<float>(size.y())/2.0f);

    if (m_drawBorder || m_drawBackground)
    {
        renderBackgroundAndBorder(oglContext, position, size, software);
        textPos.x() = 4; // Allow for margin
    }

    Vec2ui textExtent = m_font->textExtent(m_text);
    textPos.y() -= (static_cast<float>(textExtent.y())/2.0f);

    // Set the text
    m_textDrawer->removeAllTexts();
    m_textDrawer->addText(m_text, textPos);
    m_textDrawer->setTextColor(m_textColor);

    // Draw the text
    if (software)
    {
        m_textDrawer->renderSoftware(oglContext, matrixState);
    }
    else
    {
        m_textDrawer->render(oglContext, matrixState);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OverlayTextBox::renderBackgroundAndBorder(OpenGLContext* oglContext, const Vec2i& position, const Vec2ui& size, bool software)
{
    CVF_CALLSITE_OPENGL(oglContext);

    // Prepare 2D pixel exact projection to draw texts
    Camera projCam;
    projCam.setViewport(position.x(), position.y(), size.x(), size.y());
    projCam.setProjectionAsPixelExact2D();
    projCam.setViewMatrix(Mat4d::IDENTITY);

    // Turn off depth test
    RenderStateDepth depth(false, RenderStateDepth::LESS, false);
    depth.applyOpenGL(oglContext);

    ref<ShaderProgram> backgroundShader;
    float vertexArray[12];

    projCam.viewport()->applyOpenGL(oglContext, Viewport::DO_NOT_CLEAR);

    if (software)
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
#endif
        projCam.applyOpenGL();
    }
    else
    {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glEnableVertexAttribArray(ShaderProgram::VERTEX);
        glVertexAttribPointer(ShaderProgram::VERTEX, 3, GL_FLOAT, GL_FALSE, 0, vertexArray);

        backgroundShader = oglContext->resourceManager()->getLinkedUnlitColorShaderProgram(oglContext);
        if (backgroundShader->useProgram(oglContext))
        {
            MatrixState projMatrixState(projCam);
            backgroundShader->clearUniformApplyTracking();
            backgroundShader->applyFixedUniforms(oglContext, projMatrixState);
        }
    }

    Vec3f min(1.0f, 1.0f, 0.0f);
    Vec3f max(static_cast<float>(size.x() - 1), static_cast<float>(size.y() - 1), 0.0f);

    // Setup the vertex array
    float* v1 = &vertexArray[0]; 
    float* v2 = &vertexArray[3];
    float* v3 = &vertexArray[6];
    float* v4 = &vertexArray[9];
    v1[0] = min.x(); v1[1] = min.y(); v1[2] = 0.0f;
    v2[0] = max.x(); v2[1] = min.y(); v2[2] = 0.0f;
    v3[0] = max.x(); v3[1] = max.y(); v3[2] = 0.0f;
    v4[0] = min.x(); v4[1] = max.y(); v4[2] = 0.0f;

    if (m_drawBackground)
    {
        if (software)
        {
#ifndef CVF_OPENGL_ES
            glColor4fv(m_backgroundColor.ptr());
            glBegin(GL_TRIANGLE_FAN);
            glVertex3fv(v1);
            glVertex3fv(v2);
            glVertex3fv(v3);
            glVertex3fv(v4);
            glEnd();
#endif
        }
        else
        {
            // Draw background
            UniformFloat backgroundColor("u_color", Color4f(m_backgroundColor));
            backgroundShader->applyUniform(oglContext, backgroundColor);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        }
    }

    if (m_drawBorder)
    {
        if (software)
        {
#ifndef CVF_OPENGL_ES
            glColor3fv(m_borderColor.ptr());
            glBegin(GL_LINE_LOOP);
            glVertex3fv(v1);
            glVertex3fv(v2);
            glVertex3fv(v3);
            glVertex3fv(v4);
            glEnd();
#endif
        }
        else
        {
            UniformFloat borderColor("u_color", Color4f(m_borderColor));
            backgroundShader->applyUniform(oglContext, borderColor);

            RenderStateLine line(static_cast<float>(3));
            line.applyOpenGL(oglContext);

            // Draw border
            glDrawArrays(GL_LINE_LOOP, 0, 4);

            RenderStateLine resetLine;
            resetLine.applyOpenGL(oglContext);
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// Set the text to draw in the box
//--------------------------------------------------------------------------------------------------
void OverlayTextBox::setText(const String& text)
{
    m_text = text;
}


//--------------------------------------------------------------------------------------------------
/// Set the size (in pixels) of the text box
//--------------------------------------------------------------------------------------------------
void OverlayTextBox::setPixelSize( const Vec2ui& size )
{
    m_size = size;
}


//--------------------------------------------------------------------------------------------------
/// Set the size of the text box to fit the current text. 
/// 
/// The method will also add a bit of space for the border or background if enabled.
//--------------------------------------------------------------------------------------------------
void OverlayTextBox::setSizeToFitText()
{
    cvf::Vec2ui textSize = m_font->textExtent(m_text);

    // Add the size of an 'A' as the margin, same as used in the Text Drawer
    ref<Glyph> glyph = m_font->getGlyph(L'A');
    Vec2ui size = Vec2ui(textSize.x() + glyph->width(), textSize.y() + glyph->height());

    if (m_drawBorder)
    {
        size.x() += 4;
        size.y() += 4;
    }

    setPixelSize(size);
}


//--------------------------------------------------------------------------------------------------
/// Set the text color 
//--------------------------------------------------------------------------------------------------
void OverlayTextBox::setTextColor(const Color3f& color)
{
    m_textColor = color;
}


//--------------------------------------------------------------------------------------------------
/// Set the background color of the text box
//--------------------------------------------------------------------------------------------------
void OverlayTextBox::setBackgroundColor(const Color3f& color)
{
    m_backgroundColor = color;
}


//--------------------------------------------------------------------------------------------------
/// Set the border color of the text box
//--------------------------------------------------------------------------------------------------
void OverlayTextBox::setBorderColor(const Color3f& color)
{
    m_borderColor = color;
}


//--------------------------------------------------------------------------------------------------
/// Set if a filled background of the text box should be drawn or not
//--------------------------------------------------------------------------------------------------
void OverlayTextBox::setDrawBackground(bool drawBackground)
{
    m_drawBackground = drawBackground;
}


//--------------------------------------------------------------------------------------------------
/// Set if the border of the text box should be drawn or not
//--------------------------------------------------------------------------------------------------
void OverlayTextBox::setDrawBorder(bool drawBorder)
{
    m_drawBorder = drawBorder;
}


//--------------------------------------------------------------------------------------------------
/// Returns the text shown in the text box
//--------------------------------------------------------------------------------------------------
String OverlayTextBox::text() const
{
    return m_text;
}


//--------------------------------------------------------------------------------------------------
/// Returns the color used to draw the text
//--------------------------------------------------------------------------------------------------
Color3f OverlayTextBox::textColor() const
{
    return m_textColor;
}


//--------------------------------------------------------------------------------------------------
/// Returns the color of the background
//--------------------------------------------------------------------------------------------------
Color3f OverlayTextBox::backgroundColor() const
{
    return m_backgroundColor;
}


//--------------------------------------------------------------------------------------------------
/// Returns the color of the border.
//--------------------------------------------------------------------------------------------------
Color3f OverlayTextBox::borderColor() const
{
    return m_borderColor;
}


//--------------------------------------------------------------------------------------------------
/// Returns true if the background will be drawn
//--------------------------------------------------------------------------------------------------
bool OverlayTextBox::drawBackground() const
{
    return m_drawBackground;
}


//--------------------------------------------------------------------------------------------------
/// Returns true if the border will be drawn
//--------------------------------------------------------------------------------------------------
bool OverlayTextBox::drawBorder() const
{
    return m_drawBorder;
}

} // namespace cvf
