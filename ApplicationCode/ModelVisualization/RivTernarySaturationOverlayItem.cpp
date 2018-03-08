/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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


#include "cvfBase.h"

#include "RivTernarySaturationOverlayItem.h"


#include "cvfOpenGL.h"
#include "cvfViewport.h"
#include "cvfCamera.h"
#include "cvfTextDrawer.h"
#include "cvfFont.h"
#include "cvfMatrixState.h"

#include "cvfRenderState_FF.h"
#include "cvfRenderStateDepth.h"
#include "cvfRenderStatePolygonOffset.h"
#include "cafInternalLegendRenderTools.h"



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivTernarySaturationOverlayItem::RivTernarySaturationOverlayItem(cvf::Font* font)
    : m_textColor(cvf::Color3::BLACK)
    , m_font(font)
    , m_size(120, 150)
    , m_backgroundColor(1.0f, 1.0f, 1.0f, 0.8f)
    , m_backgroundFrameColor(0.0f, 0.0f, 0.0f, 0.5f)
    , m_isBackgroundEnabled(true)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivTernarySaturationOverlayItem::~RivTernarySaturationOverlayItem()
{
    // Empty destructor to avoid errors with undefined types when cvf::ref's destructor gets called
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivTernarySaturationOverlayItem::setAxisLabelsColor(const cvf::Color3f& color)
{
    m_textColor = color;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec2ui RivTernarySaturationOverlayItem::sizeHint()
{
    return m_size;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivTernarySaturationOverlayItem::setSize(const cvf::Vec2ui& size)
{
    m_size = size;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivTernarySaturationOverlayItem::render(cvf::OpenGLContext* oglContext, const cvf::Vec2i& position, const cvf::Vec2ui& size)
{
    renderGeneric(oglContext, position, size, false);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivTernarySaturationOverlayItem::renderSoftware(cvf::OpenGLContext* oglContext, const cvf::Vec2i& position, const cvf::Vec2ui& size)
{
    renderGeneric(oglContext, position, size, true);
}


//--------------------------------------------------------------------------------------------------
/// Set up camera/viewport and render
//--------------------------------------------------------------------------------------------------
void RivTernarySaturationOverlayItem::renderGeneric(cvf::OpenGLContext* oglContext, 
                                                    const cvf::Vec2i& position, 
                                                    const cvf::Vec2ui& size, 
                                                    bool software)
{
    if (size.x() <= 0 || size.y() <= 0)
    {
        return;
    }

    float border = 0.0f;

    cvf::Camera camera;
    camera.setViewport(position.x(), position.y(), size.x(), size.y());
    camera.setProjectionAsPixelExact2D();
    camera.setViewMatrix(cvf::Mat4d::IDENTITY);
    camera.applyOpenGL();
    camera.viewport()->applyOpenGL(oglContext, cvf::Viewport::CLEAR_DEPTH);

    if ( m_isBackgroundEnabled )
    {
        if ( software )
        {
            caf::InternalLegendRenderTools::renderBackgroundImmediateMode(oglContext,
                                                                          cvf::Vec2f(size),
                                                                          m_backgroundColor,
                                                                          m_backgroundFrameColor);
        }
        else
        {
            const cvf::MatrixState matrixState(camera);

            caf::InternalLegendRenderTools::renderBackgroundUsingShaders(oglContext,
                                                                         matrixState,
                                                                         cvf::Vec2f(size),
                                                                         m_backgroundColor,
                                                                         m_backgroundFrameColor);
        }
        border = 3.0f;
    }

    cvf::TextDrawer textDrawer(m_font.p());
    textDrawer.setTextColor(m_textColor);

    float lineHeightInPixels = (float)(m_font->textExtent("SWAT").y() + 2);

    float textPosY = static_cast<float>(size.y() - lineHeightInPixels);
    for (size_t it = 0; it < m_titleStrings.size(); it++)
    {
        cvf::Vec2f pos(border, textPosY);
        textDrawer.addText(m_titleStrings[it], pos);

        textPosY -= lineHeightInPixels;
    }

    cvf::Vec2f pos(border, textPosY);
    textDrawer.addText("TERNARY", pos);
    textPosY -= lineHeightInPixels;
    textPosY -= border;

    {
        cvf::uint sgasTextWidth = m_font->textExtent("SGAS").x();
        textDrawer.addText("SGAS", cvf::Vec2f(static_cast<float>( (size.x() / 2) - sgasTextWidth / 2 ), textPosY));

        cvf::uint sgasRangeTextWidth = m_font->textExtent(m_sgasRange).x();
        textPosY -= lineHeightInPixels;
        textDrawer.addText(m_sgasRange, cvf::Vec2f(static_cast<float>( (size.x() / 2) - sgasRangeTextWidth / 2 ), textPosY));
    }

    textDrawer.addText("SWAT", cvf::Vec2f((float)border, (float)(lineHeightInPixels + border)));
    textDrawer.addText(m_swatRange, cvf::Vec2f((float)border, (float)border));

    {
        cvf::uint soilTextWidth = m_font->textExtent("SOIL").x();
        textDrawer.addText("SOIL", cvf::Vec2f(static_cast<float>(size.x() - soilTextWidth - border), lineHeightInPixels + border));

        cvf::uint soilRangeTextWidth = m_font->textExtent(m_soilRange).x();
        float soilRangePos = static_cast<float>(size.x()) - soilRangeTextWidth - border;

        textDrawer.addText(m_soilRange, cvf::Vec2f(soilRangePos, (float)border));
    }

    textDrawer.renderSoftware(oglContext, camera);

    textPosY -= 3;
    renderAxisImmediateMode(textPosY, 2*lineHeightInPixels + border, border,  oglContext);

    CVF_CHECK_OGL(oglContext);
}



//--------------------------------------------------------------------------------------------------
/// Draw the axis using immediate mode OpenGL
//--------------------------------------------------------------------------------------------------
void RivTernarySaturationOverlayItem::renderAxisImmediateMode(float upperBoundY, float lowerBoundY, float border, cvf::OpenGLContext* oglContext)
{
#ifdef CVF_OPENGL_ES
    CVF_UNUSED(layout);
    CVF_FAIL_MSG("Not supported on OpenGL ES");
#else

    cvf::RenderStateDepth depth(false);
    depth.applyOpenGL(oglContext);

    cvf::RenderStateLighting_FF lighting(false);
    lighting.applyOpenGL(oglContext);

    cvf::Color3ub colA(cvf::Color3::BLUE);
    cvf::Color3ub colB(cvf::Color3::GREEN);
    cvf::Color3ub colC(cvf::Color3::RED);

    //float upperBoundY = static_cast<float>(m_size.y() - 20);

    cvf::Vec3f a(float(border), lowerBoundY, 0);
    cvf::Vec3f b(static_cast<float>(m_size.x() - border), lowerBoundY, 0);
    cvf::Vec3f c(static_cast<float>(m_size.x() / 2), upperBoundY, 0);


    // Draw filled rectangle elements
    glBegin(GL_TRIANGLE_FAN);

    glColor3ubv(colA.ptr());
    glVertex3fv(a.ptr());
    
    glColor3ubv(colB.ptr());
    glVertex3fv(b.ptr());

    glColor3ubv(colC.ptr());
    glVertex3fv(c.ptr());
    glVertex3fv(c.ptr());
    glEnd();


    // Lines
    cvf::Color3ub linesColor(cvf::Color3::WHITE);
    glColor3ubv(linesColor.ptr());
    glBegin(GL_LINE_LOOP);
    glVertex3fv(a.ptr());
    glVertex3fv(b.ptr());
    glVertex3fv(c.ptr());
    glEnd();

    cvf::RenderStateDepth resetDepth;
    resetDepth.applyOpenGL(oglContext);

    // Reset render states
    cvf::RenderStateLighting_FF resetLighting;
    resetLighting.applyOpenGL(oglContext);

    CVF_CHECK_OGL(oglContext);
#endif // CVF_OPENGL_ES

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivTernarySaturationOverlayItem::setRangeText(const cvf::String& soilRange, const cvf::String& sgasRange, const cvf::String& swatRange)
{
    m_soilRange = soilRange;
    m_sgasRange = sgasRange;
    m_swatRange = swatRange;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivTernarySaturationOverlayItem::setTitle(const cvf::String& title)
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
void RivTernarySaturationOverlayItem::enableBackground(bool enable)
{
    m_isBackgroundEnabled = enable;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivTernarySaturationOverlayItem::setBackgroundColor(const cvf::Color4f& backgroundColor)
{
    m_backgroundColor = backgroundColor;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivTernarySaturationOverlayItem::setBackgroundFrameColor(const cvf::Color4f& backgroundFrameColor)
{
    m_backgroundFrameColor = backgroundFrameColor;
}

