/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RigCell.h"
#include "RigCellGeometryTools.h"
#include "RigMainGrid.h"
#include "cvfPlane.h"
#include "cvfRay.h"

#include <cmath>

static size_t undefinedCornersArray[8] = { cvf::UNDEFINED_SIZE_T,
                                           cvf::UNDEFINED_SIZE_T,
                                           cvf::UNDEFINED_SIZE_T,
                                           cvf::UNDEFINED_SIZE_T,
                                           cvf::UNDEFINED_SIZE_T,
                                           cvf::UNDEFINED_SIZE_T,
                                           cvf::UNDEFINED_SIZE_T,
                                           cvf::UNDEFINED_SIZE_T };
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigCell::RigCell()
    : m_gridLocalCellIndex( cvf::UNDEFINED_SIZE_T )
    , m_hostGrid( nullptr )
    , m_subGrid( nullptr )
    , m_parentCellIndex( cvf::UNDEFINED_SIZE_T )
    , m_mainGridCellIndex( cvf::UNDEFINED_SIZE_T )
    , m_coarseningBoxIndex( cvf::UNDEFINED_SIZE_T )
    , m_isInvalid( false )
{
    memcpy( m_cornerIndices.data(), undefinedCornersArray, 8 * sizeof( size_t ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigCell::~RigCell()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RigCell::center() const
{
    cvf::Vec3d avg( cvf::Vec3d::ZERO );

    size_t i;
    for ( i = 0; i < 8; i++ )
    {
        avg += m_hostGrid->mainGrid()->nodes()[m_cornerIndices[i]];
    }

    avg /= 8.0;

    return avg;
}

//--------------------------------------------------------------------------------------------------
/// Get the coordinates of the 4 corners of the given face
//--------------------------------------------------------------------------------------------------
std::array<cvf::Vec3d, 4> RigCell::faceCorners( cvf::StructGridInterface::FaceType face ) const
{
    std::array<cvf::Vec3d, 4> corners;
    cvf::ubyte                faceVertexIndices[4];
    cvf::StructGridInterface::cellFaceVertexIndices( face, faceVertexIndices );

    const std::vector<cvf::Vec3d>& nodeCoords = m_hostGrid->mainGrid()->nodes();

    for ( size_t i = 0; i < 4; i++ )
    {
        corners[i] = nodeCoords[m_cornerIndices[faceVertexIndices[i]]];
    }

    return corners;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool isNear( const cvf::Vec3d& p1, const cvf::Vec3d& p2, double tolerance )
{
    if ( cvf::Math::abs( p1[0] - p2[0] ) < tolerance && cvf::Math::abs( p1[1] - p2[1] ) < tolerance &&
         cvf::Math::abs( p1[2] - p2[2] ) < tolerance )
    {
        return true;
    }
    else
    {
        return false;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigCell::isLongPyramidCell( double maxHeightFactor, double nodeNearTolerance ) const
{
    const std::vector<cvf::Vec3d>& nodes = m_hostGrid->mainGrid()->nodes();

    auto isDiagonalRatioBelowThreshold = []( const cvf::Vec3d& v1, const cvf::Vec3d& v2 ) -> bool {
        auto diagonal = v2 - v1;

        std::vector<double> components = { std::abs( diagonal.x() ), std::abs( diagonal.y() ), std::abs( diagonal.z() ) };
        std::sort( components.begin(), components.end() );

        if ( components.back() > 1e-3 )
        {
            const double ratioBetweenShortestAndLongestComponent = ( components.front() / components.back() );

            // CO2 grid models use very thin cells (large dx/dy compared to dz)
            // Consider if this threshold is valid for these grid models
            static const double ratioThreshold = 1e-3;
            if ( std::fabs( ratioBetweenShortestAndLongestComponent ) < ratioThreshold ) return true;
        }

        return false;
    };

    // Compute the ratio between smallest and largest component of the diagonal vector of the cell.
    // If the ratio is less than the threshold, the cell is considered to be a long pyramid cell
    if ( isDiagonalRatioBelowThreshold( nodes[m_cornerIndices[0]], nodes[m_cornerIndices[6]] ) ) return true;

    cvf::ubyte   faceVertexIndices[4];
    const double squaredMaxHeightFactor = maxHeightFactor * maxHeightFactor;
    for ( int face = 0; face < 6; ++face )
    {
        cvf::StructGridInterface::cellFaceVertexIndices( static_cast<cvf::StructGridInterface::FaceType>( face ),
                                                         faceVertexIndices );
        int zeroLengthEdgeCount = 0;

        const cvf::Vec3d& c0 = nodes[m_cornerIndices[faceVertexIndices[0]]];
        const cvf::Vec3d& c1 = nodes[m_cornerIndices[faceVertexIndices[1]]];
        const cvf::Vec3d& c2 = nodes[m_cornerIndices[faceVertexIndices[2]]];
        const cvf::Vec3d& c3 = nodes[m_cornerIndices[faceVertexIndices[3]]];

        if ( isNear( c0, c1, nodeNearTolerance ) )
        {
            ++zeroLengthEdgeCount;
        }
        if ( isNear( c1, c2, nodeNearTolerance ) )
        {
            ++zeroLengthEdgeCount;
        }
        if ( isNear( c2, c3, nodeNearTolerance ) )
        {
            ++zeroLengthEdgeCount;
        }

        if ( zeroLengthEdgeCount == 3 )
        {
            return true;
        }

        // Check the ratio of the length of opposite edges.
        // both ratios have to be above threshold to detect a pyramid-ish cell
        // Only test this if we have all nonzero edge lengths.
        else if ( zeroLengthEdgeCount == 0 ) // If the four first faces are ok, the two last must be as well
        {
            double e0SquareLength = ( c1 - c0 ).lengthSquared();
            double e2SquareLength = ( c3 - c2 ).lengthSquared();
            if ( e0SquareLength / e2SquareLength > squaredMaxHeightFactor ||
                 e2SquareLength / e0SquareLength > squaredMaxHeightFactor )
            {
                double e1SquareLength = ( c2 - c1 ).lengthSquared();
                double e3SquareLength = ( c0 - c3 ).lengthSquared();

                if ( e1SquareLength / e3SquareLength > squaredMaxHeightFactor ||
                     e3SquareLength / e1SquareLength > squaredMaxHeightFactor )
                {
                    return true;
                }
            }
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigCell::isCollapsedCell( double nodeNearTolerance ) const
{
    const std::vector<cvf::Vec3d>& nodes = m_hostGrid->mainGrid()->nodes();

    cvf::ubyte faceVertexIndices[4];
    cvf::ubyte oppFaceVertexIndices[4];

    int face;
    for ( face = 0; face < 6; face += 2 )
    {
        cvf::StructGridInterface::cellFaceVertexIndices( static_cast<cvf::StructGridInterface::FaceType>( face ),
                                                         faceVertexIndices );
        cvf::StructGridInterface::cellFaceVertexIndices( cvf::StructGridInterface::oppositeFace(
                                                             static_cast<cvf::StructGridInterface::FaceType>( face ) ),
                                                         oppFaceVertexIndices );

        cvf::Vec3d c0 = nodes[m_cornerIndices[faceVertexIndices[0]]];
        cvf::Vec3d c1 = nodes[m_cornerIndices[faceVertexIndices[1]]];
        cvf::Vec3d c2 = nodes[m_cornerIndices[faceVertexIndices[2]]];
        cvf::Vec3d c3 = nodes[m_cornerIndices[faceVertexIndices[3]]];

        cvf::Vec3d oc0 = nodes[m_cornerIndices[oppFaceVertexIndices[0]]];
        cvf::Vec3d oc1 = nodes[m_cornerIndices[oppFaceVertexIndices[1]]];
        cvf::Vec3d oc2 = nodes[m_cornerIndices[oppFaceVertexIndices[2]]];
        cvf::Vec3d oc3 = nodes[m_cornerIndices[oppFaceVertexIndices[3]]];

        int zeroLengthEdgeCount = 0;
        if ( isNear( c0, oc0, nodeNearTolerance ) )
        {
            ++zeroLengthEdgeCount;
        }
        if ( isNear( c1, oc3, nodeNearTolerance ) )
        {
            ++zeroLengthEdgeCount;
        }
        if ( isNear( c2, oc2, nodeNearTolerance ) )
        {
            ++zeroLengthEdgeCount;
        }
        if ( isNear( c3, oc1, nodeNearTolerance ) )
        {
            ++zeroLengthEdgeCount;
        }

        if ( zeroLengthEdgeCount >= 4 )
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RigCell::faceCenter( cvf::StructGridInterface::FaceType face ) const
{
    cvf::Vec3d avg( cvf::Vec3d::ZERO );

    cvf::ubyte faceVertexIndices[4];
    cvf::StructGridInterface::cellFaceVertexIndices( face, faceVertexIndices );

    const std::vector<cvf::Vec3d>& nodeCoords = m_hostGrid->mainGrid()->nodes();

    size_t i;
    for ( i = 0; i < 4; i++ )
    {
        avg += nodeCoords[m_cornerIndices[faceVertexIndices[i]]];
    }

    avg /= 4.0;

    return avg;
}

//--------------------------------------------------------------------------------------------------
/// Returns an area vector for the cell face. The direction is the face normal, and the length is
/// equal to the face area (projected to the plane represented by the diagonal in case of warp)
/// The components of this area vector are equal to the area of the face projection onto
/// the corresponding plane.
/// See http://geomalgorithms.com/a01-_area.html
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RigCell::faceNormalWithAreaLength( cvf::StructGridInterface::FaceType face ) const
{
    cvf::ubyte faceVertexIndices[4];
    cvf::StructGridInterface::cellFaceVertexIndices( face, faceVertexIndices );
    const std::vector<cvf::Vec3d>& nodeCoords = m_hostGrid->mainGrid()->nodes();

    return 0.5 * ( nodeCoords[m_cornerIndices[faceVertexIndices[2]]] - nodeCoords[m_cornerIndices[faceVertexIndices[0]]] ) ^
           ( nodeCoords[m_cornerIndices[faceVertexIndices[3]]] - nodeCoords[m_cornerIndices[faceVertexIndices[1]]] );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigCell::volume() const
{
    const std::vector<cvf::Vec3d>& nodeCoords = m_hostGrid->mainGrid()->nodes();

    std::array<cvf::Vec3d, 8> hexCorners;
    for ( size_t i = 0; i < 8; ++i )
    {
        hexCorners[i] = nodeCoords.at( m_cornerIndices[i] );
    }
    return RigCellGeometryTools::calculateCellVolume( hexCorners );
}

//--------------------------------------------------------------------------------------------------
/// Find the intersection between the cell and the ray. The point closest to the ray origin is returned
/// in \a intersectionPoint, while the return value is the total number of intersections with the 24 triangles
/// the cell is interpreted as.
/// If no intersection is found, the intersection point is untouched.
//--------------------------------------------------------------------------------------------------
int RigCell::firstIntersectionPoint( const cvf::Ray& ray, cvf::Vec3d* intersectionPoint ) const
{
    CVF_ASSERT( intersectionPoint != nullptr );

    cvf::ubyte                     faceVertexIndices[4];
    int                            face;
    const std::vector<cvf::Vec3d>& nodes = m_hostGrid->mainGrid()->nodes();

    cvf::Vec3d firstIntersection( cvf::Vec3d::ZERO );
    double     minLsq            = HUGE_VAL;
    int        intersectionCount = 0;

    for ( face = 0; face < 6; ++face )
    {
        cvf::StructGridInterface::cellFaceVertexIndices( static_cast<cvf::StructGridInterface::FaceType>( face ),
                                                         faceVertexIndices );
        cvf::Vec3d intersection;
        cvf::Vec3d faceCenter = this->faceCenter( static_cast<cvf::StructGridInterface::FaceType>( face ) );

        for ( size_t i = 0; i < 4; ++i )
        {
            size_t next = i < 3 ? i + 1 : 0;
            if ( ray.triangleIntersect( nodes[m_cornerIndices[faceVertexIndices[i]]],
                                        nodes[m_cornerIndices[faceVertexIndices[next]]],
                                        faceCenter,
                                        &intersection ) )
            {
                intersectionCount++;
                double lsq = ( intersection - ray.origin() ).lengthSquared();
                if ( lsq < minLsq )
                {
                    firstIntersection = intersection;
                    minLsq            = lsq;
                }
            }
        }
    }

    if ( intersectionCount > 0 )
    {
        *intersectionPoint = firstIntersection;
    }

    return intersectionCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCell::faceIndices( cvf::StructGridInterface::FaceType face, std::array<size_t, 4>* indices ) const
{
    cvf::ubyte faceVertexIndices[4];
    cvf::StructGridInterface::cellFaceVertexIndices( face, faceVertexIndices );

    ( *indices )[0] = m_cornerIndices[faceVertexIndices[0]];
    ( *indices )[1] = m_cornerIndices[faceVertexIndices[1]];
    ( *indices )[2] = m_cornerIndices[faceVertexIndices[2]];
    ( *indices )[3] = m_cornerIndices[faceVertexIndices[3]];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox RigCell::boundingBox() const
{
    cvf::BoundingBox          bb;
    std::array<cvf::Vec3d, 8> hexCorners;

    if ( m_hostGrid )
    {
        m_hostGrid->cellCornerVertices( m_gridLocalCellIndex, hexCorners.data() );
        for ( const auto& corner : hexCorners )
        {
            bb.add( corner );
        }
    }
    return bb;
}

//--------------------------------------------------------------------------------------------------
/// Return the main grid neighbor cell of the given face
//--------------------------------------------------------------------------------------------------
RigCell RigCell::neighborCell( cvf::StructGridInterface::FaceType face ) const
{
    size_t i, j, k;

    m_hostGrid->ijkFromCellIndexUnguarded( mainGridCellIndex(), &i, &j, &k );

    size_t neighborIdx;
    if ( m_hostGrid->cellIJKNeighbor( i, j, k, face, &neighborIdx ) )
    {
        return m_hostGrid->cell( neighborIdx );
    }

    RigCell retcell;
    retcell.setInvalid( true );
    return retcell;
}

//--------------------------------------------------------------------------------------------------
/// Find and return the main grid cell index of all up to 26 neighbor cells to the given cell
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RigCell::allNeighborMainGridCellIndexes() const
{
    std::vector<size_t> neighbors;

    size_t ni, nj, nk;

    m_hostGrid->ijkFromCellIndexUnguarded( mainGridCellIndex(), &ni, &nj, &nk );

    for ( size_t i = ni - 1; i <= ni + 1; i++ )
    {
        for ( size_t j = nj - 1; j <= nj + 1; j++ )
        {
            for ( size_t k = nk - 1; k <= nk + 1; k++ )
            {
                if ( m_hostGrid->isCellValid( i, j, k ) )
                {
                    size_t cellIndex = m_hostGrid->cellIndexFromIJK( i, j, k );
                    if ( ( ni == i ) && ( nj == j ) && ( nk == k ) ) continue;
                    neighbors.push_back( cellIndex );
                }
            }
        }
    }
    return neighbors;
}
