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

#pragma once

#include "cafDisplayCoordTransform.h"
#include "cvfArray.h"
#include "cvfCamera.h"
#include "cvfOverlayItem.h"
#include "cvfRect.h"
#include "cvfString.h"

namespace caf
{
class DisplayCoordTransform;
}

namespace cvf
{
class Font;
class ShaderProgram;
class MatrixState;
class TextDrawer;
class ScalarMapper;
} // namespace cvf

//==================================================================================================
//
// Overlay color legend
//
//==================================================================================================
class RivWindowEdgeAxesOverlayItem : public cvf::OverlayItem
{
    using Font          = cvf::Font;
    using Vec2ui        = cvf::Vec2ui;
    using ScalarMapper  = cvf::ScalarMapper;
    using OpenGLContext = cvf::OpenGLContext;
    using Vec2i         = cvf::Vec2i;
    using Color3f       = cvf::Color3f;
    using Color4f       = cvf::Color4f;
    using String        = cvf::String;
    using Vec2f         = cvf::Vec2f;
    using Vec3f         = cvf::Vec3f;
    using Rectf         = cvf::Rectf;
    using DoubleArray   = cvf::DoubleArray;
    using MatrixState   = cvf::MatrixState;
    using TextDrawer    = cvf::TextDrawer;
    using Camera        = cvf::Camera;

public:
    enum DomainAxes
    {
        XY_AXES,
        XZ_AXES
    };

public:
    RivWindowEdgeAxesOverlayItem( Font* font );
    ~RivWindowEdgeAxesOverlayItem() override;

    void setDisplayCoordTransform( const caf::DisplayCoordTransform* displayCoordTransform );
    void updateFromCamera( const Camera* camera );

    void           setTextColor( const Color3f& color );
    const Color3f& textColor() const;
    void           setLineColor( const Color3f& lineColor );
    const Color3f& lineColor() const;
    void           setFrameColor( const Color4f& frameColor );
    void           setDomainAxes( DomainAxes axes );
    void           setIsSwitchingYAxisSign( bool switchSign );
    int            frameBorderWidth() { return static_cast<int>( m_frameBorderWidth ); }
    int            frameBorderHeight() { return static_cast<int>( m_frameBorderHeight ); }

    void setShowAxisLines( bool showAxisLines );

protected:
    Vec2ui sizeHint() override;
    void   render( OpenGLContext* oglContext, const Vec2i& position, const Vec2ui& size ) override;
    void   renderSoftware( OpenGLContext* oglContext, const Vec2i& position, const Vec2ui& size ) override;
    bool   pick( int oglXCoord, int oglYCoord, const Vec2i& position, const Vec2ui& size ) override;

private:
    void updateGeomerySizes();
    void renderGeneric( OpenGLContext* oglContext, const Vec2i& position, const Vec2ui& size, bool software );
    void renderSoftwareFrameAndTickLines( OpenGLContext* oglContext );
    void renderShaderFrameAndTickLines( OpenGLContext* oglContext, const MatrixState& matrixState );
    void addTextToTextDrawer( TextDrawer* textDrawer );

    std::array<Vec3f, 8> frameVertexArray();

private:
    cvf::cref<caf::DisplayCoordTransform> m_dispalyCoordsTransform;

    Vec2ui         m_windowSize; // Pixel size of the window
    Vec2ui         m_textSize;
    Color3f        m_textColor;
    Color3f        m_lineColor;
    Color4f        m_frameColor;
    int            m_lineWidth;
    cvf::ref<Font> m_font;

    float      m_frameBorderHeight;
    float      m_frameBorderWidth;
    float      m_tickLineLength;
    float      m_pixelSpacing;
    bool       m_isSwitchingYAxisValueSign;
    bool       m_showAxisLines;
    DomainAxes m_domainAxes;

    std::vector<double> m_domainCoordsXValues;
    std::vector<double> m_domainCoordsYValues;
    std::vector<double> m_windowTickXValues;
    std::vector<double> m_windowTickYValues;
};
