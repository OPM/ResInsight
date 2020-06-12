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
            startIndex = i + 1;
        }
    }
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
        std::cout << "Layer boundaries: " << p.first << " - " << p.second << std::endl;
        tvdTopZone.push_back( p.first );
    }

    // TODO: convert to feet!!!!
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
        std::cerr << "NO " << curveName.toStdString() << " FOUND!!!" << std::endl;
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
    return std::vector<double>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimFractureModelPlot::calculateStressGradient() const
{
    return std::vector<double>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimFractureModelPlot::calculateYoungsModulus() const
{
    return findCurveAndComputeLayeredAverage( "Young's Modulus" );
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
    return std::vector<double>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimFractureModelPlot::calculateSpurtLoss() const
{
    return std::vector<double>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimFractureModelPlot::calculateProppandEmbedment() const
{
    return findCurveAndComputeLayeredAverage( "Proppant Embedment" );
}
