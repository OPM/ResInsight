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

#pragma once

#include "cvfArray.h"
#include "cvfVector2.h"

#include <optional>
#include <vector>

class RigContourMapGrid;
class RigMainGrid;
class RigSurface;

//==================================================================================================
///
/// Elevation of the top of the visible geometry, sampled on a raster over a contour map grid.
///
/// Used to drape contour map geometry on what the view is showing. A vertical ray is dropped at
/// every raster vertex and the highest intersection with a visible cell or surface is kept, giving
/// a raster that is cheap to interpolate. Positions where nothing was hit stay undefined, so the
/// consumer can leave a gap rather than draw a line hanging in space.
///
/// The raster is deliberately finer than the contour map grid. Cell geometry is stair stepped, and
/// the raster spacing is what limits how closely a draped curve can follow it: between two raster
/// vertices the elevation is interpolated smoothly while the geometry steps, leaving the curve
/// buried in the cell it stepped down from. Sampling the curve itself more densely does not help,
/// since that only samples the same smoothed raster more often.
///
//==================================================================================================
class RigContourMapTopography
{
public:
    // cellVisibility is indexed by global reservoir cell index and may be null, in which case all
    // cells are considered visible. Both the cells and the surfaces are optional, a view may be
    // showing only one of them.
    RigContourMapTopography( const RigContourMapGrid&        contourMapGrid,
                             const RigMainGrid&              mainGrid,
                             const cvf::UByteArray*          cellVisibility,
                             const std::vector<RigSurface*>& surfaces );

    // localPos2d is relative to RigContourMapGrid::origin2d(). Returns nothing when the position is
    // outside the map, or when any of the vertices used for the interpolation is undefined.
    std::optional<double> elevationAtLocalPos( const cvf::Vec2d& localPos2d ) const;

    // Spacing of the underlying raster, finer than the contour map grid. Elevations vary at this scale,
    // so it is also the scale a curve has to be sampled at to follow them.
    double sampleSpacing() const;

private:
    struct SurfaceAndZRange
    {
        RigSurface* surface  = nullptr;
        double      highestZ = 0.0;
        double      lowestZ  = 0.0;
    };

    static double topCellElevationAtDomainPos( const cvf::Vec2d&      domainPos2d,
                                               const RigMainGrid&     mainGrid,
                                               const cvf::UByteArray* cellVisibility,
                                               double                 highestZ,
                                               double                 lowestZ );

    static double topSurfaceElevationAtDomainPos( const cvf::Vec2d& domainPos2d, const std::vector<SurfaceAndZRange>& surfaces );

    static std::vector<SurfaceAndZRange> surfacesWithZRange( const std::vector<RigSurface*>& surfaces );

    double elevationAtVertex( unsigned int i, unsigned int j ) const;

private:
    // How much finer than the contour map grid the raster is sampled. Cell geometry is stair stepped,
    // so following it closely needs several samples across each contour map cell. Costs this squared in
    // rays, which is why it is not larger.
    static constexpr unsigned int sm_refinementFactor = 4u;

private:
    std::vector<double> m_vertexElevations; // infinity marks an undefined vertex
    cvf::Vec2ui         m_vertexCountIJ;
    double              m_sampleSpacing = 0.0;
};
