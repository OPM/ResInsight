/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "RivDiskGeometryGenerator.h"

#include "cvfArray.h"
#include "cvfBase.h"
#include "cvfMath.h"

#include "cvfGeometryBuilder.h"
#include "cvfGeometryUtils.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivDiskGeometryGenerator::RivDiskGeometryGenerator()
    : m_relativeRadius( 0.085f )
    , m_relativeLength( 0.25f )
    , m_numSlices( 20 )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivDiskGeometryGenerator::setRelativeRadius( float relativeRadius )
{
    m_relativeRadius = relativeRadius;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivDiskGeometryGenerator::setRelativeLength( float relativeLength )
{
    m_relativeLength = relativeLength;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivDiskGeometryGenerator::setNumSlices( unsigned int numSlices )
{
    m_numSlices = numSlices;
}

//--------------------------------------------------------------------------------------------------
/// Create a disc centered at origin with its normal along positive z-axis
///
/// \param radius     Outer radius of the disc
/// \param numSlices  The number of subdivisions around the z-axis. Must be >= 4
/// \param builder    Geometry builder to use when creating geometry
///
/// Creates a disc on the z = 0 plane, centered at origin and with its surface normal pointing
/// along the positive z-axis.
///
/// The disk is subdivided around the z axis into numSlices (as in pizza slices).
///
/// Each slice is a triangle which does not share any vertices with other slices. This
/// is the main different between cvf::GeometryUtils::createDisc. This method generates 3x
/// more vertices, but the result is easier to use if you need to color each slice separately.
///
/// The sourceNodes that will be produced by this method:
///
/// The following triangle connectivities will be produced:
///  (0,1,2)  (3,4,5)  (6,7,8) ...
//--------------------------------------------------------------------------------------------------
void createDisc( double radius, size_t numSlices, cvf::GeometryBuilder* builder )
{
    CVF_ASSERT( numSlices >= 4 );
    CVF_ASSERT( builder );

    double da = 2 * cvf::PI_D / numSlices;

    // Find the start point on the circle for each slice
    cvf::Vec3fArray points;
    points.reserve( numSlices );
    for ( size_t i = 0; i < numSlices; i++ )
    {
        // Precompute this one (A = i*da;)
        double sinA = cvf::Math::sin( i * da );
        double cosA = cvf::Math::cos( i * da );

        cvf::Vec3f point = cvf::Vec3f::ZERO;
        point.x()        = static_cast<float>( -sinA * radius );
        point.y()        = static_cast<float>( cosA * radius );
        points.add( point );
    }

    // Create independent vertices per slice
    cvf::Vec3fArray verts;
    verts.reserve( numSlices * 3 );
    for ( size_t i = 0; i < numSlices; i++ )
    {
        verts.add( cvf::Vec3f::ZERO );
        verts.add( points[i] );
        if ( i == numSlices - 1 )
        {
            // Last slice complete the circle.
            verts.add( points[0] );
        }
        else
            verts.add( points[i + 1] );
    }

    cvf::uint baseNodeIdx = builder->addVertices( verts );

    // Build the triangles for each slice
    for ( cvf::uint i = 0; i < static_cast<cvf::uint>( numSlices ); i++ )
    {
        cvf::uint v1 = baseNodeIdx + ( i * 3 ) + 0;
        cvf::uint v2 = baseNodeIdx + ( i * 3 ) + 1;
        cvf::uint v3 = baseNodeIdx + ( i * 3 ) + 2;

        builder->addTriangle( v1, v2, v3 );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivDiskGeometryGenerator::generate( cvf::GeometryBuilder* builder )
{
    createDisc( m_relativeRadius, m_numSlices, builder );
}
