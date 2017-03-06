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

#include "RigCaseToCaseCellMapper.h"
#include "RigFemPart.h"
#include "RigFemPartGrid.h"
#include "RigFemPartResultsCollection.h"
#include "RigGeoMechCaseData.h"

#include "RimEclipsePropertyFilter.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechPropertyFilter.h"
#include "RimGeoMechPropertyFilterCollection.h"
#include "RimGeoMechResultDefinition.h"
#include "RimGeoMechView.h"
#include "RimViewController.h"
#include "RimViewLinker.h"

#include "cvfBase.h"
#include "cvfStructGrid.h"
#include "cvfStructGridGeometryGenerator.h"

#include <cmath>

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
    CVF_ASSERT(cellVisibility != NULL);
    CVF_ASSERT(rangeFilterVisibility != NULL);
    CVF_ASSERT(propFilterColl != NULL);

    CVF_ASSERT(grid->elementCount() > 0);
    CVF_ASSERT(rangeFilterVisibility->size() == static_cast<size_t>(grid->elementCount()));

    // Copy if not equal
    if (cellVisibility != rangeFilterVisibility ) (*cellVisibility) = *rangeFilterVisibility;
    const int elementCount = grid->elementCount();

    if (propFilterColl->hasActiveFilters())
    {
        for (size_t i = 0; i < propFilterColl->propertyFilters().size(); i++)
        {
            RimGeoMechPropertyFilter* propertyFilter = propFilterColl->propertyFilters()[i];

            const RimCellFilter::FilterModeType filterType = propertyFilter->filterMode();

            RigGeoMechCaseData* caseData = propFilterColl->reservoirView()->geoMechCase()->geoMechData();

            RigFemResultAddress resVarAddress = propertyFilter->resultDefinition->resultAddress();

            // Do a "Hack" to use elm nodal and not nodal POR results
            if ( resVarAddress.resultPosType == RIG_NODAL && resVarAddress.fieldName == "POR-Bar" ) resVarAddress.resultPosType = RIG_ELEMENT_NODAL;

            const std::vector<float>& resVals = caseData->femPartResults()->resultValues(resVarAddress,
                                                                                         grid->elementPartId(),
                                                                                         timeStepIndex);

            if (propertyFilter->isActive() && propertyFilter->resultDefinition->hasResult())
            {
                if (propertyFilter->resultDefinition->resultAddress().resultPosType == RIG_FORMATION_NAMES)
                {
                    std::vector<int> integerVector = propertyFilter->selectedCategoryValues();
                    std::set<int> integerSet;
                    for (auto val : integerVector)
                    {
                        integerSet.insert(val);
                    }

                    for (int cellIndex = 0; cellIndex < elementCount; cellIndex++)
                    {
                        if ((*cellVisibility)[cellIndex])
                        {
                            size_t resultValueIndex = grid->elementNodeResultIdx(cellIndex, 0);
                            double scalarValue = resVals[resultValueIndex];
                            int intValue = nearbyint(scalarValue);
                            if (integerSet.find(intValue) != integerSet.end())
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
                else
                {
                    const double lowerBound = propertyFilter->lowerBound();
                    const double upperBound = propertyFilter->upperBound();

 
                    if (resVarAddress.resultPosType != RIG_ELEMENT_NODAL_FACE)
                    {
                        #pragma omp parallel for schedule(dynamic)
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
                    else
                    {
                        #pragma omp parallel for schedule(dynamic)
                        for(int cellIndex = 0; cellIndex < elementCount; cellIndex++)
                        {
                            if((*cellVisibility)[cellIndex])
                            {
                                for(int fpIdx = 0; fpIdx < 24; ++fpIdx)
                                {
                                    double scalarValue = resVals[cellIndex*24 + fpIdx];
                                    if(lowerBound <= scalarValue && scalarValue <= upperBound)
                                    {
                                        if(filterType == RimCellFilter::EXCLUDE)
                                        {
                                            (*cellVisibility)[cellIndex] = false;
                                        }
                                    }
                                    else
                                    {
                                        if(filterType == RimCellFilter::INCLUDE)
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
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivFemElmVisibilityCalculator::computeOverriddenCellVisibility(cvf::UByteArray* elmVisibilities, 
                                                                    const RigFemPart* femPart, 
                                                                    RimViewController* masterViewLink)
{
    CVF_ASSERT(elmVisibilities != NULL);
    CVF_ASSERT(femPart != NULL);

    RimView* masterView = masterViewLink->ownerViewLinker()->masterView();
    cvf::ref<cvf::UByteArray> totCellVisibility =  masterView->currentTotalCellVisibility();

    int elmCount = femPart->elementCount();
    elmVisibilities->resize(elmCount);
    elmVisibilities->setAll(false);

    const RigCaseToCaseCellMapper* cellMapper = masterViewLink->cellMapper();

    for (int elmIdx = 0; elmIdx < elmCount; ++elmIdx)
    {
        // We are assuming that there is only one part.  
        int cellCount = 0;
        const int* cellIndicesInMasterCase = cellMapper->masterCaseCellIndices(elmIdx, &cellCount);
        
        for (int mcIdx = 0; mcIdx < cellCount; ++mcIdx)
        {
            (*elmVisibilities)[elmIdx] |=  (*totCellVisibility)[cellIndicesInMasterCase[mcIdx]]; // If any is visible, show
        }

    }
}

