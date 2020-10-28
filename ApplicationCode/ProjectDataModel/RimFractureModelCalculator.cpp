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
#include "RimFractureModelCalculator.h"

#include "RiaDefines.h"
#include "RiaFractureModelDefines.h"
#include "RiaLogging.h"

#include "RigEclipseCaseData.h"

#include "RimEclipseResultDefinition.h"
#include "RimFractureModel.h"
#include "RimFractureModelCalculator.h"
#include "RimFractureModelElasticPropertyCalculator.h"
#include "RimFractureModelLayerCalculator.h"
#include "RimFractureModelPropertyCalculator.h"
#include "RimFractureModelStressCalculator.h"
#include "RimFractureModelWellLogCalculator.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFractureModelCalculator::RimFractureModelCalculator()
{
    m_resultCalculators.push_back(
        std::unique_ptr<RimFractureModelPropertyCalculator>( new RimFractureModelWellLogCalculator( this ) ) );
    m_resultCalculators.push_back(
        std::unique_ptr<RimFractureModelPropertyCalculator>( new RimFractureModelElasticPropertyCalculator( this ) ) );
    m_resultCalculators.push_back(
        std::unique_ptr<RimFractureModelPropertyCalculator>( new RimFractureModelLayerCalculator( this ) ) );
    m_resultCalculators.push_back(
        std::unique_ptr<RimFractureModelPropertyCalculator>( new RimFractureModelStressCalculator( this ) ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModelCalculator::setFractureModel( RimFractureModel* fractureModel )
{
    m_fractureModel = fractureModel;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFractureModel* RimFractureModelCalculator::fractureModel()
{
    return m_fractureModel;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFractureModelCalculator::extractCurveData( RiaDefines::CurveProperty curveProperty,
                                                   int                       timeStep,
                                                   std::vector<double>&      values,
                                                   std::vector<double>&      measuredDepthValues,
                                                   std::vector<double>&      tvDepthValues,
                                                   double&                   rkbDiff ) const
{
    for ( const auto& calculator : m_resultCalculators )
    {
        if ( calculator->isMatching( curveProperty ) )
        {
            return calculator
                ->calculate( curveProperty, m_fractureModel, timeStep, values, measuredDepthValues, tvDepthValues, rkbDiff );
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimFractureModelCalculator::extractValues( RiaDefines::CurveProperty curveProperty, int timeStep ) const
{
    std::vector<double> values;
    std::vector<double> measuredDepthValues;
    std::vector<double> tvDepthValues;
    double              rkbDiff;

    extractCurveData( curveProperty, timeStep, values, measuredDepthValues, tvDepthValues, rkbDiff );

    return values;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModelCalculator::calculateLayers( std::vector<std::pair<double, double>>& layerBoundaryDepths,
                                                  std::vector<std::pair<size_t, size_t>>& layerBoundaryIndexes ) const
{
    std::vector<double> layerValues;
    std::vector<double> measuredDepthValues;
    std::vector<double> depths;
    double              rkbDiff;

    extractCurveData( RiaDefines::CurveProperty::LAYERS,
                      m_fractureModel->timeStep(),
                      layerValues,
                      measuredDepthValues,
                      depths,
                      rkbDiff );

    if ( layerValues.size() != depths.size() ) return;

    size_t startIndex = 0;
    for ( size_t i = 0; i < depths.size(); i++ )
    {
        if ( startIndex != i && ( layerValues[startIndex] != layerValues[i] || i == depths.size() - 1 ) )
        {
            layerBoundaryDepths.push_back( std::make_pair( depths[startIndex], depths[i] ) );
            layerBoundaryIndexes.push_back( std::make_pair( startIndex, i ) );
            startIndex = i;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFractureModelCalculator::findValueAtTopOfLayer( const std::vector<double>&                    values,
                                                          const std::vector<std::pair<size_t, size_t>>& layerBoundaryIndexes,
                                                          size_t                                        layerNo )
{
    size_t index = layerBoundaryIndexes[layerNo].first;
    return values.at( index );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFractureModelCalculator::findValueAtBottomOfLayer( const std::vector<double>&                    values,
                                                             const std::vector<std::pair<size_t, size_t>>& layerBoundaryIndexes,
                                                             size_t                                        layerNo )
{
    size_t index = layerBoundaryIndexes[layerNo].second;
    return values.at( index );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModelCalculator::computeAverageByLayer( const std::vector<std::pair<size_t, size_t>>& layerBoundaryIndexes,
                                                        const std::vector<double>&                    inputVector,
                                                        std::vector<double>&                          result )
{
    for ( auto boundaryIndex : layerBoundaryIndexes )
    {
        double sum     = 0.0;
        int    nValues = 0;
        for ( size_t i = boundaryIndex.first; i < boundaryIndex.second; i++ )
        {
            sum += inputVector[i];
            nValues++;
        }
        result.push_back( sum / nValues );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModelCalculator::extractTopOfLayerValues( const std::vector<std::pair<size_t, size_t>>& layerBoundaryIndexes,
                                                          const std::vector<double>&                    inputVector,
                                                          std::vector<double>&                          result )
{
    for ( size_t i = 0; i < layerBoundaryIndexes.size(); i++ )
    {
        result.push_back( findValueAtTopOfLayer( inputVector, layerBoundaryIndexes, i ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimFractureModelCalculator::calculateTrueVerticalDepth() const
{
    std::vector<std::pair<double, double>> layerBoundaryDepths;
    std::vector<std::pair<size_t, size_t>> layerBoundaryIndexes;

    calculateLayers( layerBoundaryDepths, layerBoundaryIndexes );

    std::vector<double> tvdTopZone;
    for ( auto p : layerBoundaryDepths )
    {
        double depthInFeet = RiaEclipseUnitTools::meterToFeet( p.first );
        tvdTopZone.push_back( depthInFeet );
    }

    return tvdTopZone;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double>
    RimFractureModelCalculator::findCurveAndComputeLayeredAverage( RiaDefines::CurveProperty curveProperty ) const
{
    std::vector<std::pair<double, double>> layerBoundaryDepths;
    std::vector<std::pair<size_t, size_t>> layerBoundaryIndexes;
    calculateLayers( layerBoundaryDepths, layerBoundaryIndexes );

    std::vector<double> values = extractValues( curveProperty, m_fractureModel->timeStep() );
    std::vector<double> result;
    computeAverageByLayer( layerBoundaryIndexes, values, result );
    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimFractureModelCalculator::findCurveAndComputeTopOfLayer( RiaDefines::CurveProperty curveProperty ) const
{
    std::vector<std::pair<double, double>> layerBoundaryDepths;
    std::vector<std::pair<size_t, size_t>> layerBoundaryIndexes;
    calculateLayers( layerBoundaryDepths, layerBoundaryIndexes );

    std::vector<double> values = extractValues( curveProperty, m_fractureModel->timeStep() );
    std::vector<double> result;
    extractTopOfLayerValues( layerBoundaryIndexes, values, result );
    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimFractureModelCalculator::calculatePorosity() const
{
    return findCurveAndComputeLayeredAverage( RiaDefines::CurveProperty::POROSITY );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimFractureModelCalculator::calculateReservoirPressure() const
{
    std::vector<double> pressureBar = findCurveAndComputeTopOfLayer( RiaDefines::CurveProperty::PRESSURE );

    std::vector<double> pressurePsi;
    for ( double p : pressureBar )
    {
        pressurePsi.push_back( RiaEclipseUnitTools::barToPsi( p ) );
    }

    return pressurePsi;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimFractureModelCalculator::calculateHorizontalPermeability() const
{
    return findCurveAndComputeLayeredAverage( RiaDefines::CurveProperty::PERMEABILITY_X );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimFractureModelCalculator::calculateVerticalPermeability() const
{
    return findCurveAndComputeLayeredAverage( RiaDefines::CurveProperty::PERMEABILITY_Z );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimFractureModelCalculator::calculateStress() const
{
    std::vector<double> stress;
    std::vector<double> stressGradients;
    std::vector<double> initialStress;
    calculateStressWithGradients( stress, stressGradients, initialStress );
    return stress;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimFractureModelCalculator::calculateInitialStress() const
{
    std::vector<double> stress;
    std::vector<double> stressGradients;
    std::vector<double> initialStress;
    calculateStressWithGradients( stress, stressGradients, initialStress );
    return initialStress;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFractureModelCalculator::calculateStressWithGradients( std::vector<double>& stress,
                                                               std::vector<double>& stressGradients,
                                                               std::vector<double>& initialStress ) const
{
    // Reference stress
    const double verticalStressRef         = m_fractureModel->verticalStress();
    const double verticalStressGradientRef = m_fractureModel->verticalStressGradient();
    const double stressDepthRef            = m_fractureModel->stressDepth();

    std::vector<std::pair<double, double>> layerBoundaryDepths;
    std::vector<std::pair<size_t, size_t>> layerBoundaryIndexes;
    calculateLayers( layerBoundaryDepths, layerBoundaryIndexes );

    int timeStep = m_fractureModel->timeStep();

    // Biot coefficient
    std::vector<double> biotData = extractValues( RiaDefines::CurveProperty::BIOT_COEFFICIENT, timeStep );

    // K0
    std::vector<double> k0Data = extractValues( RiaDefines::CurveProperty::K0, timeStep );

    // Pressure at the give time step
    std::vector<double> timeStepPressureData = extractValues( RiaDefines::CurveProperty::PRESSURE, timeStep );

    // Initial pressure
    std::vector<double> initialPressureData = extractValues( RiaDefines::CurveProperty::INITIAL_PRESSURE, timeStep );

    // Poissons ratio
    std::vector<double> poissonsRatioData = extractValues( RiaDefines::CurveProperty::POISSONS_RATIO, timeStep );

    // Check that we have data from all curves
    if ( biotData.empty() || k0Data.empty() || timeStepPressureData.empty() || initialPressureData.empty() ||
         poissonsRatioData.empty() )
    {
        return false;
    }

    if ( biotData.size() < layerBoundaryIndexes.size() || k0Data.size() < layerBoundaryIndexes.size() ||
         timeStepPressureData.size() < layerBoundaryIndexes.size() ||
         initialPressureData.size() < layerBoundaryIndexes.size() ||
         poissonsRatioData.size() < layerBoundaryIndexes.size() )
    {
        return false;
    }

    std::vector<double> stressForGradients;
    std::vector<double> pressureForGradients;
    std::vector<double> depthForGradients;

    // Calculate the stress
    for ( size_t i = 0; i < layerBoundaryDepths.size(); i++ )
    {
        double depthTopOfZone    = layerBoundaryDepths[i].first;
        double depthBottomOfZone = layerBoundaryDepths[i].second;

        // Data from curves at the top zone depth
        double k0               = findValueAtTopOfLayer( k0Data, layerBoundaryIndexes, i );
        double biot             = findValueAtTopOfLayer( biotData, layerBoundaryIndexes, i );
        double poissonsRatio    = findValueAtTopOfLayer( poissonsRatioData, layerBoundaryIndexes, i );
        double initialPressure  = findValueAtTopOfLayer( initialPressureData, layerBoundaryIndexes, i );
        double timeStepPressure = findValueAtTopOfLayer( timeStepPressureData, layerBoundaryIndexes, i );

        // Vertical stress
        // Use difference between reference depth and depth of top of zone
        double depthDiff = depthTopOfZone - stressDepthRef;
        double Sv        = verticalStressRef + verticalStressGradientRef * depthDiff;

        double Sh_init      = k0 * Sv + initialPressure * ( 1.0 - k0 );
        double pressureDiff = timeStepPressure - initialPressure;

        // Vertical stress diff assumed to be zero
        double Sv_diff               = 0.0;
        double deltaHorizontalStress = poissonsRatio / ( 1.0 - poissonsRatio ) * ( Sv_diff - biot * pressureDiff ) +
                                       ( biot * pressureDiff );

        double depletionStress = Sh_init + deltaHorizontalStress;
        stress.push_back( RiaEclipseUnitTools::barToPsi( depletionStress ) );

        initialStress.push_back( RiaEclipseUnitTools::barToPsi( Sh_init ) );

        // Cache some results for the gradients calculation
        stressForGradients.push_back( Sv );
        pressureForGradients.push_back( initialPressure );
        depthForGradients.push_back( depthTopOfZone );

        if ( i == layerBoundaryDepths.size() - 1 )
        {
            // Use the bottom of the last layer to compute gradient for last layer
            double bottomInitialPressure = findValueAtBottomOfLayer( initialPressureData, layerBoundaryIndexes, i );

            double bottomDepthDiff = depthBottomOfZone - stressDepthRef;
            double bottomSv        = verticalStressRef + verticalStressGradientRef * bottomDepthDiff;
            stressForGradients.push_back( bottomSv );
            pressureForGradients.push_back( bottomInitialPressure );
            depthForGradients.push_back( depthBottomOfZone );
        }
    }

    assert( stressForGradients.size() == layerBoundaryDepths.size() + 1 );
    assert( pressureForGradients.size() == layerBoundaryDepths.size() + 1 );
    assert( depthForGradients.size() == layerBoundaryDepths.size() + 1 );

    // Second pass to calculate the stress gradients
    for ( size_t i = 0; i < layerBoundaryDepths.size(); i++ )
    {
        double diffStress     = stressForGradients[i + 1] - stressForGradients[i];
        double diffPressure   = pressureForGradients[i + 1] - pressureForGradients[i];
        double diffDepth      = depthForGradients[i + 1] - depthForGradients[i];
        double k0             = findValueAtTopOfLayer( k0Data, layerBoundaryIndexes, i );
        double stressGradient = ( diffStress * k0 + diffPressure * ( 1.0 - k0 ) ) / diffDepth;
        stressGradients.push_back( RiaEclipseUnitTools::barPerMeterToPsiPerFeet( stressGradient ) );
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimFractureModelCalculator::calculateStressGradient() const
{
    std::vector<double> stress;
    std::vector<double> stressGradients;
    std::vector<double> initialStress;
    calculateStressWithGradients( stress, stressGradients, initialStress );
    return stressGradients;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModelCalculator::calculateTemperature( std::vector<double>& temperatures ) const
{
    // Reference temperature. Unit: degrees celsius
    const double referenceTemperature = m_fractureModel->referenceTemperature();

    // Reference temperature gradient. Unit: degrees Celsius per meter
    const double referenceTemperatureGradient = m_fractureModel->referenceTemperatureGradient();

    // Reference depth for temperature. Unit: meter.
    const double referenceTemperatureDepth = m_fractureModel->referenceTemperatureDepth();

    std::vector<std::pair<double, double>> layerBoundaryDepths;
    std::vector<std::pair<size_t, size_t>> layerBoundaryIndexes;
    calculateLayers( layerBoundaryDepths, layerBoundaryIndexes );

    // Calculate the temperatures
    for ( size_t i = 0; i < layerBoundaryDepths.size(); i++ )
    {
        double depthTopOfZone = layerBoundaryDepths[i].first;

        // Use difference between reference depth and depth of top of zone
        double depthDiff   = depthTopOfZone - referenceTemperatureDepth;
        double temperature = referenceTemperature + referenceTemperatureGradient * depthDiff;

        temperatures.push_back( temperature );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimFractureModelCalculator::calculateYoungsModulus() const
{
    std::vector<double> valuesGPa = findCurveAndComputeLayeredAverage( RiaDefines::CurveProperty::YOUNGS_MODULUS );
    std::vector<double> valuesMMpsi;
    for ( auto value : valuesGPa )
    {
        valuesMMpsi.push_back( value * 0.14503773773 );
    }

    return valuesMMpsi;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimFractureModelCalculator::calculatePoissonsRatio() const
{
    return findCurveAndComputeLayeredAverage( RiaDefines::CurveProperty::POISSONS_RATIO );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimFractureModelCalculator::calculateKIc() const
{
    return findCurveAndComputeLayeredAverage( RiaDefines::CurveProperty::K_IC );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimFractureModelCalculator::calculateFluidLossCoefficient() const
{
    return findCurveAndComputeLayeredAverage( RiaDefines::CurveProperty::FLUID_LOSS_COEFFICIENT );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimFractureModelCalculator::calculateSpurtLoss() const
{
    return findCurveAndComputeLayeredAverage( RiaDefines::CurveProperty::SPURT_LOSS );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimFractureModelCalculator::calculateProppandEmbedment() const
{
    return findCurveAndComputeLayeredAverage( RiaDefines::CurveProperty::PROPPANT_EMBEDMENT );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimFractureModelCalculator::calculateImmobileFluidSaturation() const
{
    return findCurveAndComputeLayeredAverage( RiaDefines::CurveProperty::IMMOBILE_FLUID_SATURATION );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimFractureModelCalculator::calculateTemperature() const
{
    std::vector<double> temperaturesCelsius;
    calculateTemperature( temperaturesCelsius );

    // Convert to Fahrenheit
    std::vector<double> temperaturesFahrenheit;
    for ( double t : temperaturesCelsius )
    {
        temperaturesFahrenheit.push_back( t * 1.8 + 32.0 );
    }

    return temperaturesFahrenheit;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimFractureModelCalculator::calculateRelativePermeabilityFactor() const
{
    return findCurveAndComputeLayeredAverage( RiaDefines::CurveProperty::RELATIVE_PERMEABILITY_FACTOR );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimFractureModelCalculator::calculatePoroElasticConstant() const
{
    return findCurveAndComputeLayeredAverage( RiaDefines::CurveProperty::PORO_ELASTIC_CONSTANT );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimFractureModelCalculator::calculateThermalExpansionCoefficient() const
{
    // SI unit is 1/Celsius
    std::vector<double> coefficientCelsius =
        findCurveAndComputeLayeredAverage( RiaDefines::CurveProperty::THERMAL_EXPANSION_COEFFICIENT );

    // Field unit is 1/Fahrenheit
    std::vector<double> coefficientFahrenheit;
    for ( double c : coefficientCelsius )
    {
        coefficientFahrenheit.push_back( c / 1.8 );
    }

    return coefficientFahrenheit;
}
