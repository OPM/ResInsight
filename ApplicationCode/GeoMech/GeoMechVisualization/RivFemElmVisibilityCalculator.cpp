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

#include "RivFemElmVisibilityCalculator.h"
#include "cvfBase.h"
#include "RigFemPart.h"
#include "RigFemPartGrid.h"
#include "cvfStructGrid.h"
#include "cvfStructGridGeometryGenerator.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivFemElmVisibilityCalculator::computeAllVisible(cvf::UByteArray* elmVisibilities, const RigFemPart* femPart)
{
    elmVisibilities->resize(femPart->elementCount());
    elmVisibilities->setAll(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivFemElmVisibilityCalculator::computeRangeVisibility(cvf::UByteArray* elmVisibilities, RigFemPart* femPart, 
                                                        const cvf::CellRangeFilter& rangeFilter)
{
    elmVisibilities->resize(femPart->elementCount());
    
    const RigFemPartGrid* grid = femPart->structGrid();
 
    if (rangeFilter.hasIncludeRanges())
    {
        for (int elmIdx = 0; elmIdx < femPart->elementCount(); ++elmIdx)
        {
            size_t mainGridI;
            size_t mainGridJ;
            size_t mainGridK;

            grid->ijkFromCellIndex(elmIdx, &mainGridI, &mainGridJ, &mainGridK);
            (*elmVisibilities)[elmIdx] = rangeFilter.isCellVisible(mainGridI, mainGridJ, mainGridK, false);
        }
    }
    else
    {
        for (int elmIdx = 0; elmIdx < femPart->elementCount(); ++elmIdx)
        {
            size_t mainGridI;
            size_t mainGridJ;
            size_t mainGridK;

            grid->ijkFromCellIndex(elmIdx, &mainGridI, &mainGridJ, &mainGridK);
            (*elmVisibilities)[elmIdx] = !rangeFilter.isCellExcluded(mainGridI, mainGridJ, mainGridK, false);
        }
    }
}



