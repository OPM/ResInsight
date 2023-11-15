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

#include "RimFaultReactivationDataAccessorStress.h"

#include "RigFemAddressDefines.h"
#include "RigFemPartCollection.h"
#include "RigFemPartResultsCollection.h"
#include "RigFemResultAddress.h"
#include "RigFemResultPosEnum.h"
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
RimFaultReactivationDataAccessorStress::RimFaultReactivationDataAccessorStress( RimGeoMechCase*                geoMechCase,
                                                                                RimFaultReactivation::Property property )
    : m_geoMechCase( geoMechCase )
    , m_property( property )
{
    m_geoMechCaseData = geoMechCase->geoMechData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaultReactivationDataAccessorStress::~RimFaultReactivationDataAccessorStress()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultReactivationDataAccessorStress::updateResultAccessor()
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

    auto femParts = m_geoMechCaseData->femPartResults();
    m_s33Frames   = loadFrameLambda( femParts, getResultAddress( "ST", "S33" ) );
    m_s11Frames   = loadFrameLambda( femParts, getResultAddress( "ST", "S11" ) );
    m_s22Frames   = loadFrameLambda( femParts, getResultAddress( "ST", "S22" ) );
    m_porFrames   = loadFrameLambda( femParts, RigFemAddressDefines::elementNodalPorBarAddress() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemResultAddress RimFaultReactivationDataAccessorStress::getResultAddress( const std::string& fieldName, const std::string& componentName )
{
    return RigFemResultAddress( RIG_ELEMENT_NODAL, fieldName, componentName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFaultReactivationDataAccessorStress::isMatching( RimFaultReactivation::Property property ) const
{
    return property == m_property;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFaultReactivationDataAccessorStress::valueAtPosition( const cvf::Vec3d& position ) const
{
    if ( !m_porFrames || !m_s11Frames || !m_s22Frames || !m_s33Frames ) return std::numeric_limits<double>::infinity();

    const int partIndex = 0;
    auto      femPart   = m_geoMechCaseData->femParts()->part( partIndex );

    RimWellIADataAccess iaDataAccess( m_geoMechCase );
    int                 elementIdx = iaDataAccess.elementIndex( position );
    if ( elementIdx != -1 )
    {
        int timeStepIndex = 0;
        int frameIndex    = 0;

        const std::vector<float>& s11Data = m_s11Frames->frameData( timeStepIndex, frameIndex );
        const std::vector<float>& s22Data = m_s22Frames->frameData( timeStepIndex, frameIndex );
        const std::vector<float>& s33Data = m_s33Frames->frameData( timeStepIndex, frameIndex );
        const std::vector<float>& porData = m_porFrames->frameData( timeStepIndex, frameIndex );

        if ( elementIdx >= static_cast<int>( porData.size() ) ) return std::numeric_limits<double>::infinity();

        int    elmNodIdx    = 0;
        size_t elmNodResIdx = femPart->elementNodeResultIdx( elementIdx, elmNodIdx );

        if ( m_property == RimFaultReactivation::Property::StressTop || m_property == RimFaultReactivation::Property::StressBottom )
        {
            if ( elmNodResIdx < porData.size() ) return s33Data[elmNodResIdx] - porData[elmNodResIdx];
        }
        else if ( m_property == RimFaultReactivation::Property::DepthTop || m_property == RimFaultReactivation::Property::DepthBottom )
        {
            return position.z();
        }
        else if ( m_property == RimFaultReactivation::Property::LateralStressComponentX )
        {
            if ( elmNodResIdx < porData.size() ) return s11Data[elmNodResIdx] / s33Data[elmNodResIdx];
        }
        else if ( m_property == RimFaultReactivation::Property::LateralStressComponentY )
        {
            if ( elmNodResIdx < porData.size() ) return s22Data[elmNodResIdx] / s33Data[elmNodResIdx];
        }
    }

    return std::numeric_limits<double>::infinity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFaultReactivationDataAccessorStress::hasValidDataAtPosition( const cvf::Vec3d& position ) const
{
    double value = valueAtPosition( position );
    return !std::isinf( value );
}
