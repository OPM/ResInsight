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
#include "RimStimPlanModelStressCalculator.h"

#include "RiaDefines.h"
#include "RiaStimPlanModelDefines.h"
#include "RiaLogging.h"

#include "RigEclipseCaseData.h"
#include "RigEclipseWellLogExtractor.h"
#include "RigWellPath.h"

#include "RigWellPathGeometryTools.h"
#include "RimCase.h"
#include "RimEclipseCase.h"
#include "RimStimPlanModel.h"
#include "RimStimPlanModelCalculator.h"
#include "RimStimPlanModelStressCalculator.h"
#include "RimModeledWellPath.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStimPlanModelStressCalculator::RimStimPlanModelStressCalculator( RimStimPlanModelCalculator* stimPlanModelCalculator )
    : m_stimPlanModelCalculator( stimPlanModelCalculator )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimStimPlanModelStressCalculator::isMatching( RiaDefines::CurveProperty curveProperty ) const
{
    std::vector<RiaDefines::CurveProperty> matching = {RiaDefines::CurveProperty::INITIAL_STRESS,
                                                       RiaDefines::CurveProperty::STRESS,
                                                       RiaDefines::CurveProperty::STRESS_GRADIENT,
                                                       RiaDefines::CurveProperty::TEMPERATURE};

    return std::find( matching.begin(), matching.end(), curveProperty ) != matching.end();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimStimPlanModelStressCalculator::calculate( RiaDefines::CurveProperty curveProperty,
                                                  const RimStimPlanModel*   stimPlanModel,
                                                  int                       timeStep,
                                                  std::vector<double>&      values,
                                                  std::vector<double>&      measuredDepthValues,
                                                  std::vector<double>&      tvDepthValues,
                                                  double&                   rkbDiff ) const
{
    RimEclipseCase* eclipseCase = stimPlanModel->eclipseCase();
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
        RiaLogging::error( "No well path geometry found for stress data exctration." );
        return false;
    }

    std::vector<double> tvDepthInFeet = m_stimPlanModelCalculator->calculateTrueVerticalDepth();
    for ( double f : tvDepthInFeet )
    {
        tvDepthValues.push_back( RiaEclipseUnitTools::feetToMeter( f ) );
    }

    if ( curveProperty == RiaDefines::CurveProperty::STRESS )
    {
        values                              = m_stimPlanModelCalculator->calculateStress();
        std::vector<double> stressGradients = m_stimPlanModelCalculator->calculateStressGradient();
        addDatapointsForBottomOfLayers( tvDepthValues, values, stressGradients );
    }
    else if ( curveProperty == RiaDefines::CurveProperty::INITIAL_STRESS )
    {
        values                              = m_stimPlanModelCalculator->calculateInitialStress();
        std::vector<double> stressGradients = m_stimPlanModelCalculator->calculateStressGradient();
        addDatapointsForBottomOfLayers( tvDepthValues, values, stressGradients );
    }
    else if ( curveProperty == RiaDefines::CurveProperty::STRESS_GRADIENT )
    {
        values = m_stimPlanModelCalculator->calculateStressGradient();
    }
    else if ( curveProperty == RiaDefines::CurveProperty::TEMPERATURE )
    {
        m_stimPlanModelCalculator->calculateTemperature( values );
    }

    if ( eclipseCase )
    {
        RigEclipseWellLogExtractor eclExtractor( eclipseCase->eclipseCaseData(), wellPathGeometry, "fracture model" );

        rkbDiff = wellPathGeometry->rkbDiff();

        // Generate MD data by interpolation
        const std::vector<double>& mdValuesOfWellPath  = wellPathGeometry->measuredDepths();
        std::vector<double>        tvdValuesOfWellPath = wellPathGeometry->trueVerticalDepths();
        if ( mdValuesOfWellPath.empty() )
        {
            RiaLogging::error( "Well path geometry had no MD values." );
            return false;
        }

        measuredDepthValues =
            RigWellPathGeometryTools::interpolateMdFromTvd( mdValuesOfWellPath, tvdValuesOfWellPath, tvDepthValues );
        CVF_ASSERT( measuredDepthValues.size() == tvDepthValues.size() );
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelStressCalculator::addDatapointsForBottomOfLayers( std::vector<double>&       tvDepthValues,
                                                                       std::vector<double>&       stress,
                                                                       const std::vector<double>& stressGradients )
{
    std::vector<double> tvdWithBottomLayers;
    std::vector<double> valuesWithBottomLayers;
    for ( size_t i = 0; i < stress.size(); i++ )
    {
        // Add the data point at top of the layer
        double topLayerDepth = tvDepthValues[i];
        double stressValue   = stress[i];
        tvdWithBottomLayers.push_back( topLayerDepth );
        valuesWithBottomLayers.push_back( stressValue );

        // Add extra data points for bottom part of the layer
        if ( i < stress.size() - 1 )
        {
            double bottomLayerDepth = tvDepthValues[i + 1];
            double diffDepthFeet    = RiaEclipseUnitTools::meterToFeet( bottomLayerDepth - topLayerDepth );
            double bottomStress     = stressValue + diffDepthFeet * stressGradients[i];

            tvdWithBottomLayers.push_back( bottomLayerDepth );
            valuesWithBottomLayers.push_back( bottomStress );
        }
    }

    stress        = valuesWithBottomLayers;
    tvDepthValues = tvdWithBottomLayers;
}
