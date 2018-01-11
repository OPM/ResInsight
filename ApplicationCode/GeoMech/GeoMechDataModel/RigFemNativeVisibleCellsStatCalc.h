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
#include "RigFemResultAddress.h"
#include "RigGeoMechCaseData.h"
#include "RigFemPartCollection.h"
#include "RigFemPartResultsCollection.h"

#include "cvfArray.h"

class RigFemPartResultsCollection;


class RigFemNativeVisibleCellsStatCalc : public RigStatisticsCalculator
{
public:
    RigFemNativeVisibleCellsStatCalc(RigGeoMechCaseData* femCase, 
                                     const RigFemResultAddress& resVarAddr, 
                                     const cvf::UByteArray* cellVisibilities);

    virtual void    minMaxCellScalarValues(size_t timeStepIndex, double& min, double& max);
    virtual void    posNegClosestToZero(size_t timeStepIndex, double& pos, double& neg);
    virtual void    valueSumAndSampleCount(size_t timeStepIndex, double& valueSum, size_t& sampleCount);
    virtual void    addDataToHistogramCalculator(size_t timeStepIndex, RigHistogramCalculator& histogramCalculator);
    virtual void    uniqueValues(size_t timeStepIndex, std::set<int>& values);
    virtual size_t  timeStepCount();

private:
    RigGeoMechCaseData*          m_caseData;
    RigFemPartResultsCollection* m_resultsData;
    RigFemResultAddress          m_resVarAddr;
    cvf::cref<cvf::UByteArray>   m_cellVisibilities;

    template <typename StatisticsAccumulator>
    void traverseElementNodes(StatisticsAccumulator& accumulator, size_t timeStepIndex)
    {
        int partCount = m_caseData->femParts()->partCount();

        if (m_resVarAddr.resultPosType == RIG_NODAL)
        {
            for (int pIdx = 0; pIdx < partCount; ++pIdx)
            {
                RigFemPart* part = m_caseData->femParts()->part(pIdx);
                const std::vector<float>& values = m_resultsData->resultValues(m_resVarAddr, pIdx, (int)timeStepIndex);

                size_t nodeCount = values.size();
                cvf::UByteArray nodeVisibilities(nodeCount);
                nodeVisibilities.setAll(false);

                int elmCount = part->elementCount();
                for (int elmIdx = 0; elmIdx < elmCount; ++elmIdx)
                {
                    if (!(*m_cellVisibilities)[elmIdx]) continue;

                    int elmNodeCount = RigFemTypes::elmentNodeCount(part->elementType(elmIdx));
                    for (int elmLocIdx = 0; elmLocIdx < elmNodeCount; ++elmLocIdx)
                    {
                        size_t elmNodeResIdx = part->elementNodeResultIdx(elmIdx, elmLocIdx);
                        int nodeIdx = part->nodeIdxFromElementNodeResultIdx(elmNodeResIdx);
                        nodeVisibilities[nodeIdx] = true;
                    }
                }

                for (size_t nodeIdx = 0; nodeIdx < nodeCount; ++nodeIdx)
                {
                    if (nodeVisibilities[nodeIdx])
                    {
                        accumulator.addValue(values[nodeIdx]);
                    }
                }
            }
        }
        else if (m_resVarAddr.resultPosType == RIG_ELEMENT)
        {
            for (int pIdx = 0; pIdx < partCount; ++pIdx)
            {
                RigFemPart* part = m_caseData->femParts()->part(pIdx);
                const std::vector<float>& values = m_resultsData->resultValues(m_resVarAddr, pIdx, (int)timeStepIndex);
                int elmCount = part->elementCount();

                for (int elmIdx = 0; elmIdx < elmCount; ++elmIdx)
                {
                    if (!(*m_cellVisibilities)[elmIdx]) continue;

                    accumulator.addValue(values[elmIdx]);
                }
            }
        }
        else
        {
            for (int pIdx = 0; pIdx < partCount; ++pIdx)
            {
                RigFemPart* part = m_caseData->femParts()->part(pIdx);
                const std::vector<float>& values = m_resultsData->resultValues(m_resVarAddr, pIdx, (int)timeStepIndex);
                int elmCount = part->elementCount();

                for (int elmIdx = 0; elmIdx < elmCount; ++elmIdx)
                {
                    if (!(*m_cellVisibilities)[elmIdx]) continue;

                    int elmNodeCount = RigFemTypes::elmentNodeCount(part->elementType(elmIdx));
                    for (int elmLocIdx = 0; elmLocIdx < elmNodeCount; ++elmLocIdx)
                    {
                        size_t elmNodeResIdx = part->elementNodeResultIdx(elmIdx, elmLocIdx);
                        accumulator.addValue(values[elmNodeResIdx]);
                    }
                }
            }
        }
    }

};



