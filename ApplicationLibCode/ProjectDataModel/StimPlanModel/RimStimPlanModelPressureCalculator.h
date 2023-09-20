/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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

#include "RimStimPlanModelCalculator.h"
#include "RimStimPlanModelPropertyCalculator.h"
#include "RimStimPlanModelWellLogCalculator.h"

#include "RiaStimPlanModelDefines.h"

#include "cvfObject.h"

#include <vector>

class RigEclipseCaseData;
class RimEclipseInputPropertyCollection;
class RimEclipseResultDefinition;
class RigResultAccessor;

//==================================================================================================
///
///
//==================================================================================================
class RimStimPlanModelPressureCalculator : public RimStimPlanModelWellLogCalculator
{
public:
    RimStimPlanModelPressureCalculator( RimStimPlanModelCalculator* stimPlanModelCalculator );

    bool isMatching( RiaDefines::CurveProperty curveProperty ) const override;

    static double pressureDifferenceInterpolationOffset();

    static std::tuple<std::vector<double>, std::vector<double>, std::vector<double>>
        interpolateMissingValues( const std::vector<double>& staticTvDepthValues,
                                  const std::vector<double>& staticMeasuredDepthValues,
                                  const std::vector<double>& measuredDepthValues,
                                  const std::vector<double>& values );

protected:
    bool extractValuesForProperty( RiaDefines::CurveProperty curveProperty,
                                   const RimStimPlanModel*   stimPlanModel,
                                   int                       timeStep,
                                   std::vector<double>&      values,
                                   std::vector<double>&      measuredDepthValues,
                                   std::vector<double>&      tvDepthValues,
                                   double&                   rkbDiff ) const override;

    bool extractPressureDataFromTable( RiaDefines::CurveProperty curveProperty,
                                       const RimStimPlanModel*   stimPlanModel,
                                       std::vector<double>&      values,
                                       std::vector<double>&      measuredDepthValues,
                                       std::vector<double>&      tvDepthValues,
                                       double                    minimumTvd,
                                       double                    maximumTvd ) const;

    bool interpolateInitialPressureByEquilibrationRegion( RiaDefines::CurveProperty  curveProperty,
                                                          const RimStimPlanModel*    stimPlanModel,
                                                          int                        timeStep,
                                                          const std::vector<double>& measuredDepthValues,
                                                          const std::vector<double>& tvDepthValues,
                                                          std::vector<double>&       values ) const;
    bool interpolatePressureDifferenceByEquilibrationRegion( RiaDefines::CurveProperty  curveProperty,
                                                             const RimStimPlanModel*    stimPlanModel,
                                                             int                        timeStep,
                                                             const std::vector<double>& measuredDepthValues,
                                                             const std::vector<double>& tvDepthValues,
                                                             const std::vector<double>& initialPressureValues,
                                                             std::vector<double>&       values ) const;
    bool handleFaciesWithInitialPressure( const RimStimPlanModel*    stimPlanModel,
                                          int                        timeStep,
                                          const std::vector<double>& faciesValues,
                                          std::vector<double>&       values ) const;

    using DepthValuePair            = std::pair<double, double>;
    using DepthValuePairVector      = std::vector<DepthValuePair>;
    using EqlNumToDepthValuePairMap = std::map<int, DepthValuePairVector>;

    static void sortAndRemoveDuplicates( DepthValuePairVector& depthValuePairs );
    static bool buildPressureTablesPerEqlNum( const RimStimPlanModel*    stimPlanModel,
                                              EqlNumToDepthValuePairMap& valuesPerEqlNum,
                                              const std::set<int>&       presentEqlNums );

    static std::set<int> findUniqueValues( const std::vector<double>& values );

    static double interpolatePressure( const DepthValuePairVector& depthValuePairs, double depth, int eqlNum );

    static void binByDepthAndAverage( DepthValuePairVector& depthValuePairs );
};
