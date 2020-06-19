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

#pragma once

#include "cafDisplayCoordTransform.h"
#include "cafTitledOverlayFrame.h"
#include "cvfArray.h"
#include "cvfCamera.h"
#include "cvfOverlayItem.h"
#include "cvfRect.h"
#include "cvfString.h"

#include <array>

namespace cvf
{
class Font;
class ShaderProgram;
class MatrixState;
class TextDrawer;
class ScalarMapper;
} // namespace cvf

namespace caf
{
//==================================================================================================
//
// Overlay color legend
//
//==================================================================================================
class OverlayScaleLegend : public caf::TitledOverlayFrame
{
    using Font          = cvf::Font;
    using OpenGLContext = cvf::OpenGLContext;
    using Vec2i         = cvf::Vec2i;
    using Vec2ui        = cvf::Vec2ui;
    using String        = cvf::String;
    using MatrixState   = cvf::MatrixState;
    using Vec2f         = cvf::Vec2f;
    using TextDrawer    = cvf::TextDrawer;
    using Camera        = cvf::Camera;

public:
    enum Orientation
    {
        HORIZONTAL,
        VERTICAL
    };

public:
    OverlayScaleLegend( Font* font );
    ~OverlayScaleLegend() override;

    void setTickPrecision( int precision );
    enum NumberFormat
    {
        AUTO,
        SCIENTIFIC,
        FIXED
    };
    void        setTickFormat( NumberFormat format );
    void        setOrientation( Orientation orientation );
    Orientation orientation() const;

    cvf::Vec2ui preferredSize() override;

    void setDisplayCoordTransform( const caf::DisplayCoordTransform* displayCoordTransform );
    void updateFromCamera( const Camera* camera );

protected:
    void render( OpenGLContext* oglContext, const Vec2i& position, const Vec2ui& size ) override;
    void renderSoftware( OpenGLContext* oglContext, const Vec2i& position, const Vec2ui& size ) override;

    struct LayoutInfo
    {
        struct Tick
        {
            Tick( double displayValue, double domainValue, bool isMajor )
                : displayValue( displayValue )
                , domainValue( domainValue )
                , isMajor( isMajor )
            {
            }
            double displayValue;
            double domainValue;
            bool   isMajor;
        };

        LayoutInfo( const Vec2ui& setSize )
        {
            charWidth         = 0.0f;
            charHeight        = 0.0f;
            lineSpacing       = 0.0f;
            margins           = cvf::Vec2f::ZERO;
            tickTextLeadSpace = 0.0f;

            axisStartPt   = cvf::Vec2f::ZERO;
            axisLength    = 0.0f;
            majorTickSize = 0.0f;
            minorTickSize = 0.0f;

            overallLegendSize = setSize;
        }

        float charWidth;
        float charHeight;
        float lineSpacing;
        Vec2f margins;
        float tickTextLeadSpace;

        // Rectf colorBarRect;
        Vec2f axisStartPt;
        float axisLength;
        float majorTickSize;
        float minorTickSize;

        std::vector<Tick> ticks;

        Vec2ui overallLegendSize;
    };

    void layoutInfo( LayoutInfo* layout );

    void renderGeneric( OpenGLContext* oglContext, const Vec2i& position, const Vec2ui& size, bool software );
    void renderLegendUsingShaders( OpenGLContext* oglContext, LayoutInfo* layout, const MatrixState& matrixState );
    void renderLegendImmediateMode( OpenGLContext* oglContext, LayoutInfo* layout );
    void setupHorizontalTextDrawer( TextDrawer* textDrawer, const LayoutInfo* layout );
    void setupVerticalTextDrawer( TextDrawer* textDrawer, const LayoutInfo* layout );

protected:
    std::vector<bool> m_visibleTickLabels; // Skip tick labels ending up on top of previous visible label
    int               m_tickNumberPrecision;
    NumberFormat      m_numberFormat;

    Orientation          m_orientation;
    LayoutInfo           m_Layout;
    cvf::ref<TextDrawer> m_textDrawer;

    cvf::ref<Font> m_font;

    cvf::cref<caf::DisplayCoordTransform> m_dispalyCoordsTransform;
    double                                m_currentScale; // [pixels/length]
    std::vector<double>                   m_ticksInDomain;
};

} // namespace caf
