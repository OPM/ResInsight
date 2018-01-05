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

#include "RigTimeHistoryResultAccessor.h"

#include "RigResultAccessor.h"
#include "RigResultAccessorFactory.h"
#include "RigEclipseCaseData.h"
#include "RigGridBase.h"

//#include <cmath> // Needed for HUGE_VAL on Linux

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<double> RigTimeHistoryResultAccessor::timeHistoryValues(RigEclipseCaseData* eclipseCaseData, 
                                                                    RimEclipseResultDefinition* resultDefinition, 
                                                                    size_t gridIndex, 
                                                                    size_t cellIndex, 
                                                                    size_t timeStepCount)
{
    std::vector<double> values;

    RigHugeValResultAccessor hugeVal;

    for (size_t i = 0; i < timeStepCount; i++)
    {
        // TODO: Consider rewrite RigResultAccessorFactory::createFromResultDefinition so the function always returns a valid
        // result accessor. Use hugeVal result accessor if no valid result is found

        cvf::ref<RigResultAccessor> resultAccessor = RigResultAccessorFactory::createFromResultDefinition(eclipseCaseData, gridIndex, i, resultDefinition);
        if (resultAccessor.notNull())
        {
            values.push_back(resultAccessor->cellScalar(cellIndex));
        }
        else
        {
            values.push_back(hugeVal.cellScalar(cellIndex));
        }
    }

    return values;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RigTimeHistoryResultAccessor::geometrySelectionText(RigEclipseCaseData* eclipseCaseData, size_t gridIndex, size_t cellIndex)
{
    QString text;

    if (eclipseCaseData)
    {
        if (cellIndex != cvf::UNDEFINED_SIZE_T)
        {
            size_t i = 0;
            size_t j = 0;
            size_t k = 0;
            if (eclipseCaseData->grid(gridIndex)->ijkFromCellIndex(cellIndex, &i, &j, &k))
            {
                // Adjust to 1-based Eclipse indexing
                i++;
                j++;
                k++;

                text += QString("Cell : [%1, %2, %3]").arg(i).arg(j).arg(k);
            }
        }
    }

    return text;
}

