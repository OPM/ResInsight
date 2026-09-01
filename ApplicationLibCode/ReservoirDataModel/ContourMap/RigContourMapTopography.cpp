/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RigContourMapTopography.h"

#include "RigCell.h"
#include "RigContourMapGrid.h"
#include "RigGridBase.h"
#include "RigHexIntersectionTools.h"
#include "RigMainGrid.h"
#include "Surface/RigSurface.h"
#include "Surface/RigSurfaceResampler.h"

#include "cvfBoundingBox.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigContourMapTopography::RigContourMapTopography( const RigContourMapGrid&        contourMapGrid,
                                                  const RigMainGrid&              mainGrid,
                                                  const cvf::UByteArray*          cellVisibility,
                                                  const std::vector<RigSurface*>& surfaces )
{
    // The contour map grid is far too coarse to follow stair stepped cell geometry. Between two of its
    // vertices the interpolated elevation ramps smoothly while the geometry steps, so a curve drawn on
    // it sinks into the cell it just stepped down from. Sample on a finer lattice over the same extent
    // instead, which is what decides how closely a draped curve can follow the geometry.
    const cvf::Vec2ui mapSize = contourMapGrid.numberOfElementsIJ();

    m_sampleSpacing = contourMapGrid.sampleSpacing() / sm_refinementFactor;
    m_vertexCountIJ = cvf::Vec2ui( mapSize.x() * sm_refinementFactor + 1u, mapSize.y() * sm_refinementFactor + 1u );

    const cvf::Vec2d origin2d = contourMapGrid.origin2d();

    // The rays are cast against the grid of the view the map is shown in, which need not be the grid of
    // the case the contour map was computed from. A ray has to clear that grid at both ends, otherwise
    // it starts inside a cell and only the exit point is found, and cells above it are missed entirely.
    double                 highestZ = 0.0;
    double                 lowestZ  = 0.0;
    const cvf::BoundingBox gridBBox = mainGrid.boundingBox();
    if ( gridBBox.isValid() )
    {
        const double margin = std::max( 1.0, gridBBox.extent().z() * 0.01 );
        highestZ            = gridBBox.max().z() + margin;
        lowestZ             = gridBBox.min().z() - margin;
    }

    // A surface can lie outside the depth range of the case, so each one is given its own ray range
    const std::vector<SurfaceAndZRange> surfacesToSearch = surfacesWithZRange( surfaces );

    const size_t vertexCount = static_cast<size_t>( m_vertexCountIJ.x() ) * m_vertexCountIJ.y();

    m_vertexElevations.resize( vertexCount, std::numeric_limits<double>::infinity() );

#pragma omp parallel for
    for ( int index = 0; index < static_cast<int>( vertexCount ); ++index )
    {
        const unsigned int i = static_cast<unsigned int>( index ) % m_vertexCountIJ.x();
        const unsigned int j = static_cast<unsigned int>( index ) / m_vertexCountIJ.x();

        // Vertex (0,0) of the contour map grid sits on the local origin, so the finer lattice shares it
        cvf::Vec2d domainPos2d( origin2d.x() + i * m_sampleSpacing, origin2d.y() + j * m_sampleSpacing );

        const double cellElevation    = topCellElevationAtDomainPos( domainPos2d, mainGrid, cellVisibility, highestZ, lowestZ );
        const double surfaceElevation = topSurfaceElevationAtDomainPos( domainPos2d, surfacesToSearch );

        // Whatever is on top of what the view is showing
        if ( cellElevation == std::numeric_limits<double>::infinity() )
        {
            m_vertexElevations[index] = surfaceElevation;
        }
        else if ( surfaceElevation == std::numeric_limits<double>::infinity() )
        {
            m_vertexElevations[index] = cellElevation;
        }
        else
        {
            m_vertexElevations[index] = std::max( cellElevation, surfaceElevation );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigContourMapTopography::SurfaceAndZRange> RigContourMapTopography::surfacesWithZRange( const std::vector<RigSurface*>& surfaces )
{
    std::vector<SurfaceAndZRange> surfacesToSearch;

    for ( RigSurface* surface : surfaces )
    {
        if ( !surface || surface->vertices().empty() ) continue;

        SurfaceAndZRange entry;
        entry.surface  = surface;
        entry.highestZ = -std::numeric_limits<double>::max();
        entry.lowestZ  = std::numeric_limits<double>::max();

        for ( const cvf::Vec3d& vertex : surface->vertices() )
        {
            entry.highestZ = std::max( entry.highestZ, vertex.z() );
            entry.lowestZ  = std::min( entry.lowestZ, vertex.z() );
        }

        // The ray has to clear the surface at both ends for the intersection test to find it
        const double margin = std::max( 1.0, ( entry.highestZ - entry.lowestZ ) * 0.01 );
        entry.highestZ += margin;
        entry.lowestZ -= margin;

        surface->ensureIntersectionSearchTreeIsBuilt();

        surfacesToSearch.push_back( entry );
    }

    return surfacesToSearch;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigContourMapTopography::topSurfaceElevationAtDomainPos( const cvf::Vec2d& domainPos2d, const std::vector<SurfaceAndZRange>& surfaces )
{
    double topElevation = std::numeric_limits<double>::infinity();

    for ( const SurfaceAndZRange& entry : surfaces )
    {
        cvf::Vec3d pointAbove( domainPos2d, entry.highestZ );
        cvf::Vec3d pointBelow( domainPos2d, entry.lowestZ );

        cvf::Vec3d intersectionPoint;
        if ( !RigSurfaceResampler::computeIntersectionWithLine( entry.surface, pointAbove, pointBelow, intersectionPoint ) ) continue;

        if ( topElevation == std::numeric_limits<double>::infinity() || intersectionPoint.z() > topElevation )
        {
            topElevation = intersectionPoint.z();
        }
    }

    return topElevation;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigContourMapTopography::topCellElevationAtDomainPos( const cvf::Vec2d&      domainPos2d,
                                                             const RigMainGrid&     mainGrid,
                                                             const cvf::UByteArray* cellVisibility,
                                                             double                 highestZ,
                                                             double                 lowestZ )
{
    if ( highestZ <= lowestZ ) return std::numeric_limits<double>::infinity();

    cvf::Vec3d highestPoint( domainPos2d, highestZ );
    cvf::Vec3d lowestPoint( domainPos2d, lowestZ );

    cvf::BoundingBox rayBBox;
    rayBBox.add( highestPoint );
    rayBBox.add( lowestPoint );

    const std::vector<size_t> candidateCells = mainGrid.findIntersectingCells( rayBBox );

    double topElevation = std::numeric_limits<double>::infinity();

    for ( size_t globalCellIdx : candidateCells )
    {
        const RigCell& cell = mainGrid.cell( globalCellIdx );
        if ( cell.isInvalid() ) continue;

        // The visibility array is the authority on what the view is showing, and already accounts for
        // cell filters, property filters and cells refined by a local grid.
        if ( cellVisibility && globalCellIdx < cellVisibility->size() && !( *cellVisibility )[globalCellIdx] ) continue;

        RigGridBase* localGrid = cell.hostGrid();
        if ( !localGrid ) continue;

        std::array<cvf::Vec3d, 8>        hexCorners = localGrid->cellCornerVertices( cell.gridLocalCellIndex() );
        std::vector<HexIntersectionInfo> intersections;

        if ( RigHexIntersectionTools::lineHexCellIntersection( highestPoint, lowestPoint, hexCorners, 0, &intersections ) )
        {
            for ( const HexIntersectionInfo& intersection : intersections )
            {
                double z = intersection.m_intersectionPoint.z();
                if ( topElevation == std::numeric_limits<double>::infinity() || z > topElevation )
                {
                    topElevation = z;
                }
            }
        }
    }

    return topElevation;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigContourMapTopography::sampleSpacing() const
{
    return m_sampleSpacing;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigContourMapTopography::elevationAtVertex( unsigned int i, unsigned int j ) const
{
    if ( i >= m_vertexCountIJ.x() || j >= m_vertexCountIJ.y() ) return std::numeric_limits<double>::infinity();

    size_t index = static_cast<size_t>( j ) * m_vertexCountIJ.x() + i;
    if ( index >= m_vertexElevations.size() ) return std::numeric_limits<double>::infinity();

    return m_vertexElevations[index];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<double> RigContourMapTopography::elevationAtLocalPos( const cvf::Vec2d& localPos2d ) const
{
    if ( m_sampleSpacing <= 0.0 || m_vertexCountIJ.x() < 2u || m_vertexCountIJ.y() < 2u ) return {};

    // The vertices form a regular lattice with the sample spacing as the step, with vertex (0,0) at
    // the local origin. Interpolate bilinearly between the four vertices surrounding the position.
    double u = localPos2d.x() / m_sampleSpacing;
    double v = localPos2d.y() / m_sampleSpacing;

    const double maxU = static_cast<double>( m_vertexCountIJ.x() - 1u );
    const double maxV = static_cast<double>( m_vertexCountIJ.y() - 1u );

    // The lattice covers the full extent of the map, so a position on the far edge is inside it
    const double tolerance = 1.0e-6;
    if ( u < -tolerance || v < -tolerance || u > maxU + tolerance || v > maxV + tolerance ) return {};

    u = std::clamp( u, 0.0, maxU );
    v = std::clamp( v, 0.0, maxV );

    // A position exactly on the far edge belongs to the last cell, not to a cell past the end
    double uFloor = std::min( std::floor( u ), maxU - 1.0 );
    double vFloor = std::min( std::floor( v ), maxV - 1.0 );

    auto i = static_cast<unsigned int>( uFloor );
    auto j = static_cast<unsigned int>( vFloor );

    double fractionX = u - uFloor;
    double fractionY = v - vFloor;

    std::array<double, 4> cornerElevations = { elevationAtVertex( i, j ),
                                               elevationAtVertex( i + 1u, j ),
                                               elevationAtVertex( i, j + 1u ),
                                               elevationAtVertex( i + 1u, j + 1u ) };

    for ( double elevation : cornerElevations )
    {
        if ( elevation == std::numeric_limits<double>::infinity() ) return {};
    }

    double bottom = cornerElevations[0] * ( 1.0 - fractionX ) + cornerElevations[1] * fractionX;
    double top    = cornerElevations[2] * ( 1.0 - fractionX ) + cornerElevations[3] * fractionX;

    return bottom * ( 1.0 - fractionY ) + top * fractionY;
}
