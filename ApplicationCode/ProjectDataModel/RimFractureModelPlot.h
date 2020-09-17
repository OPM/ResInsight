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
#pragma once

#include "RimDepthTrackPlot.h"

#include "RiaFractureModelDefines.h"

#include "cafPdmField.h"
#include "cafPdmPtrField.h"

#include <vector>

class RimWellLogExtractionCurve;
class RimFractureModel;

class RimFractureModelPlot : public RimDepthTrackPlot
{
    CAF_PDM_HEADER_INIT;

public:
    RimFractureModelPlot();

    void              setFractureModel( RimFractureModel* fractureModel );
    RimFractureModel* fractureModel();

    void getPorosityValues( std::vector<double>& values ) const;
    void getFaciesValues( std::vector<double>& values ) const;
    void getCurvePropertyValues( RiaDefines::CurveProperty curveProperty, std::vector<double>& values ) const;

    std::vector<double> calculateTrueVerticalDepth() const;
    std::vector<double> calculatePorosity() const;
    std::vector<double> calculateVerticalPermeability() const;
    std::vector<double> calculateHorizontalPermeability() const;
    std::vector<double> calculateReservoirPressure() const;
    std::vector<double> calculateStress() const;
    std::vector<double> calculateStressGradient() const;
    std::vector<double> calculateYoungsModulus() const;
    std::vector<double> calculatePoissonsRatio() const;
    std::vector<double> calculateKIc() const;
    std::vector<double> calculateFluidLossCoefficient() const;
    std::vector<double> calculateSpurtLoss() const;
    std::vector<double> calculateProppandEmbedment() const;

    std::vector<double> calculateImmobileFluidSaturation() const;
    std::vector<double> calculateTemperature() const;
    std::vector<double> calculateRelativePermeabilityFactor() const;
    std::vector<double> calculatePoroElasticConstant() const;
    std::vector<double> calculateThermalExpansionCoefficient() const;

    void calculateTemperature( std::vector<double>& temperatures ) const;

protected:
    std::vector<double> findCurveAndComputeLayeredAverage( RiaDefines::CurveProperty curveProperty ) const;
    std::vector<double> findCurveXValuesByProperty( RiaDefines::CurveProperty curveProperty ) const;
    std::vector<double> findCurveAndComputeTopOfLayer( RiaDefines::CurveProperty curveProperty ) const;

    void                       calculateLayers( std::vector<std::pair<double, double>>& layerBoundaryDepths,
                                                std::vector<std::pair<size_t, size_t>>& layerBoundaryIndexes ) const;
    RimWellLogExtractionCurve* findCurveByProperty( RiaDefines::CurveProperty curveProperty ) const;
    bool calculateStressWithGradients( std::vector<double>& stress, std::vector<double>& stressGradients ) const;

    static double findValueAtTopOfLayer( const std::vector<double>&                    values,
                                         const std::vector<std::pair<size_t, size_t>>& layerBoundaryIndexes,
                                         size_t                                        layerNo );
    static double findValueAtBottomOfLayer( const std::vector<double>&                    values,
                                            const std::vector<std::pair<size_t, size_t>>& layerBoundaryIndexes,
                                            size_t                                        layerNo );
    static void   computeAverageByLayer( const std::vector<std::pair<size_t, size_t>>& layerBoundaryIndexes,
                                         const std::vector<double>&                    inputVector,
                                         std::vector<double>&                          result );
    static void   extractTopOfLayerValues( const std::vector<std::pair<size_t, size_t>>& layerBoundaryIndexes,
                                           const std::vector<double>&                    inputVector,
                                           std::vector<double>&                          result );

    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

    void onLoadDataAndUpdate() override;

private:
    void applyDataSource();

    caf::PdmPtrField<RimFractureModel*> m_fractureModel;
};
