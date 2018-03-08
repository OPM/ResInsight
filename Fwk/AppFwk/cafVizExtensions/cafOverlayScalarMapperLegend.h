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

#include "cvfOverlayItem.h"
#include "cvfArray.h"
#include "cvfCamera.h"
#include "cvfString.h"
#include "cvfRect.h"

namespace cvf {
class Font;
class ShaderProgram;
class MatrixState;
class TextDrawer;
class ScalarMapper;
}

namespace caf {


//==================================================================================================
//
// Overlay color legend
//
//==================================================================================================
class OverlayScalarMapperLegend : public cvf::OverlayItem
{
    using Font = cvf::Font;
    using ScalarMapper = cvf::ScalarMapper;
    using OpenGLContext = cvf::OpenGLContext;
    using Vec2i = cvf::Vec2i;
    using Vec2ui = cvf::Vec2ui;
    using Color3f = cvf::Color3f;
    using Color4f = cvf::Color4f;
    using String = cvf::String;
    using DoubleArray = cvf::DoubleArray;
    using MatrixState = cvf::MatrixState;
    using Vec2f = cvf::Vec2f;
    using Rectf = cvf::Rectf;
    using TextDrawer = cvf::TextDrawer;

public:
    OverlayScalarMapperLegend(Font* font);
    virtual ~OverlayScalarMapperLegend();

    void            setScalarMapper(const ScalarMapper* scalarMapper);

    void            setSizeHint(const Vec2ui& size);

    void            setTextColor(const Color3f& color);
    void            setLineColor(const Color3f& lineColor);

    void            setLineWidth(int lineWidth);

    void            setTickPrecision(int precision);
    enum            NumberFormat { AUTO, SCIENTIFIC, FIXED};
    void            setTickFormat(NumberFormat format);

    void            enableBackground(bool enable);
    void            setBackgroundColor(const Color4f& backgroundColor); 
    void            setBackgroundFrameColor(const Color4f& backgroundFrameColor);

    void            setTitle(const String& title);

protected:
    Vec2ui          sizeHint() override;
    void            render(OpenGLContext* oglContext, const Vec2i& position, const Vec2ui& size) override;
    void            renderSoftware(OpenGLContext* oglContext, const Vec2i& position, const Vec2ui& size) override;
    bool            pick(int oglXCoord, int oglYCoord, const Vec2i& position, const Vec2ui& size) override;

    struct OverlayColorLegendLayoutInfo
    {
        OverlayColorLegendLayoutInfo(const Vec2i& pos, const Vec2ui& setSize)
        {
            position = pos;
            size = setSize;
        }

        float charHeight;
        float lineSpacing;
        Vec2f margins;
        float tickX;
        float x0, x1;

        Rectf legendRect;

        cvf::ref<DoubleArray> tickPixelPos;

        Vec2i position;
        Vec2ui size;
    };

    void         layoutInfo(OverlayColorLegendLayoutInfo* layout);

    void         renderGeneric(OpenGLContext* oglContext,
                               const Vec2i& position,
                               const Vec2ui& size,
                               bool software);
    void         renderLegendUsingShaders(OpenGLContext* oglContext,
                                          OverlayColorLegendLayoutInfo* layout,
                                          const MatrixState& matrixState);
    void         renderLegendImmediateMode(OpenGLContext* oglContext,
                                           OverlayColorLegendLayoutInfo* layout);
    void         setupTextDrawer(TextDrawer* textDrawer,
                                 const OverlayColorLegendLayoutInfo* layout,
                                 float* maxLegendRightPos);

protected:
    DoubleArray         m_tickValues;           // Ticks between each level + top and bottom of legend (n+1 entries)
    std::vector<bool>   m_visibleTickLabels;    // Skip tick labels ending up on top of previous visible label
    int                 m_tickNumberPrecision;
    NumberFormat        m_numberFormat;

    Vec2ui              m_sizeHint;     // Pixel size of the color legend area
    
    Color3f             m_textColor;
    Color3f             m_lineColor;
    bool                m_isBackgroundEnabled;
    Color4f             m_backgroundColor;
    Color4f             m_backgroundFrameColor;

    int                 m_lineWidth;

    std::vector<String> m_titleStrings;
    cvf::ref<Font>      m_font;

    cvf::cref<ScalarMapper> m_scalarMapper;
};

}
