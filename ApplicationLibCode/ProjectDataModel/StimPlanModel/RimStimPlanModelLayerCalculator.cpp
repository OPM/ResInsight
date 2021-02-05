/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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
#include "RimStimPlanModelLayerCalculator.h"

#include "RiaDefines.h"
#include "RiaLogging.h"
#include "RiaStimPlanModelDefines.h"

#include "RigEclipseCaseData.h"
#include "RigEclipseWellLogExtractor.h"
#include "RigResultAccessor.h"
#include "RigResultAccessorFactory.h"
#include "RigWellPath.h"

#include "RimCase.h"
#include "RimEclipseCase.h"
#include "RimEclipseResultDefinition.h"
#include "RimModeledWellPath.h"
#include "RimNonNetLayers.h"
#include "RimStimPlanModel.h"
#include "RimStimPlanModelCalculator.h"
#include "RimStimPlanModelLayerCalculator.h"
#include "RimStimPlanModelTemplate.h"
#include "RimWellLogTrack.h"
#include "RimWellPath.h"

#include "cafAssert.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStimPlanModelLayerCalculator::RimStimPlanModelLayerCalculator( RimStimPlanModelCalculator* stimPlanModelCalculator )
    : m_stimPlanModelCalculator( stimPlanModelCalculator )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimStimPlanModelLayerCalculator::isMatching( RiaDefines::CurveProperty curveProperty ) const
{
    return curveProperty == RiaDefines::CurveProperty::LAYERS;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimStimPlanModelLayerCalculator::calculate( RiaDefines::CurveProperty curveProperty,
                                                 const RimStimPlanModel*   stimPlanModel,
                                                 int                       timeStep,
                                                 std::vector<double>&      values,
                                                 std::vector<double>&      measuredDepthValues,
                                                 std::vector<double>&      tvDepthValues,
                                                 double&                   rkbDiff ) const
{
    RimEclipseCase* eclipseCase = stimPlanModel->eclipseCaseForProperty( RiaDefines::CurveProperty::FACIES );
    if ( !eclipseCase )
    {
        return false;
    }

    if ( !stimPlanModel->thicknessDirectionWellPath() )
    {
        return false;
    }

    RigWellPath* wellPathGeometry = stimPlanModel->thicknessDirectionWellPath()->wellPathGeometry();
    if ( !wellPathGeometry )
    {
        RiaLogging::error( "No well path geometry found for layer data exctration." );
        return false;
    }

    RigEclipseWellLogExtractor eclExtractor( eclipseCase->eclipseCaseData(), wellPathGeometry, "fracture model" );

    rkbDiff = eclExtractor.wellPathGeometry()->rkbDiff();

    // Extract formation data
    cvf::ref<RigResultAccessor> formationResultAccessor =
        RigResultAccessorFactory::createFromResultAddress( eclipseCase->eclipseCaseData(),
                                                           0,
                                                           RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                           0,
                                                           RigEclipseResultAddress( RiaDefines::ResultCatType::FORMATION_NAMES,
                                                                                    RiaResultNames::activeFormationNamesResultName() ) );
    if ( !formationResultAccessor.notNull() )
    {
        RiaLogging::error( QString( "No formation result found." ) );
        return false;
    }

    CurveSamplingPointData curveData =
        RimWellLogTrack::curveSamplingPointData( &eclExtractor, formationResultAccessor.p() );

    std::vector<QString> formationNamesVector = RimWellLogTrack::formationNamesVector( eclipseCase );

    double overburdenHeight = stimPlanModel->overburdenHeight();
    if ( overburdenHeight > 0.0 )
    {
        RimWellLogTrack::addOverburden( formationNamesVector, curveData, overburdenHeight );
    }

    double underburdenHeight = stimPlanModel->underburdenHeight();
    if ( underburdenHeight > 0.0 )
    {
        RimWellLogTrack::addUnderburden( formationNamesVector, curveData, underburdenHeight );
    }

    // Extract facies data
    std::vector<double> faciesValues =
        m_stimPlanModelCalculator->extractValues( RiaDefines::CurveProperty::FACIES, timeStep );
    if ( faciesValues.empty() )
    {
        RiaLogging::error( QString( "Empty facies data found for layer curve." ) );
        return false;
    }

    std::vector<double> netToGrossValues =
        m_stimPlanModelCalculator->extractValues( RiaDefines::CurveProperty::NET_TO_GROSS, timeStep );
    if ( netToGrossValues.empty() )
    {
        RiaLogging::warning( QString( "Empty net-to-gross data found for layer curve." ) );
    }

    measuredDepthValues = curveData.md;
    tvDepthValues       = curveData.tvd;

    CAF_ASSERT( faciesValues.size() == curveData.data.size() );

    values.resize( faciesValues.size() );

    int    layerNo            = 0;
    double previousFormation  = -1.0;
    double previousFacies     = -1.0;
    double previousNetToGross = -1.0;
    double netToGrossCutoff   = 1.0;
    bool   useNetToGross      = false;

    if ( stimPlanModel->stimPlanModelTemplate() && stimPlanModel->stimPlanModelTemplate()->nonNetLayers() )
    {
        netToGrossCutoff = stimPlanModel->stimPlanModelTemplate()->nonNetLayers()->cutOff();
        useNetToGross = !netToGrossValues.empty() && stimPlanModel->stimPlanModelTemplate()->nonNetLayers()->isChecked();
    }

    for ( size_t i = 0; i < faciesValues.size(); i++ )
    {
        if ( previousFormation != curveData.data[i] || previousFacies != faciesValues[i] ||
             ( useNetToGross && netToGrossValues[i] <= netToGrossCutoff && previousNetToGross != netToGrossValues[i] ) )
        {
            layerNo++;
        }

        values[i]         = layerNo;
        previousFormation = curveData.data[i];
        previousFacies    = faciesValues[i];
        if ( useNetToGross )
        {
            previousNetToGross = netToGrossValues[i];
        }
    }

    return true;
}
