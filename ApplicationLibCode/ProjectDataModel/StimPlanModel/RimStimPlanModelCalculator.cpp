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
#include "RimStimPlanModelCalculator.h"

#include "RiaDefines.h"
#include "RiaEclipseUnitTools.h"
#include "RiaLogging.h"
#include "RiaStimPlanModelDefines.h"

#include "RigEclipseCaseData.h"

#include "RimColorLegend.h"
#include "RimEclipseCase.h"
#include "RimEclipseResultDefinition.h"
#include "RimFaciesProperties.h"
#include "RimStimPlanModel.h"
#include "RimStimPlanModelCalculator.h"
#include "RimStimPlanModelElasticPropertyCalculator.h"
#include "RimStimPlanModelLayerCalculator.h"
#include "RimStimPlanModelPressureCalculator.h"
#include "RimStimPlanModelPropertyCalculator.h"
#include "RimStimPlanModelStressCalculator.h"
#include "RimStimPlanModelTemplate.h"
#include "RimStimPlanModelWellLogCalculator.h"
#include "RimWellLogTrack.h"

#include <cmath>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStimPlanModelCalculator::RimStimPlanModelCalculator()
{
    m_resultCalculators.push_back(
        std::unique_ptr<RimStimPlanModelPropertyCalculator>( new RimStimPlanModelWellLogCalculator( this ) ) );
    m_resultCalculators.push_back(
        std::unique_ptr<RimStimPlanModelPropertyCalculator>( new RimStimPlanModelPressureCalculator( this ) ) );
    m_resultCalculators.push_back(
        std::unique_ptr<RimStimPlanModelPropertyCalculator>( new RimStimPlanModelElasticPropertyCalculator( this ) ) );
    m_resultCalculators.push_back(
        std::unique_ptr<RimStimPlanModelPropertyCalculator>( new RimStimPlanModelLayerCalculator( this ) ) );
    m_resultCalculators.push_back(
        std::unique_ptr<RimStimPlanModelPropertyCalculator>( new RimStimPlanModelStressCalculator( this ) ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelCalculator::setStimPlanModel( RimStimPlanModel* stimPlanModel )
{
    m_stimPlanModel = stimPlanModel;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStimPlanModel* RimStimPlanModelCalculator::stimPlanModel()
{
    return m_stimPlanModel;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimStimPlanModelCalculator::extractCurveData( RiaDefines::CurveProperty curveProperty,
                                                   int                       timeStep,
                                                   std::vector<double>&      values,
                                                   std::vector<double>&      measuredDepthValues,
                                                   std::vector<double>&      tvDepthValues,
                                                   double&                   rkbDiff ) const
{
    ResultKey key  = std::make_pair( curveProperty, timeStep );
    auto      data = m_resultCache.find( key );
    if ( data != m_resultCache.end() )
    {
        // Cache hit: reuse previous result
        auto [cachedValues, cachedMeasuredDepthValues, cachedTvDepthValues, cachedRkbDiff] = data->second;

        values              = cachedValues;
        measuredDepthValues = cachedMeasuredDepthValues;
        tvDepthValues       = cachedTvDepthValues;
        rkbDiff             = cachedRkbDiff;
        return true;
    }
    else
    {
        // Cache miss: try to calculate the request data
        for ( const auto& calculator : m_resultCalculators )
        {
            if ( calculator->isMatching( curveProperty ) )
            {
                bool isOk = calculator->calculate( curveProperty,
                                                   m_stimPlanModel,
                                                   timeStep,
                                                   values,
                                                   measuredDepthValues,
                                                   tvDepthValues,
                                                   rkbDiff );

                if ( isOk )
                {
                    // Populate cache when calculation succeeds
                    m_resultCache[key] = std::make_tuple( values, measuredDepthValues, tvDepthValues, rkbDiff );
                }

                return isOk;
            }
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelCalculator::clearCache()
{
    m_resultCache.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimStimPlanModelCalculator::extractValues( RiaDefines::CurveProperty curveProperty, int timeStep ) const
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
void RimStimPlanModelCalculator::calculateLayers( std::vector<std::pair<double, double>>& layerBoundaryDepths,
                                                  std::vector<std::pair<size_t, size_t>>& layerBoundaryIndexes ) const
{
    std::vector<double> layerValues;
    std::vector<double> measuredDepthValues;
    std::vector<double> depths;
    double              rkbDiff;

    extractCurveData( RiaDefines::CurveProperty::LAYERS,
                      m_stimPlanModel->timeStep(),
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
double RimStimPlanModelCalculator::findValueAtTopOfLayer( const std::vector<double>&                    values,
                                                          const std::vector<std::pair<size_t, size_t>>& layerBoundaryIndexes,
                                                          size_t                                        layerNo )
{
    size_t index = layerBoundaryIndexes[layerNo].first;
    return values.at( index );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModelCalculator::findValueAtBottomOfLayer( const std::vector<double>&                    values,
                                                             const std::vector<std::pair<size_t, size_t>>& layerBoundaryIndexes,
                                                             size_t                                        layerNo )
{
    size_t index = layerBoundaryIndexes[layerNo].second;
    return values.at( index );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelCalculator::computeAverageByLayer( const std::vector<std::pair<size_t, size_t>>& layerBoundaryIndexes,
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
void RimStimPlanModelCalculator::extractTopOfLayerValues( const std::vector<std::pair<size_t, size_t>>& layerBoundaryIndexes,
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
std::vector<double> RimStimPlanModelCalculator::calculateTrueVerticalDepth() const
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
    RimStimPlanModelCalculator::findCurveAndComputeLayeredAverage( RiaDefines::CurveProperty curveProperty ) const
{
    std::vector<std::pair<double, double>> layerBoundaryDepths;
    std::vector<std::pair<size_t, size_t>> layerBoundaryIndexes;
    calculateLayers( layerBoundaryDepths, layerBoundaryIndexes );

    std::vector<double> values = extractValues( curveProperty, m_stimPlanModel->timeStep() );
    std::vector<double> result;
    computeAverageByLayer( layerBoundaryIndexes, values, result );
    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimStimPlanModelCalculator::findCurveAndComputeTopOfLayer( RiaDefines::CurveProperty curveProperty ) const
{
    std::vector<std::pair<double, double>> layerBoundaryDepths;
    std::vector<std::pair<size_t, size_t>> layerBoundaryIndexes;
    calculateLayers( layerBoundaryDepths, layerBoundaryIndexes );

    std::vector<double> values = extractValues( curveProperty, m_stimPlanModel->timeStep() );
    std::vector<double> result;
    extractTopOfLayerValues( layerBoundaryIndexes, values, result );
    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimStimPlanModelCalculator::calculatePorosity() const
{
    return findCurveAndComputeLayeredAverage( RiaDefines::CurveProperty::POROSITY );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimStimPlanModelCalculator::calculateReservoirPressure() const
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
std::vector<double> RimStimPlanModelCalculator::calculateHorizontalPermeability() const
{
    return findCurveAndComputeLayeredAverage( RiaDefines::CurveProperty::PERMEABILITY_X );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimStimPlanModelCalculator::calculateVerticalPermeability() const
{
    return findCurveAndComputeLayeredAverage( RiaDefines::CurveProperty::PERMEABILITY_Z );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimStimPlanModelCalculator::calculateStress() const
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
std::vector<double> RimStimPlanModelCalculator::calculateInitialStress() const
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
bool RimStimPlanModelCalculator::calculateStressWithGradients( std::vector<double>& stress,
                                                               std::vector<double>& stressGradients,
                                                               std::vector<double>& initialStress ) const
{
    // Reference stress
    const double verticalStressRef         = m_stimPlanModel->verticalStress();
    const double verticalStressGradientRef = m_stimPlanModel->verticalStressGradient();
    const double stressDepthRef            = m_stimPlanModel->stressDepth();

    std::vector<std::pair<double, double>> layerBoundaryDepths;
    std::vector<std::pair<size_t, size_t>> layerBoundaryIndexes;
    calculateLayers( layerBoundaryDepths, layerBoundaryIndexes );

    int timeStep = m_stimPlanModel->timeStep();

    std::vector<RiaDefines::CurveProperty> inputProperties = { RiaDefines::CurveProperty::BIOT_COEFFICIENT,
                                                               RiaDefines::CurveProperty::K0,
                                                               RiaDefines::CurveProperty::PRESSURE,
                                                               RiaDefines::CurveProperty::INITIAL_PRESSURE,
                                                               RiaDefines::CurveProperty::POISSONS_RATIO,
                                                               RiaDefines::CurveProperty::PRESSURE_GRADIENT };
    std::map<RiaDefines::CurveProperty, std::vector<double>> inputData;
    for ( auto inputProperty : inputProperties )
    {
        QString             propertyName = caf::AppEnum<RiaDefines::CurveProperty>::uiText( inputProperty );
        std::vector<double> values       = extractValues( inputProperty, timeStep );
        // Check that we have data for the curve
        if ( values.empty() )
        {
            RiaLogging::error(
                QString( "Unexpected empty input data for %1 in stress calculation for StimPlan Model: %2" )
                    .arg( propertyName )
                    .arg( m_stimPlanModel->name() ) );
            return false;
        }

        // Check that there is enough data
        if ( values.size() < layerBoundaryIndexes.size() )
        {
            RiaLogging::error(
                QString( "Unexpected input data size for %1 in stress calculation for StimPlan Model: %2" )
                    .arg( propertyName )
                    .arg( m_stimPlanModel->name() ) );
            return false;
        }

        // Pressure gradient is allowed to have infs.
        if ( inputProperty != RiaDefines::CurveProperty::PRESSURE_GRADIENT &&
             !isValidInputData( values, propertyName, layerBoundaryIndexes, layerBoundaryDepths ) )
        {
            RiaLogging::error( QString( "Unexpected bad data for %1 in stress calculation for StimPlan Model: %2." )
                                   .arg( propertyName )
                                   .arg( m_stimPlanModel->name() ) );
            return false;
        }

        inputData[inputProperty] = values;
    }

    // Calculate the stress
    for ( size_t i = 0; i < layerBoundaryDepths.size(); i++ )
    {
        double depthTopOfZone = layerBoundaryDepths[i].first;

        // Data from curves at the top zone depth
        double k0 = findValueAtTopOfLayer( inputData[RiaDefines::CurveProperty::K0], layerBoundaryIndexes, i );
        double biot =
            findValueAtTopOfLayer( inputData[RiaDefines::CurveProperty::BIOT_COEFFICIENT], layerBoundaryIndexes, i );
        double poissonsRatio =
            findValueAtTopOfLayer( inputData[RiaDefines::CurveProperty::POISSONS_RATIO], layerBoundaryIndexes, i );
        double initialPressure =
            findValueAtTopOfLayer( inputData[RiaDefines::CurveProperty::INITIAL_PRESSURE], layerBoundaryIndexes, i );
        double timeStepPressure =
            findValueAtTopOfLayer( inputData[RiaDefines::CurveProperty::PRESSURE], layerBoundaryIndexes, i );

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
        if ( std::isnan( deltaHorizontalStress ) || std::isinf( deltaHorizontalStress ) )
        {
            RiaLogging::warning( "Invalid horizontal stress delta calculated. Setting to zero." );
            deltaHorizontalStress = 0.0;
        }

        double depletionStress = Sh_init + deltaHorizontalStress;
        stress.push_back( RiaEclipseUnitTools::barToPsi( depletionStress ) );

        initialStress.push_back( RiaEclipseUnitTools::barToPsi( Sh_init ) );

        // Calculate the stress gradient inside each layer
        double depthBottomOfZone = layerBoundaryDepths[i].second;
        double stressGradient    = calculateStressGradientForLayer( i,
                                                                 layerBoundaryIndexes,
                                                                 depthTopOfZone,
                                                                 depthBottomOfZone,
                                                                 Sv,
                                                                 inputData[RiaDefines::CurveProperty::INITIAL_PRESSURE],
                                                                 inputData[RiaDefines::CurveProperty::PRESSURE_GRADIENT],
                                                                 stressDepthRef,
                                                                 verticalStressRef,
                                                                 verticalStressGradientRef,
                                                                 k0 );
        stressGradients.push_back( RiaEclipseUnitTools::barPerMeterToPsiPerFeet( stressGradient ) );
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModelCalculator::calculateStressGradientForLayer( size_t                                 i,
                                                                    std::vector<std::pair<size_t, size_t>> layerBoundaryIndexes,
                                                                    double                     depthTopOfZone,
                                                                    double                     depthBottomOfZone,
                                                                    double                     topSv,
                                                                    const std::vector<double>& initialPressureData,
                                                                    const std::vector<double>& pressureDiffData,
                                                                    double                     stressDepthRef,
                                                                    double                     verticalStressRef,
                                                                    double verticalStressGradientRef,
                                                                    double k0 ) const
{
    double bottomInitialPressure = findValueAtBottomOfLayer( initialPressureData, layerBoundaryIndexes, i );
    double topInitialPressure    = findValueAtBottomOfLayer( initialPressureData, layerBoundaryIndexes, i );

    double bottomDepthDiff = depthBottomOfZone - stressDepthRef;
    double bottomSv        = verticalStressRef + verticalStressGradientRef * bottomDepthDiff;

    double lengthOfLayer     = depthBottomOfZone - depthTopOfZone;
    double diffStressLayer   = bottomSv - topSv;
    double diffPressureLayer = bottomInitialPressure - topInitialPressure;

    // The pressure difference result is only defined where there was no pressure data.
    // Diff pressure is interpolated in the equilibration region.
    double diffPressureEQLTop    = findValueAtTopOfLayer( pressureDiffData, layerBoundaryIndexes, i );
    double diffPressureEQLBottom = findValueAtBottomOfLayer( pressureDiffData, layerBoundaryIndexes, i );
    if ( !std::isinf( diffPressureEQLTop ) )
    {
        double offset     = RimStimPlanModelPressureCalculator::pressureDifferenceInterpolationOffset();
        lengthOfLayer     = offset * 2.0;
        diffPressureLayer = diffPressureEQLTop;
        diffStressLayer   = calculateStressDifferenceAtDepth( depthTopOfZone,
                                                            offset,
                                                            stressDepthRef,
                                                            verticalStressRef,
                                                            verticalStressGradientRef );
    }
    else if ( !std::isinf( diffPressureEQLBottom ) )
    {
        double offset     = RimStimPlanModelPressureCalculator::pressureDifferenceInterpolationOffset();
        lengthOfLayer     = offset * 2.0;
        diffPressureLayer = diffPressureEQLBottom;
        diffStressLayer   = calculateStressDifferenceAtDepth( depthBottomOfZone,
                                                            offset,
                                                            stressDepthRef,
                                                            verticalStressRef,
                                                            verticalStressGradientRef );
    }

    double stressGradient = ( diffStressLayer * k0 + diffPressureLayer * ( 1.0 - k0 ) ) / lengthOfLayer;
    return stressGradient;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModelCalculator::calculateStressDifferenceAtDepth( double depth,
                                                                     double offset,
                                                                     double stressDepthRef,
                                                                     double verticalStressRef,
                                                                     double verticalStressGradientRef )
{
    return calculateStressAtDepth( depth + offset, stressDepthRef, verticalStressRef, verticalStressGradientRef ) -
           calculateStressAtDepth( depth - offset, stressDepthRef, verticalStressRef, verticalStressGradientRef );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimStimPlanModelCalculator::calculateStressGradient() const
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
void RimStimPlanModelCalculator::calculateTemperature( std::vector<double>& temperatures ) const
{
    // Reference temperature. Unit: degrees celsius
    const double referenceTemperature = m_stimPlanModel->referenceTemperature();

    // Reference temperature gradient. Unit: degrees Celsius per meter
    const double referenceTemperatureGradient = m_stimPlanModel->referenceTemperatureGradient();

    // Reference depth for temperature. Unit: meter.
    const double referenceTemperatureDepth = m_stimPlanModel->referenceTemperatureDepth();

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
std::vector<double> RimStimPlanModelCalculator::calculateYoungsModulus() const
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
std::vector<double> RimStimPlanModelCalculator::calculatePoissonsRatio() const
{
    return findCurveAndComputeLayeredAverage( RiaDefines::CurveProperty::POISSONS_RATIO );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimStimPlanModelCalculator::calculateKIc() const
{
    return findCurveAndComputeLayeredAverage( RiaDefines::CurveProperty::K_IC );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimStimPlanModelCalculator::calculateFluidLossCoefficient() const
{
    return findCurveAndComputeLayeredAverage( RiaDefines::CurveProperty::FLUID_LOSS_COEFFICIENT );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimStimPlanModelCalculator::calculateSpurtLoss() const
{
    return findCurveAndComputeLayeredAverage( RiaDefines::CurveProperty::SPURT_LOSS );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimStimPlanModelCalculator::calculateProppandEmbedment() const
{
    return findCurveAndComputeLayeredAverage( RiaDefines::CurveProperty::PROPPANT_EMBEDMENT );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimStimPlanModelCalculator::calculateImmobileFluidSaturation() const
{
    return findCurveAndComputeLayeredAverage( RiaDefines::CurveProperty::IMMOBILE_FLUID_SATURATION );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimStimPlanModelCalculator::calculateTemperature() const
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
std::vector<double> RimStimPlanModelCalculator::calculateRelativePermeabilityFactor() const
{
    return findCurveAndComputeLayeredAverage( RiaDefines::CurveProperty::RELATIVE_PERMEABILITY_FACTOR );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimStimPlanModelCalculator::calculatePoroElasticConstant() const
{
    return findCurveAndComputeLayeredAverage( RiaDefines::CurveProperty::PORO_ELASTIC_CONSTANT );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimStimPlanModelCalculator::calculateThermalExpansionCoefficient() const
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModelCalculator::calculateStressAtDepth( double depth,
                                                           double stressDepthRef,
                                                           double verticalStressRef,
                                                           double verticalStressGradientRef )
{
    double depthDiff = depth - stressDepthRef;
    double stress    = verticalStressRef + verticalStressGradientRef * depthDiff;
    return stress;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<std::vector<double>, std::vector<QString>> RimStimPlanModelCalculator::calculateFacies() const
{
    std::vector<double>  values = findCurveAndComputeTopOfLayer( RiaDefines::CurveProperty::FACIES );
    std::vector<QString> faciesNames;

    RimStimPlanModelTemplate* stimPlanModelTemplate = m_stimPlanModel->stimPlanModelTemplate();
    if ( !stimPlanModelTemplate )
    {
        RiaLogging::error( QString( "No fracture model template found" ) );
        return std::make_pair( values, faciesNames );
    }

    RimFaciesProperties* faciesProperties = stimPlanModelTemplate->faciesProperties();
    if ( !faciesProperties )
    {
        RiaLogging::error( QString( "No facies properties found when extracting elastic properties." ) );
        return std::make_pair( values, faciesNames );
    }

    RimColorLegend* colorLegend = faciesProperties->colorLegend();
    if ( !colorLegend )
    {
        RiaLogging::error( QString( "No color legend found when extracting elastic properties." ) );
        return std::make_pair( values, faciesNames );
    }

    for ( auto value : values )
    {
        faciesNames.push_back( RimStimPlanModelElasticPropertyCalculator::findFaciesName( *colorLegend, value ) );
    }

    return std::make_pair( values, faciesNames );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<std::vector<double>, std::vector<QString>> RimStimPlanModelCalculator::calculateFormation() const
{
    std::vector<double> values = findCurveAndComputeTopOfLayer( RiaDefines::CurveProperty::FORMATIONS );

    RimEclipseCase*      eclipseCase = m_stimPlanModel->eclipseCaseForProperty( RiaDefines::CurveProperty::FACIES );
    std::vector<QString> formationNamesVector = RimWellLogTrack::formationNamesVector( eclipseCase );

    std::vector<QString> formationNames;
    for ( auto value : values )
    {
        int idx = static_cast<int>( value );
        if ( idx < static_cast<int>( formationNamesVector.size() ) )
            formationNames.push_back( formationNamesVector[idx] );
        else
            formationNames.push_back( "_" );
    }

    return std::make_pair( values, formationNames );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimStimPlanModelCalculator::isValidInputData( const std::vector<double>&                    values,
                                                   const QString&                                propertyName,
                                                   const std::vector<std::pair<size_t, size_t>>& layerBoundaryIndexes,
                                                   const std::vector<std::pair<double, double>>& layerBoundaryDepths

)
{
    for ( size_t i = 0; i < layerBoundaryIndexes.size(); i++ )
    {
        double value = findValueAtTopOfLayer( values, layerBoundaryIndexes, i );
        if ( std::isinf( value ) || std::isnan( value ) )
        {
            double depthTopOfZone = layerBoundaryDepths[i].first;
            RiaLogging::error( QString( "Found invalid value for property %1 top of zone %2, depth %3: %4." )
                                   .arg( propertyName )
                                   .arg( i )
                                   .arg( depthTopOfZone )
                                   .arg( value ) );
            return false;
        }
    }

    return true;
}
