/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021 -    Equinor ASA
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

#include "RimWellIAModelBox.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellIAModelBox::RimWellIAModelBox()
{
    m_vertices.resize( 8 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellIAModelBox::~RimWellIAModelBox()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> RimWellIAModelBox::vertices() const
{
    return m_vertices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimWellIAModelBox::center() const
{
    return m_center;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellIAModelBox::updateBox( cvf::Vec3d startPos, cvf::Vec3d endPos, double xyBuffer, double depthBuffer )
{
    m_center = startPos + endPos;
    m_center /= 2.0;

    cvf::Vec3d upwards = startPos - endPos;
    upwards.normalize();
    cvf::Vec3d downwards = upwards * -1.0;
    cvf::Vec3d xdir      = upwards.perpendicularVector();
    xdir.normalize();
    cvf::Vec3d ydir = upwards ^ xdir;
    ydir.normalize();

    cvf::Vec3d topCenter    = startPos + upwards * depthBuffer;
    cvf::Vec3d bottomCenter = endPos + downwards * depthBuffer;

    std::vector<cvf::Vec3d> topVertices    = generateRectangle( topCenter, xdir, ydir, xyBuffer );
    std::vector<cvf::Vec3d> bottomVertices = generateRectangle( bottomCenter, xdir, ydir, xyBuffer );

    for ( size_t i = 0; i < 4; i++ )
    {
        m_vertices[i]     = bottomVertices[i];
        m_vertices[i + 4] = topVertices[i];
    }

    return true;
}

std::vector<cvf::Vec3d>
    RimWellIAModelBox::generateRectangle( cvf::Vec3d center, cvf::Vec3d unitX, cvf::Vec3d unitY, double buffer )
{
    std::vector<cvf::Vec3d> corners;
    corners.resize( 4 );

    corners[0] = center;
    corners[0] -= unitX * buffer;
    corners[0] -= unitY * buffer;

    corners[1] = center;
    corners[1] += unitX * buffer;
    corners[1] -= unitY * buffer;

    corners[2] = center;
    corners[2] += unitX * buffer;
    corners[2] += unitY * buffer;

    corners[3] = center;
    corners[3] -= unitX * buffer;
    corners[3] += unitY * buffer;

    return corners;
}
