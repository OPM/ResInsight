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
#include "RiaLogging.h"

#include "RigFaultReactivationModel.h"
#include "RigFemAddressDefines.h"
#include "RigFemPartCollection.h"
#include "RigFemPartResultsCollection.h"
#include "RigFemResultAddress.h"
#include "RigFemScalarResultFrames.h"
#include "RigGeoMechCaseData.h"
#include "RigGeoMechWellLogExtractor.h"
#include "RigGriddedPart3d.h"
#include "RigResultAccessorFactory.h"
#include "RigWellPath.h"

#include "RimFaultReactivationDataAccessorWellLogExtraction.h"
#include "RimFaultReactivationEnums.h"
#include "RimGeoMechCase.h"
#include "RimWellIADataAccess.h"

#include "cvfVector3.h"

#include <cmath>
#include <limits>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaultReactivationDataAccessorStress::RimFaultReactivationDataAccessorStress( RimGeoMechCase*                geoMechCase,
                                                                                RimFaultReactivation::Property property,
                                                                                double                         gradient,
                                                                                double                         seabedDepth )
    : m_geoMechCase( geoMechCase )
    , m_property( property )
    , m_gradient( gradient )
    , m_seabedDepth( seabedDepth )
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

    auto loadFrameLambda = [&]( auto femParts, RigFemResultAddress addr, int timeStepIndex ) -> RigFemScalarResultFrames*
    {
        auto result     = femParts->findOrLoadScalarResult( partIndex, addr );
        int  frameIndex = result->frameCount( timeStepIndex ) - 1;
        if ( result->frameData( timeStepIndex, frameIndex ).empty() )
        {
            return nullptr;
        }
        return result;
    };

    auto femParts     = m_geoMechCaseData->femPartResults();
    m_femPart         = femParts->parts()->part( partIndex );
    int timeStepIndex = 0;
    m_s33Frames       = loadFrameLambda( femParts, getResultAddress( "ST", "S33" ), timeStepIndex );
    m_s11Frames       = loadFrameLambda( femParts, getResultAddress( "ST", "S11" ), timeStepIndex );
    m_s22Frames       = loadFrameLambda( femParts, getResultAddress( "ST", "S22" ), timeStepIndex );

    auto [faultTopPosition, faultBottomPosition] = m_model->faultTopBottom();
    auto   faultNormal                           = m_model->faultNormal();
    double distanceFromFault                     = 1.0;

    RigFemPartCollection* geoMechPartCollection = m_geoMechCaseData->femParts();
    std::string           errorName             = "fault reactivation data access";

    {
        std::vector<cvf::Vec3d> wellPoints =
            RimFaultReactivationDataAccessorWellLogExtraction::generateWellPoints( faultTopPosition,
                                                                                   faultBottomPosition,
                                                                                   m_seabedDepth,
                                                                                   faultNormal * distanceFromFault );
        m_faceAWellPath = new RigWellPath( wellPoints, RimFaultReactivationDataAccessorWellLogExtraction::generateMds( wellPoints ) );
        m_partIndexA    = geoMechPartCollection->getPartIndexFromPoint( wellPoints[1] );
        m_extractorA    = new RigGeoMechWellLogExtractor( m_geoMechCaseData, partIndex, m_faceAWellPath.p(), errorName );
    }

    {
        std::vector<cvf::Vec3d> wellPoints =
            RimFaultReactivationDataAccessorWellLogExtraction::generateWellPoints( faultTopPosition,
                                                                                   faultBottomPosition,
                                                                                   m_seabedDepth,
                                                                                   -faultNormal * distanceFromFault );
        m_faceBWellPath = new RigWellPath( wellPoints, RimFaultReactivationDataAccessorWellLogExtraction::generateMds( wellPoints ) );
        m_partIndexB    = geoMechPartCollection->getPartIndexFromPoint( wellPoints[1] );
        m_extractorB    = new RigGeoMechWellLogExtractor( m_geoMechCaseData, partIndex, m_faceBWellPath.p(), errorName );
    }
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
                                                                double                           bottomDepth,
                                                                size_t                           elementIndex ) const
{
    if ( !m_s11Frames || !m_s22Frames || !m_s33Frames || !m_femPart ) return std::numeric_limits<double>::infinity();

    RimWellIADataAccess iaDataAccess( m_geoMechCase );
    int                 centerElementIdx = iaDataAccess.elementIndex( position );

    cvf::Vec3d topPosition( position.x(), position.y(), topDepth );
    int        topElementIdx = iaDataAccess.elementIndex( topPosition );

    cvf::Vec3d bottomPosition( position.x(), position.y(), bottomDepth );
    int        bottomElementIdx = iaDataAccess.elementIndex( bottomPosition );
    if ( centerElementIdx != -1 && topElementIdx != -1 && bottomElementIdx != -1 )
    {
        int timeStepIndex = 0;
        int frameIndex    = m_s33Frames->frameCount( timeStepIndex ) - 1;

        const std::vector<float>& s33Data = m_s33Frames->frameData( timeStepIndex, frameIndex );

        if ( m_property == RimFaultReactivation::Property::StressTop )
        {
            auto [porBar, extractionPos] = calculatePorBar( topPosition, m_gradient, timeStepIndex, frameIndex );
            if ( std::isinf( porBar ) ) return porBar;
            double s33 = interpolatedResultValue( iaDataAccess, m_femPart, extractionPos, s33Data );
            return RiaEclipseUnitTools::barToPascal( s33 - porBar );
        }
        else if ( m_property == RimFaultReactivation::Property::StressBottom )
        {
            auto [porBar, extractionPos] = calculatePorBar( bottomPosition, m_gradient, timeStepIndex, frameIndex );
            if ( std::isinf( porBar ) ) return porBar;
            double s33 = interpolatedResultValue( iaDataAccess, m_femPart, extractionPos, s33Data );
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
            auto [porBar, extractionPos] = calculatePorBar( position, m_gradient, timeStepIndex, frameIndex );
            if ( std::isinf( porBar ) ) return porBar;

            const std::vector<float>& s11Data = m_s11Frames->frameData( timeStepIndex, frameIndex );
            double                    s11     = interpolatedResultValue( iaDataAccess, m_femPart, extractionPos, s11Data );
            double                    s33     = interpolatedResultValue( iaDataAccess, m_femPart, extractionPos, s33Data );
            return ( s11 - porBar ) / ( s33 - porBar );
        }
        else if ( m_property == RimFaultReactivation::Property::LateralStressComponentY )
        {
            auto [porBar, extractionPos] = calculatePorBar( position, m_gradient, timeStepIndex, frameIndex );
            if ( std::isinf( porBar ) ) return porBar;

            const std::vector<float>& s22Data = m_s22Frames->frameData( timeStepIndex, frameIndex );
            double                    s22     = interpolatedResultValue( iaDataAccess, m_femPart, extractionPos, s22Data );
            double                    s33     = interpolatedResultValue( iaDataAccess, m_femPart, extractionPos, s33Data );
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, cvf::Vec3d>
    RimFaultReactivationDataAccessorStress::calculatePorBar( const cvf::Vec3d& position, double gradient, int timeStepIndex, int frameIndex ) const
{
    RigFemPartCollection*                partCollection = m_geoMechCaseData->femParts();
    cvf::ref<RigGeoMechWellLogExtractor> extractor      = m_partIndexA == partCollection->getPartIndexFromPoint( position ) ? m_extractorA
                                                                                                                            : m_extractorB;
    if ( !extractor->valid() )
    {
        RiaLogging::error( "Invalid extractor when extracting PorBar" );
        return { std::numeric_limits<double>::infinity(), cvf::Vec3d::UNDEFINED };
    }

    RigFemResultAddress resAddr = RigFemAddressDefines::nodalPorBarAddress();
    std::vector<double> values;
    extractor->curveData( resAddr, timeStepIndex, frameIndex, &values );

    return RimFaultReactivationDataAccessorWellLogExtraction::calculatePorBar( extractor->intersections(), values, position, gradient );
}
