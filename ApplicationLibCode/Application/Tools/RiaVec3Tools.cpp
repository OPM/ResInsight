////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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

#include "RiaVec3Tools.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RiaVec3Tools::invertZSign( const cvf::Vec3d& source )
{
    return { source.x(), source.y(), -source.z() };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> RiaVec3Tools::invertZSign( const std::vector<cvf::Vec3d>& source )
{
    std::vector<cvf::Vec3d> points;

    points.reserve( source.size() );
    for ( const auto& p : source )
    {
        points.emplace_back( invertZSign( p ) );
    }

    return points;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3f RiaVec3Tools::invertZSign( const cvf::Vec3f& source )
{
    return { source.x(), source.y(), -source.z() };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3f> RiaVec3Tools::invertZSign( const std::vector<cvf::Vec3f>& source )
{
    std::vector<cvf::Vec3f> points;

    points.reserve( source.size() );
    for ( const auto& p : source )
    {
        points.emplace_back( invertZSign( p ) );
    }

    return points;
}
