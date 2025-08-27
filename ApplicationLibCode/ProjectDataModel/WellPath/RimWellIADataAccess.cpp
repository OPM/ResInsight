/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021 -    Equinor ASA
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

#include "RimWellIADataAccess.h"

#include "RigFemClosestResultIndexCalculator.h"
#include "RigFemPartCollection.h"
#include "RigFemPartResultsCollection.h"
#include "RigGeoMechCaseData.h"
#include "RigHexIntersectionTools.h"

#include "RimGeoMechCase.h"

#include "../cafHexInterpolator/cafHexInterpolator.h" // Use relative path, as this is a header only file not part of a library

#include "cvfBoundingBox.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellIADataAccess::RimWellIADataAccess( RimGeoMechCase* thecase )
    : m_case( thecase )
    , m_caseData( nullptr )
{
    if ( m_case ) m_caseData = m_case->geoMechData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellIADataAccess::~RimWellIADataAccess()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimWellIADataAccess::resultIndex( RigFemResultPosEnum resultType, cvf::Vec3d position )
{
    int closestCell = elementIndex( position );

    if ( closestCell < 0 ) return -1;

    RigFemClosestResultIndexCalculator closestIndexCalc( m_caseData->femParts()->part( 0 ), resultType, closestCell, -1, position );

    return closestIndexCalc.resultIndexToClosestResult();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimWellIADataAccess::elementIndex( cvf::Vec3d position )
{
    cvf::BoundingBox bb;
    bb.add( position );

    auto part = m_caseData->femParts()->part( 0 );

    std::vector<size_t> closeElements = part->findIntersectingElementIndices( bb );
    if ( closeElements.empty() ) return -1;

    for ( auto elmIdx : closeElements )
    {
        std::array<cvf::Vec3d, 8> coordinates;
        if ( !part->fillElementCoordinates( elmIdx, coordinates ) ) continue;

        if ( RigHexIntersectionTools::isPointInCell( position, coordinates ) ) return (int)elmIdx;
    }

    return -1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellIADataAccess::resultValue( QString             fieldName,
                                         QString             componentName,
                                         RigFemResultPosEnum resultType,
                                         size_t              resultIndex,
                                         int                 timeStep,
                                         int                 frameId )
{
    RigFemResultAddress address( resultType, fieldName.toStdString(), componentName.toStdString() );

    const std::vector<float>& scalarResults = m_caseData->femPartResults()->resultValues( address, 0, timeStep, frameId );

    if ( resultIndex < scalarResults.size() ) return scalarResults[resultIndex];

    return 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellIADataAccess::interpolatedResultValue( QString             fieldName,
                                                     QString             componentName,
                                                     RigFemResultPosEnum resultType,
                                                     cvf::Vec3d          position,
                                                     int                 timeStep,
                                                     int                 frameId )
{
    RigFemResultAddress address( resultType, fieldName.toStdString(), componentName.toStdString() );

    RigFemPart*               femPart       = m_caseData->femParts()->part( 0 );
    const std::vector<float>& scalarResults = m_caseData->femPartResults()->resultValues( address, 0, timeStep, frameId );

    return interpolatedResultValue( femPart, scalarResults, resultType, position );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellIADataAccess::interpolatedResultValue( const RigFemPart*         femPart,
                                                     const std::vector<float>& scalarResults,
                                                     RigFemResultPosEnum       resultType,
                                                     const cvf::Vec3d&         position )
{
    int elmIdx = elementIndex( position );

    RigElementType elmType      = femPart->elementType( elmIdx );
    const int*     elementConn  = femPart->connectivities( elmIdx );
    int            elmNodeCount = RigFemTypes::elementNodeCount( elmType );

    std::array<double, 8>     nodeResults;
    std::array<cvf::Vec3d, 8> nodeCorners;

    for ( int lNodeIdx = 0; lNodeIdx < elmNodeCount; ++lNodeIdx )
    {
        int    nodeIdx = elementConn[lNodeIdx];
        size_t resIdx  = femPart->resultValueIdxFromResultPosType( resultType, elmIdx, lNodeIdx );
        if ( resIdx >= scalarResults.size() )
            nodeResults[lNodeIdx] = 0.0;
        else
            nodeResults[lNodeIdx] = scalarResults[resIdx];
        nodeCorners[lNodeIdx] = cvf::Vec3d( femPart->nodes().coordinates[nodeIdx] );
    }

    return caf::HexInterpolator::interpolateHex( nodeCorners, nodeResults, position );
}
