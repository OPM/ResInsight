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

#include "RimFaultReactivationDataAccessorStressGeoMech.h"

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

#include "RimFaultReactivationDataAccessorStress.h"
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
RimFaultReactivationDataAccessorStressGeoMech::RimFaultReactivationDataAccessorStressGeoMech( RimGeoMechCase*                geoMechCase,
                                                                                              RimFaultReactivation::Property property,
                                                                                              double                         gradient,
                                                                                              double                         seabedDepth )
    : RimFaultReactivationDataAccessorStress( property, gradient, seabedDepth )
    , m_geoMechCase( geoMechCase )
{
    m_geoMechCaseData = geoMechCase->geoMechData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaultReactivationDataAccessorStressGeoMech::~RimFaultReactivationDataAccessorStressGeoMech()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultReactivationDataAccessorStressGeoMech::updateResultAccessor()
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
    m_porBarFrames    = loadFrameLambda( femParts, RigFemAddressDefines::nodalPorBarAddress(), timeStepIndex );

    auto [faultTopPosition, faultBottomPosition] = m_model->faultTopBottom();
    auto faultNormal                             = m_model->modelNormal() ^ cvf::Vec3d::Z_AXIS;
    faultNormal.normalize();

    double distanceFromFault     = 1.0;
    auto [topDepth, bottomDepth] = m_model->depthTopBottom();

    for ( auto gridPart : m_model->allGridParts() )
    {
        double                  sign = m_model->normalPointsAt() == gridPart ? -1.0 : 1.0;
        std::vector<cvf::Vec3d> wellPoints =
            RimFaultReactivationDataAccessorWellLogExtraction::generateWellPoints( faultTopPosition,
                                                                                   faultBottomPosition,
                                                                                   m_seabedDepth,
                                                                                   bottomDepth,
                                                                                   sign * faultNormal * distanceFromFault );

        cvf::ref<RigWellPath> wellPath =
            new RigWellPath( wellPoints, RimFaultReactivationDataAccessorWellLogExtraction::generateMds( wellPoints ) );
        m_wellPaths[gridPart] = wellPath;

        std::string                          errorName = "fault reactivation data access";
        cvf::ref<RigGeoMechWellLogExtractor> extractor =
            new RigGeoMechWellLogExtractor( m_geoMechCaseData, partIndex, wellPath.p(), errorName );

        m_extractors[gridPart] = extractor;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemResultAddress RimFaultReactivationDataAccessorStressGeoMech::getResultAddress( const std::string& fieldName,
                                                                                     const std::string& componentName )
{
    return RigFemResultAddress( RIG_ELEMENT_NODAL, fieldName, componentName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFaultReactivationDataAccessorStressGeoMech::isDataAvailable() const
{
    return m_s11Frames && m_s22Frames && m_s33Frames && m_porBarFrames && m_femPart;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFaultReactivationDataAccessorStressGeoMech::extractStressValue( StressType                     stressType,
                                                                          const cvf::Vec3d&              position,
                                                                          RimFaultReactivation::GridPart gridPart ) const
{
    RimWellIADataAccess iaDataAccess( m_geoMechCase );

    int timeStepIndex = 0;

    RigFemScalarResultFrames* frames     = dataFrames( stressType );
    int                       frameIndex = frames->frameCount( timeStepIndex ) - 1;
    const std::vector<float>& s11Data    = frames->frameData( timeStepIndex, frameIndex );
    return interpolatedResultValue( iaDataAccess, m_femPart, position, s11Data );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RimFaultReactivationDataAccessorStressGeoMech::dataFrames( StressType stressType ) const
{
    if ( stressType == StressType::S11 )
        return m_s11Frames;
    else if ( stressType == StressType::S22 )
        return m_s22Frames;
    else
    {
        CAF_ASSERT( stressType == StressType::S33 );
        return m_s33Frames;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, cvf::Vec3d> RimFaultReactivationDataAccessorStressGeoMech::calculatePorBar( const cvf::Vec3d& position,
                                                                                              double            gradient,
                                                                                              RimFaultReactivation::GridPart gridPart ) const
{
    int timeStepIndex = 0;
    int frameIndex    = m_s33Frames->frameCount( timeStepIndex ) - 1;
    return calculatePorBar( position, m_gradient, gridPart, timeStepIndex, frameIndex );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFaultReactivationDataAccessorStressGeoMech::isPositionValid( const cvf::Vec3d&              position,
                                                                     const cvf::Vec3d&              topPosition,
                                                                     const cvf::Vec3d&              bottomPosition,
                                                                     RimFaultReactivation::GridPart gridPart ) const
{
    RimWellIADataAccess iaDataAccess( m_geoMechCase );
    int                 centerElementIdx = iaDataAccess.elementIndex( position );
    int                 bottomElementIdx = iaDataAccess.elementIndex( bottomPosition );
    int                 topElementIdx    = iaDataAccess.elementIndex( topPosition );
    return ( centerElementIdx != -1 && topElementIdx != -1 && bottomElementIdx != -1 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFaultReactivationDataAccessorStressGeoMech::interpolatedResultValue( RimWellIADataAccess&      iaDataAccess,
                                                                               const RigFemPart*         femPart,
                                                                               const cvf::Vec3d&         position,
                                                                               const std::vector<float>& scalarResults ) const
{
    return iaDataAccess.interpolatedResultValue( femPart, scalarResults, RIG_ELEMENT_NODAL, position );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, cvf::Vec3d> RimFaultReactivationDataAccessorStressGeoMech::calculatePorBar( const cvf::Vec3d&              position,
                                                                                              double                         gradient,
                                                                                              RimFaultReactivation::GridPart gridPart,
                                                                                              int                            timeStepIndex,
                                                                                              int frameIndex ) const
{
    CAF_ASSERT( m_extractors.find( gridPart ) != m_extractors.end() );
    auto extractor = m_extractors.find( gridPart )->second;

    if ( !extractor->valid() )
    {
        RiaLogging::error( "Invalid extractor when extracting PorBar" );
        return { std::numeric_limits<double>::infinity(), cvf::Vec3d::UNDEFINED };
    }

    RigFemResultAddress resAddr = RigFemAddressDefines::nodalPorBarAddress();
    std::vector<double> values;
    extractor->curveData( resAddr, timeStepIndex, frameIndex, &values );

    auto [value, extractionPos] = RimFaultReactivationDataAccessorWellLogExtraction::calculatePorBar( *m_model,
                                                                                                      gridPart,
                                                                                                      extractor->intersections(),
                                                                                                      values,
                                                                                                      position,
                                                                                                      gradient );

    if ( extractionPos.isUndefined() )
    {
        // If extraction position is not defined the position is not close to the border between the two parts.
        // This means it should be safe to use POR-BAR from the model.
        const std::vector<float>& frameData = m_porBarFrames->frameData( timeStepIndex, frameIndex );

        // Use data from geo mech grid if defined (only position is reservoir).
        RimWellIADataAccess iaDataAccess( m_geoMechCase );
        double              gridValue = iaDataAccess.interpolatedResultValue( m_femPart, frameData, RIG_NODAL, position );
        if ( !std::isinf( gridValue ) )
        {
            return { gridValue, position };
        }

        // Use calculated value when POR-BAR is inf (outside of reservoir).
        return { value, position };
    }
    else
    {
        return { value, extractionPos };
    }
}
