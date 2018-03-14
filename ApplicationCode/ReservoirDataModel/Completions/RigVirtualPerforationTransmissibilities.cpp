/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
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

#include "RigVirtualPerforationTransmissibilities.h"
#include "RigStatisticsMath.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
CompletionDataFrame::CompletionDataFrame() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void CompletionDataFrame::setCompletionData(const std::vector<RigCompletionData>& completions)
{
    for (auto& completion : completions)
    {
        auto it = m_multipleCompletionsPerEclipseCell.find(completion.completionDataGridCell());
        if (it != m_multipleCompletionsPerEclipseCell.end())
        {
            it->second.push_back(completion);
        }
        else
        {
            m_multipleCompletionsPerEclipseCell.insert(std::pair<RigCompletionDataGridCell, std::vector<RigCompletionData>>(
                completion.completionDataGridCell(), std::vector<RigCompletionData>{completion}));
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::map<RigCompletionDataGridCell, std::vector<RigCompletionData>>&
    CompletionDataFrame::multipleCompletionsPerEclipseCell() const
{
    return m_multipleCompletionsPerEclipseCell;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigVirtualPerforationTransmissibilities::RigVirtualPerforationTransmissibilities() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigVirtualPerforationTransmissibilities::~RigVirtualPerforationTransmissibilities() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigVirtualPerforationTransmissibilities::setCompletionDataForWellPath(
    RimWellPath*                                 wellPath,
    std::vector<std::vector<RigCompletionData>>& completionsPerTimeStep)
{
    auto item = m_mapFromWellToCompletionData.find(wellPath);

    CVF_ASSERT(item == m_mapFromWellToCompletionData.end());

    {
        std::vector<CompletionDataFrame> values;

        for (const auto& c : completionsPerTimeStep)
        {
            CompletionDataFrame oneTimeStep;
            oneTimeStep.setCompletionData(c);
            values.push_back(oneTimeStep);
        }

        auto pair = std::pair<RimWellPath*, std::vector<CompletionDataFrame>>(wellPath, values);

        m_mapFromWellToCompletionData.insert(pair);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::map<RigCompletionDataGridCell, std::vector<RigCompletionData>>&
    RigVirtualPerforationTransmissibilities::multipleCompletionsPerEclipseCell(RimWellPath* wellPath, size_t timeStepIndex) const
{
    static std::map<RigCompletionDataGridCell, std::vector<RigCompletionData>> dummy;

    auto item = m_mapFromWellToCompletionData.find(wellPath);
    if (item != m_mapFromWellToCompletionData.end())
    {
        size_t indexToUse = timeStepIndex;
        if (item->second.size() == 1)
        {
            indexToUse = 0;
        }

        return item->second[indexToUse].multipleCompletionsPerEclipseCell();
    }

    return dummy;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigVirtualPerforationTransmissibilities::computeMinMax(double* minValue,
                                                            double* maxValue,
                                                            double* posClosestToZero,
                                                            double* negClosestToZero) const
{
    MinMaxAccumulator minMaxAccumulator;
    PosNegAccumulator posNegAccumulator;

    for (const auto& item : m_mapFromWellToCompletionData)
    {
        auto dataForWellPath = item.second;

        for (const auto& timeStepFrame : dataForWellPath)
        {
            for (const auto& allCompletionsForWell : timeStepFrame.multipleCompletionsPerEclipseCell())
            {
                for (const auto& completionData : allCompletionsForWell.second)
                {
                    double transmissibility = completionData.transmissibility();

                    minMaxAccumulator.addValue(transmissibility);
                    posNegAccumulator.addValue(transmissibility);
                }
            }
        }
    }

    if (*minValue) *minValue = minMaxAccumulator.min;
    if (*maxValue) *maxValue = minMaxAccumulator.max;
    if (*posClosestToZero) *posClosestToZero = posNegAccumulator.pos;
    if (*negClosestToZero) *negClosestToZero = posNegAccumulator.neg;
}
