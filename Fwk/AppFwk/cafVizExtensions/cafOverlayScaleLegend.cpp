//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2018- Ceetron Solutions AS
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

#include "cafOverlayScaleLegend.h"
#include "cvfBase.h"
#include "cvfBufferObjectManaged.h"
#include "cvfCamera.h"
#include "cvfFont.h"
#include "cvfGeometryBuilderDrawableGeo.h"
#include "cvfGeometryUtils.h"
#include "cvfGlyph.h"
#include "cvfMatrixState.h"
#include "cvfOpenGL.h"
#include "cvfOpenGLResourceManager.h"
#include "cvfRenderStateDepth.h"
#include "cvfRenderStateLine.h"
#include "cvfShaderProgram.h"
#include "cvfShaderProgramGenerator.h"
#include "cvfShaderSourceProvider.h"
#include "cvfShaderSourceRepository.h"
#include "cvfTextDrawer.h"
#include "cvfUniform.h"
#include "cvfViewport.h"

#include "cafInternalLegendRenderTools.h"
#include "cafTickMarkGenerator.h"

#ifndef CVF_OPENGL_ES
#include "cvfRenderState_FF.h"
#endif

#include "cvfRenderStateBlending.h"
#include "cvfScalarMapper.h"
#include <algorithm>
#include <array>
#include <cmath>

