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
#include "cvfOverlayColorLegend.h"
#include "cvfOpenGL.h"
#include "cvfOpenGLResourceManager.h"
#include "cvfGeometryBuilderDrawableGeo.h"
#include "cvfGeometryUtils.h"
#include "cvfViewport.h"
#include "cvfCamera.h"
#include "cvfTextDrawer.h"
#include "cvfFont.h"
#include "cvfShaderProgram.h"
#include "cvfShaderProgramGenerator.h"
#include "cvfShaderSourceProvider.h"
#include "cvfShaderSourceRepository.h"
#include "cvfUniform.h"
#include "cvfMatrixState.h"
#include "cvfBufferObjectManaged.h"
#include "cvfGlyph.h"
#include "cvfRenderStateDepth.h"
#include "cvfRenderStateLine.h"

#ifndef CVF_OPENGL_ES
#include "cvfRenderState_FF.h"
#endif

namespace cvf {


//==================================================================================================
///
/// \class cvf::OverlayColorLegend
/// \ingroup Render
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// Constructor
//--------------------------------------------------------------------------------------------------
OverlayColorLegend::OverlayColorLegend(Font* font)
:   m_sizeHint(200, 200),
    m_color(Color3::BLACK),
    m_lineColor(Color3::BLACK),
    m_lineWidth(1),
    m_font(font),
    m_margin(4)
{
    CVF_ASSERT(font);
    CVF_ASSERT(!font->isEmpty());

    m_levelColors.reserve(2);
    m_levelColors.add(Color3::RED);
    m_levelColors.add(Color3::BLUE);

    m_tickValues.reserve(3);
    m_tickValues.add(0.0);
    m_tickValues.add(0.5);
    m_tickValues.add(1.0);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
OverlayColorLegend::~OverlayColorLegend()
{
    // Empty destructor to avoid errors with undefined types when cvf::ref's destructor gets called
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec2ui OverlayColorLegend::sizeHint()
{
    return m_sizeHint;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OverlayColorLegend::configureLevels(const Color3ubArray& levelColors, const DoubleArray& tickValues)
{
    CVF_ASSERT(levelColors.size() > 0);
    CVF_ASSERT(levelColors.size() + 1 == tickValues.size());

    m_levelColors = levelColors;
    m_tickValues = tickValues;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OverlayColorLegend::setSizeHint(const Vec2ui& size)
{
    m_sizeHint = size;
}


//--------------------------------------------------------------------------------------------------
/// Set color of the text and lines to be rendered
//--------------------------------------------------------------------------------------------------
void OverlayColorLegend::setColor(const Color3f& color)
{
    m_color = color;
}


//--------------------------------------------------------------------------------------------------
/// Returns the color of the text and lines
//--------------------------------------------------------------------------------------------------
const Color3f&  OverlayColorLegend::color() const
{
    return m_color;
}




//--------------------------------------------------------------------------------------------------
/// Set the title (text that will be rendered above the legend)
/// 
/// The legend supports multi-line titles. Separate each line with a '\n' character
//--------------------------------------------------------------------------------------------------
void OverlayColorLegend::setTitle(const String& title)
{
    // Title
    if (title.isEmpty())
    {
        m_titleStrings.clear();
    }
    else
    {
        m_titleStrings = title.split("\n");
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
String OverlayColorLegend::title() const
{
    String title;
    for (size_t i = 0; i < m_titleStrings.size(); ++i)
    {
        title += m_titleStrings[i];

        if (i != m_titleStrings.size() - 1)
        {
            title += "\n";
        }
    }

    return title;
}


//--------------------------------------------------------------------------------------------------
/// Hardware rendering using shader programs
//--------------------------------------------------------------------------------------------------
void OverlayColorLegend::render(OpenGLContext* oglContext, const Vec2i& position, const Vec2ui& size)
{
    render(oglContext, position, size, false);
}


//--------------------------------------------------------------------------------------------------
/// Software rendering using software
//--------------------------------------------------------------------------------------------------
void OverlayColorLegend::renderSoftware(OpenGLContext* oglContext, const Vec2i& position, const Vec2ui& size)
{
    render(oglContext, position, size, true);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool OverlayColorLegend::pick(int x, int y, const Vec2i& position, const Vec2ui& size)
{
    Recti oglRect(position, static_cast<int>(size.x()), static_cast<int>(size.y()));

    OverlayColorLegendLayoutInfo layoutInViewPortCoords;
    layoutInViewPortCoords.position = oglRect.min();
    layoutInViewPortCoords.size     = Vec2ui(static_cast<uint>(oglRect.width()), static_cast<uint>(oglRect.height()));
    layoutInfo(&layoutInViewPortCoords);

    Vec2i legendBarOrigin = oglRect.min();
    legendBarOrigin.x() += static_cast<uint>(layoutInViewPortCoords.legendRect.min().x());
    legendBarOrigin.y() += static_cast<uint>(layoutInViewPortCoords.legendRect.min().y());

    Recti legendBarRect = Recti(legendBarOrigin, static_cast<int>(layoutInViewPortCoords.legendRect.width()), static_cast<int>(layoutInViewPortCoords.legendRect.height()));

    if ((x > legendBarRect.min().x()) && (x < legendBarRect.max().x()) &&
        (y > legendBarRect.min().y()) && (y < legendBarRect.max().y()))
    {
        return true;
    }

    return false;
}


//--------------------------------------------------------------------------------------------------
/// Set up camera/viewport and render
//--------------------------------------------------------------------------------------------------
void OverlayColorLegend::render(OpenGLContext* oglContext, const Vec2i& position, const Vec2ui& size, bool software)
{   
    if (size.x() <= 0 || size.y() <= 0)
    {
        return;
    }

    Camera camera;
    camera.setViewport(position.x(), position.y(), size.x(), size.y());
    camera.setProjectionAsPixelExact2D();
    camera.setViewMatrix(Mat4d::IDENTITY);
    camera.applyOpenGL();
    camera.viewport()->applyOpenGL(oglContext, Viewport::CLEAR_DEPTH);

    // Get layout information
    // Todo: Cache this between renderings. Update only when needed.
    OverlayColorLegendLayoutInfo layout;
    layout.position = position;
    layout.size     = size;
    layoutInfo(&layout);

    // Set up text drawer
    TextDrawer textDrawer(m_font.p());
    setupTextDrawer(&textDrawer, &layout);

    // Do the actual rendering
    if (software)
    {
        renderLegendImmediateMode(oglContext, &layout);
        textDrawer.renderSoftware(oglContext, camera);
    }
    else
    {
        const MatrixState matrixState(camera);
        renderLegend(oglContext, &layout, matrixState);
        textDrawer.render(oglContext, camera);
    }

    CVF_CHECK_OGL(oglContext);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OverlayColorLegend::setupTextDrawer(TextDrawer* textDrawer, OverlayColorLegendLayoutInfo* layout)
{
    CVF_ASSERT(layout);

    textDrawer->setVerticalAlignment(TextDrawer::CENTER);
    textDrawer->setTextColor(m_color);

    m_visibleTickLabels.clear();

    const float textX = layout->tickX + 5;

    const float overlapTolerance = 1.2f * layout->charHeight;
    float lastVisibleTextY = 0.0;

    size_t numTicks = m_tickValues.size();
    size_t it;
    for (it = 0; it < numTicks; it++)
    {
        float textY = static_cast<float>(layout->legendRect.min().y() + layout->tickPixelPos->get(it));
        
        // Always draw first and last tick label. For all others, skip drawing if text ends up
        // on top of the previous label
        if (it != 0 && it != (numTicks - 1))
        {
            if (cvf::Math::abs(textY - lastVisibleTextY) < overlapTolerance)
            {
                m_visibleTickLabels.push_back(false);
                continue;
            }
        }

        double tickValue = m_tickValues[it];
        String valueString = String::number(tickValue);
        Vec2f pos(textX, textY);
        textDrawer->addText(valueString, pos);

        lastVisibleTextY = textY;
        m_visibleTickLabels.push_back(true);
    }

    float titleY = static_cast<float>(layout->size.y()) - layout->margins.y() - layout->charHeight/2.0f;
    for (it = 0; it < m_titleStrings.size(); it++)
    {
        Vec2f pos(layout->margins.x(), titleY);
        textDrawer->addText(m_titleStrings[it], pos);

        titleY -= layout->lineSpacing;
    }

}


//--------------------------------------------------------------------------------------------------
/// Draw the legend using shader programs
//--------------------------------------------------------------------------------------------------
void OverlayColorLegend::renderLegend(OpenGLContext* oglContext, OverlayColorLegendLayoutInfo* layout, const MatrixState& matrixState)
{
    CVF_CALLSITE_OPENGL(oglContext);

    CVF_TIGHT_ASSERT(layout);
    CVF_TIGHT_ASSERT(layout->size.x() > 0);
    CVF_TIGHT_ASSERT(layout->size.y() > 0);

    RenderStateDepth depth(false);
    depth.applyOpenGL(oglContext);

    // All vertices. Initialized here to set Z to zero once and for all.
    static float vertexArray[] = 
    {
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f 
    };

    // Per vector convenience pointers
    float* v1 = &vertexArray[0];    // x0, y0
    float* v2 = &vertexArray[3];    // x1, y0
    float* v3 = &vertexArray[6];    // tickX, y0
    float* v4 = &vertexArray[9];    // x0, y1
    float* v5 = &vertexArray[12];   // x1, y1
    float* v6 = &vertexArray[15];   // tickX, y1

    // Constant coordinates
    v1[0] = v4[0] = layout->x0;
    v2[0] = v5[0] = layout->x1;

    // Connects
    static const ushort trianglesConnects[] = { 0, 1, 4, 0, 4, 3 };
    static const ushort linesConnects[] = { 0, 3, 1, 4, 0, 2, 3, 5 };

    ref<ShaderProgram> shaderProgram = oglContext->resourceManager()->getLinkedUnlitColorShaderProgram(oglContext);
    CVF_TIGHT_ASSERT(shaderProgram.notNull());

    if (shaderProgram->useProgram(oglContext))
    {
        shaderProgram->clearUniformApplyTracking();
        shaderProgram->applyFixedUniforms(oglContext, matrixState);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glEnableVertexAttribArray(ShaderProgram::VERTEX);
    glVertexAttribPointer(ShaderProgram::VERTEX, 3, GL_FLOAT, GL_FALSE, 0, vertexArray);

    // Render colored quads and lines
    size_t numColors = m_levelColors.size();
    CVF_ASSERT(numColors == m_tickValues.size() - 1);
    size_t ic;
    for (ic = 0; ic < numColors; ic++)
    {
        const Color3ub& clr = m_levelColors[ic];
        float y0 = static_cast<float>(layout->legendRect.min().y() + layout->tickPixelPos->get(ic));
        float y1 = static_cast<float>(layout->legendRect.min().y() + layout->tickPixelPos->get(ic + 1));

        // Dynamic coordinates for rectangle
        v1[1] = v2[1] = y0;
        v4[1] = v5[1] = y1;

        // Dynamic coordinates for tickmarks-lines
        v3[0] = m_visibleTickLabels[ic] ? layout->tickX : layout->x1;
        v6[0] = m_visibleTickLabels[ic + 1] ? layout->tickX : layout->x1;
        v3[1] = y0;
        v6[1] = y1;

        // Draw filled rectangle elements
        {
            UniformFloat uniformColor("u_color", Color4f(Color3f(clr)));
            shaderProgram->applyUniform(oglContext, uniformColor);

#ifdef CVF_OPENGL_ES
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, trianglesConnects);
#else
            glDrawRangeElements(GL_TRIANGLES, 0, 4, 6, GL_UNSIGNED_SHORT, trianglesConnects);
#endif
        }

        // Draw legend lines
        {
            RenderStateLine line(static_cast<float>(m_lineWidth));
            line.applyOpenGL(oglContext);
            UniformFloat uniformColor("u_color", Color4f(m_lineColor));
            shaderProgram->applyUniform(oglContext, uniformColor);

#ifdef CVF_OPENGL_ES
            glDrawElements(GL_LINES, 8, GL_UNSIGNED_SHORT, linesConnects);
#else
            glDrawRangeElements(GL_LINES, 0, 5, 8, GL_UNSIGNED_SHORT, linesConnects);
#endif
        }
    }

    glDisableVertexAttribArray(ShaderProgram::VERTEX);

    CVF_TIGHT_ASSERT(shaderProgram.notNull());
    shaderProgram->useNoProgram(oglContext);

    // Reset render states
    RenderStateDepth resetDepth;
    resetDepth.applyOpenGL(oglContext);

    RenderStateLine resetLine;
    resetLine.applyOpenGL(oglContext);

    CVF_CHECK_OGL(oglContext);
}


//--------------------------------------------------------------------------------------------------
/// Draw the legend using immediate mode OpenGL
//--------------------------------------------------------------------------------------------------
void OverlayColorLegend::renderLegendImmediateMode(OpenGLContext* oglContext, OverlayColorLegendLayoutInfo* layout)
{
#ifdef CVF_OPENGL_ES
    CVF_UNUSED(layout);
    CVF_FAIL_MSG("Not supported on OpenGL ES");
#else
    CVF_TIGHT_ASSERT(layout);
    CVF_TIGHT_ASSERT(layout->size.x() > 0);
    CVF_TIGHT_ASSERT(layout->size.y() > 0);

    RenderStateDepth depth(false);
    depth.applyOpenGL(oglContext);

    RenderStateLighting_FF lighting(false);
    lighting.applyOpenGL(oglContext);

    // All vertices. Initialized here to set Z to zero once and for all.
    static float vertexArray[] = 
    {
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f 
    };

    // Per vector convenience pointers
    float* v1 = &vertexArray[0];    // x0, y0
    float* v2 = &vertexArray[3];    // x1, y0
    float* v3 = &vertexArray[6];    // tickX, y0
    float* v4 = &vertexArray[9];    // x0, y1
    float* v5 = &vertexArray[12];   // x1, y1
    float* v6 = &vertexArray[15];   // tickX, y1

    // Constant coordinates
    v1[0] = v4[0] = layout->x0;
    v2[0] = v5[0] = layout->x1;

    // Render colored quads and lines
    size_t numColors = m_levelColors.size();
    CVF_ASSERT(numColors == m_tickValues.size() - 1);
    size_t ic;
    for (ic = 0; ic < numColors; ic++)
    {
        const Color3ub& levelColor = m_levelColors[ic];
        float y0 = static_cast<float>(layout->margins.y() + layout->tickPixelPos->get(ic));
        float y1 = static_cast<float>(layout->margins.y() + layout->tickPixelPos->get(ic + 1));

        // Dynamic coordinates for rectangle
        v1[1] = v2[1] = y0;
        v4[1] = v5[1] = y1;

        // Dynamic coordinates for tickmarks-lines
        v3[0] = m_visibleTickLabels[ic] ? layout->tickX : layout->x1;
        v6[0] = m_visibleTickLabels[ic + 1] ? layout->tickX : layout->x1;
        v3[1] = y0;
        v6[1] = y1;

        // Draw filled rectangle elements
        glColor3ubv(levelColor.ptr());
        glBegin(GL_TRIANGLE_FAN);
        glVertex3fv(v1);
        glVertex3fv(v2);
        glVertex3fv(v5);
        glVertex3fv(v4);
        glEnd();

        // Draw legend lines
        glColor3fv(m_color.ptr());
        glBegin(GL_LINES);
        glVertex3fv(v1);
        glVertex3fv(v4);
        glVertex3fv(v2);
        glVertex3fv(v5);
        glVertex3fv(v1);
        glVertex3fv(v3);
        glVertex3fv(v4);
        glVertex3fv(v6);
        glEnd();
    }

    // Reset render states
    RenderStateLighting_FF resetLighting;
    resetLighting.applyOpenGL(oglContext);
    RenderStateDepth resetDepth;
    resetDepth.applyOpenGL(oglContext);

    CVF_CHECK_OGL(oglContext);
#endif // CVF_OPENGL_ES
}


//--------------------------------------------------------------------------------------------------
/// Get layout information
//--------------------------------------------------------------------------------------------------
void OverlayColorLegend::layoutInfo(OverlayColorLegendLayoutInfo* layout)
{
    CVF_TIGHT_ASSERT(layout);

    ref<Glyph> glyph = m_font->getGlyph(L'A');
    layout->charHeight = static_cast<float>(glyph->height());
    layout->lineSpacing = layout->charHeight*1.5f;
    layout->margins = Vec2f(static_cast<float>(m_margin), static_cast<float>(m_margin));

    float legendWidth = 25.0f;
    float legendHeight = static_cast<float>(layout->size.y()) - 2*layout->margins.y() - static_cast<float>(m_titleStrings.size())*layout->lineSpacing - layout->lineSpacing;
    layout->legendRect = Rectf(layout->margins.x(), layout->margins.y() + layout->charHeight/2.0f, legendWidth, legendHeight);

    if (layout->legendRect.width() < 1 || layout->legendRect.height() < 1)
    {
        return;
    }

    layout->x0 = layout->margins.x();
    layout->x1 = layout->margins.x() + layout->legendRect.width();
    layout->tickX = layout->x1 + 5;

    size_t numTicks = m_tickValues.size();
    if (numTicks < 1)
    {
        return;
    }

    // Get legend range (the slightly odd test on the range should guard against any NaNs
    double minVal = m_tickValues[0];
    double maxVal = m_tickValues[numTicks - 1];
    double valueRange = (maxVal - minVal);
    if (!(valueRange >= 0))
    {
        layout->tickPixelPos = NULL;
        return;
    }

    // Build array containing the pixel positions of all the ticks
    layout->tickPixelPos = new DoubleArray(numTicks);
    if (valueRange > 0)
    {
        size_t i;
        for (i = 0; i < numTicks; i++)
        {
            double t = (m_tickValues[i] - minVal)/valueRange;
            t = Math::clamp(t, 0.0, 1.1);
            layout->tickPixelPos->set(i, t*layout->legendRect.height());
        }
    }
    else
    {
        size_t i;
        for (i = 0; i < numTicks; i++)
        {
            layout->tickPixelPos->set(i, static_cast<double>(i)*(layout->legendRect.height()/static_cast<double>(numTicks)));
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OverlayColorLegend::setLineColor(const Color3f& lineColor)
{
    m_lineColor = lineColor;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const Color3f& OverlayColorLegend::lineColor() const
{
    return m_lineColor;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OverlayColorLegend::setLineWidth(int lineWidth)
{
    m_lineWidth = lineWidth;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int OverlayColorLegend::lineWidth() const
{
    return m_lineWidth;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OverlayColorLegend::setWidthToFitText()
{
    cvf::uint textWidth = 0;
    for (size_t i = 0; i < m_titleStrings.size(); ++i)
    {
        cvf::uint lineWidth = m_font->textExtent(m_titleStrings[i]).x();

        if (lineWidth > textWidth)
        {
            textWidth = lineWidth;
        }
    }
    
    cvf::uint minWidth = m_font->textExtent("-0.00000e-000").x() + 40;

    // +1 to cater for any rasterization inaccuracy
    textWidth = CVF_MAX(minWidth + 1, textWidth + m_margin + 1);

    m_sizeHint.x() = textWidth;
}

} // namespace cvf

