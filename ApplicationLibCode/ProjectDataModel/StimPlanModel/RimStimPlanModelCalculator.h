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

#include "RiaStimPlanModelDefines.h"

#include "RimStimPlanModelPropertyCalculator.h"

#include <map>
#include <memory>
#include <vector>

class RimStimPlanModel;

class RimStimPlanModelCalculator
{
public:
    RimStimPlanModelCalculator();

    void              setStimPlanModel( RimStimPlanModel* stimPlanModel );
    RimStimPlanModel* stimPlanModel();

    bool extractCurveData( RiaDefines::CurveProperty curveProperty,
                           int                       timeStep,
                           std::vector<double>&      values,
                           std::vector<double>&      measuredDepthValues,
                           std::vector<double>&      tvDepthValues,
                           double&                   rkbDiff ) const;

    std::vector<double> extractValues( RiaDefines::CurveProperty curveProperty, int timeStep ) const;

    std::vector<double> calculateTrueVerticalDepth() const;
    std::vector<double> calculatePorosity() const;
    std::vector<double> calculateVerticalPermeability() const;
    std::vector<double> calculateHorizontalPermeability() const;
    std::vector<double> calculateReservoirPressure() const;
    std::vector<double> calculateStress() const;
    std::vector<double> calculateInitialStress() const;
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

    void clearCache();

protected:
    std::vector<double> findCurveAndComputeLayeredAverage( RiaDefines::CurveProperty curveProperty ) const;
    std::vector<double> findCurveXValuesByProperty( RiaDefines::CurveProperty curveProperty ) const;
    std::vector<double> findCurveAndComputeTopOfLayer( RiaDefines::CurveProperty curveProperty ) const;

    void calculateLayers( std::vector<std::pair<double, double>>& layerBoundaryDepths,
                          std::vector<std::pair<size_t, size_t>>& layerBoundaryIndexes ) const;
    bool calculateStressWithGradients( std::vector<double>& stress,
                                       std::vector<double>& stressGradients,
                                       std::vector<double>& initialStress ) const;

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

    static double calculateStressDifferenceAtDepth( double depth,
                                                    double offset,
                                                    double stressDepthRef,
                                                    double verticalStressRef,
                                                    double verticalStressGradientRef );

    static double calculateStressAtDepth( double depth,
                                          double stressDepthRef,
                                          double verticalStressRef,
                                          double verticalStressGradientRef );

private:
    RimStimPlanModel*                                                m_stimPlanModel;
    std::vector<std::unique_ptr<RimStimPlanModelPropertyCalculator>> m_resultCalculators;

    typedef std::pair<RiaDefines::CurveProperty, int>                                         ResultKey;
    typedef std::tuple<std::vector<double>, std::vector<double>, std::vector<double>, double> ResultData;
    mutable std::map<ResultKey, ResultData>                                                   m_resultCache;
};
