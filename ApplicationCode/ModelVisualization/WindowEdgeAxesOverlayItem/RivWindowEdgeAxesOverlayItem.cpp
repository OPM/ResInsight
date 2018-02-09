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
    m_lineWidth(1),
    m_font(font),
    m_isSwitchingYAxisValueSign(true)
{
    CVF_ASSERT(font);
    CVF_ASSERT(!font->isEmpty());

    m_tickValues.reserve(3);
    m_tickValues.add(0.0);
    m_tickValues.add(0.5);
    m_tickValues.add(1.0);

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
    String str = String::number(99999.0);
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

    double domainMinY = windowOrigoInDomain.z();
    double domainMaxY = windowMaxInDomain.z();

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
        Vec3d displayDomainTick(domainX, 0, domainMinY);
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
        Vec3d displayDomainTick(domainMinX, 0, domainY);
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
    render(oglContext, position, size, false);
}


//--------------------------------------------------------------------------------------------------
/// Software rendering using software
//--------------------------------------------------------------------------------------------------
void RivWindowEdgeAxesOverlayItem::renderSoftware(OpenGLContext* oglContext, const Vec2i& position, const Vec2ui& size)
{
    render(oglContext, position, size, true);
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
void RivWindowEdgeAxesOverlayItem::render(OpenGLContext* oglContext, const Vec2i& position, const Vec2ui& size, bool software)
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

    TextDrawer textDrawer(m_font.p());
    addTextToTextDrawer(&textDrawer);

    renderLegendImmediateMode(oglContext);

    if (software)
    {
        textDrawer.renderSoftware(oglContext, camera);
    }
    else
    {
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
        const float textY = m_pixelSpacing + m_textSize.y()*0.5f;
        size_t numTicks = m_domainCoordsXValues.size();
        size_t i;
        for ( i = 0; i < numTicks; i++ )
        {
            float textX = static_cast<float>(m_windowTickXValues[i]);

            double tickValue = m_domainCoordsXValues[i];
            String valueString;

            valueString = String::number(tickValue);
            auto labelSize = m_font->textExtent(valueString);

            Vec2f pos(textX - labelSize.x()*0.5f, textY);
            textDrawer->addText(valueString, pos);
        }
    }

    // Right Y - axis texts
    {
        const float textX = m_windowSize.x() - m_pixelSpacing - m_textSize.x();

        size_t numTicks = m_domainCoordsYValues.size();
        size_t i;
        for ( i = 0; i < numTicks; i++ )
        {
            float textY = static_cast<float>(m_windowTickYValues[i]);

            double tickValue = m_isSwitchingYAxisValueSign ? -m_domainCoordsYValues[i]: m_domainCoordsYValues[i];
            String valueString;

            valueString = String::number(tickValue);
            auto labelSize = m_font->textExtent(valueString);

            Vec2f pos(textX, textY);
            textDrawer->addText(valueString, pos);
        }
    }

    // Left Y - axis texts
    {
        const float textX = m_frameBorderWidth - m_tickLineLength - m_pixelSpacing;

        size_t numTicks = m_domainCoordsYValues.size();
        size_t i;
        for ( i = 0; i < numTicks; i++ )
        {
            float textY = static_cast<float>(m_windowTickYValues[i]);

            double tickValue =  m_isSwitchingYAxisValueSign ? -m_domainCoordsYValues[i]: m_domainCoordsYValues[i];
            String valueString;

            valueString = String::number(tickValue);
            auto labelSize = m_font->textExtent(valueString);

            Vec2f pos(textX - labelSize.x(), textY);
            textDrawer->addText(valueString, pos);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Draw the legend using immediate mode OpenGL
//--------------------------------------------------------------------------------------------------
void RivWindowEdgeAxesOverlayItem::renderLegendImmediateMode(OpenGLContext* oglContext)
{
    RenderStateDepth depth(false);
    depth.applyOpenGL(oglContext);

    RenderStateLighting_FF lighting(false);
    lighting.applyOpenGL(oglContext);

    RenderStateBlending blend;
    blend.configureTransparencyBlending();
    blend.applyOpenGL(oglContext);

    // Frame vertices
      
    Vec3f v0(Vec3f::ZERO);   
    Vec3f v1(Vec3f::ZERO);   
    Vec3f v2(Vec3f::ZERO);   
    Vec3f v3(Vec3f::ZERO);   
    Vec3f v4(Vec3f::ZERO);   
    Vec3f v5(Vec3f::ZERO);   
    Vec3f v6(Vec3f::ZERO);   
    Vec3f v7(Vec3f::ZERO);   

    v1[0] = static_cast<float>(m_windowSize.x());
    v2[0] = static_cast<float>(m_windowSize.x());
    v2[1] = static_cast<float>(m_windowSize.y());
    v3[1] = static_cast<float>(m_windowSize.y());
    
    v4[0] = m_frameBorderWidth;
    v4[1] = m_frameBorderHeight;

    v5[0] = v1[0] - m_frameBorderWidth;
    v5[1] = m_frameBorderHeight;

    v6[0] = v2[0] - m_frameBorderWidth;
    v6[1] = v2[1];
    
    v7[0] = m_frameBorderWidth;
    v7[1] = v3[1];

    glColor4fv(Vec4f(1.0f,1.0f,1.0f,0.5f).ptr());
    glBegin(GL_TRIANGLE_FAN);
    glVertex3fv(v0.ptr());
    glVertex3fv(v1.ptr());
    glVertex3fv(v5.ptr());
    glVertex3fv(v4.ptr());
    glEnd();
    glBegin(GL_TRIANGLE_FAN);
    glVertex3fv(v1.ptr());
    glVertex3fv(v2.ptr());
    glVertex3fv(v6.ptr());
    glVertex3fv(v5.ptr());
    glEnd();
    glBegin(GL_TRIANGLE_FAN);
    glVertex3fv(v3.ptr());
    glVertex3fv(v0.ptr());
    glVertex3fv(v4.ptr());
    glVertex3fv(v7.ptr());
    glEnd();


    // Render Line around

    {
        glColor3fv(m_lineColor.ptr());
        glBegin(GL_LINES);
        // Frame lines
        glVertex3fv(v7.ptr());
        glVertex3fv(v4.ptr());
        glVertex3fv(v4.ptr());
        glVertex3fv(v5.ptr());
        glVertex3fv(v5.ptr());
        glVertex3fv(v6.ptr());

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

