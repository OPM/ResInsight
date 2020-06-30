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
#include "RimFractureModelPlot.h"

#include "RiaDefines.h"
#include "RiaLogging.h"

#include "RicfCommandObject.h"

#include "RimEclipseCase.h"
#include "RimFractureModel.h"
#include "RimFractureModelCurve.h"
#include "RimLayerCurve.h"

#include "RigWellLogCurveData.h"

#include "cafPdmBase.h"
#include "cafPdmFieldIOScriptability.h"
#include "cafPdmObject.h"
#include "cafPdmUiGroup.h"

CAF_PDM_SOURCE_INIT( RimFractureModelPlot, "FractureModelPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFractureModelPlot::RimFractureModelPlot()
{
    CAF_PDM_InitScriptableObject( "Fracture Model Plot", "", "", "A fracture model plot" );

    CAF_PDM_InitFieldNoDefault( &m_fractureModel, "FractureModel", "Fracture Model", "", "", "" );
    m_fractureModel.uiCapability()->setUiTreeChildrenHidden( true );
    m_fractureModel.uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModelPlot::setFractureModel( RimFractureModel* fractureModel )
{
    m_fractureModel = fractureModel;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModelPlot::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiGroup* depthGroup = uiOrdering.addNewGroup( "Depth Axis" );
    RimDepthTrackPlot::uiOrderingForDepthAxis( uiConfigName, *depthGroup );

    caf::PdmUiGroup* titleGroup = uiOrdering.addNewGroup( "Plot Title" );
    RimDepthTrackPlot::uiOrderingForAutoName( uiConfigName, *titleGroup );

    caf::PdmUiGroup* plotLayoutGroup = uiOrdering.addNewGroup( "Plot Layout" );
    RimPlotWindow::uiOrderingForPlotLayout( uiConfigName, *plotLayoutGroup );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModelPlot::onLoadDataAndUpdate()
{
    RimDepthTrackPlot::onLoadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModelPlot::applyDataSource()
{
    this->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModelPlot::getPorosityValues( std::vector<double>& values ) const
{
    std::vector<RimFractureModelCurve*> curves;
    descendantsIncludingThisOfType( curves );

    for ( RimFractureModelCurve* curve : curves )
    {
        if ( curve->eclipseResultVariable() == "PORO" )
        {
            values = curve->curveData()->xValues();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModelPlot::getFaciesValues( std::vector<double>& values ) const
{
    std::vector<RimFractureModelCurve*> curves;
    descendantsIncludingThisOfType( curves );

    for ( RimFractureModelCurve* curve : curves )
    {
        if ( curve->eclipseResultVariable() == "OPERNUM_1" )
        {
            values = curve->curveData()->xValues();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModelPlot::calculateLayers( std::vector<std::pair<double, double>>& layerBoundaryDepths,
                                            std::vector<std::pair<size_t, size_t>>& layerBoundaryIndexes ) const
{
    std::vector<RimLayerCurve*> curves;
    descendantsIncludingThisOfType( curves );

    if ( curves.empty() )
    {
        return;
    }

    // Expect to have only one of these
    RimLayerCurve* layerCurve = curves[0];

    const RigWellLogCurveData* curveData = layerCurve->curveData();

    // Find
    std::vector<double> depths      = curveData->depths( RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH );
    std::vector<double> layerValues = curveData->xValues();

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
double RimFractureModelPlot::computeValueAtDepth( const std::vector<double>&              values,
                                                  std::vector<std::pair<double, double>>& layerBoundaryDepths,
                                                  double                                  depth )
{
    for ( size_t i = 0; i < layerBoundaryDepths.size(); i++ )
    {
        if ( layerBoundaryDepths[i].first <= depth && layerBoundaryDepths[i].second >= depth )
        {
            return values[i];
        }
    }

    RiaLogging::error( QString( "Failed to compute value at depth: %1" ).arg( depth ) );
    return std::numeric_limits<double>::infinity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModelPlot::computeAverageByLayer( const std::vector<std::pair<size_t, size_t>>& layerBoundaryIndexes,
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
RimWellLogExtractionCurve* RimFractureModelPlot::findCurveByName( const QString& curveName ) const
{
    std::vector<RimWellLogExtractionCurve*> curves;
    descendantsIncludingThisOfType( curves );

    for ( auto curve : curves )
    {
        // TODO: This will not work if the user has changed the name of the curve: do something smarter.
        if ( curve->curveName() == curveName )
        {
            return curve;
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimFractureModelPlot::calculateTrueVerticalDepth() const
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
std::vector<double> RimFractureModelPlot::findCurveAndComputeLayeredAverage( const QString& curveName ) const
{
    RimWellLogExtractionCurve* curve = findCurveByName( curveName );
    if ( !curve )
    {
        RiaLogging::error( QString( "No curve named '%1' found" ).arg( curveName ) );
        return std::vector<double>();
    }

    std::vector<std::pair<double, double>> layerBoundaryDepths;
    std::vector<std::pair<size_t, size_t>> layerBoundaryIndexes;
    calculateLayers( layerBoundaryDepths, layerBoundaryIndexes );

    const RigWellLogCurveData* curveData = curve->curveData();
    std::vector<double>        values    = curveData->xValues();
    std::vector<double>        result;
    computeAverageByLayer( layerBoundaryIndexes, values, result );
    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimFractureModelPlot::calculatePorosity() const
{
    return findCurveAndComputeLayeredAverage( "PORO" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimFractureModelPlot::calculateReservoirPressure() const
{
    return findCurveAndComputeLayeredAverage( "PRESSURE" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimFractureModelPlot::calculateHorizontalPermeability() const
{
    return findCurveAndComputeLayeredAverage( "PERMX" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimFractureModelPlot::calculateVerticalPermeability() const
{
    return findCurveAndComputeLayeredAverage( "PERMZ" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimFractureModelPlot::calculateStress() const
{
    std::vector<double> stress;
    std::vector<double> stressGradients;
    calculateStressWithGradients( stress, stressGradients );
    return stress;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFractureModelPlot::calculateStressWithGradients( std::vector<double>& stress,
                                                         std::vector<double>& stressGradients ) const
{
    // Reference stress
    const double verticalStressRef         = m_fractureModel->verticalStress();
    const double verticalStressGradientRef = m_fractureModel->verticalStressGradient();
    const double stressDepthRef            = m_fractureModel->stressDepth();

    std::vector<std::pair<double, double>> layerBoundaryDepths;
    std::vector<std::pair<size_t, size_t>> layerBoundaryIndexes;
    calculateLayers( layerBoundaryDepths, layerBoundaryIndexes );

    // Biot coefficient
    RimWellLogExtractionCurve* biotCurve = findCurveByName( "Biot Coefficient" );
    if ( !biotCurve )
    {
        RiaLogging::error( "Biot coefficient data not found." );
        return false;
    }
    std::vector<double> biotData = biotCurve->curveData()->xValues();

    // Biot coefficient
    RimWellLogExtractionCurve* k0Curve = findCurveByName( "k0" );
    if ( !k0Curve )
    {
        RiaLogging::error( "k0 data not found." );
        return false;
    }
    std::vector<double> k0Data = k0Curve->curveData()->xValues();

    // Pressure at the give time step
    RimWellLogExtractionCurve* timeStepPressureCurve = findCurveByName( "PRESSURE" );
    if ( !timeStepPressureCurve )
    {
        RiaLogging::error( "Pressure data for time step not found." );
        return false;
    }
    std::vector<double> timeStepPressureData = timeStepPressureCurve->curveData()->xValues();

    // Initial pressure
    RimWellLogExtractionCurve* initialPressureCurve = findCurveByName( "INITIAL PRESSURE" );
    if ( !initialPressureCurve )
    {
        RiaLogging::error( "Initial pressure data not found." );
        return false;
    }
    std::vector<double> initialPressureData = initialPressureCurve->curveData()->xValues();

    // Poissons ratio
    RimWellLogExtractionCurve* poissonsRatioCurve = findCurveByName( "Poisson's Ratio" );
    if ( !poissonsRatioCurve )
    {
        RiaLogging::error( "Poisson's ratio data not found." );
        return false;
    }
    std::vector<double> poissonsRatioData = poissonsRatioCurve->curveData()->xValues();

    std::vector<double> stressForGradients;
    std::vector<double> pressureForGradients;
    std::vector<double> depthForGradients;

    // Calculate the stress
    for ( size_t i = 0; i < layerBoundaryDepths.size(); i++ )
    {
        double depthTopOfZone    = layerBoundaryDepths[i].first;
        double depthBottomOfZone = layerBoundaryDepths[i].second;

        // Data from curves at the top zone depth
        double k0               = computeValueAtDepth( k0Data, layerBoundaryDepths, depthTopOfZone );
        double biot             = computeValueAtDepth( biotData, layerBoundaryDepths, depthTopOfZone );
        double poissonsRatio    = computeValueAtDepth( poissonsRatioData, layerBoundaryDepths, depthTopOfZone );
        double initialPressure  = computeValueAtDepth( initialPressureData, layerBoundaryDepths, depthTopOfZone );
        double timeStepPressure = computeValueAtDepth( timeStepPressureData, layerBoundaryDepths, depthTopOfZone );

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

        // Cache some results for the gradients calculation
        stressForGradients.push_back( Sv );
        pressureForGradients.push_back( initialPressure );
        depthForGradients.push_back( depthTopOfZone );

        if ( i == layerBoundaryDepths.size() - 1 )
        {
            // Use the bottom of the last layer to compute gradient for last layer
            double bottomInitialPressure =
                computeValueAtDepth( initialPressureData, layerBoundaryDepths, depthBottomOfZone );
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
        double diffStress   = stressForGradients[i + 1] - stressForGradients[i];
        double diffPressure = pressureForGradients[i + 1] - pressureForGradients[i];
        double diffDepth    = depthForGradients[i + 1] - depthForGradients[i];
        double k0           = computeValueAtDepth( k0Data, layerBoundaryDepths, depthForGradients[i] );
        double gradient     = ( diffStress * k0 + diffPressure * ( 1.0 - k0 ) ) / diffDepth;
        stressGradients.push_back( RiaEclipseUnitTools::barPerMeterToPsiPerFeet( gradient ) );
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimFractureModelPlot::calculateStressGradient() const
{
    std::vector<double> stress;
    std::vector<double> stressGradients;
    calculateStressWithGradients( stress, stressGradients );
    return stressGradients;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimFractureModelPlot::calculateYoungsModulus() const
{
    std::vector<double> valuesGPa = findCurveAndComputeLayeredAverage( "Young's Modulus" );
    std::vector<double> valuesMMpsi;
    for ( auto value : valuesGPa )
    {
        valuesMMpsi.push_back( value * 0.000145037737 );
    }

    return valuesMMpsi;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimFractureModelPlot::calculatePoissonsRatio() const
{
    return findCurveAndComputeLayeredAverage( "Poisson's Ratio" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimFractureModelPlot::calculateKIc() const
{
    return findCurveAndComputeLayeredAverage( "K-Ic" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimFractureModelPlot::calculateFluidLossCoefficient() const
{
    return findCurveAndComputeLayeredAverage( "Fluid Loss Coefficient" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimFractureModelPlot::calculateSpurtLoss() const
{
    return findCurveAndComputeLayeredAverage( "Spurt Loss" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimFractureModelPlot::calculateProppandEmbedment() const
{
    return findCurveAndComputeLayeredAverage( "Proppant Embedment" );
}
