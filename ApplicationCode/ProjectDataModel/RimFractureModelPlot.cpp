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
#include "RimFractureModelPropertyCurve.h"
#include "RimLayerCurve.h"

#include "RigWellLogCurveData.h"

#include "cafPdmBase.h"
#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObject.h"
#include "cafPdmUiGroup.h"

CAF_PDM_SOURCE_INIT( RimFractureModelPlot, "FractureModelPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFractureModelPlot::RimFractureModelPlot()
{
    CAF_PDM_InitScriptableObject( "Fracture Model Plot", "", "", "A fracture model plot" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_fractureModel, "FractureModel", "Fracture Model", "", "", "" );
    m_fractureModel.uiCapability()->setUiTreeChildrenHidden( true );
    m_fractureModel.uiCapability()->setUiHidden( true );

    setDeletable( true );
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
RimFractureModel* RimFractureModelPlot::fractureModel()
{
    return m_fractureModel;
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
void RimFractureModelPlot::getCurvePropertyValues( RiaDefines::CurveProperty curveProperty, std::vector<double>& values ) const
{
    RimWellLogExtractionCurve* curve = findCurveByProperty( curveProperty );
    if ( curve )
    {
        values = curve->curveData()->xValues();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModelPlot::getPorosityValues( std::vector<double>& values ) const
{
    getCurvePropertyValues( RiaDefines::CurveProperty::POROSITY, values );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModelPlot::getFaciesValues( std::vector<double>& values ) const
{
    getCurvePropertyValues( RiaDefines::CurveProperty::FACIES, values );
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
double RimFractureModelPlot::findValueAtTopOfLayer( const std::vector<double>&                    values,
                                                    const std::vector<std::pair<size_t, size_t>>& layerBoundaryIndexes,
                                                    size_t                                        layerNo )
{
    size_t index = layerBoundaryIndexes[layerNo].first;
    return values.at( index );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFractureModelPlot::findValueAtBottomOfLayer( const std::vector<double>&                    values,
                                                       const std::vector<std::pair<size_t, size_t>>& layerBoundaryIndexes,
                                                       size_t                                        layerNo )
{
    size_t index = layerBoundaryIndexes[layerNo].second;
    return values.at( index );
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
void RimFractureModelPlot::extractTopOfLayerValues( const std::vector<std::pair<size_t, size_t>>& layerBoundaryIndexes,
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
RimWellLogExtractionCurve* RimFractureModelPlot::findCurveByProperty( RiaDefines::CurveProperty curveProperty ) const
{
    std::vector<RimFractureModelPropertyCurve*> curves;
    descendantsIncludingThisOfType( curves );

    for ( RimFractureModelPropertyCurve* curve : curves )
    {
        if ( curve->curveProperty() == curveProperty )
        {
            return dynamic_cast<RimWellLogExtractionCurve*>( curve );
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
std::vector<double> RimFractureModelPlot::findCurveAndComputeLayeredAverage( RiaDefines::CurveProperty curveProperty ) const
{
    RimWellLogExtractionCurve* curve = findCurveByProperty( curveProperty );
    if ( !curve )
    {
        QString curveName = caf::AppEnum<RiaDefines::CurveProperty>::uiText( curveProperty );
        RiaLogging::error( QString( "No curve for '%1' found" ).arg( curveName ) );
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
std::vector<double> RimFractureModelPlot::findCurveAndComputeTopOfLayer( RiaDefines::CurveProperty curveProperty ) const
{
    RimWellLogExtractionCurve* curve = findCurveByProperty( curveProperty );
    if ( !curve )
    {
        QString curveName = caf::AppEnum<RiaDefines::CurveProperty>::uiText( curveProperty );
        RiaLogging::error( QString( "No curve for '%1' found" ).arg( curveName ) );
        return std::vector<double>();
    }

    std::vector<std::pair<double, double>> layerBoundaryDepths;
    std::vector<std::pair<size_t, size_t>> layerBoundaryIndexes;
    calculateLayers( layerBoundaryDepths, layerBoundaryIndexes );

    const RigWellLogCurveData* curveData = curve->curveData();
    std::vector<double>        values    = curveData->xValues();
    std::vector<double>        result;
    extractTopOfLayerValues( layerBoundaryIndexes, values, result );
    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimFractureModelPlot::calculatePorosity() const
{
    return findCurveAndComputeLayeredAverage( RiaDefines::CurveProperty::POROSITY );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimFractureModelPlot::calculateReservoirPressure() const
{
    return findCurveAndComputeTopOfLayer( RiaDefines::CurveProperty::PRESSURE );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimFractureModelPlot::calculateHorizontalPermeability() const
{
    return findCurveAndComputeLayeredAverage( RiaDefines::CurveProperty::PERMEABILITY_X );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimFractureModelPlot::calculateVerticalPermeability() const
{
    return findCurveAndComputeLayeredAverage( RiaDefines::CurveProperty::PERMEABILITY_Z );
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
std::vector<double> RimFractureModelPlot::findCurveXValuesByProperty( RiaDefines::CurveProperty curveProperty ) const
{
    RimWellLogExtractionCurve* curve = findCurveByProperty( curveProperty );
    if ( !curve )
    {
        QString curveName = caf::AppEnum<RiaDefines::CurveProperty>::uiText( curveProperty );
        RiaLogging::error( QString( "%1 data not found." ).arg( curveName ) );
        return std::vector<double>();
    }
    return curve->curveData()->xValues();
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
    std::vector<double> biotData = findCurveXValuesByProperty( RiaDefines::CurveProperty::BIOT_COEFFICIENT );

    // K0
    std::vector<double> k0Data = findCurveXValuesByProperty( RiaDefines::CurveProperty::K0 );

    // Pressure at the give time step
    std::vector<double> timeStepPressureData = findCurveXValuesByProperty( RiaDefines::CurveProperty::PRESSURE );

    // Initial pressure
    std::vector<double> initialPressureData = findCurveXValuesByProperty( RiaDefines::CurveProperty::INITIAL_PRESSURE );

    // Poissons ratio
    std::vector<double> poissonsRatioData = findCurveXValuesByProperty( RiaDefines::CurveProperty::POISSONS_RATIO );

    // Check that we have data from all curves
    if ( biotData.empty() || k0Data.empty() || timeStepPressureData.empty() || initialPressureData.empty() ||
         poissonsRatioData.empty() )
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
        double diffStress   = stressForGradients[i + 1] - stressForGradients[i];
        double diffPressure = pressureForGradients[i + 1] - pressureForGradients[i];
        double diffDepth    = depthForGradients[i + 1] - depthForGradients[i];
        double k0           = findValueAtTopOfLayer( k0Data, layerBoundaryIndexes, i );
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
std::vector<double> RimFractureModelPlot::calculatePoissonsRatio() const
{
    return findCurveAndComputeLayeredAverage( RiaDefines::CurveProperty::POISSONS_RATIO );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimFractureModelPlot::calculateKIc() const
{
    return findCurveAndComputeLayeredAverage( RiaDefines::CurveProperty::K_IC );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimFractureModelPlot::calculateFluidLossCoefficient() const
{
    return findCurveAndComputeLayeredAverage( RiaDefines::CurveProperty::FLUID_LOSS_COEFFICIENT );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimFractureModelPlot::calculateSpurtLoss() const
{
    return findCurveAndComputeLayeredAverage( RiaDefines::CurveProperty::SPURT_LOSS );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimFractureModelPlot::calculateProppandEmbedment() const
{
    return findCurveAndComputeLayeredAverage( RiaDefines::CurveProperty::PROPPANT_EMBEDMENT );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimFractureModelPlot::calculateImmobileFluidSaturation() const
{
    return std::vector<double>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimFractureModelPlot::calculateTemperature() const
{
    return std::vector<double>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimFractureModelPlot::calculateRelativePermeabilityFactor() const
{
    return std::vector<double>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimFractureModelPlot::calculatePoroElasticConstant() const
{
    return std::vector<double>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimFractureModelPlot::calculateThermalExpansionCoefficient() const
{
    return std::vector<double>();
}
