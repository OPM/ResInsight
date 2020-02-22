/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

//==================================================================================================
///
//==================================================================================================
#include "RigStatisticsCalculator.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"

#include "cvfArray.h"

class RigEclipseNativeVisibleCellsStatCalc : public RigStatisticsCalculator
{
public:
    RigEclipseNativeVisibleCellsStatCalc( RigCaseCellResultsData*        cellResultsData,
                                          const RigEclipseResultAddress& scalarResultIndex,
                                          const cvf::UByteArray*         cellVisibilities );

    void   minMaxCellScalarValues( size_t timeStepIndex, double& min, double& max ) override;
    void   posNegClosestToZero( size_t timeStepIndex, double& pos, double& neg ) override;
    void   valueSumAndSampleCount( size_t timeStepIndex, double& valueSum, size_t& sampleCount ) override;
    void   addDataToHistogramCalculator( size_t timeStepIndex, RigHistogramCalculator& histogramCalculator ) override;
    void   uniqueValues( size_t timeStepIndex, std::set<int>& values ) override;
    size_t timeStepCount() override;
    void   mobileVolumeWeightedMean( size_t timeStepIndex, double& result ) override;

private:
    RigCaseCellResultsData*    m_caseData;
    RigEclipseResultAddress    m_resultAddress;
    cvf::cref<cvf::UByteArray> m_cellVisibilities;

    template <typename StatisticsAccumulator>
    void traverseCells( StatisticsAccumulator& accumulator, size_t timeStepIndex )
    {
        const std::vector<double>& values = m_caseData->cellScalarResults( m_resultAddress, timeStepIndex );

        if ( values.empty() )
        {
            // Can happen if values do not exist for the current time step index.
            return;
        }

        const RigActiveCellInfo* actCellInfo = m_caseData->activeCellInfo();
        size_t                   cellCount   = actCellInfo->reservoirCellCount();

        CVF_TIGHT_ASSERT( cellCount == m_cellVisibilities->size() );

        for ( size_t cIdx = 0; cIdx < cellCount; ++cIdx )
        {
            if ( !( *m_cellVisibilities )[cIdx] ) continue;

            size_t cellResultIndex = cIdx;
            if ( m_caseData->isUsingGlobalActiveIndex( m_resultAddress ) )
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
