/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023     Equinor ASA
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

#include "RigBasicPlane.h"

#include "cvfTextureImage.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigBasicPlane::RigBasicPlane()
    : m_isRectValid( false )
    , m_maxHorzExtent( 0.0 )
    , m_maxVertExtentAbove( 0.0 )
    , m_maxVertExtentBelow( 0.0 )
{
    m_texture = new cvf::TextureImage();
    m_texture->allocate( 1, 1 );
    m_texture->fill( cvf::Color4ub( 0, 0, 0, 0 ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigBasicPlane::~RigBasicPlane()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigBasicPlane::reset()
{
    m_isRectValid = false;
    m_rect.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigBasicPlane::isValid() const
{
    return m_isRectValid;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigBasicPlane::setPlane( cvf::Vec3d anchorPoint, cvf::Vec3d normal )
{
    m_planeAnchor = anchorPoint;
    m_planeNormal = normal;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigBasicPlane::setColor( cvf::Color3f color )
{
    m_color = color;
    m_texture->fill( cvf::Color4ub( color.rByte(), color.gByte(), color.bByte(), 255 ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigBasicPlane::setMaxExtentFromAnchor( double maxExtentHorz, double maxExtentVertAbove, double maxExtentVertBelow )
{
    m_maxHorzExtent      = maxExtentHorz;
    m_maxVertExtentAbove = maxExtentVertAbove;
    m_maxVertExtentBelow = maxExtentVertBelow;
}

//--------------------------------------------------------------------------------------------------
///             ti
///      1 ----------- 2
///        |         |
///     ml |         | mr
///        |         |
///      0 ----------- 3
///             bi
//--------------------------------------------------------------------------------------------------
void RigBasicPlane::updateRect()
{
    if ( ( m_maxHorzExtent <= 0.0 ) || ( m_maxVertExtentAbove <= 0.0 ) || ( m_maxVertExtentBelow <= 0.0 ) )
    {
        m_isRectValid = false;
        return;
    }

    cvf::Vec3d zDirection( 0, 0, 1 );

    auto alongPlane = m_planeNormal ^ zDirection;
    auto upwards    = m_planeNormal ^ alongPlane;

    upwards.normalize();
    alongPlane.normalize();

    const double extHorz = m_maxHorzExtent / 2.0;
    auto         ml      = m_planeAnchor + alongPlane * extHorz;
    auto         mr      = m_planeAnchor - alongPlane * extHorz;

    m_rect.resize( 4 );
    m_rect[0] = ml - upwards * m_maxVertExtentAbove;
    m_rect[1] = ml + upwards * m_maxVertExtentBelow;
    m_rect[2] = mr + upwards * m_maxVertExtentBelow;
    m_rect[3] = mr - upwards * m_maxVertExtentAbove;

    m_isRectValid = true;

    m_topIntersect    = m_planeAnchor - upwards * m_maxVertExtentAbove;
    m_bottomIntersect = m_planeAnchor + upwards * m_maxVertExtentBelow;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3dArray RigBasicPlane::rect() const
{
    return m_rect;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::TextureImage> RigBasicPlane::texture() const
{
    return m_texture;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigBasicPlane::maxDepth()
{
    if ( !m_isRectValid ) return 0.0;

    double maxdepth = 0.0;
    for ( auto p : m_rect )
    {
        maxdepth = std::max( maxdepth, std::abs( p.z() ) );
    }
    return maxdepth;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<cvf::Vec3d, cvf::Vec3d> RigBasicPlane::intersectTopBottomLine()
{
    return std::make_pair( m_topIntersect, m_bottomIntersect );
}
