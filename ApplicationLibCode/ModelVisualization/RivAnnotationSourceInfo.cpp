/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023- Equinor ASA
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

#include "RivAnnotationSourceInfo.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivAnnotationSourceInfo::RivAnnotationSourceInfo( const std::string& text, const std::vector<cvf::Vec3d>& displayCoords )
    : m_text( text )
    , m_showColor( false )
    , m_color( cvf::Color3f( cvf::Color3f::BLACK ) )
    , m_labelPositionHint( RivAnnotationTools::LabelPositionStrategy::RIGHT )
    , m_anchorPointsInDisplayCoords( displayCoords )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivAnnotationSourceInfo::RivAnnotationSourceInfo( const std::vector<std::string>& texts, const std::vector<cvf::Vec3d>& displayCoords )
    : m_showColor( false )
    , m_color( cvf::Color3f( cvf::Color3f::BLACK ) )
    , m_labelPositionHint( RivAnnotationTools::LabelPositionStrategy::COUNT_HINT )
    , m_anchorPointsInDisplayCoords( displayCoords )
    , m_texts( texts )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivAnnotationTools::LabelPositionStrategy RivAnnotationSourceInfo::labelPositionStrategyHint() const
{
    return m_labelPositionHint;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivAnnotationSourceInfo::setLabelPositionStrategyHint( RivAnnotationTools::LabelPositionStrategy strategy )
{
    m_labelPositionHint = strategy;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RivAnnotationSourceInfo::text() const
{
    return m_text;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::string> RivAnnotationSourceInfo::texts() const
{
    return m_texts;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> RivAnnotationSourceInfo::anchorPointsInDisplayCoords() const
{
    return m_anchorPointsInDisplayCoords;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RivAnnotationSourceInfo::showColor() const
{
    return m_showColor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivAnnotationSourceInfo::setShowColor( bool showColor )
{
    m_showColor = showColor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RivAnnotationSourceInfo::color() const
{
    return m_color;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivAnnotationSourceInfo::setColor( const cvf::Color3f& color )
{
    m_color = color;
}
