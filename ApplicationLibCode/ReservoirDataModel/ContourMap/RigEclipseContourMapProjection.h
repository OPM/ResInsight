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

#include "RigContourMapCalculator.h"
#include "RigContourMapProjection.h"
#include "RigEclipseResultAddress.h"
#include "RigFloodingSettings.h"

#include "cvfBoundingBox.h"

#include <set>

class RigActiveCellInfo;
class RigMainGrid;
class RigContourMapGrid;
class RigResultAccessor;
class RigEclipseCaseData;
class RigCaseCellResultsData;

//==================================================================================================
///
///
//==================================================================================================
class RigEclipseContourMapProjection : public RigContourMapProjection
{
public:
    RigEclipseContourMapProjection( const RigContourMapGrid& contourMapGrid,
                                    RigEclipseCaseData&      eclipseCaseData,
                                    RigCaseCellResultsData&  resultData );
    virtual ~RigEclipseContourMapProjection();

    void generateAndSaveResults( const RigEclipseResultAddress&                 resultAddress,
                                 RigContourMapCalculator::ResultAggregationType resultAggregation,
                                 int                                            timeStep,
                                 RigFloodingSettings&                           floodingSettings );

    std::vector<double> generateResults( const RigEclipseResultAddress&                 resultAddress,
                                         RigContourMapCalculator::ResultAggregationType resultAggregation,
                                         int                                            timeStep,
                                         RigFloodingSettings&                           floodingSettings ) const;

    static std::pair<bool, std::vector<double>> generateResults( const RigEclipseContourMapProjection&          contourMapProjection,
                                                                 const RigContourMapGrid&                       contourMapGrid,
                                                                 RigCaseCellResultsData&                        resultData,
                                                                 const RigEclipseResultAddress&                 resultAddress,
                                                                 RigContourMapCalculator::ResultAggregationType resultAggregation,
                                                                 int                                            timeStep,
                                                                 RigFloodingSettings&                           floodingSettings );

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
    size_t gridResultIndex( size_t globalCellIdx ) const override;

    static std::vector<double> calculateColumnResult( RigCaseCellResultsData&                        resultData,
                                                      RigContourMapCalculator::ResultAggregationType resultAggregation,
                                                      int                                            timeStep,
                                                      RigFloodingSettings&                           floodingSettings );

    static std::set<RigEclipseResultAddress> neededResults( RigContourMapCalculator::ResultAggregationType resultAggregation,
                                                            RigFloodingSettings&                           floodingSettings );

protected:
    RigEclipseCaseData&         m_eclipseCaseData;
    RigCaseCellResultsData&     m_resultData;
    cvf::ref<RigMainGrid>       m_mainGrid;
    cvf::ref<RigActiveCellInfo> m_activeCellInfo;
    size_t                      m_kLayers;
    bool                        m_useActiveCellInfo;
};
