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
    updateRect();
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
void RigBasicPlane::setMaxExtentFromAnchor( double maxExtentHorz, double maxExtentVert )
{
    m_maxExtent = std::make_pair( maxExtentHorz, maxExtentVert );
    updateRect();
}

//--------------------------------------------------------------------------------------------------
///
///      1 ----------- 2
///        |         |
///     ml |         | mr
///        |         |
///      0 ----------- 3
///
//--------------------------------------------------------------------------------------------------
void RigBasicPlane::updateRect()
{
    auto [extHorz, extVert] = m_maxExtent;

    if ( ( extHorz == 0.0 ) || ( extVert == 0.0 ) )
    {
        m_isRectValid = false;
        return;
    }

    cvf::Vec3d zDirection( 0, 0, 1 );

    auto alongPlane = m_planeNormal ^ zDirection;
    auto upwards    = m_planeNormal ^ alongPlane;

    upwards.normalize();
    alongPlane.normalize();

    extHorz /= 2.0;
    extVert /= 2.0;

    auto ml = m_planeAnchor + alongPlane * extHorz;
    auto mr = m_planeAnchor - alongPlane * extHorz;

    m_rect.resize( 4 );
    m_rect[0] = ml - upwards * extVert;
    m_rect[1] = ml + upwards * extVert;
    m_rect[2] = mr + upwards * extVert;
    m_rect[3] = mr - upwards * extVert;

    m_isRectValid = true;
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
