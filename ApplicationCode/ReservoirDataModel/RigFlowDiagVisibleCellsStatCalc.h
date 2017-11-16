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
#include "RigFlowDiagResultAddress.h"
#include "RigFlowDiagResults.h"
#include "RigActiveCellInfo.h"

#include "cvfArray.h"

class RigFlowDiagResults;
class RigActiveCellInfo;

class RigFlowDiagVisibleCellsStatCalc : public RigStatisticsCalculator
{
public:
    RigFlowDiagVisibleCellsStatCalc(RigFlowDiagResults* resultsData, 
                                     const RigFlowDiagResultAddress& resVarAddr, 
                                     const cvf::UByteArray* cellVisibilities);

    virtual void    minMaxCellScalarValues(size_t timeStepIndex, double& min, double& max);
    virtual void    posNegClosestToZero(size_t timeStepIndex, double& pos, double& neg);
    virtual void    valueSumAndSampleCount(size_t timeStepIndex, double& valueSum, size_t& sampleCount);
    virtual void    addDataToHistogramCalculator(size_t timeStepIndex, RigHistogramCalculator& histogramCalculator);
    virtual void    uniqueValues(size_t timeStepIndex, std::set<int>& values);
    virtual size_t  timeStepCount();
    virtual void    mobileVolumeWeightedMean(size_t timeStepIndex, double &result);

private:
    RigFlowDiagResults*             m_resultsData;
    RigFlowDiagResultAddress        m_resVarAddr;
    cvf::cref<cvf::UByteArray>      m_cellVisibilities;

    template <typename StatisticsAccumulator>
    void traverseElementNodes(StatisticsAccumulator& accumulator, size_t timeStepIndex)
    {
        const std::vector<double>* values = m_resultsData->resultValues(m_resVarAddr, timeStepIndex);
        if (!values) return;

        const RigActiveCellInfo* actCellInfo = m_resultsData->activeCellInfo(m_resVarAddr);

        size_t cellCount = actCellInfo->reservoirCellCount();

        CVF_TIGHT_ASSERT(cellCount == m_cellVisibilities->size());

        for ( size_t cIdx = 0; cIdx < cellCount; ++cIdx )
        {
            if ( !(*m_cellVisibilities)[cIdx] ) continue;

            size_t cellResultIndex = actCellInfo->cellResultIndex(cIdx);

            if ( cellResultIndex != cvf::UNDEFINED_SIZE_T ) accumulator.addValue((*values)[cellResultIndex]);
        }
    }
};
