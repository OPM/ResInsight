/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023  Equinor ASA
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

#include "RimFaultReactivationDataAccessorGeoMech.h"

#include "RiaEclipseUnitTools.h"

#include "RigFemPartCollection.h"
#include "RigFemPartResultsCollection.h"
#include "RigFemResultAddress.h"
#include "RigFemScalarResultFrames.h"
#include "RigGeoMechCaseData.h"
#include "RigHexIntersectionTools.h"
#include "RigMainGrid.h"
#include "RigResultAccessorFactory.h"

#include "RimFaultReactivationEnums.h"
#include "RimGeoMechCase.h"
#include "RimWellIADataAccess.h"

#include <cmath>
#include <limits>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaultReactivationDataAccessorGeoMech::RimFaultReactivationDataAccessorGeoMech( RimGeoMechCase*                geoMechCase,
                                                                                  RimFaultReactivation::Property property )
    : m_geoMechCase( geoMechCase )
    , m_property( property )
{
    m_geoMechCaseData = geoMechCase->geoMechData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaultReactivationDataAccessorGeoMech::~RimFaultReactivationDataAccessorGeoMech()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultReactivationDataAccessorGeoMech::updateResultAccessor()
{
    const int partIndex = 0;

    auto loadFrameLambda = [&]( auto femParts, RigFemResultAddress addr ) -> std::vector<float>
    {
        auto result = femParts->findOrLoadScalarResult( partIndex, addr );
        return result->frameData( 0, 0 );
    };

    auto femParts = m_geoMechCaseData->femPartResults();
    m_data        = loadFrameLambda( femParts, getResultAddress( m_property ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemResultAddress RimFaultReactivationDataAccessorGeoMech::getResultAddress( RimFaultReactivation::Property property )
{
    if ( property == RimFaultReactivation::Property::YoungsModulus ) return RigFemResultAddress( RIG_ELEMENT, "MODULUS", "" );
    if ( property == RimFaultReactivation::Property::PoissonsRatio ) return RigFemResultAddress( RIG_ELEMENT, "RATIO", "" );
    CAF_ASSERT( property == RimFaultReactivation::Property::Density );
    return RigFemResultAddress( RIG_ELEMENT, "DENSITY", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFaultReactivationDataAccessorGeoMech::isMatching( RimFaultReactivation::Property property ) const
{
    return property == m_property;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFaultReactivationDataAccessorGeoMech::valueAtPosition( const cvf::Vec3d&                position,
                                                                 const RigFaultReactivationModel& model,
                                                                 RimFaultReactivation::GridPart   gridPart,
                                                                 double                           topDepth,
                                                                 double                           bottomDepth,
                                                                 size_t                           elementIndex ) const
{
    if ( !m_data.empty() ) return std::numeric_limits<double>::infinity();

    RimWellIADataAccess iaDataAccess( m_geoMechCase );
    int                 elementIdx = iaDataAccess.elementIndex( position );
    if ( elementIdx != -1 )
    {
        if ( elementIdx >= static_cast<int>( m_data.size() ) ) return std::numeric_limits<double>::infinity();

        if ( m_property == RimFaultReactivation::Property::YoungsModulus )
        {
            return RiaEclipseUnitTools::gigaPascalToPascal( m_data[elementIdx] );
        }
        return m_data[elementIdx];
    }

    return std::numeric_limits<double>::infinity();
}
