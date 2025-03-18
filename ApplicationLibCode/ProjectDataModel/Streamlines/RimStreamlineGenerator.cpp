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

#include "RimStreamlineGenerator.h"

#include "RigCell.h"
#include "RigMainGrid.h"
#include "RigNNCData.h"

#include "RimStreamline.h"
#include "RimStreamlineDataAccess.h"

#include <cmath>

//--------------------------------------------------------------------------------------------------
/// Helper class for prioritizing streamline seed points
//--------------------------------------------------------------------------------------------------
class StreamlineSeedPoint
{
public:
    StreamlineSeedPoint( double rate, size_t cellIdx, RimStreamlineGenerator::CellFaceType faceIdx )
        : m_rate( rate )
        , m_cellIdx( cellIdx )
        , m_faceIdx( faceIdx ){};
    ~StreamlineSeedPoint(){};

    bool operator<( const StreamlineSeedPoint& other ) const { return rate() < other.rate(); };

    double                               rate() const { return m_rate; };
    size_t                               cellIdx() const { return m_cellIdx; };
    RimStreamlineGenerator::CellFaceType face() const { return m_faceIdx; };

private:
    double                               m_rate;
    size_t                               m_cellIdx;
    RimStreamlineGenerator::CellFaceType m_faceIdx;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStreamlineGenerator::RimStreamlineGenerator( std::set<size_t>& wellCells )
    : RimStreamlineGeneratorBase( wellCells )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStreamlineGenerator::~RimStreamlineGenerator()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStreamlineGenerator::generateTracer( RigCell cell, double direction, QString simWellName, std::list<RimStreamline*>& outStreamlines )
{
    RiaDefines::PhaseType dominantPhase = m_phases.front();

    const size_t cellIdx = cell.gridLocalCellIndex();

    // try to generate a tracer for all faces in the selected cell with positive flow (or negative flow if we are
    // backtracking from a producer)
    for ( auto faceIdx : m_allFaces )
    {
        double rate = m_dataAccess->combinedFaceRate( cell, faceIdx, m_phases, direction, dominantPhase ) * direction;
        if ( rate > m_flowThreshold )
        {
            m_seeds.push( StreamlineSeedPoint( rate, cellIdx, faceIdx ) );
        }
    }

    auto nncs = nncCandidates( cell.mainGridCellIndex() );

    while ( !m_seeds.empty() )
    {
        auto&                                    seed    = m_seeds.top();
        const size_t                             cellIdx = seed.cellIdx();
        const cvf::StructGridInterface::FaceType faceIdx = seed.face();
        m_seeds.pop();

        RimStreamline* streamline = new RimStreamline( simWellName );

        growStreamline( streamline, cellIdx, faceIdx, direction );

        if ( direction < 0.0 ) streamline->reverse();

        if ( streamline->tracer().totalDistance() >= m_minLength )
            outStreamlines.push_back( streamline );
        else
            delete streamline;
    }

    return;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStreamlineGenerator::growStreamline( RimStreamline* streamline, size_t cellIdx, CellFaceType faceIdx, double direction )
{
    // get the cell
    RigCell cell = m_dataAccess->grid()->cell( cellIdx );

    // get rate
    RiaDefines::PhaseType dominantPhaseOut;

    double rate = m_dataAccess->combinedFaceRate( cell, faceIdx, m_phases, direction, dominantPhaseOut );

    // if we go backwards from a producer, the rate needs to be flipped
    rate *= direction;

    // grow from start cell center to face center, exiting if we reach the max length
    if ( !growStreamlineFromTo( streamline, cell.center(), cell.faceCenter( faceIdx ), rate, dominantPhaseOut ) ) return;

    while ( rate >= m_flowThreshold )
    {
        // find next cell and entry face
        cell = cell.neighborCell( faceIdx );
        if ( cell.isInvalid() ) break;
        faceIdx = cvf::StructGridInterface::oppositeFace( faceIdx );

        // grow from given face center to cell center, exiting if we reach the max length
        if ( !growStreamlineFromTo( streamline, cell.faceCenter( faceIdx ), cell.center(), rate, dominantPhaseOut ) ) break;

        const size_t cellIdx = cell.gridLocalCellIndex();

        if ( m_visitedCells.count( cellIdx ) > 0 ) break;
        if ( m_wellCells.count( cellIdx ) > 0 ) break;

        m_visitedCells.insert( cellIdx );

        // find the face with max flow where we should exit the cell
        CellFaceType                   exitFace = cvf::StructGridInterface::FaceType::NO_FACE;
        std::map<CellFaceType, double> rateMap;

        double maxRate = 0.0;

        for ( auto face : m_allFaces )
        {
            // skip the entry face
            if ( face == faceIdx ) continue;

            RiaDefines::PhaseType tempDominantFace;
            double                faceRate = m_dataAccess->combinedFaceRate( cell, face, m_phases, direction, tempDominantFace );

            // if we go backwards from a producer, the rate needs to be flipped
            faceRate *= direction;

            if ( faceRate > maxRate )
            {
                exitFace         = face;
                maxRate          = faceRate;
                dominantPhaseOut = tempDominantFace;
            }

            rateMap[face] = faceRate;
        }

        // did we find an exit?
        if ( exitFace == cvf::StructGridInterface::FaceType::NO_FACE ) break;

        // add seeds for other faces with flow > threshold
        for ( auto& kvp : rateMap )
        {
            if ( kvp.first == exitFace ) continue;

            if ( kvp.second >= m_flowThreshold ) m_seeds.push( StreamlineSeedPoint( kvp.second, cell.gridLocalCellIndex(), kvp.first ) );
        }

        rate = maxRate;

        // grow from cell center to exit face center, stopping if we reach the max point limit
        if ( !growStreamlineFromTo( streamline, cell.center(), cell.faceCenter( exitFace ), rate, dominantPhaseOut ) ) break;

        faceIdx = exitFace;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimStreamlineGenerator::growStreamlineFromTo( RimStreamline*        streamline,
                                                   cvf::Vec3d            startPos,
                                                   cvf::Vec3d            endPos,
                                                   double                rate,
                                                   RiaDefines::PhaseType dominantPhase )
{
    double totDistance = endPos.pointDistance( startPos );

    if ( totDistance < 0.1 ) return true;
    if ( rate < m_flowThreshold ) return false;

    cvf::Vec3d movementDirection = endPos - startPos;
    movementDirection.normalize();
    movementDirection *= rate * m_resolution;

    int nSteps = (int)std::round( totDistance / ( rate * m_resolution ) );

    cvf::Vec3d curpos = startPos;
    streamline->addTracerPoint( curpos, movementDirection, dominantPhase );

    for ( int i = 1; i < nSteps; i++ )
    {
        curpos = curpos + movementDirection;
        streamline->addTracerPoint( curpos, movementDirection, dominantPhase );

        if ( streamline->size() >= m_maxPoints ) return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::list<StreamlineSeedPoint> RimStreamlineGenerator::nncCandidates( size_t cellIdx )
{
    std::list<StreamlineSeedPoint> foundCells;

    auto mainGrid = m_dataAccess->grid();

    if ( mainGrid->nncData() == nullptr ) return foundCells;

    auto& connections = mainGrid->nncData()->allConnections();
    for ( size_t i = 0; i < connections.size(); i++ )
    {
        if ( connections[i].c1GlobIdx() == cellIdx )
        {
            // todo - get rate here, handle direction!
            double rate = 0.0;
            if ( rate > m_flowThreshold )
            {
                foundCells.emplace_back( rate, i, cvf::StructGridInterface::FaceType::NO_FACE );
            }
        }
    }

    return foundCells;
}
