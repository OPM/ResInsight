/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021     Equinor ASA
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

#include "StreamlineGeneratorBase.h"
#include "StreamlineDataAccess.h"

#include "RigCell.h"
#include "RigMainGrid.h"
#include "RimStreamline.h"

const std::list<cvf::StructGridInterface::FaceType> _internal_faces = { cvf::StructGridInterface::FaceType::POS_I,
                                                                        cvf::StructGridInterface::FaceType::NEG_I,
                                                                        cvf::StructGridInterface::FaceType::POS_J,
                                                                        cvf::StructGridInterface::FaceType::NEG_J,
                                                                        cvf::StructGridInterface::FaceType::POS_K,
                                                                        cvf::StructGridInterface::FaceType::NEG_K };

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
StreamlineGeneratorBase::StreamlineGeneratorBase( std::set<size_t>& wellCells )
    : m_density( 0 )
    , m_maxDays( 10000 )
    , m_flowThreshold( 0.0 )
    , m_resolution( 10.0 )
    , m_wellCells( wellCells )
    , m_dataAccess( nullptr )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
StreamlineGeneratorBase::~StreamlineGeneratorBase()
{
}

void StreamlineGeneratorBase::setLimits( double flowThreshold, int maxDays, double resolutionInDays )
{
    m_flowThreshold = flowThreshold;
    m_maxDays       = maxDays;
    m_resolution    = resolutionInDays;
}

void StreamlineGeneratorBase::initGenerator( StreamlineDataAccess*            dataAccess,
                                             std::list<RiaDefines::PhaseType> phases,
                                             int                              density )
{
    m_dataAccess = dataAccess;

    m_phases.clear();
    for ( auto phase : phases )
        m_phases.push_back( phase );

    m_density = density;
}

//--------------------------------------------------------------------------------------------------
/// Generate multiple start posisions for the given cell face, using the user specified tracer density
//--------------------------------------------------------------------------------------------------
void StreamlineGeneratorBase::generateStartPositions( RigCell                            cell,
                                                      cvf::StructGridInterface::FaceType faceIdx,
                                                      std::list<cvf::Vec3d>&             positions )
{
    cvf::Vec3d center = cell.faceCenter( faceIdx );

    std::array<cvf::Vec3d, 4> corners;
    cell.faceCorners( faceIdx, &corners );

    positions.push_back( center );

    // if density is zero, just return face center
    if ( m_density == 0 ) return;

    for ( const auto& pos : corners )
        positions.push_back( pos );

    // if density is 1, return face center and corners
    if ( m_density == 1 ) return;

    // if density is 2, add some more points in-between
    for ( size_t cornerIdx = 0; cornerIdx < 4; cornerIdx++ )
    {
        cvf::Vec3d xa = corners[cornerIdx] - center;
        positions.push_back( center + xa / 2.0 );
        cvf::Vec3d xab  = corners[( cornerIdx + 1 ) % 4] - corners[cornerIdx];
        cvf::Vec3d ab_2 = corners[cornerIdx] + xab / 2.0;
        positions.push_back( ab_2 );
        cvf::Vec3d xab_2 = ab_2 - center;
        positions.push_back( center + xab_2 / 2.0 );
    }
}
