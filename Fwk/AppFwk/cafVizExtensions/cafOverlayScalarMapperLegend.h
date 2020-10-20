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

#include "cafTitledOverlayFrame.h"
#include "cvfArray.h"
#include "cvfCamera.h"
#include "cvfOverlayItem.h"
#include "cvfRect.h"
#include "cvfString.h"

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
class OverlayScalarMapperLegend : public caf::TitledOverlayFrame
{
    using Font          = cvf::Font;
    using ScalarMapper  = cvf::ScalarMapper;
    using OpenGLContext = cvf::OpenGLContext;
    using Vec2i         = cvf::Vec2i;
    using Vec2ui        = cvf::Vec2ui;
    using Color3f       = cvf::Color3f;
    using Color4f       = cvf::Color4f;
    using String        = cvf::String;
    using DoubleArray   = cvf::DoubleArray;
    using MatrixState   = cvf::MatrixState;
    using Vec2f         = cvf::Vec2f;
    using Rectf         = cvf::Rectf;
    using TextDrawer    = cvf::TextDrawer;

public:
    OverlayScalarMapperLegend( Font* font );
    ~OverlayScalarMapperLegend() override;

    void setScalarMapper( const ScalarMapper* scalarMapper );

    void setTickPrecision( int precision );
    enum NumberFormat
    {
        AUTO,
        SCIENTIFIC,
        FIXED
    };
    void setTickFormat( NumberFormat format );

    cvf::Vec2ui preferredSize() override;

protected:
    void render( OpenGLContext* oglContext, const Vec2i& position, const Vec2ui& size ) override;
    void renderSoftware( OpenGLContext* oglContext, const Vec2i& position, const Vec2ui& size ) override;
    bool pick( int oglXCoord, int oglYCoord, const Vec2i& position, const Vec2ui& size ) override;

    struct OverlayColorLegendLayoutInfo
    {
        OverlayColorLegendLayoutInfo( const Vec2ui& setSize )
        {
            charHeight        = 0.0f;
            lineSpacing       = 0.0f;
            margins           = cvf::Vec2f::ZERO;
            tickStartX        = 0.0f;
            tickMidX          = 0.0f;
            tickEndX          = 0.0f;
            tickTextLeadSpace = 0.0f;

            overallLegendSize = setSize;
        }

        float charHeight;
        float lineSpacing;
        Vec2f margins;
        float tickStartX, tickMidX, tickEndX;
        float tickTextLeadSpace;

        Rectf colorBarRect;

        cvf::ref<DoubleArray> tickYPixelPos;

        Vec2ui overallLegendSize;
    };

    void layoutInfo( OverlayColorLegendLayoutInfo* layout );

    void renderGeneric( OpenGLContext* oglContext, const Vec2i& position, const Vec2ui& size, bool software );
    void renderLegendUsingShaders( OpenGLContext*                oglContext,
                                   OverlayColorLegendLayoutInfo* layout,
                                   const MatrixState&            matrixState );
    void renderLegendImmediateMode( OpenGLContext* oglContext, OverlayColorLegendLayoutInfo* layout );
    void setupTextDrawer( TextDrawer* textDrawer, const OverlayColorLegendLayoutInfo* layout );

protected:
    DoubleArray       m_tickValues; // Ticks between each level + top and bottom of legend (n+1 entries)
    std::vector<bool> m_visibleTickLabels; // Skip tick labels ending up on top of previous visible label
    int               m_tickNumberPrecision;
    NumberFormat      m_numberFormat;

    OverlayColorLegendLayoutInfo m_Layout;
    cvf::ref<TextDrawer>         m_textDrawer;
    cvf::cref<ScalarMapper>      m_scalarMapper;
};

} // namespace caf
