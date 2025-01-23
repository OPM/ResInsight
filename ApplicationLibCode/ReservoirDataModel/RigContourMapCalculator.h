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

#include <set>
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

    enum ResultAggregationType
    {
        TOP_VALUE,
        MEAN,
        GEOMETRIC_MEAN,
        HARMONIC_MEAN,
        MIN_VALUE,
        MAX_VALUE,
        VOLUME_SUM,
        SUM,
        OIL_COLUMN,
        GAS_COLUMN,
        HYDROCARBON_COLUMN
    };

    static std::vector<std::vector<std::pair<size_t, double>>> generateGridMapping( RigContourMapProjection&   contourMapProjection,
                                                                                    const RigContourMapGrid&   contourMapGrid,
                                                                                    ResultAggregationType      resultAggregation,
                                                                                    const std::vector<double>& weightingResultValues,
                                                                                    const std::set<int>&       kLayers );

    static double calculateValueInMapCell( const RigContourMapProjection&                contourMapProjection,
                                           const std::vector<std::pair<size_t, double>>& matchingCells,
                                           const std::vector<double>&                    gridCellValues,
                                           ResultAggregationType                         resultAggregation );

    static std::vector<CellIndexAndResult> cellOverlapVolumesAndResults( const RigContourMapProjection& contourMapProjection,
                                                                         const RigContourMapGrid&       contourMapGrid,
                                                                         const cvf::Vec2d&              globalPos2d,
                                                                         const std::vector<double>&     weightingResultValues,
                                                                         const std::set<int>&           kLayers );

    static std::vector<CellIndexAndResult> cellRayIntersectionAndResults( const RigContourMapProjection& contourMapProjection,
                                                                          const RigContourMapGrid&       contourMapGrid,
                                                                          const cvf::Vec2d&              globalPos2d,
                                                                          const std::vector<double>&     weightingResultValues,
                                                                          const std::set<int>&           kLayers );

    static bool isColumnResult( ResultAggregationType aggregationType );
    static bool isMeanResult( ResultAggregationType aggregationType );
    static bool isStraightSummationResult( ResultAggregationType aggregationType );

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
