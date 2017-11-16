/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RigWeightedMeanCalc.h"

#include "RiaDefines.h"

#include "RigActiveCellInfo.h"
#include "RigFlowDiagResults.h"

#include <cmath> // Needed for HUGE_VAL on Linux

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigWeightedMeanCalc::weightedMeanOverCells(const std::vector<double>* weights,
                                                const std::vector<double>* values,
                                                const cvf::UByteArray* cellVisibilities,
                                                bool isUsingVisibleCells,
                                                const RigActiveCellInfo* actCellInfo, 
                                                bool isUsingActiveIndex, 
                                                double *result)
{
    if (!(weights && values && result)) return;
    if (!cellVisibilities && isUsingVisibleCells) return;
    if (!actCellInfo && isUsingActiveIndex) return;

    if (weights->empty() || values->empty()) return;

    double weightedSum = 0.0;
    double weightSum = 0.0;

    for (size_t cIdx = 0; cIdx < actCellInfo->reservoirCellCount(); ++cIdx)
    {
        if (isUsingVisibleCells)
        {
            if (!(*cellVisibilities)[cIdx]) continue;
        }

        size_t cellResultIndex = actCellInfo->cellResultIndex(cIdx);
        if (cellResultIndex == cvf::UNDEFINED_SIZE_T || cellResultIndex > weights->size())
        {
            continue;
        }

        double value;

        if (isUsingActiveIndex)
        {
            value = (*values)[cellResultIndex];
        }
        else
        {
            value = (*values)[cIdx];
        }

        double weight = (*weights)[cellResultIndex];

        if (weight == HUGE_VAL || value == HUGE_VAL)
        {
            continue;
        }

        weightedSum += weight * value;
        weightSum += weight;
    }

    if (weightSum != 0)
    {
        *result = weightedSum / weightSum;
    }
    else
    {
        *result = HUGE_VAL;
    }
}
