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

#include "RimWellIADisplacementData.h"

#include "RigFemClosestResultIndexCalculator.h"
#include "RigFemPartCollection.h"
#include "RigFemPartResultsCollection.h"
#include "RigGeoMechCaseData.h"
#include "RimGeoMechCase.h"

#include "cvfBoundingBox.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellIADisplacementData::RimWellIADisplacementData( RimGeoMechCase* thecase )
    : m_case( thecase )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellIADisplacementData::~RimWellIADisplacementData()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimWellIADisplacementData::getDisplacement( cvf::Vec3d position, int timeStep )
{
    RigGeoMechCaseData* caseData = m_case->geoMechData();
    if ( !caseData ) return cvf::Vec3d( 0, 0, 0 );

    cvf::BoundingBox bb;
    bb.add( position );

    std::vector<size_t> closeCells;
    caseData->femParts()->part( 0 )->findIntersectingCells( bb, &closeCells );
    if ( closeCells.size() == 0 ) return cvf::Vec3d( 0, 0, 0 );

    RigFemClosestResultIndexCalculator closestIndexCalc( caseData->femParts()->part( 0 ),
                                                         RigFemResultPosEnum::RIG_NODAL,
                                                         (int)closeCells[0],
                                                         -1,
                                                         position );

    int resultIndex = closestIndexCalc.resultIndexToClosestResult();

    double u1 = displacementValue( caseData, "U1", resultIndex, timeStep );
    double u2 = displacementValue( caseData, "U2", resultIndex, timeStep );
    double u3 = displacementValue( caseData, "U3", resultIndex, timeStep );

    return cvf::Vec3d( u1, u2, u3 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellIADisplacementData::displacementValue( RigGeoMechCaseData* caseData,
                                                     QString             componentName,
                                                     size_t              resultIndex,
                                                     int                 timeStep )
{
    RigFemResultAddress address( RigFemResultPosEnum::RIG_NODAL, "U", componentName.toStdString(), timeStep );

    const std::vector<float>& scalarResults = caseData->femPartResults()->resultValues( address, 0, 0 );

    if ( ( resultIndex >= 0 ) && ( resultIndex < scalarResults.size() ) ) return scalarResults[resultIndex];

    return 0.0;
}
