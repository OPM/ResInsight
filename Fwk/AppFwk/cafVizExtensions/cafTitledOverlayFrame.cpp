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

#include "cafTitledOverlayFrame.h"
#include "cafCategoryMapper.h"
#include "cvfFont.h"

#include <algorithm>

using namespace cvf;

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TitledOverlayFrame::TitledOverlayFrame( Font* font, unsigned int width, unsigned int height )
    : m_font( font )
    , m_renderSize( width, height )
    , m_textColor( Color3::BLACK )
    , m_lineColor( Color3::BLACK )
    , m_lineWidth( 1 )
    , m_isBackgroundEnabled( true )
    , m_backgroundColor( 1.0f, 1.0f, 1.0f, 0.8f )
    , m_backgroundFrameColor( 0.0f, 0.0f, 0.0f, 0.5f )
{
}

TitledOverlayFrame::~TitledOverlayFrame()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void TitledOverlayFrame::setRenderSize( const Vec2ui& size )
{
    m_renderSize = size;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec2ui TitledOverlayFrame::renderSize() const
{
    return m_renderSize;
}

//--------------------------------------------------------------------------------------------------
/// Set color of the text
//--------------------------------------------------------------------------------------------------
void TitledOverlayFrame::setTextColor( const Color3f& color )
{
    m_textColor = color;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void TitledOverlayFrame::setFont( cvf::Font* font )
{
    m_font = font;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void TitledOverlayFrame::setLineColor( const Color3f& lineColor )
{
    m_lineColor = lineColor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void TitledOverlayFrame::setLineWidth( int lineWidth )
{
    m_lineWidth = lineWidth;
}

//--------------------------------------------------------------------------------------------------
/// Set the title (text that will be rendered above the legend)
///
/// The legend supports multi-line titles. Separate each line with a '\n' character
//--------------------------------------------------------------------------------------------------
void TitledOverlayFrame::setTitle( const String& title )
{
    // Title
    if ( title.isEmpty() )
    {
        m_titleStrings.clear();
    }
    else
    {
        m_titleStrings = title.split( "\n" );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void TitledOverlayFrame::enableBackground( bool enable )
{
    m_isBackgroundEnabled = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void TitledOverlayFrame::setBackgroundColor( const Color4f& backgroundColor )
{
    m_backgroundColor = backgroundColor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void TitledOverlayFrame::setBackgroundFrameColor( const Color4f& backgroundFrameColor )
{
    m_backgroundFrameColor = backgroundFrameColor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec2ui TitledOverlayFrame::sizeHint()
{
    return m_renderSize;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f TitledOverlayFrame::textColor() const
{
    return m_textColor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f TitledOverlayFrame::lineColor() const
{
    return m_lineColor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int TitledOverlayFrame::lineWidth() const
{
    return m_lineWidth;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool TitledOverlayFrame::backgroundEnabled() const
{
    return m_isBackgroundEnabled;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color4f TitledOverlayFrame::backgroundColor() const
{
    return m_backgroundColor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color4f TitledOverlayFrame::backgroundFrameColor() const
{
    return m_backgroundFrameColor;
}

std::vector<cvf::String>& TitledOverlayFrame::titleStrings()
{
    return m_titleStrings;
}

cvf::Font* TitledOverlayFrame::font()
{
    return m_font.p();
}
} // namespace caf
