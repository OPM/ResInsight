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
#include "RimGeoMechCase.h"

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

    std::vector<size_t> closeCells;
    m_caseData->femParts()->part( 0 )->findIntersectingCells( bb, &closeCells );
    if ( closeCells.size() == 0 ) return -1;

    return (int)closeCells[0];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellIADataAccess::resultValue( QString             fieldName,
                                         QString             componentName,
                                         RigFemResultPosEnum resultType,
                                         size_t              resultIndex,
                                         int                 timeStep )
{
    RigFemResultAddress address( resultType, fieldName.toStdString(), componentName.toStdString() );

    const std::vector<float>& scalarResults = m_caseData->femPartResults()->resultValues( address, 0, timeStep );

    if ( ( resultIndex >= 0 ) && ( resultIndex < scalarResults.size() ) ) return scalarResults[resultIndex];

    return 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellIADataAccess::interpolatedResultValue( QString             fieldname,
                                                     QString             componentName,
                                                     RigFemResultPosEnum resultType,
                                                     cvf::Vec3d          position,
                                                     int                 timeStep )
{
    return 0.0;
}
