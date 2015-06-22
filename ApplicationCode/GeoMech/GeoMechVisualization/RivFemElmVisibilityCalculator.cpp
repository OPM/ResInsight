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



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivFemElmVisibilityCalculator::computePropertyVisibility(cvf::UByteArray* cellVisibility, 
                                                              const RigFemPart* grid, 
                                                              int timeStepIndex, 
                                                              const cvf::UByteArray* rangeFilterVisibility, 
                                                              RimGeoMechPropertyFilterCollection* propFilterColl)
{
#if 0
    CVF_ASSERT(cellVisibility != NULL);
    CVF_ASSERT(rangeFilterVisibility != NULL);
    CVF_ASSERT(propFilterColl != NULL);

    CVF_ASSERT(grid->elementCount() > 0);
    CVF_ASSERT(rangeFilterVisibility->size() == grid->elementCount());

    // Copy if not equal
    if (cellVisibility != rangeFilterVisibility ) (*cellVisibility) = *rangeFilterVisibility;
    const int elementCount = grid->elementCount();

    if (propFilterColl->hasActiveFilters())
    {
        for (size_t i = 0; i < propFilterColl->propertyFilters().size(); i++)
        {
            RimCellPropertyFilter* propertyFilter = propFilterColl->propertyFilters()[i];

            if (propertyFilter->isActive() && propertyFilter->resultDefinition->hasResult())
            {
                const double lowerBound = propertyFilter->lowerBound();
                const double upperBound = propertyFilter->upperBound();

                RigFemResultAddress resVarAddress = propertyFilter->resultDefinition->resulAddress();

                size_t adjustedTimeStepIndex = timeStepIndex;

                // Set time step to zero for static results
                if (propertyFilter->resultDefinition()->hasStaticResult())
                {
                    adjustedTimeStepIndex = 0;
                }

                const RimCellFilter::FilterModeType filterType = propertyFilter->filterMode();

                RigGeoMechCaseData* caseData = propFilterColl->reservoirView()->geoMechCase()->geoMechCaseData();

                const std::vector<float>& resVals = caseData->femPartResults()->resultValues(resVarAddress, grid->elementPartId(), timeStepIndex);
                //#pragma omp parallel for schedule(dynamic)
                for (int cellIndex = 0; cellIndex < elementCount; cellIndex++)
                {
                    if ( (*cellVisibility)[cellIndex] )
                    {
                        RigElementType eType = grid->elementType(cellIndex);
                        int elmNodeCount = RigFemTypes::elmentNodeCount(eType);

                        const int* elmNodeIndices = grid->connectivities(cellIndex);
                        for(int enIdx = 0; enIdx < elmNodeCount; ++enIdx)
                        {
                            size_t resultValueIndex = cvf::UNDEFINED_SIZE_T;
                            if (resVarAddress.resultPosType == RIG_NODAL)
                            {
                                resultValueIndex = elmNodeIndices[enIdx];
                            }
                            else
                            {
                                resultValueIndex = grid->elementNodeResultIdx(cellIndex, enIdx);
                            }

                            double scalarValue = resVals[resultValueIndex];
                            if (lowerBound <= scalarValue && scalarValue <= upperBound)
                            {
                                if (filterType == RimCellFilter::EXCLUDE)
                                {
                                    (*cellVisibility)[cellIndex] = false;
                                }
                            }
                            else
                            {
                                if (filterType == RimCellFilter::INCLUDE)
                                {
                                    (*cellVisibility)[cellIndex] = false;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
#endif
}

