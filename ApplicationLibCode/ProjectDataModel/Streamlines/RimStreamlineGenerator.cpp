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
#include "RigNncConnection.h"

#include "RimStreamline.h"
#include "RimStreamlineDataAccess.h"

#include <cmath>

//--------------------------------------------------------------------------------------------------
/// Helper class for prioritizing streamline seed points
//--------------------------------------------------------------------------------------------------
class StreamlineSeedPoint
{
public:
    StreamlineSeedPoint( double rate, size_t index, RimStreamlineGenerator::CellFaceType faceIdx )
        : m_rate( rate )
        , m_index( index )
        , m_faceIdx( faceIdx ) {};
    ~StreamlineSeedPoint() {};

    bool operator<( const StreamlineSeedPoint& other ) const { return rate() < other.rate(); };

    double                               rate() const { return m_rate; };
    size_t                               index() const { return m_index; };
    RimStreamlineGenerator::CellFaceType face() const { return m_faceIdx; };

private:
    double                               m_rate;
    size_t                               m_index;
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
            m_seeds.emplace( rate, cellIdx, faceIdx );
        }
    }

    // add any nnc connections from this cell, too
    auto nncSeeds = nncCandidates( cell.mainGridCellIndex(), m_phases, direction );
    for ( auto& seed : nncSeeds )
    {
        m_seeds.push( seed );
    }

    // keep adding streamlines until we have no more seeds left in our prioritized (by flow rate) list
    while ( !m_seeds.empty() )
    {
        auto&                                    seed    = m_seeds.top();
        const size_t                             index   = seed.index();
        const cvf::StructGridInterface::FaceType faceIdx = seed.face();
        m_seeds.pop();

        RimStreamline* streamline = new RimStreamline( simWellName );
        growStreamline( streamline, index, faceIdx, direction );

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
void RimStreamlineGenerator::growStreamline( RimStreamline* streamline, size_t index, CellFaceType faceIdx, double direction )
{
    double                rate = 0.0;
    RiaDefines::PhaseType dominantPhaseOut;
    size_t                currentCellIdx = 0;
    RigCell               currentCell;

    while ( true )
    {
        // no face? -> nnc connection
        if ( faceIdx == CellFaceType::NO_FACE )
        {
            const auto    nncIdx = index;
            RigConnection nnc    = m_dataAccess->nncConnection( nncIdx );

            size_t localCellIdx1 = 0;
            auto   localGrid1    = m_dataAccess->grid()->gridAndGridLocalIdxFromGlobalCellIdx( nnc.c1GlobIdx(), &localCellIdx1 );
            size_t localCellIdx2 = 0;
            auto   localGrid2    = m_dataAccess->grid()->gridAndGridLocalIdxFromGlobalCellIdx( nnc.c2GlobIdx(), &localCellIdx2 );

            // no support for LGRs, yet
            if ( localGrid1->gridId() != 0 ) return;
            if ( localGrid2->gridId() != 0 ) return;

            RigCell cell1 = m_dataAccess->grid()->cell( localCellIdx1 );
            RigCell cell2 = m_dataAccess->grid()->cell( localCellIdx2 );

            // get rate
            rate = m_dataAccess->combinedNNCRate( nncIdx, m_phases, direction, dominantPhaseOut );
            // if we go backwards from a producer, the rate needs to be flipped
            rate *= direction;

            // grow from cell1 center to cell2 center, exiting if we reach max length
            if ( !growStreamlineFromTo( streamline, cell1.center(), cell2.center(), rate, dominantPhaseOut ) ) return;

            // give up if too low rate
            if ( rate < m_flowThreshold ) return;

            currentCellIdx = localCellIdx2;
            currentCell    = cell2;
        }
        else
        {
            // get the current cell
            const auto cellIdx = index;
            RigCell    cell    = m_dataAccess->grid()->cell( cellIdx );

            // get rate
            rate = m_dataAccess->combinedFaceRate( cell, faceIdx, m_phases, direction, dominantPhaseOut );

            // if we go backwards from a producer, the rate needs to be flipped
            rate *= direction;

            // grow from start cell center to face center, exiting if we reach the max length
            if ( !growStreamlineFromTo( streamline, cell.center(), cell.faceCenter( faceIdx ), rate, dominantPhaseOut ) ) return;

            // find next cell and entry face
            currentCell = cell.neighborCell( faceIdx );
            if ( currentCell.isInvalid() ) return;
            faceIdx = cvf::StructGridInterface::oppositeFace( faceIdx );

            // grow from given face center to neighbour cell center, exiting if we reach the max length
            if ( !growStreamlineFromTo( streamline, currentCell.faceCenter( faceIdx ), currentCell.center(), rate, dominantPhaseOut ) )
                break;

            // give up if too low rate
            if ( rate < m_flowThreshold ) return;

            currentCellIdx = currentCell.gridLocalCellIndex();
        }

        // have we been here before?
        if ( m_visitedCells.count( currentCellIdx ) > 0 ) return;
        if ( m_wellCells.count( currentCellIdx ) > 0 ) return;

        m_visitedCells.insert( currentCellIdx );

        // find the face with max flow where we should exit the cell
        CellFaceType                   exitFace = cvf::StructGridInterface::FaceType::NO_FACE;
        std::map<CellFaceType, double> rateMap;

        double maxRate = 0.0;

        for ( auto face : m_allFaces )
        {
            // skip the entry face
            if ( face == faceIdx ) continue;

            RiaDefines::PhaseType tempDominantFace;
            double                faceRate = m_dataAccess->combinedFaceRate( currentCell, face, m_phases, direction, tempDominantFace );

            // if we go backwards from a producer, the rate needs to be flipped
            faceRate *= direction;

            if ( faceRate > maxRate )
            {
                exitFace         = face;
                maxRate          = faceRate;
                dominantPhaseOut = tempDominantFace;
                index            = currentCellIdx;
            }

            rateMap[face] = faceRate;
        }

        // check if we have any NNC connections here
        int  nncId          = -1;
        auto nncConnections = nncCandidates( currentCellIdx, m_phases, direction );
        for ( auto& nncPoint : nncConnections )
        {
            RiaDefines::PhaseType tempDominantFace;
            double nncRate = m_dataAccess->combinedNNCRate( nncPoint.index(), m_phases, direction, tempDominantFace ) * direction;
            if ( nncRate > maxRate )
            {
                nncId            = (int)nncPoint.index();
                maxRate          = nncRate;
                dominantPhaseOut = tempDominantFace;
                exitFace         = CellFaceType::NO_FACE;
            }
        }

        // did we find an exit?
        if ( exitFace == cvf::StructGridInterface::FaceType::NO_FACE )
        {
            if ( nncId < 0 ) return;
            index = (size_t)nncId;

            for ( auto& nncPoint : nncConnections )
            {
                if ( nncPoint.index() == index ) continue;
                m_seeds.push( nncPoint );
            }
        }

        // add seeds for other faces with flow > threshold
        for ( auto& kvp : rateMap )
        {
            if ( kvp.first == exitFace ) continue;

            if ( kvp.second >= m_flowThreshold )
                m_seeds.push( StreamlineSeedPoint( kvp.second, currentCell.gridLocalCellIndex(), kvp.first ) );
        }

        rate    = maxRate;
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
std::list<StreamlineSeedPoint> RimStreamlineGenerator::nncCandidates( size_t cellIdx, std::list<RiaDefines::PhaseType> phases, double direction )
{
    std::list<StreamlineSeedPoint> foundCells;

    auto mainGrid = m_dataAccess->grid();

    if ( mainGrid->nncData() == nullptr ) return foundCells;

    auto& connections = mainGrid->nncData()->allConnections();
    for ( size_t i = 0; i < connections.size(); i++ )
    {
        if ( connections[i].c1GlobIdx() == cellIdx )
        {
            RiaDefines::PhaseType domPhase;
            double                rate = m_dataAccess->combinedNNCRate( i, phases, direction, domPhase ) * direction;
            if ( rate > m_flowThreshold )
            {
                foundCells.emplace_back( rate, i, cvf::StructGridInterface::FaceType::NO_FACE );
            }
        }
    }

    return foundCells;
}
