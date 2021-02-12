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

#include "StreamlineGenerator.h"
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
StreamlineGenerator::StreamlineGenerator( std::set<size_t>& wellCells )
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
StreamlineGenerator::~StreamlineGenerator()
{
}

void StreamlineGenerator::setLimits( double flowThreshold, int maxDays, double resolutionInDays )
{
    m_flowThreshold = flowThreshold;
    m_maxDays       = maxDays;
    m_resolution    = resolutionInDays;
}

void StreamlineGenerator::initGenerator( StreamlineDataAccess* dataAccess, std::list<RiaDefines::PhaseType> phases, int density )
{
    m_dataAccess = dataAccess;

    m_phases.clear();
    for ( auto phase : phases )
        m_phases.push_back( phase );

    m_density = density;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void StreamlineGenerator::generateTracer( RigCell                    cell,
                                          double                     direction,
                                          QString                    simWellName,
                                          std::list<RimStreamline*>& outStreamlines )
{
    // calculate the max number of steps based on user settings for length and resolution
    const int maxSteps = (int)( m_maxDays / m_resolution );

    const RigMainGrid* grid = m_dataAccess->grid();

    cvf::Vec3d cellCenter = cell.center();

    RiaDefines::PhaseType dominantPhase = m_phases.front();

    // try to generate a tracer for all faces in the selected cell
    for ( auto faceIdx : _internal_faces )
    {
        // get the face normal for the current face, scale it with the flow, and check that it is still valid
        cvf::Vec3d startDirection = m_dataAccess->cellDirection( cell,
                                                                 m_phases,
                                                                 std::list<cvf::StructGridInterface::FaceType>( faceIdx ),
                                                                 dominantPhase );

        // skip vectors with inf values
        if ( startDirection.isUndefined() ) continue;
        // if too little flow, skip making tracer for this face
        if ( startDirection.length() <= m_flowThreshold ) continue;

        // generate a set of start positions starting in this face
        std::list<cvf::Vec3d> positions;
        generateStartPositions( cell, faceIdx, positions );

        // get the neighbour cell for this face, this is where the tracer should start growing
        RigCell curCell = cell.neighborCell( faceIdx );
        if ( curCell.isInvalid() ) continue;

        for ( const cvf::Vec3d& startPosition : positions )
        {
            if ( startPosition.isUndefined() ) continue;

            cvf::Vec3d curPos  = startPosition;
            int        curStep = 0;

            std::set<size_t> visitedCellsIdx;

            // create the streamline we should store the tracer points in
            RimStreamline* streamLine = new RimStreamline( simWellName );
            streamLine->addTracerPoint( cellCenter, startDirection, dominantPhase );

            // get the current cell bounding box and average direction movement vector
            cvf::BoundingBox bb = curCell.boundingBox();
            cvf::Vec3d       curDirection =
                m_dataAccess->cellDirection( curCell, m_phases, _internal_faces, dominantPhase ) * direction;

            while ( curStep < maxSteps )
            {
                // stop if too little flow
                if ( curDirection.length() < m_flowThreshold ) break;

                // keep track of where we have been to avoid loops
                visitedCellsIdx.insert( curCell.mainGridCellIndex() );

                // is this a well cell, if so, stop growing
                if ( m_wellCells.count( curCell.mainGridCellIndex() ) > 0 ) break;

                // while we stay in the cell, keep moving in the same direction
                bool stop = false;
                while ( bb.contains( curPos ) )
                {
                    streamLine->addTracerPoint( curPos, curDirection, dominantPhase );
                    curPos += curDirection * m_resolution;
                    curStep++;
                    stop = ( curStep >= maxSteps ) || ( curDirection.length() < m_flowThreshold );
                    if ( stop ) break;
                }
                if ( stop ) break;

                // we have exited the cell we were in, find the next cell (should be one of our neighbours)
                RigCell             tmpCell;
                std::vector<size_t> neighbors = curCell.allNeighborMainGridCellIndexes();
                for ( auto cellIdx : neighbors )
                {
                    tmpCell = grid->cell( cellIdx );

                    bb = tmpCell.boundingBox();
                    if ( bb.contains( curPos ) )
                    {
                        curCell = tmpCell;
                        break;
                    }
                }

                // no neighbour found, stop this tracer
                if ( curCell.isInvalid() ) break;

                // have we been here, if so stop?
                if ( visitedCellsIdx.count( curCell.mainGridCellIndex() ) > 0 ) break;

                // update our direction
                curDirection = m_dataAccess->cellDirection( curCell, m_phases, _internal_faces, dominantPhase ) * direction;
            }

            // make sure the streamline points with the flow towards the producer
            if ( direction < 0.0 ) streamLine->reverse();

            outStreamlines.push_back( streamLine );
        }
    }

    return;
}

//--------------------------------------------------------------------------------------------------
/// Generate multiple start posisions for the given cell face, using the user specified tracer density
//--------------------------------------------------------------------------------------------------
void StreamlineGenerator::generateStartPositions( RigCell                            cell,
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
