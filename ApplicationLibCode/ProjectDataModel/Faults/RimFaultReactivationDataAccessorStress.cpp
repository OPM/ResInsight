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

#include "RiaEclipseUnitTools.h"

#include "RigFemAddressDefines.h"
#include "RigFemPartCollection.h"
#include "RigFemPartResultsCollection.h"
#include "RigFemResultAddress.h"
#include "RigFemScalarResultFrames.h"
#include "RigGeoMechCaseData.h"
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
    m_femPart     = femParts->parts()->part( partIndex );
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
double RimFaultReactivationDataAccessorStress::valueAtPosition( const cvf::Vec3d&                position,
                                                                const RigFaultReactivationModel& model,
                                                                RimFaultReactivation::GridPart   gridPart,
                                                                double                           topDepth,
                                                                double                           bottomDepth ) const
{
    if ( !m_porFrames || !m_s11Frames || !m_s22Frames || !m_s33Frames || !m_femPart ) return std::numeric_limits<double>::infinity();

    RimWellIADataAccess iaDataAccess( m_geoMechCase );
    int                 centerElementIdx = iaDataAccess.elementIndex( position );

    cvf::Vec3d topPosition( position.x(), position.y(), topDepth );
    int        topElementIdx = iaDataAccess.elementIndex( topPosition );

    cvf::Vec3d bottomPosition( position.x(), position.y(), bottomDepth );
    int        bottomElementIdx = iaDataAccess.elementIndex( bottomPosition );
    if ( centerElementIdx != -1 && topElementIdx != -1 && bottomElementIdx != -1 )
    {
        int timeStepIndex = 0;
        int frameIndex    = 0;

        const std::vector<float>& s11Data = m_s11Frames->frameData( timeStepIndex, frameIndex );
        const std::vector<float>& s22Data = m_s22Frames->frameData( timeStepIndex, frameIndex );
        const std::vector<float>& s33Data = m_s33Frames->frameData( timeStepIndex, frameIndex );
        const std::vector<float>& porData = m_porFrames->frameData( timeStepIndex, frameIndex );

        if ( m_property == RimFaultReactivation::Property::StressTop )
        {
            double s33    = interpolatedResultValue( iaDataAccess, m_femPart, topPosition, s33Data );
            double porBar = interpolatedResultValue( iaDataAccess, m_femPart, topPosition, porData );
            return RiaEclipseUnitTools::barToPascal( s33 - porBar );
        }
        else if ( m_property == RimFaultReactivation::Property::StressBottom )
        {
            double s33    = interpolatedResultValue( iaDataAccess, m_femPart, bottomPosition, s33Data );
            double porBar = interpolatedResultValue( iaDataAccess, m_femPart, bottomPosition, porData );
            return RiaEclipseUnitTools::barToPascal( s33 - porBar );
        }
        else if ( m_property == RimFaultReactivation::Property::DepthTop )
        {
            return topDepth;
        }
        else if ( m_property == RimFaultReactivation::Property::DepthBottom )
        {
            return bottomDepth;
        }
        else if ( m_property == RimFaultReactivation::Property::LateralStressComponentX )
        {
            double s11    = interpolatedResultValue( iaDataAccess, m_femPart, position, s11Data );
            double s33    = interpolatedResultValue( iaDataAccess, m_femPart, position, s33Data );
            double porBar = interpolatedResultValue( iaDataAccess, m_femPart, position, porData );
            return ( s11 - porBar ) / ( s33 - porBar );
        }
        else if ( m_property == RimFaultReactivation::Property::LateralStressComponentY )
        {
            double s22    = interpolatedResultValue( iaDataAccess, m_femPart, position, s22Data );
            double s33    = interpolatedResultValue( iaDataAccess, m_femPart, position, s33Data );
            double porBar = interpolatedResultValue( iaDataAccess, m_femPart, position, porData );
            return ( s22 - porBar ) / ( s33 - porBar );
        }
    }

    return std::numeric_limits<double>::infinity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFaultReactivationDataAccessorStress::interpolatedResultValue( RimWellIADataAccess&      iaDataAccess,
                                                                        const RigFemPart*         femPart,
                                                                        const cvf::Vec3d&         position,
                                                                        const std::vector<float>& scalarResults ) const
{
    return iaDataAccess.interpolatedResultValue( femPart, scalarResults, RIG_ELEMENT_NODAL, position );
}
