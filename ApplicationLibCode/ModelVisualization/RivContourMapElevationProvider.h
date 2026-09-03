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

#include "cvfVector2.h"

#include <memory>
#include <optional>

class RigContourMapTopography;

//==================================================================================================
///
/// Supplies the domain z coordinate used when contour map geometry is placed in a scene.
///
/// Contour map geometry is generated in a flat, contour map local coordinate space where z is
/// always zero. Consumers normally lift it to RigContourMapGrid::origin3d(), which places the map
/// at the minimum z of the expanded bounding box. In a 3d view the map must be placed elsewhere,
/// and the contour lines may be draped on the visible grid geometry. This interface is the single
/// point where that decision is made.
///
//==================================================================================================
class RivContourMapElevationProvider
{
public:
    virtual ~RivContourMapElevationProvider();

    // localPos2d is relative to RigContourMapGrid::origin2d(). Returns the domain z to use, or
    // nothing when no elevation is defined at the given position.
    virtual std::optional<double> domainElevation( const cvf::Vec2d& localPos2d ) const = 0;

    // How closely a polyline has to be sampled for the elevation to be followed faithfully. Contour
    // polygons are simplified, so a single segment can span a long distance and would otherwise cut
    // straight through everything between its end points. Zero means the elevation does not vary, so
    // the end points are enough.
    virtual double resamplingDistance() const;
};

//==================================================================================================
///
/// Places the whole contour map on a single horizontal plane.
///
//==================================================================================================
class RivContourMapFlatElevation : public RivContourMapElevationProvider
{
public:
    explicit RivContourMapFlatElevation( double domainElevation );
    ~RivContourMapFlatElevation() override;

    std::optional<double> domainElevation( const cvf::Vec2d& localPos2d ) const override;

private:
    double m_domainElevation;
};

//==================================================================================================
///
/// Follows the top of the visible grid geometry, lifted by a constant offset. Returns nothing where
/// no visible geometry is found, which makes the consumer drop the affected geometry.
///
//==================================================================================================
class RivContourMapTopographyElevation : public RivContourMapElevationProvider
{
public:
    RivContourMapTopographyElevation( std::shared_ptr<const RigContourMapTopography> topography, double offset );
    ~RivContourMapTopographyElevation() override;

    std::optional<double> domainElevation( const cvf::Vec2d& localPos2d ) const override;
    double                resamplingDistance() const override;

private:
    std::shared_ptr<const RigContourMapTopography> m_topography;
    double                                         m_offset;
};