namespace caf
{
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
OverlayScaleLegend::OverlayScaleLegend( Font* font )
    : TitledOverlayFrame( font, 200, 200 )
    , m_tickNumberPrecision( 4 )
    , m_numberFormat( AUTO )
    , m_Layout( Vec2ui( 200u, 200u ) )
    , m_font( font )
    , m_orientation( HORIZONTAL )
    , m_currentScale( 1.0 )
{
    CVF_ASSERT( font );
    CVF_ASSERT( !font->isEmpty() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
OverlayScaleLegend::~OverlayScaleLegend()
{
    // Empty destructor to avoid errors with undefined types when cvf::ref's destructor gets called
}

//--------------------------------------------------------------------------------------------------
/// Hardware rendering using shader programs
//--------------------------------------------------------------------------------------------------
void OverlayScaleLegend::render( OpenGLContext* oglContext, const Vec2i& position, const Vec2ui& size )
{
    renderGeneric( oglContext, position, size, false );
}

//--------------------------------------------------------------------------------------------------
/// Software rendering using software
//--------------------------------------------------------------------------------------------------
void OverlayScaleLegend::renderSoftware( OpenGLContext* oglContext, const Vec2i& position, const Vec2ui& size )
{
    renderGeneric( oglContext, position, size, true );
}

//--------------------------------------------------------------------------------------------------
/// Set up camera/viewport and render
//--------------------------------------------------------------------------------------------------
void OverlayScaleLegend::renderGeneric( OpenGLContext* oglContext, const Vec2i& position, const Vec2ui& size, bool software )
{
    if ( size.x() <= 0 || size.y() <= 0 )
    {
        return;
    }

    Camera camera;
    camera.setViewport( position.x(), position.y(), size.x(), size.y() );
    camera.setProjectionAsPixelExact2D();
    camera.setViewMatrix( Mat4d::IDENTITY );
    camera.applyOpenGL();
    camera.viewport()->applyOpenGL( oglContext, Viewport::CLEAR_DEPTH );

    m_Layout = LayoutInfo( size );
    layoutInfo( &m_Layout );
    m_textDrawer = new TextDrawer( this->font() );

    // Set up text drawer
    float maxLegendRightPos = 0;
    if ( m_orientation == HORIZONTAL )
        setupHorizontalTextDrawer( m_textDrawer.p(), &m_Layout );
    else
        setupVerticalTextDrawer( m_textDrawer.p(), &m_Layout );

    Vec2f backgroundSize( size );

    // Do the actual rendering
    if ( software )
    {
        if ( this->backgroundEnabled() )
        {
            InternalLegendRenderTools::renderBackgroundImmediateMode( oglContext,
                                                                      backgroundSize,
                                                                      this->backgroundColor(),
                                                                      this->backgroundFrameColor() );
        }
        renderLegendImmediateMode( oglContext, &m_Layout );
        m_textDrawer->renderSoftware( oglContext, camera );
    }
    else
    {
        const MatrixState matrixState( camera );
        if ( this->backgroundEnabled() )
        {
            InternalLegendRenderTools::renderBackgroundUsingShaders( oglContext,
                                                                     matrixState,
                                                                     backgroundSize,
                                                                     this->backgroundColor(),
                                                                     this->backgroundFrameColor() );
        }
        renderLegendUsingShaders( oglContext, &m_Layout, matrixState );
        m_textDrawer->render( oglContext, camera );
    }

    CVF_CHECK_OGL( oglContext );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void OverlayScaleLegend::setupHorizontalTextDrawer( TextDrawer* textDrawer, const LayoutInfo* layout )
{
    CVF_ASSERT( layout );

    textDrawer->setVerticalAlignment( TextDrawer::CENTER );
    textDrawer->setTextColor( this->textColor() );

    m_visibleTickLabels.clear();

    const float textY = layout->axisStartPt.y() + layout->majorTickSize / 2.0f + layout->tickTextLeadSpace +
                        layout->charHeight;

    const float overlapTolerance = 1.2f * layout->charWidth;
    float       lastVisibleTextX = 0.0;

    size_t numTicks = layout->ticks.size();
    size_t numMajorTicks =
        std::count_if( layout->ticks.begin(), layout->ticks.end(), []( const LayoutInfo::Tick& t ) { return t.isMajor; } );
    size_t it;
    for ( it = 0; it < numTicks; it++ )
    {
        if ( /*numMajorTicks > 4 && */ !layout->ticks[it].isMajor ) continue;

        double tickValue = layout->ticks[it].domainValue;
        String valueString;
        switch ( m_numberFormat )
        {
            case FIXED:
                valueString = String::number( tickValue, 'f', m_tickNumberPrecision );
                break;
            case SCIENTIFIC:
                valueString = String::number( tickValue, 'e', m_tickNumberPrecision );
                break;
            default:
                valueString = String::number( tickValue );
                break;
        }

        auto textSize = m_font->textExtent( valueString );
        float textX = static_cast<float>( layout->axisStartPt.x() + layout->ticks[it].displayValue - textSize.x() / 2.0f );

        // Always draw first and last tick label. For all others, skip drawing if text ends up
        // on top of the previous label.
        if ( it != 0 && it != ( numTicks - 1 ) )
        {
            if ( cvf::Math::abs( textX - lastVisibleTextX ) < overlapTolerance )
            {
                m_visibleTickLabels.push_back( false );
                continue;
            }
            // Make sure it does not overlap the last tick as well

            float lastTickY = static_cast<float>( layout->axisStartPt.y() + layout->axisLength );

            if ( cvf::Math::abs( textX - lastTickY ) < overlapTolerance )
            {
                m_visibleTickLabels.push_back( false );
                continue;
            }
        }

        Vec2f pos( textX, textY );
        textDrawer->addText( valueString, pos );

        lastVisibleTextX = textX;
        m_visibleTickLabels.push_back( true );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void OverlayScaleLegend::setupVerticalTextDrawer( TextDrawer* textDrawer, const LayoutInfo* layout )
{
    CVF_ASSERT( layout );

    textDrawer->setVerticalAlignment( TextDrawer::CENTER );
    textDrawer->setTextColor( this->textColor() );

    m_visibleTickLabels.clear();

    const float textX = layout->axisStartPt.x() + layout->majorTickSize / 2.0f + layout->tickTextLeadSpace;

    const float overlapTolerance = 1.2f * layout->charHeight;
    float       lastVisibleTextY = 0.0;

    size_t numTicks = layout->ticks.size();
    size_t it;
    for ( it = 0; it < numTicks; it++ )
    {
        if ( numTicks > 4 && !layout->ticks[it].isMajor ) continue;

        float textY = static_cast<float>( layout->axisStartPt.y() + layout->ticks[it].displayValue );

        // Always draw first and last tick label. For all others, skip drawing if text ends up
        // on top of the previous label.
        if ( it != 0 && it != ( numTicks - 1 ) )
        {
            if ( cvf::Math::abs( textY - lastVisibleTextY ) < overlapTolerance )
            {
                m_visibleTickLabels.push_back( false );
                continue;
            }
            // Make sure it does not overlap the last tick as well

            float lastTickY = static_cast<float>( layout->axisStartPt.y() + layout->axisLength );

            if ( cvf::Math::abs( textY - lastTickY ) < overlapTolerance )
            {
                m_visibleTickLabels.push_back( false );
                continue;
            }
        }

        double tickValue = layout->ticks[it].domainValue;
        String valueString;
        switch ( m_numberFormat )
        {
            case FIXED:
                valueString = String::number( tickValue, 'f', m_tickNumberPrecision );
                break;
            case SCIENTIFIC:
                valueString = String::number( tickValue, 'e', m_tickNumberPrecision );
                break;
            default:
                valueString = String::number( tickValue );
                break;
        }

        Vec2f pos( textX, textY );
        textDrawer->addText( valueString, pos );

        lastVisibleTextY = textY;
        m_visibleTickLabels.push_back( true );
    }
}

//--------------------------------------------------------------------------------------------------
/// Draw the legend using shader programs
//--------------------------------------------------------------------------------------------------
void OverlayScaleLegend::renderLegendUsingShaders( OpenGLContext* oglContext, LayoutInfo* layout, const MatrixState& matrixState )
{
    CVF_CALLSITE_OPENGL( oglContext );

    CVF_TIGHT_ASSERT( layout );
    CVF_TIGHT_ASSERT( layout->overallLegendSize.x() > 0 );
    CVF_TIGHT_ASSERT( layout->overallLegendSize.y() > 0 );

    RenderStateDepth depth( false );
    depth.applyOpenGL( oglContext );
    RenderStateLine line( static_cast<float>( this->lineWidth() ) );
    line.applyOpenGL( oglContext );

    // All vertices. Initialized here to set Z to zero once and for all.
    static float vertexArray[] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

    // Per vector convenience pointers
    float* v0 = &vertexArray[0];
    float* v1 = &vertexArray[3];
    float* v2 = &vertexArray[6];
    float* v3 = &vertexArray[9];
    float* v4 = &vertexArray[12];

    // Connects
    static const ushort trianglesConnects[] = { 0, 1, 4, 0, 4, 3 };

    ref<ShaderProgram> shaderProgram = oglContext->resourceManager()->getLinkedUnlitColorShaderProgram( oglContext );
    CVF_TIGHT_ASSERT( shaderProgram.notNull() );

    if ( shaderProgram->useProgram( oglContext ) )
    {
        shaderProgram->clearUniformApplyTracking();
        shaderProgram->applyFixedUniforms( oglContext, matrixState );
    }

    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glEnableVertexAttribArray( ShaderProgram::VERTEX );
    glVertexAttribPointer( ShaderProgram::VERTEX, 3, GL_FLOAT, GL_FALSE, 0, vertexArray );

    // Draw axis
    {
        v0[0] = layout->axisStartPt.x();
        v0[1] = layout->axisStartPt.y();

        if ( m_orientation == HORIZONTAL )
        {
            v1[0] = v0[0] + layout->axisLength;
            v1[1] = v0[1];
        }
        else
        {
            v1[0] = v0[0];
            v1[1] = v0[1] + layout->axisLength;
        }

        static const ushort axisConnects[] = { 0, 1 };

        UniformFloat uniformColor( "u_color", Color4f( this->lineColor() ) );
        shaderProgram->applyUniform( oglContext, uniformColor );

#ifdef CVF_OPENGL_ES
        glDrawElements( GL_LINES, 2, GL_UNSIGNED_SHORT, axisConnects );
#else
        glDrawRangeElements( GL_LINES, 0, 3, 2, GL_UNSIGNED_SHORT, axisConnects );
#endif
    }

    // Draw ticks
    for ( const auto& tickInfo : layout->ticks )
    {
        float currTickSize = tickInfo.isMajor ? layout->majorTickSize : layout->minorTickSize;

        if ( m_orientation == HORIZONTAL )
        {
            v0[0] = layout->axisStartPt.x() + static_cast<float>( tickInfo.displayValue );
            v0[1] = layout->axisStartPt.y() - currTickSize / 2.0f;
            v1[0] = v0[0];
            v1[1] = v0[1] + currTickSize;
        }
        else
        {
            v0[0] = layout->axisStartPt.x() - currTickSize / 2.0f;
            v0[1] = layout->axisStartPt.y() + static_cast<float>( tickInfo.displayValue );
            v1[0] = v0[0] + currTickSize;
            v1[1] = v0[1];
        }

        static const ushort tickConnects[] = { 0, 1 };

        UniformFloat uniformColor( "u_color", Color4f( this->lineColor() ) );
        shaderProgram->applyUniform( oglContext, uniformColor );

#ifdef CVF_OPENGL_ES
        glDrawElements( GL_LINES, 2, GL_UNSIGNED_SHORT, axisConnects );
#else
        glDrawRangeElements( GL_LINES, 0, 3, 2, GL_UNSIGNED_SHORT, tickConnects );
#endif
    }

    glDisableVertexAttribArray( ShaderProgram::VERTEX );

    CVF_TIGHT_ASSERT( shaderProgram.notNull() );
    shaderProgram->useNoProgram( oglContext );

    // Reset render states
    RenderStateDepth resetDepth;
    resetDepth.applyOpenGL( oglContext );

    RenderStateLine resetLine;
    resetLine.applyOpenGL( oglContext );

    CVF_CHECK_OGL( oglContext );
}

//--------------------------------------------------------------------------------------------------
/// Draw the legend using immediate mode OpenGL
//--------------------------------------------------------------------------------------------------
void OverlayScaleLegend::renderLegendImmediateMode( OpenGLContext* oglContext, LayoutInfo* layout )
{
#if 0
#ifdef CVF_OPENGL_ES
    CVF_UNUSED(layout);
    CVF_FAIL_MSG("Not supported on OpenGL ES");
#else
    CVF_TIGHT_ASSERT(layout);
    CVF_TIGHT_ASSERT(layout->overallLegendSize.x() > 0);
    CVF_TIGHT_ASSERT(layout->overallLegendSize.y() > 0);

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
    };

    // Per vector convenience pointers
    float* v0 = &vertexArray[0];    
    float* v1 = &vertexArray[3];    
    float* v2 = &vertexArray[6];    
    float* v3 = &vertexArray[9];    
    float* v4 = &vertexArray[12];   

    // Constant coordinates
    v0[0] = v3[0] = layout->tickStartX;
    v1[0] = v4[0] = layout->tickMidX;

    // Render color bar as one colored quad per pixel

    int legendHeightPixelCount = static_cast<int>(layout->tickYPixelPos->get(m_tickValues.size() - 1) - layout->tickYPixelPos->get(0) + 0.01);
    if (m_scalarMapper.notNull())
    {
        int iPx;
        for (iPx = 0; iPx < legendHeightPixelCount; iPx++)
        {
            const Color3ub& clr = m_scalarMapper->mapToColor(m_scalarMapper->domainValue((iPx+0.5)/legendHeightPixelCount));
            float y0 = static_cast<float>(layout->colorBarRect.min().y() + iPx);
            float y1 = static_cast<float>(layout->colorBarRect.min().y() + iPx + 1);

            // Dynamic coordinates for rectangle
            v0[1] = v1[1] = y0;
            v3[1] = v4[1] = y1;

            // Draw filled rectangle elements
            glColor3ubv(clr.ptr());
            glBegin(GL_TRIANGLE_FAN);
            glVertex3fv(v0);
            glVertex3fv(v1);
            glVertex3fv(v4);
            glVertex3fv(v3);
            glEnd();
        }
    }

    // Render frame

    // Dynamic coordinates for  tickmarks-lines
    bool isRenderingFrame = true;
    if (isRenderingFrame)
    {
        v0[0] = v2[0] = layout->colorBarRect.min().x()-0.5f;
        v1[0] = v3[0] = layout->colorBarRect.max().x()-0.5f;
        v0[1] = v1[1] = layout->colorBarRect.min().y()-0.5f;
        v2[1] = v3[1] = layout->colorBarRect.max().y()-0.5f;

        glColor3fv(this->textColor().ptr());
        glBegin(GL_LINES);
        glVertex3fv(v0);
        glVertex3fv(v1);
        glVertex3fv(v1);
        glVertex3fv(v3);
        glVertex3fv(v3);
        glVertex3fv(v2);
        glVertex3fv(v2);
        glVertex3fv(v0);
        glEnd();

    }

    // Render tickmarks
    bool isRenderingTicks = true;

    if (isRenderingTicks)
    {
        // Constant coordinates
        v0[0] = layout->tickStartX;
        v1[0] = layout->tickMidX - 0.5f*(layout->tickEndX - layout->tickMidX) - 0.5f;
        v2[0] = layout->tickMidX;
        v3[0] = layout->tickEndX - 0.5f*(layout->tickEndX - layout->tickMidX) - 0.5f;
        v4[0] = layout->tickEndX;

        size_t ic;
        for (ic = 0; ic < m_tickValues.size(); ic++)
        {
            float y0 = static_cast<float>(layout->colorBarRect.min().y() + layout->tickYPixelPos->get(ic) - 0.5f);

            // Dynamic coordinates for  tickmarks-lines
            v0[1] = v1[1] = v2[1] = v3[1] = v4[1] = y0;

            glColor3fv(this->textColor().ptr());
            glBegin(GL_LINES);
            if ( m_visibleTickLabels[ic])
            {
                glVertex3fv(v0);
                glVertex3fv(v4); 
            }
            else
            {
                glVertex3fv(v2);
                glVertex3fv(v3);
            }
            glEnd();
        }
    }

    // Reset render states
    RenderStateLighting_FF resetLighting;
    resetLighting.applyOpenGL(oglContext);
    RenderStateDepth resetDepth;
    resetDepth.applyOpenGL(oglContext);

    CVF_CHECK_OGL(oglContext);
#endif // CVF_OPENGL_ES
#endif // 0
}

//--------------------------------------------------------------------------------------------------
/// Get layout information
//--------------------------------------------------------------------------------------------------
void OverlayScaleLegend::layoutInfo( LayoutInfo* layout )
{
    CVF_TIGHT_ASSERT( layout );

    // Input values
    float marginAlongAxis   = 8.0f;
    float marginAcrossAxis  = 8.0f;
    float tickTextLeadSpace = 5.0f;
    float majorTickSize     = 9.0f;
    float minorTickSize     = 5.0f;

    ref<Glyph> glyph          = this->font()->getGlyph( L'A' );
    layout->charWidth         = static_cast<float>( glyph->width() );
    layout->charHeight        = static_cast<float>( glyph->height() );
    layout->lineSpacing       = layout->charHeight * 1.5f;
    layout->tickTextLeadSpace = tickTextLeadSpace;
    layout->majorTickSize     = majorTickSize;
    layout->minorTickSize     = minorTickSize;

    double overallSizeValue;
    float  marginValue;

    if ( m_orientation == HORIZONTAL )
    {
        layout->margins     = Vec2f( marginAlongAxis, marginAcrossAxis );
        overallSizeValue    = layout->overallLegendSize.x();
        marginValue         = layout->margins.x();
        layout->axisStartPt = { layout->margins.x() + layout->charWidth / 2.0f,
                                layout->margins.y() + layout->majorTickSize / 2.0f };
    }
    else
    {
        layout->margins     = Vec2f( marginAcrossAxis, marginAlongAxis );
        overallSizeValue    = layout->overallLegendSize.y();
        marginValue         = layout->margins.y();
        layout->axisStartPt = { layout->margins.x() + layout->majorTickSize / 2.0f,
                                layout->margins.y() + layout->charHeight / 2.0f };
    }

    layout->axisLength = static_cast<float>( overallSizeValue ) - 2 * marginValue -
                         static_cast<float>( this->titleStrings().size() ) * layout->lineSpacing - layout->lineSpacing;

    auto currentScale = m_currentScale != 0.0 ? m_currentScale : 1.0;

    layout->ticks.clear();
    size_t numTicks = m_ticksInDomain.size();

    if ( numTicks > 1 )
    {
        double tickSpacingInDomain = m_ticksInDomain[1] - m_ticksInDomain[0];
        bool   finished            = false;
        for ( size_t i = 0; i < numTicks; i++ )
        {
            size_t intermediateTickCount = i == 0 ? 9 : 1;

            // Add one extra tick per domain tick
            for ( size_t j = 0; j < intermediateTickCount + 1; j++ )
            {
                double tickInDomain = m_ticksInDomain[i] +
                                      ( (double)j * tickSpacingInDomain / (double)( intermediateTickCount + 1 ) );
                double tickInDisplay = tickInDomain * currentScale;

                if ( tickInDisplay < layout->axisLength )
                {
                    bool isCenterTick = ( j == ( intermediateTickCount + 1 ) / 2 );
                    bool isMajorTick  = j == 0 || isCenterTick;
                    layout->ticks.emplace_back( LayoutInfo::Tick( tickInDisplay, tickInDomain, isMajorTick ) );

                    if ( i == 0 && isCenterTick ) break;
                }
                else
                {
                    finished = true;
                    break;
                }
            }
            if ( finished ) break;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void OverlayScaleLegend::setTickPrecision( int precision )
{
    m_tickNumberPrecision = precision;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void OverlayScaleLegend::setTickFormat( NumberFormat format )
{
    m_numberFormat = format;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void OverlayScaleLegend::setOrientation( Orientation orientation )
{
    m_orientation = orientation;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::OverlayScaleLegend::Orientation OverlayScaleLegend::orientation() const
{
    return m_orientation;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec2ui OverlayScaleLegend::preferredSize()
{
    uint preferredXSize = 100;
    uint preferredYSize = 100;
    return { (unsigned int)( std::ceil( preferredXSize ) ), (unsigned int)( std::ceil( preferredYSize ) ) };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void OverlayScaleLegend::setDisplayCoordTransform( const caf::DisplayCoordTransform* displayCoordTransform )
{
    m_dispalyCoordsTransform = displayCoordTransform;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void OverlayScaleLegend::updateFromCamera( const Camera* camera )
{
    auto windowSize = Vec2ui( camera->viewport()->width(), camera->viewport()->height() );

    Vec3d windowOrigoInDomain;
    Vec3d windowMaxInDomain;
    camera->unproject( Vec3d( 0, 0, 0 ), &windowOrigoInDomain );
    camera->unproject( Vec3d( windowSize.x(), windowSize.y(), 0 ), &windowMaxInDomain );

    if ( m_dispalyCoordsTransform.notNull() )
    {
        windowOrigoInDomain = m_dispalyCoordsTransform->transformToDomainCoord( windowOrigoInDomain );
        windowMaxInDomain   = m_dispalyCoordsTransform->transformToDomainCoord( windowMaxInDomain );
    }

    Vec3d windowOrigoPoint;
    Vec3d windowMaxPoint;
    camera->project( windowOrigoInDomain, &windowOrigoPoint );
    camera->project( windowMaxInDomain, &windowMaxPoint );

    double minStepSizeInDomain;
    double windowOrigoInDomainValue;
    double windowMaxInDomainValue;
    double windowOrigoPointValue;
    double windowMaxPointValue;
    int    tickMaxCount;

    auto textSize = m_font->textExtent( String::number( -1.999e-17 ) );
    if ( m_orientation == HORIZONTAL )
    {
        windowOrigoInDomainValue = windowOrigoInDomain.x();
        windowMaxInDomainValue   = windowMaxInDomain.x();
        windowOrigoPointValue    = windowOrigoPoint.x();
        windowMaxPointValue      = windowMaxPoint.x();
        tickMaxCount             = windowSize.x() / ( 2 * textSize.x() );
    }
    else
    {
        windowOrigoInDomainValue = windowOrigoInDomain.y();
        windowMaxInDomainValue   = windowMaxInDomain.y();
        windowOrigoPointValue    = windowOrigoPoint.y();
        windowMaxPointValue      = windowMaxPoint.y();
        tickMaxCount             = windowSize.y() / ( 2 * textSize.x() );
    }

    m_currentScale =
        ( windowMaxPointValue - windowOrigoPointValue ) / ( windowMaxInDomainValue - windowOrigoInDomainValue );
    minStepSizeInDomain = ( windowMaxInDomainValue - windowOrigoInDomainValue ) / tickMaxCount;

    caf::TickMarkGenerator tickCreator( windowOrigoInDomainValue, windowMaxInDomainValue, minStepSizeInDomain );
    auto                   ticks = tickCreator.tickMarkValues();

    m_ticksInDomain.clear();
    for ( const auto& tick : ticks )
    {
        m_ticksInDomain.push_back( tick - ticks.front() );
    }
}

} // namespace caf
