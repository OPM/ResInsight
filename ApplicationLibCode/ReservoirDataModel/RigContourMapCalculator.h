/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018- Equinor ASA
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

#include "cvfVector2.h"

#include <utility>
#include <vector>

class RigContourMapGrid;
class RigContourMapProjection;

//==================================================================================================
///
///
//==================================================================================================
class RigContourMapCalculator
{
public:
    using CellIndexAndResult = std::pair<size_t, double>;

    enum ResultAggregationEnum
    {
        RESULTS_TOP_VALUE,
        RESULTS_MEAN_VALUE,
        RESULTS_GEOM_VALUE,
        RESULTS_HARM_VALUE,
        RESULTS_MIN_VALUE,
        RESULTS_MAX_VALUE,
        RESULTS_VOLUME_SUM,
        RESULTS_SUM,
        RESULTS_OIL_COLUMN,
        RESULTS_GAS_COLUMN,
        RESULTS_HC_COLUMN
    };

    static std::vector<std::vector<std::pair<size_t, double>>> generateGridMapping( RigContourMapProjection&   contourMapProjection,
                                                                                    const RigContourMapGrid&   contourMapGrid,
                                                                                    ResultAggregationEnum      resultAggregation,
                                                                                    const std::vector<double>& weightingResultValues );

    static double calculateValueInMapCell( const RigContourMapProjection&                contourMapProjection,
                                           const std::vector<std::pair<size_t, double>>& matchingCells,
                                           const std::vector<double>&                    gridCellValues,
                                           ResultAggregationEnum                         resultAggregation );

    static std::vector<CellIndexAndResult> cellOverlapVolumesAndResults( const RigContourMapProjection& contourMapProjection,
                                                                         const RigContourMapGrid&       contourMapGrid,
                                                                         const cvf::Vec2d&              globalPos2d,
                                                                         const std::vector<double>&     weightingResultValues );

    static std::vector<CellIndexAndResult> cellRayIntersectionAndResults( const RigContourMapProjection& contourMapProjection,
                                                                          const RigContourMapGrid&       contourMapGrid,
                                                                          const cvf::Vec2d&              globalPos2d,
                                                                          const std::vector<double>&     weightingResultValues );

    static bool isColumnResult( ResultAggregationEnum aggregationType );
    static bool isMeanResult( ResultAggregationEnum aggregationType );
    static bool isStraightSummationResult( ResultAggregationEnum aggregationType );

private:
    static double calculateTopValue( const RigContourMapProjection&                contourMapProjection,
                                     const std::vector<std::pair<size_t, double>>& matchingCells,
                                     const std::vector<double>&                    gridCellValues );
    static double calculateMeanValue( const RigContourMapProjection&                contourMapProjection,
                                      const std::vector<std::pair<size_t, double>>& matchingCells,
                                      const std::vector<double>&                    gridCellValues );

    static double calculateGeometricMeanValue( const RigContourMapProjection&                contourMapProjection,
                                               const std::vector<std::pair<size_t, double>>& matchingCells,
                                               const std::vector<double>&                    gridCellValues );
    static double calculateHarmonicMeanValue( const RigContourMapProjection&                contourMapProjection,
                                              const std::vector<std::pair<size_t, double>>& matchingCells,
                                              const std::vector<double>&                    gridCellValues );
    static double calculateMaxValue( const RigContourMapProjection&                contourMapProjection,
                                     const std::vector<std::pair<size_t, double>>& matchingCells,
                                     const std::vector<double>&                    gridCellValues );
    static double calculateMinValue( const RigContourMapProjection&                contourMapProjection,
                                     const std::vector<std::pair<size_t, double>>& matchingCells,
                                     const std::vector<double>&                    gridCellValues );
    static double calculateSum( const RigContourMapProjection&                contourMapProjection,
                                const std::vector<std::pair<size_t, double>>& matchingCells,
                                const std::vector<double>&                    gridCellValues );
};
