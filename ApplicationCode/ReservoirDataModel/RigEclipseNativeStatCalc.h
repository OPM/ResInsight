/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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

#include "RigStatisticsCalculator.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"

class RigHistogramCalculator;

//==================================================================================================
///
//==================================================================================================

class RigEclipseNativeStatCalc : public RigStatisticsCalculator
{
public:
    RigEclipseNativeStatCalc( RigCaseCellResultsData* cellResultsData, const RigEclipseResultAddress& eclipseResultAddress );

    void   minMaxCellScalarValues( size_t timeStepIndex, double& min, double& max ) override;
    void   posNegClosestToZero( size_t timeStepIndex, double& pos, double& neg ) override;
    void   valueSumAndSampleCount( size_t timeStepIndex, double& valueSum, size_t& sampleCount ) override;
    void   addDataToHistogramCalculator( size_t timeStepIndex, RigHistogramCalculator& histogramCalculator ) override;
    void   uniqueValues( size_t timeStepIndex, std::set<int>& values ) override;
    size_t timeStepCount() override;
    void   mobileVolumeWeightedMean( size_t timeStepIndex, double& mean ) override;

private:
    RigCaseCellResultsData* m_resultsData;
    RigEclipseResultAddress m_eclipseResultAddress;

    template <typename StatisticsAccumulator>
    void traverseCells( StatisticsAccumulator& accumulator, size_t timeStepIndex )
    {
        if ( timeStepIndex >= m_resultsData->cellScalarResults( m_eclipseResultAddress ).size() )
        {
            return;
        }

        const std::vector<double>& values = m_resultsData->cellScalarResults( m_eclipseResultAddress, timeStepIndex );

        if ( values.empty() )
        {
            // Can happen if values do not exist for the current time step index.
            return;
        }

        const RigActiveCellInfo* actCellInfo = m_resultsData->activeCellInfo();
        size_t                   cellCount   = actCellInfo->reservoirCellCount();

        bool isUsingGlobalActiveIndex = m_resultsData->isUsingGlobalActiveIndex( m_eclipseResultAddress );
        for ( size_t cIdx = 0; cIdx < cellCount; ++cIdx )
        {
            // Filter out inactive cells
            if ( !actCellInfo->isActive( cIdx ) ) continue;

            size_t cellResultIndex = cIdx;
            if ( isUsingGlobalActiveIndex )
            {
                cellResultIndex = actCellInfo->cellResultIndex( cIdx );
            }

            if ( cellResultIndex != cvf::UNDEFINED_SIZE_T && cellResultIndex < values.size() )
            {
                accumulator.addValue( values[cellResultIndex] );
            }
        }
    }
};
