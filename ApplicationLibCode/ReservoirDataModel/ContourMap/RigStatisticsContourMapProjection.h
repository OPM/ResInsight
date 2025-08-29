/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024- Equinor ASA
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

#include "RigContourMapCalculator.h"
#include "RigContourMapProjection.h"

#include "cvfBoundingBox.h"

class RigContourMapGrid;

//==================================================================================================
///
///
//==================================================================================================
class RigStatisticsContourMapProjection : public RigContourMapProjection
{
public:
    RigStatisticsContourMapProjection( const RigContourMapGrid& contourMapGrid );
    virtual ~RigStatisticsContourMapProjection();

    std::vector<std::pair<size_t, double>> cellsAtIJ( unsigned int i, unsigned int j ) const override;

    void generateAndSaveResults( const std::vector<double>& results );

    std::vector<std::vector<std::pair<size_t, double>>>
        generateGridMapping( RigContourMapCalculator::ResultAggregationType resultAggregation,
                             const std::vector<double>&                     weights,
                             const std::set<int>&                           kLayers,
                             const std::vector<std::vector<cvf::Vec3d>>&    limitToPolygons ) override;

    std::vector<bool> getMapCellVisibility( int viewStepIndex, RigContourMapCalculator::ResultAggregationType resultAggregation ) override;
    bool              isCellActive( size_t globalCellIdx ) const override;

protected:
    using CellIndexAndResult = RigContourMapProjection::CellIndexAndResult;

    std::vector<size_t> findIntersectingCells( const cvf::BoundingBox& bbox ) const override;
    size_t              kLayer( size_t globalCellIdx ) const override;
    size_t              kLayers() const override;
    double              calculateOverlapVolume( size_t globalCellIdx, const cvf::BoundingBox& bbox ) const override;
    double calculateRayLengthInCell( size_t globalCellIdx, const cvf::Vec3d& highestPoint, const cvf::Vec3d& lowestPoint ) const override;
    double getParameterWeightForCell( size_t cellResultIdx, const std::vector<double>& parameterWeights ) const override;
};
