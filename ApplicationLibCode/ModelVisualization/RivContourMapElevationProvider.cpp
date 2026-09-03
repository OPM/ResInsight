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

#include "RivContourMapElevationProvider.h"

#include "ContourMap/RigContourMapTopography.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivContourMapElevationProvider::~RivContourMapElevationProvider()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RivContourMapElevationProvider::resamplingDistance() const
{
    return 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivContourMapFlatElevation::RivContourMapFlatElevation( double domainElevation )
    : m_domainElevation( domainElevation )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivContourMapFlatElevation::~RivContourMapFlatElevation()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<double> RivContourMapFlatElevation::domainElevation( const cvf::Vec2d& localPos2d ) const
{
    return m_domainElevation;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivContourMapTopographyElevation::RivContourMapTopographyElevation( std::shared_ptr<const RigContourMapTopography> topography, double offset )
    : m_topography( topography )
    , m_offset( offset )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivContourMapTopographyElevation::~RivContourMapTopographyElevation()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<double> RivContourMapTopographyElevation::domainElevation( const cvf::Vec2d& localPos2d ) const
{
    if ( !m_topography ) return {};

    auto elevation = m_topography->elevationAtLocalPos( localPos2d );
    if ( !elevation ) return {};

    return *elevation + m_offset;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RivContourMapTopographyElevation::resamplingDistance() const
{
    if ( !m_topography ) return 0.0;

    // Several samples per raster cell, so a polyline picks up the shape of the geometry between the
    // raster vertices instead of only at them
    const double samplesPerRasterCell = 8.0;

    return m_topography->sampleSpacing() / samplesPerRasterCell;
}
