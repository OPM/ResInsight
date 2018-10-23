/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Statoil ASA
// 
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
// 
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
// 
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html> 
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////
#include "RivWindowEdgeAxesOverlayItem.h"

#include "cvfBase.h"
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

#include "cvfScalarMapper.h"
#include "cvfRenderStateBlending.h"
#include "cafTickMarkGenerator.h"
#include <array>

using namespace cvf;


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
RivWindowEdgeAxesOverlayItem::RivWindowEdgeAxesOverlayItem(Font* font)
:   m_windowSize(600, 600),
    m_textColor(Color3::BLACK),
    m_lineColor(Color3::BLACK),
    m_frameColor(Color3::WHITE),
    m_lineWidth(1),
    m_font(font),
    m_isSwitchingYAxisValueSign(true),
    m_domainAxes(XZ_AXES)
{
    CVF_ASSERT(font);
    CVF_ASSERT(!font->isEmpty());

    setLayoutFixedPosition({0,0});
    updateGeomerySizes();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivWindowEdgeAxesOverlayItem::~RivWindowEdgeAxesOverlayItem()
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivWindowEdgeAxesOverlayItem::setDisplayCoordTransform(const caf::DisplayCoordTransform* displayCoordTransform)
{
    m_dispalyCoordsTransform = displayCoordTransform;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivWindowEdgeAxesOverlayItem::updateGeomerySizes()
{
    String str = String::number(-1.999e-17);
    m_textSize =  m_font->textExtent(str);
    m_pixelSpacing = 2.0f;
    m_tickLineLength = m_textSize.y() *0.3f;
    m_frameBorderHeight = m_pixelSpacing + m_textSize.y() + m_pixelSpacing + m_tickLineLength +  m_lineWidth;
    m_frameBorderWidth  = m_pixelSpacing + m_textSize.x() + m_pixelSpacing + m_tickLineLength +  m_lineWidth;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivWindowEdgeAxesOverlayItem::updateFromCamera(const Camera* camera)
{
    if (!camera || camera->projection() != Camera::ORTHO )
    {
        m_domainCoordsXValues.clear();
        m_domainCoordsYValues.clear();
        m_windowTickXValues.clear();
        m_windowTickYValues.clear();

        return;
    }

    m_windowSize = Vec2ui( camera->viewport()->width(), camera->viewport()->height());
    Vec3d windowOrigoInDomain;
    Vec3d windowMaxInDomain;
    camera->unproject(Vec3d(0,0,0), &windowOrigoInDomain );
    camera->unproject(Vec3d(m_windowSize.x(),m_windowSize.y(),0), &windowMaxInDomain );

    if (m_dispalyCoordsTransform.notNull())
    {
        windowOrigoInDomain = m_dispalyCoordsTransform->transformToDomainCoord(windowOrigoInDomain);
        windowMaxInDomain = m_dispalyCoordsTransform->transformToDomainCoord(windowMaxInDomain);
    }

    double domainMinX = windowOrigoInDomain.x();
    double domainMaxX = windowMaxInDomain.x();

    double domainMinY = m_domainAxes == XY_AXES ? windowOrigoInDomain.y() : windowOrigoInDomain.z();
    double domainMaxY = m_domainAxes == XY_AXES ? windowMaxInDomain.y() : windowMaxInDomain.z();

    int xTickMaxCount = m_windowSize.x()/(2*m_textSize.x());
    int yTickMaxCount = m_windowSize.y()/(2*m_textSize.x());

    double minDomainXStepSize = (domainMaxX - domainMinX)/xTickMaxCount;
    caf::TickMarkGenerator xTickCreator(domainMinX, domainMaxX,  minDomainXStepSize);
    m_domainCoordsXValues = xTickCreator.tickMarkValues();

    double minDomainYStepSize = (domainMaxY - domainMinY)/yTickMaxCount;
    caf::TickMarkGenerator yTickCreator(domainMinY, domainMaxY,  minDomainYStepSize);
    m_domainCoordsYValues = yTickCreator.tickMarkValues();

 
    m_windowTickXValues.clear();
    Vec3d windowPoint;
    for (double domainX : m_domainCoordsXValues)
    {
        Vec3d displayDomainTick;
        if (m_domainAxes == XY_AXES)
        {
            displayDomainTick = Vec3d(domainX, domainMinY, 0);
        }
        else
        {
            displayDomainTick = Vec3d(domainX, 0, domainMinY);
        }
        if ( m_dispalyCoordsTransform.notNull() )
        {
            displayDomainTick = m_dispalyCoordsTransform->transformToDisplayCoord(displayDomainTick);
        }
        camera->project(displayDomainTick, &windowPoint);
        m_windowTickXValues.push_back(windowPoint.x());
    }

    m_windowTickYValues.clear();
    for (double domainY : m_domainCoordsYValues)
    {
        Vec3d displayDomainTick;

        if (m_domainAxes == XY_AXES)
        {
            displayDomainTick = Vec3d(domainMinX, domainY, 0);
        }
        else
        {
            displayDomainTick = Vec3d(domainMinX, 0, domainY);
        }

        if ( m_dispalyCoordsTransform.notNull() )
        {
            displayDomainTick = m_dispalyCoordsTransform->transformToDisplayCoord(displayDomainTick);
        }
        camera->project(displayDomainTick, &windowPoint);
        m_windowTickYValues.push_back(windowPoint.y());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec2ui RivWindowEdgeAxesOverlayItem::sizeHint()
{
    return m_windowSize;
}

//--------------------------------------------------------------------------------------------------
/// Set color of the text and lines to be rendered
//--------------------------------------------------------------------------------------------------
void RivWindowEdgeAxesOverlayItem::setTextColor(const Color3f& color)
{
    m_textColor = color;
}


//--------------------------------------------------------------------------------------------------
/// Returns the color of the text and lines
//--------------------------------------------------------------------------------------------------
const Color3f&  RivWindowEdgeAxesOverlayItem::textColor() const
{
    return m_textColor;
}

//--------------------------------------------------------------------------------------------------
/// Hardware rendering using shader programs
//--------------------------------------------------------------------------------------------------
void RivWindowEdgeAxesOverlayItem::render(OpenGLContext* oglContext, const Vec2i& position, const Vec2ui& size)
{
    renderGeneric(oglContext, position, size, false);
}


//--------------------------------------------------------------------------------------------------
/// Software rendering using software
//--------------------------------------------------------------------------------------------------
void RivWindowEdgeAxesOverlayItem::renderSoftware(OpenGLContext* oglContext, const Vec2i& position, const Vec2ui& size)
{
    renderGeneric(oglContext, position, size, true);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RivWindowEdgeAxesOverlayItem::pick(int oglXCoord, int oglYCoord, const Vec2i& position, const Vec2ui& size)
{
    return false;
}


//--------------------------------------------------------------------------------------------------
/// Set up camera/viewport and render
//--------------------------------------------------------------------------------------------------
void RivWindowEdgeAxesOverlayItem::renderGeneric(OpenGLContext* oglContext, const Vec2i& position, const Vec2ui& size, bool software)
{   
    if (size.x() <= 0 || size.y() <= 0
    || (m_windowTickXValues.size() == 0 && m_windowTickYValues.size() == 0 ) )
    {
        return;
    }

    Camera camera;
    camera.setViewport(position.x(), position.y(), size.x(), size.y());
    camera.setProjectionAsPixelExact2D();
    camera.setViewMatrix(Mat4d::IDENTITY);
    camera.applyOpenGL();
    camera.viewport()->applyOpenGL(oglContext, Viewport::CLEAR_DEPTH);

    TextDrawer textDrawer(m_font.p());
    addTextToTextDrawer(&textDrawer);


    if (software)
    {
        renderSoftwareFrameAndTickLines(oglContext);
        textDrawer.renderSoftware(oglContext, camera);
    }
    else
    {
        const MatrixState matrixState(camera);
        renderShaderFrameAndTickLines(oglContext, matrixState);

        textDrawer.render(oglContext, camera);
    }

    CVF_CHECK_OGL(oglContext);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivWindowEdgeAxesOverlayItem::addTextToTextDrawer(TextDrawer* textDrawer)
{
    textDrawer->setVerticalAlignment(TextDrawer::CENTER);
    textDrawer->setTextColor(m_textColor);

    // Bottom X - axis text
    {
        const float textYTop = m_windowSize.y() - m_pixelSpacing - m_textSize.y()*0.5f;
        const float textYBott = m_pixelSpacing + m_textSize.y()*0.5f;

        size_t numTicks = m_domainCoordsXValues.size();
        size_t i;
        for ( i = 0; i < numTicks; i++ )
        {
            float textX = static_cast<float>(m_windowTickXValues[i]);

            double tickValue = m_domainCoordsXValues[i];
            String valueString;

            valueString = String::number(tickValue);
            auto labelSize = m_font->textExtent(valueString);

            Vec2f pos(textX - labelSize.x()*0.5f, textYBott);
            textDrawer->addText(valueString, pos);
            pos[1] = textYTop;
            textDrawer->addText(valueString, pos);
        }
    }
 
    // Right Y - axis texts
    {
        const float textXRight = m_windowSize.x() - m_pixelSpacing - m_textSize.x();
        const float textXLeft = m_frameBorderWidth - m_tickLineLength - m_pixelSpacing;

        size_t numTicks = m_domainCoordsYValues.size();
        size_t i;
        for ( i = 0; i < numTicks; i++ )
        {
            float textY = static_cast<float>(m_windowTickYValues[i]);

            double tickValue = m_isSwitchingYAxisValueSign ? -m_domainCoordsYValues[i]: m_domainCoordsYValues[i];
            String valueString;

            valueString = String::number(tickValue);
            auto labelSize = m_font->textExtent(valueString);

            Vec2f pos(textXRight, textY);
            textDrawer->addText(valueString, pos);
            Vec2f posl(textXLeft - labelSize.x(), textY);
            textDrawer->addText(valueString, posl);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::array<Vec3f, 8> RivWindowEdgeAxesOverlayItem::frameVertexArray()
{
    float windowWidth = static_cast<float>(m_windowSize.x());
    float windowHeight = static_cast<float>(m_windowSize.y());

    //  3            2
    //    7       6
    //
    //    4       5
    //  0            1

    std::array<Vec3f, 8> vertexArray ={
        Vec3f(0.0f                            , 0.0f                              , 0.0f),
        Vec3f(windowWidth                     , 0.0f                              , 0.0f),
        Vec3f(windowWidth                     , windowHeight                      , 0.0f),
        Vec3f(0.0f                            , windowHeight                      , 0.0f),
                                                                                  
        Vec3f(m_frameBorderWidth              , m_frameBorderHeight               , 0.0f),
        Vec3f(windowWidth - m_frameBorderWidth, m_frameBorderHeight               , 0.0f),
        Vec3f(windowWidth - m_frameBorderWidth, windowHeight - m_frameBorderHeight, 0.0f),
        Vec3f(m_frameBorderWidth              , windowHeight - m_frameBorderHeight, 0.0f),
    };

    return vertexArray;
}

//--------------------------------------------------------------------------------------------------
/// Draw the legend using immediate mode OpenGL
//--------------------------------------------------------------------------------------------------
void RivWindowEdgeAxesOverlayItem::renderSoftwareFrameAndTickLines(OpenGLContext* oglContext)
{
    RenderStateDepth depth(false);
    depth.applyOpenGL(oglContext);

    RenderStateLighting_FF lighting(false);
    lighting.applyOpenGL(oglContext);

    RenderStateBlending blend;
    blend.configureTransparencyBlending();
    blend.applyOpenGL(oglContext);

    // Frame vertices

    std::array<Vec3f, 8> vertexArray = frameVertexArray();

    glColor4fv(m_frameColor.ptr());
    glBegin(GL_TRIANGLE_FAN);
    glVertex3fv(vertexArray[0].ptr());
    glVertex3fv(vertexArray[1].ptr());
    glVertex3fv(vertexArray[5].ptr());
    glVertex3fv(vertexArray[4].ptr());
    glEnd();
    glBegin(GL_TRIANGLE_FAN);
    glVertex3fv(vertexArray[1].ptr());
    glVertex3fv(vertexArray[2].ptr());
    glVertex3fv(vertexArray[6].ptr());
    glVertex3fv(vertexArray[5].ptr());
    glEnd();
    glBegin(GL_TRIANGLE_FAN);
    glVertex3fv(vertexArray[3].ptr());
    glVertex3fv(vertexArray[0].ptr());
    glVertex3fv(vertexArray[4].ptr());
    glVertex3fv(vertexArray[7].ptr());
    glEnd();
    glBegin(GL_TRIANGLE_FAN);
    glVertex3fv(vertexArray[2].ptr());
    glVertex3fv(vertexArray[3].ptr());
    glVertex3fv(vertexArray[7].ptr());
    glVertex3fv(vertexArray[6].ptr());
    glEnd();


    // Render Line around

    {
        glColor3fv(m_lineColor.ptr());
        glBegin(GL_LINES);
        // Frame lines
        glVertex3fv(vertexArray[7].ptr());
        glVertex3fv(vertexArray[4].ptr());
        glVertex3fv(vertexArray[4].ptr());
        glVertex3fv(vertexArray[5].ptr());
        glVertex3fv(vertexArray[5].ptr());
        glVertex3fv(vertexArray[6].ptr());
        glVertex3fv(vertexArray[6].ptr());
        glVertex3fv(vertexArray[7].ptr());

        // X - axis Tick lines
        for (double txpos : m_windowTickXValues)
        {
            Vec3f p1(Vec3f::ZERO);
            Vec3f p2(Vec3f::ZERO);

            p1[0] = (float)txpos;
            p1[1] = m_frameBorderHeight;
            p2[0] = (float)txpos;
            p2[1] = m_frameBorderHeight - m_tickLineLength;

            glVertex3fv(p1.ptr());
            glVertex3fv(p2.ptr());

            p1[0] = (float)txpos;
            p1[1] = m_windowSize.y() - m_frameBorderHeight;
            p2[0] = (float)txpos;
            p2[1] = m_windowSize.y() - m_frameBorderHeight + m_tickLineLength;

            glVertex3fv(p1.ptr());
            glVertex3fv(p2.ptr());
        }

        // Left Y - axis Tick lines
        for (double typos : m_windowTickYValues)
        {
            Vec3f p1(Vec3f::ZERO);
            Vec3f p2(Vec3f::ZERO);

            p1[0] = m_frameBorderWidth;
            p1[1] = (float)typos;
            p2[0] = m_frameBorderWidth - m_tickLineLength;
            p2[1] = (float)typos;

            glVertex3fv(p1.ptr());
            glVertex3fv(p2.ptr());

            p1[0] = m_windowSize.x() - m_frameBorderWidth;
            p1[1] = (float)typos;
            p2[0] = m_windowSize.x() - m_frameBorderWidth + m_tickLineLength;
            p2[1] = (float)typos;

            glVertex3fv(p1.ptr());
            glVertex3fv(p2.ptr());
        }

        glEnd();
    }

    // Reset render states

    RenderStateLighting_FF resetLighting;
    resetLighting.applyOpenGL(oglContext);
    RenderStateDepth resetDepth;
    resetDepth.applyOpenGL(oglContext);
    RenderStateBlending resetblend;
    resetblend.applyOpenGL(oglContext);
    CVF_CHECK_OGL(oglContext);
}

//--------------------------------------------------------------------------------------------------
/// Draw the frame using shader programs
//--------------------------------------------------------------------------------------------------
void RivWindowEdgeAxesOverlayItem::renderShaderFrameAndTickLines(OpenGLContext* oglContext, const MatrixState& matrixState)
{
    CVF_CALLSITE_OPENGL(oglContext);

    RenderStateDepth depth(false);
    depth.applyOpenGL(oglContext);

    RenderStateLine line(static_cast<float>(m_lineWidth));
    line.applyOpenGL(oglContext);

    RenderStateBlending blend;
    blend.configureTransparencyBlending();
    blend.applyOpenGL(oglContext);

    // Shader program
    
    ref<ShaderProgram> shaderProgram = oglContext->resourceManager()->getLinkedUnlitColorShaderProgram(oglContext);
    CVF_TIGHT_ASSERT(shaderProgram.notNull());

    if (shaderProgram->useProgram(oglContext))
    {
        shaderProgram->clearUniformApplyTracking();
        shaderProgram->applyFixedUniforms(oglContext, matrixState);
    }

    // Frame vertices
    std::array<Vec3f, 8> vertexArray = frameVertexArray();

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glEnableVertexAttribArray(ShaderProgram::VERTEX);
    glVertexAttribPointer(ShaderProgram::VERTEX, 3, GL_FLOAT, GL_FALSE, 0, vertexArray.data());

    // Draw frame background

    UniformFloat backgroundColorUniform("u_color", m_frameColor);
    shaderProgram->applyUniform(oglContext, backgroundColorUniform);

    // Triangle indices for the frame background

    static const ushort backgroundTriangleIndices[] = { 0, 1, 5,  0, 5, 4,
                                                        1, 2, 6,  1, 6, 5,
                                                        3, 0, 4,  3, 4, 7,
                                                        2, 3, 6,  3, 7, 6 };

    glDrawRangeElements(GL_TRIANGLES, 0, 7, 24, GL_UNSIGNED_SHORT, backgroundTriangleIndices);


    // Draw frame border lines

    UniformFloat uniformColor("u_color", Color4f(m_lineColor));
    shaderProgram->applyUniform(oglContext, uniformColor);

    static const ushort frameLineIndices[] = { 7, 4, 
                                               4, 5, 
                                               5, 6,
                                               6, 7 };

    glDrawRangeElements(GL_LINES, 0, 7, 8, GL_UNSIGNED_SHORT, frameLineIndices);

    // Render tickmarks

    static const ushort tickLineIndices[] = { 0, 1 };

    // X - axis Tick lines

    for (double txpos : m_windowTickXValues)
    {
        vertexArray[0][0] = (float)txpos;
        vertexArray[0][1] = m_frameBorderHeight;
        vertexArray[1][0] = (float)txpos;
        vertexArray[1][1] = m_frameBorderHeight - m_tickLineLength;

        glDrawRangeElements(GL_LINES, 0, 1, 2, GL_UNSIGNED_SHORT, tickLineIndices);

        vertexArray[0][0] = (float)txpos;
        vertexArray[0][1] = m_windowSize.y() - m_frameBorderHeight;
        vertexArray[1][0] = (float)txpos;
        vertexArray[1][1] = m_windowSize.y() - m_frameBorderHeight + m_tickLineLength;

        glDrawRangeElements(GL_LINES, 0, 1, 2, GL_UNSIGNED_SHORT, tickLineIndices);
    }

    // Left Y - axis Tick lines
    
    for (double typos : m_windowTickYValues)
    {
        vertexArray[0][0] = m_frameBorderWidth;
        vertexArray[0][1] = (float)typos;
        vertexArray[1][0] = m_frameBorderWidth - m_tickLineLength;
        vertexArray[1][1] = (float)typos;

        glDrawRangeElements(GL_LINES, 0, 1, 2, GL_UNSIGNED_SHORT, tickLineIndices);

        vertexArray[0][0] = m_windowSize.x() - m_frameBorderWidth;
        vertexArray[0][1] = (float)typos;
        vertexArray[1][0] = m_windowSize.x() - m_frameBorderWidth + m_tickLineLength;
        vertexArray[1][1] = (float)typos;

        glDrawRangeElements(GL_LINES, 0, 1, 2, GL_UNSIGNED_SHORT, tickLineIndices);
    }
    
    glDisableVertexAttribArray(ShaderProgram::VERTEX);

    CVF_TIGHT_ASSERT(shaderProgram.notNull());
    shaderProgram->useNoProgram(oglContext);

    // Reset render states
    RenderStateDepth resetDepth;
    resetDepth.applyOpenGL(oglContext);

    RenderStateLine resetLine;
    resetLine.applyOpenGL(oglContext);

    RenderStateBlending resetblend;
    resetblend.applyOpenGL(oglContext);

    CVF_CHECK_OGL(oglContext);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivWindowEdgeAxesOverlayItem::setLineColor(const Color3f& lineColor)
{
    m_lineColor = lineColor;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const Color3f& RivWindowEdgeAxesOverlayItem::lineColor() const
{
    return m_lineColor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivWindowEdgeAxesOverlayItem::setDomainAxes(DomainAxes axes)
{
    m_domainAxes = axes;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivWindowEdgeAxesOverlayItem::setIsSwitchingYAxisSign(bool switchSign)
{
    m_isSwitchingYAxisValueSign = switchSign;
}

