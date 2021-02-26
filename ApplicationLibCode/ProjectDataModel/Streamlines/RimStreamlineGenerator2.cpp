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

#include "RimStreamlineGenerator2.h"

#include "RigCell.h"
#include "RigMainGrid.h"

#include "RimStreamline.h"
#include "RimStreamlineDataAccess.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStreamlineGenerator2::RimStreamlineGenerator2( std::set<size_t>& wellCells )
    : RimStreamlineGeneratorBase( wellCells )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStreamlineGenerator2::~RimStreamlineGenerator2()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStreamlineGenerator2::generateTracer( RigCell                    cell,
                                              double                     direction,
                                              QString                    simWellName,
                                              std::list<RimStreamline*>& outStreamlines )
{
    RiaDefines::PhaseType dominantPhase = m_phases.front();

    const size_t cellIdx = cell.gridLocalCellIndex();

    // try to generate a tracer for all faces in the selected cell with positive flow
    for ( auto faceIdx : m_allFaces )
    {
        double flowVelocity = m_dataAccess->combinedFaceValueByArea( cell, faceIdx, m_phases, dominantPhase ) * direction;
        if ( std::abs( flowVelocity ) > m_flowThreshold )
        {
            m_seeds.push_back( std::make_pair( cellIdx, faceIdx ) );
        }
    }

    while ( m_seeds.size() > 0 )
    {
        const size_t                             cellIdx = m_seeds.front().first;
        const cvf::StructGridInterface::FaceType faceIdx = m_seeds.front().second;

        RimStreamline* streamline = new RimStreamline( simWellName );

        growStreamline( streamline, cellIdx, faceIdx, direction );

        if ( direction < 0.0 ) streamline->reverse();

        outStreamlines.push_back( streamline );

        m_seeds.pop_front();
    }

    return;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStreamlineGenerator2::growStreamline( RimStreamline*                     streamline,
                                              size_t                             cellIdx,
                                              cvf::StructGridInterface::FaceType faceIdx,
                                              double                             direction )
{
    // get the cell
    RigCell cell = m_dataAccess->grid()->cell( cellIdx );

    // get rate
    RiaDefines::PhaseType dominantPhaseOut;
    double flowVelocity = std::abs( m_dataAccess->combinedFaceValueByArea( cell, faceIdx, m_phases, dominantPhaseOut ) );

    while ( flowVelocity >= m_flowThreshold )
    {
        // grow from given face center to cell center, exiting if we reach the max length
        if ( !growStreamlineFromTo( streamline, cell.faceCenter( faceIdx ), cell.center(), flowVelocity, dominantPhaseOut ) )
            break;

        // move to next cell
        RigCell neighbor = cell.neighborCell( faceIdx );
        if ( neighbor.isInvalid() ) break;

        cvf::StructGridInterface::FaceType neighborFaceIdx = cvf::StructGridInterface::oppositeFace( faceIdx );

        // get rate
        flowVelocity =
            std::abs( m_dataAccess->combinedFaceValueByArea( neighbor, neighborFaceIdx, m_phases, dominantPhaseOut ) );

        // grow from face center to cell center, exiting if we reach the max point limit
        if ( !growStreamlineFromTo( streamline,
                                    neighbor.faceCenter( neighborFaceIdx ),
                                    neighbor.center(),
                                    flowVelocity,
                                    dominantPhaseOut ) )
            break;

        // have we been here?
        if ( m_visitedCells.count( neighbor.gridLocalCellIndex() ) > 0 ) break;

        // is this a well?
        if ( m_wellCells.count( neighbor.gridLocalCellIndex() ) > 0 ) break;

        m_visitedCells.insert( neighbor.gridLocalCellIndex() );

        // find the face with max flow where we should exit the cell
        cvf::StructGridInterface::FaceType exitFace = cvf::StructGridInterface::FaceType::NO_FACE;
        double                             maxRate  = 0.0;

        std::map<cvf::StructGridInterface::FaceType, double> rateMap;

        for ( auto face : m_allFaces )
        {
            RiaDefines::PhaseType dummy;
            if ( face == neighborFaceIdx ) continue;
            double faceRate = m_dataAccess->combinedFaceValueByArea( neighbor, face, m_phases, dummy ) * direction;

            if ( ( ( direction < 0.0 ) && ( faceRate < maxRate ) ) || ( ( direction > 0.0 ) && ( faceRate > maxRate ) ) )
            {
                exitFace = face;
                maxRate  = faceRate;
            }

            if ( face % 2 != 0 )
                rateMap[face] = -1.0 * faceRate * direction;
            else
                rateMap[face] = faceRate * direction;
        }

        flowVelocity = std::abs( maxRate );
        faceIdx      = exitFace;
        cell         = neighbor;

        // add seeds for other faces with flow > threshold
        for ( auto& kvp : rateMap )
        {
            if ( kvp.first == exitFace ) continue;
            if ( std::abs( kvp.second ) < m_flowThreshold ) continue;
            m_seeds.push_back( std::make_pair( neighbor.gridLocalCellIndex(), kvp.first ) );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimStreamlineGenerator2::growStreamlineFromTo( RimStreamline*        streamline,
                                                    cvf::Vec3d            startPos,
                                                    cvf::Vec3d            endPos,
                                                    double                flowVelocity,
                                                    RiaDefines::PhaseType dominantPhase )
{
    double totDistance = endPos.pointDistance( startPos );

    if ( totDistance < 0.1 ) return true;
    if ( flowVelocity < m_flowThreshold ) return false;

    cvf::Vec3d direction = endPos - startPos;
    direction.normalize();
    direction *= flowVelocity;

    int nSteps = (int)std::round( ( totDistance / flowVelocity ) / m_resolution );

    cvf::Vec3d curpos = startPos;

    for ( int i = 0; i < nSteps; i++ )
    {
        streamline->addTracerPoint( curpos, direction, dominantPhase );
        curpos = startPos + direction;

        if ( streamline->size() > m_maxPoints ) return false;
    }

    return true;
}
