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

    auto loadFrameLambda = [&]( auto femParts, RigFemResultAddress addr ) -> RigFemScalarResultFrames*
    {
        auto result = femParts->findOrLoadScalarResult( partIndex, addr );
        if ( result->frameData( 0, 0 ).empty() )
        {
            return nullptr;
        }
        return result;
    };

    auto femParts  = m_geoMechCaseData->femPartResults();
    m_resultFrames = loadFrameLambda( femParts, getResultAddress( m_property ) );
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
double RimFaultReactivationDataAccessorGeoMech::valueAtPosition( const cvf::Vec3d& position ) const
{
    if ( !m_resultFrames ) return std::numeric_limits<double>::infinity();

    auto findCloseCells = [this]( const cvf::BoundingBox& bb, int partId ) -> std::vector<size_t>
    {
        std::vector<size_t> closeCells;

        if ( m_geoMechCaseData->femParts()->partCount() )
        {
            m_geoMechCaseData->femParts()->part( partId )->findIntersectingElementIndices( bb, &closeCells );
        }
        return closeCells;
    };

    const int                      partId     = 0;
    const RigFemPart*              femPart    = m_geoMechCaseData->femParts()->part( partId );
    const std::vector<cvf::Vec3f>& nodeCoords = femPart->nodes().coordinates;

    cvf::BoundingBox bb;
    bb.add( position );

    std::vector<size_t> closeCells = findCloseCells( bb, partId );

    cvf::Vec3d hexCorners[8];
    for ( size_t ccIdx = 0; ccIdx < closeCells.size(); ++ccIdx )
    {
        size_t elementIdx = closeCells[ccIdx];

        RigElementType elmType = femPart->elementType( elementIdx );
        if ( elmType != HEX8 && elmType != HEX8P ) continue;

        const int* cornerIndices = femPart->connectivities( elementIdx );

        hexCorners[0] = cvf::Vec3d( nodeCoords[cornerIndices[0]] );
        hexCorners[1] = cvf::Vec3d( nodeCoords[cornerIndices[1]] );
        hexCorners[2] = cvf::Vec3d( nodeCoords[cornerIndices[2]] );
        hexCorners[3] = cvf::Vec3d( nodeCoords[cornerIndices[3]] );
        hexCorners[4] = cvf::Vec3d( nodeCoords[cornerIndices[4]] );
        hexCorners[5] = cvf::Vec3d( nodeCoords[cornerIndices[5]] );
        hexCorners[6] = cvf::Vec3d( nodeCoords[cornerIndices[6]] );
        hexCorners[7] = cvf::Vec3d( nodeCoords[cornerIndices[7]] );

        if ( RigHexIntersectionTools::isPointInCell( position, hexCorners ) )
        {
            int timeStepIndex = 0;
            int frameIndex    = 0;

            const std::vector<float>& data = m_resultFrames->frameData( timeStepIndex, frameIndex );
            if ( elementIdx >= data.size() ) return std::numeric_limits<double>::infinity();

            return data[elementIdx];
        }
    }

    return std::numeric_limits<double>::infinity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFaultReactivationDataAccessorGeoMech::hasValidDataAtPosition( const cvf::Vec3d& position ) const
{
    double value = valueAtPosition( position );
    return !std::isinf( value );
}
