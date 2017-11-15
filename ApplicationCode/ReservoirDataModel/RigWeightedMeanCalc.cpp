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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigWeightedMeanCalc::weightedMeanOverCells(const std::vector<double>* weights, 
                                                const std::vector<double>* values, 
                                                const cvf::UByteArray* cellVisibilities, 
                                                const RigActiveCellInfo* actCellInfo, 
                                                bool isUsingActiveIndex, 
                                                double *result)
{
    if (!(weights && values && cellVisibilities && actCellInfo && result)) return;

    if (weights->empty() || values->empty()) return;
    if (weights->size() != values->size()) return;

    double weightedSum = 0.0;
    double weightSum = 0.0;

    for (size_t cIdx = 0; cIdx < cellVisibilities->size(); ++cIdx)
    {
        if (!(*cellVisibilities)[cIdx]) continue;

        size_t cellResultIndex = cIdx;
        if (isUsingActiveIndex)
        {
            cellResultIndex = actCellInfo->cellResultIndex(cIdx);
        }

        if (cellResultIndex == cvf::UNDEFINED_SIZE_T || cellResultIndex > values->size()) continue;

        double weight = (*weights)[cellResultIndex];
        double value = (*values)[cellResultIndex];

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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigWeightedMeanCalc::weightedMean(const std::vector<double>* weights, const std::vector<double>* values, double* result)
{
    if (!weights || !values) return;
    if (weights->size() != values->size()) return;

    double weightedSum = 0;
    double weightSum = 0;

    for (size_t i = 0; i < values->size(); i++)
    {
        double weight = weights->at(i);
        double value = values->at(i);

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
