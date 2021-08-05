/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021 -     Equinor ASA
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

#include "RivDrawableSpheres.h"

#include "cvfRay.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivDrawableSpheres::RivDrawableSpheres()
    : cvf::DrawableVectors()
    , m_radius( 1.0 )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivDrawableSpheres::RivDrawableSpheres( cvf::String vectorMatrixUniformName, cvf::String colorUniformName )
    : cvf::DrawableVectors( vectorMatrixUniformName, colorUniformName )
    , m_radius( 1.0 )
{
}

//--------------------------------------------------------------------------------------------------
/// Estimate the intersection of a sphere by the sphere inscribed in a bounding box
//--------------------------------------------------------------------------------------------------
bool RivDrawableSpheres::rayIntersectCreateDetail( const cvf::Ray&           ray,
                                                   cvf::Vec3d*               intersectionPoint,
                                                   cvf::ref<cvf::HitDetail>* hitDetail ) const
{
    if ( m_centerCoordArray.isNull() ) return false;

    for ( size_t i = 0; i < m_centerCoordArray->size(); i++ )
    {
        cvf::BoundingBox bb;

        cvf::Vec3f center  = m_centerCoordArray->get( i );
        cvf::Vec3f corner1 = cvf::Vec3f( center.x() + m_radius, center.y() + m_radius, center.z() + m_radius );
        cvf::Vec3f corner2 = cvf::Vec3f( center.x() - m_radius, center.y() - m_radius, center.z() - m_radius );

        bb.add( corner1 );
        bb.add( corner2 );

        if ( ray.boxIntersect( bb, intersectionPoint ) )
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivDrawableSpheres::setRadius( float radius )
{
    m_radius = radius;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivDrawableSpheres::setCenterCoords( cvf::Vec3fArray* vertexArray )
{
    m_centerCoordArray = vertexArray;
}
