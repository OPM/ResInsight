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

#include "RimWellIAStressData.h"

#include "RigFemClosestResultIndexCalculator.h"
#include "RigFemPartCollection.h"
#include "RigFemPartResultsCollection.h"
#include "RigGeoMechCaseData.h"
#include "RimGeoMechCase.h"

#include "cvfBoundingBox.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellIAStressData::RimWellIAStressData( RimGeoMechCase* thecase )
    : m_case( thecase )
    , m_pp( 0.0 )
{
    m_stressValues.resize( 6, 0.0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellIAStressData::~RimWellIAStressData()
{
}
#pragma optimize( "", off )

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellIAStressData::extractData( cvf::Vec3d position )
{
    RigGeoMechCaseData* caseData = m_case->geoMechData();
    if ( !caseData ) return false;

    cvf::BoundingBox bb;
    bb.add( position );

    std::vector<size_t> closeCells;
    caseData->femParts()->part( 0 )->findIntersectingCells( bb, &closeCells );
    if ( closeCells.size() == 0 ) return false;

    RigFemClosestResultIndexCalculator closestIndexCalc( caseData->femParts()->part( 0 ),
                                                         RigFemResultPosEnum::RIG_ELEMENT_NODAL,
                                                         (int)closeCells[0],
                                                         -1,
                                                         position );

    int resultIndex = closestIndexCalc.resultIndexToClosestResult();

    std::vector<QString> keys{ "S11", "S22", "S33", "S12", "S13", "S23" };

    for ( size_t i = 0; i < keys.size(); i++ )
        m_stressValues[i] = stressValue( caseData, keys[i], resultIndex );

    m_pp = ppValue( caseData, closeCells[0], position );

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellIAStressData::stressValue( RigGeoMechCaseData* caseData, QString componentName, size_t resultIndex )
{
    RigFemResultAddress address( RigFemResultPosEnum::RIG_ELEMENT_NODAL, "ST", componentName.toStdString() );

    const std::vector<float>& scalarResults = caseData->femPartResults()->resultValues( address, 0, 0 );

    if ( ( resultIndex > 0 ) && ( resultIndex < scalarResults.size() ) ) return scalarResults[resultIndex];

    return 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellIAStressData::ppValue( RigGeoMechCaseData* caseData, size_t cellIdx, cvf::Vec3d position )
{
    RigFemResultAddress address( RigFemResultPosEnum::RIG_NODAL, "POR-Bar", "" );

    RigFemClosestResultIndexCalculator closestIndexCalc( caseData->femParts()->part( 0 ),
                                                         RigFemResultPosEnum::RIG_NODAL,
                                                         (int)cellIdx,
                                                         -1,
                                                         position );

    int resultIndex = closestIndexCalc.resultIndexToClosestResult();

    const std::vector<float>& scalarResults = caseData->femPartResults()->resultValues( address, 0, 0 );

    if ( ( resultIndex > 0 ) && ( resultIndex < scalarResults.size() ) ) return scalarResults[resultIndex];

    return 0.0;
}

#pragma optimize( "", on )

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellIAStressData::sxx() const
{
    return m_stressValues[0];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellIAStressData::syy() const
{
    return m_stressValues[1];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellIAStressData::szz() const
{
    return m_stressValues[2];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellIAStressData::sxy() const
{
    return m_stressValues[3];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellIAStressData::sxz() const
{
    return m_stressValues[4];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellIAStressData::syz() const
{
    return m_stressValues[5];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellIAStressData::pp() const
{
    return m_pp;
}
