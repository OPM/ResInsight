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
#include "RimFractureModelStressCalculator.h"

#include "RiaDefines.h"
#include "RiaFractureModelDefines.h"

#include "RigEclipseCaseData.h"
#include "RigEclipseWellLogExtractor.h"
#include "RigWellPath.h"

#include "RigWellPathGeometryTools.h"
#include "RimCase.h"
#include "RimEclipseCase.h"
#include "RimFractureModel.h"
#include "RimFractureModelCalculator.h"
#include "RimFractureModelStressCalculator.h"
#include "RimModeledWellPath.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFractureModelStressCalculator::RimFractureModelStressCalculator( RimFractureModelCalculator* fractureModelCalculator )
    : m_fractureModelCalculator( fractureModelCalculator )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFractureModelStressCalculator::isMatching( RiaDefines::CurveProperty curveProperty ) const
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
bool RimFractureModelStressCalculator::calculate( RiaDefines::CurveProperty curveProperty,
                                                  const RimFractureModel*   fractureModel,
                                                  int                       timeStep,
                                                  std::vector<double>&      values,
                                                  std::vector<double>&      measuredDepthValues,
                                                  std::vector<double>&      tvDepthValues,
                                                  double&                   rkbDiff ) const
{
    RimEclipseCase* eclipseCase = fractureModel->eclipseCase();
    if ( !eclipseCase )
    {
        return false;
    }

    std::vector<double> tvDepthInFeet = m_fractureModelCalculator->calculateTrueVerticalDepth();
    for ( double f : tvDepthInFeet )
    {
        tvDepthValues.push_back( RiaEclipseUnitTools::feetToMeter( f ) );
    }

    if ( curveProperty == RiaDefines::CurveProperty::STRESS )
    {
        values                              = m_fractureModelCalculator->calculateStress();
        std::vector<double> stressGradients = m_fractureModelCalculator->calculateStressGradient();
        addDatapointsForBottomOfLayers( tvDepthValues, values, stressGradients );
    }
    else if ( curveProperty == RiaDefines::CurveProperty::INITIAL_STRESS )
    {
        values                              = m_fractureModelCalculator->calculateInitialStress();
        std::vector<double> stressGradients = m_fractureModelCalculator->calculateStressGradient();
        addDatapointsForBottomOfLayers( tvDepthValues, values, stressGradients );
    }
    else if ( curveProperty == RiaDefines::CurveProperty::STRESS_GRADIENT )
    {
        values = m_fractureModelCalculator->calculateStressGradient();
    }
    else if ( curveProperty == RiaDefines::CurveProperty::TEMPERATURE )
    {
        m_fractureModelCalculator->calculateTemperature( values );
    }

    if ( eclipseCase )
    {
        RigWellPath*               wellPathGeometry = fractureModel->thicknessDirectionWellPath()->wellPathGeometry();
        RigEclipseWellLogExtractor eclExtractor( eclipseCase->eclipseCaseData(), wellPathGeometry, "fracture model" );

        rkbDiff = wellPathGeometry->rkbDiff();

        // Generate MD data by interpolation
        const std::vector<double>& mdValuesOfWellPath  = wellPathGeometry->measuredDepths();
        std::vector<double>        tvdValuesOfWellPath = wellPathGeometry->trueVerticalDepths();

        measuredDepthValues =
            RigWellPathGeometryTools::interpolateMdFromTvd( mdValuesOfWellPath, tvdValuesOfWellPath, tvDepthValues );
        CVF_ASSERT( measuredDepthValues.size() == tvDepthValues.size() );
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModelStressCalculator::addDatapointsForBottomOfLayers( std::vector<double>&       tvDepthValues,
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
