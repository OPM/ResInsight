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
#include "RigFemAddressDefines.h"
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

#include "cvfStructGrid.h"
#include "cvfStructGridGeometryGenerator.h"

#include <cmath>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivFemElmVisibilityCalculator::computeAllVisible( cvf::UByteArray* elmVisibilities, const RigFemPart* femPart )
{
    elmVisibilities->resize( femPart->elementCount() );
    elmVisibilities->setAll( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivFemElmVisibilityCalculator::computeRangeVisibility( cvf::UByteArray*            elmVisibilities,
                                                            const RigFemPart*           femPart,
                                                            const cvf::CellRangeFilter& rangeFilter,
                                                            const cvf::UByteArray*      indexIncludeVisibility,
                                                            const cvf::UByteArray*      indexExcludeVisibility,
                                                            bool                        useIndexInclude,
                                                            bool                        useAndOperation )
{
    elmVisibilities->resize( femPart->elementCount() );

    const RigFemPartGrid* grid = femPart->getOrCreateStructGrid();

    size_t mainGridI;
    size_t mainGridJ;
    size_t mainGridK;

    if ( rangeFilter.hasIncludeRanges() )
    {
        if ( useIndexInclude )
        {
            for ( int elmIdx = 0; elmIdx < femPart->elementCount(); ++elmIdx )
            {
                grid->ijkFromCellIndex( elmIdx, &mainGridI, &mainGridJ, &mainGridK );

                if ( useAndOperation )
                {
                    ( *elmVisibilities )[elmIdx] =
                        ( ( *indexIncludeVisibility )[elmIdx] && rangeFilter.isCellVisible( mainGridI, mainGridJ, mainGridK, false ) ) &&
                        ( *indexExcludeVisibility )[elmIdx];
                }
                else
                {
                    ( *elmVisibilities )[elmIdx] =
                        ( ( *indexIncludeVisibility )[elmIdx] || rangeFilter.isCellVisible( mainGridI, mainGridJ, mainGridK, false ) ) &&
                        ( *indexExcludeVisibility )[elmIdx];
                }
            }
        }
        else
        {
            for ( int elmIdx = 0; elmIdx < femPart->elementCount(); ++elmIdx )
            {
                grid->ijkFromCellIndex( elmIdx, &mainGridI, &mainGridJ, &mainGridK );
                ( *elmVisibilities )[elmIdx] = rangeFilter.isCellVisible( mainGridI, mainGridJ, mainGridK, false ) &&
                                               ( *indexExcludeVisibility )[elmIdx];
            }
        }
    }
    else
    {
        if ( useIndexInclude )
        {
            for ( int elmIdx = 0; elmIdx < femPart->elementCount(); ++elmIdx )
            {
                grid->ijkFromCellIndex( elmIdx, &mainGridI, &mainGridJ, &mainGridK );
                ( *elmVisibilities )[elmIdx] = ( *indexIncludeVisibility )[elmIdx] &&
                                               !rangeFilter.isCellExcluded( mainGridI, mainGridJ, mainGridK, false ) &&
                                               ( *indexExcludeVisibility )[elmIdx];
            }
        }
        else
        {
            for ( int elmIdx = 0; elmIdx < femPart->elementCount(); ++elmIdx )
            {
                grid->ijkFromCellIndex( elmIdx, &mainGridI, &mainGridJ, &mainGridK );
                ( *elmVisibilities )[elmIdx] = !rangeFilter.isCellExcluded( mainGridI, mainGridJ, mainGridK, false ) &&
                                               ( *indexExcludeVisibility )[elmIdx];
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivFemElmVisibilityCalculator::computePropertyVisibility( cvf::UByteArray*                    cellVisibility,
                                                               const RigFemPart*                   part,
                                                               int                                 timeStepIndex,
                                                               int                                 frameIndex,
                                                               const cvf::UByteArray*              rangeFilterVisibility,
                                                               RimGeoMechPropertyFilterCollection* propFilterColl )
{
    CVF_ASSERT( cellVisibility != nullptr );
    CVF_ASSERT( rangeFilterVisibility != nullptr );
    CVF_ASSERT( propFilterColl != nullptr );

    CVF_ASSERT( part->elementCount() > 0 );
    CVF_ASSERT( rangeFilterVisibility->size() == static_cast<size_t>( part->elementCount() ) );

    // Copy if not equal
    if ( cellVisibility != rangeFilterVisibility ) ( *cellVisibility ) = *rangeFilterVisibility;
    const int elementCount = part->elementCount();

    if ( !propFilterColl->hasActiveFilters() ) return;

    for ( size_t i = 0; i < propFilterColl->propertyFilters().size(); i++ )
    {
        RimGeoMechPropertyFilter* propertyFilter = propFilterColl->propertyFilters()[i];

        if ( !propertyFilter->isActiveAndHasResult() ) continue;

        const RimCellFilter::FilterModeType filterType = propertyFilter->filterMode();

        RigGeoMechCaseData* caseData = propFilterColl->reservoirView()->geoMechCase()->geoMechData();

        RigFemResultAddress resVarAddress = RigFemAddressDefines::getResultLookupAddress( propertyFilter->resultDefinition()->resultAddress() );

        const std::vector<float>& resVals =
            caseData->femPartResults()->resultValues( resVarAddress, part->elementPartId(), timeStepIndex, frameIndex );

        if ( !propertyFilter->isActive() ) continue;
        if ( !propertyFilter->resultDefinition()->hasResult() ) continue;
        if ( resVals.empty() ) continue;

        const double lowerBound = propertyFilter->lowerBound();
        const double upperBound = propertyFilter->upperBound();

        if ( propertyFilter->resultDefinition()->resultAddress().resultPosType == RIG_FORMATION_NAMES )
        {
            std::vector<int> integerVector = propertyFilter->selectedCategoryValues();
            std::set<int>    integerSet;
            for ( auto val : integerVector )
            {
                integerSet.insert( val );
            }

            for ( int cellIndex = 0; cellIndex < elementCount; cellIndex++ )
            {
                if ( !( *cellVisibility )[cellIndex] ) continue;

                size_t resultValueIndex = part->elementNodeResultIdx( cellIndex, 0 );
                double scalarValue      = resVals[resultValueIndex];
                int    intValue         = nearbyint( scalarValue );
                if ( integerSet.find( intValue ) != integerSet.end() )
                {
                    if ( filterType == RimCellFilter::EXCLUDE )
                    {
                        ( *cellVisibility )[cellIndex] = false;
                    }
                }
                else
                {
                    if ( filterType == RimCellFilter::INCLUDE )
                    {
                        ( *cellVisibility )[cellIndex] = false;
                    }
                }
            }
        }
        else if ( resVarAddress.resultPosType == RIG_ELEMENT )
        {
#pragma omp parallel for schedule( dynamic )
            for ( int cellIndex = 0; cellIndex < elementCount; cellIndex++ )
            {
                if ( !( *cellVisibility )[cellIndex] ) continue;

                double scalarValue = resVals[cellIndex];
                evaluateAndSetCellVisibiliy( cellIndex, scalarValue, lowerBound, upperBound, filterType, cellVisibility );
            }
        }
        else if ( resVarAddress.resultPosType == RIG_ELEMENT_NODAL_FACE )
        {
#pragma omp parallel for schedule( dynamic )
            for ( int cellIndex = 0; cellIndex < elementCount; cellIndex++ )
            {
                if ( !( *cellVisibility )[cellIndex] ) continue;

                for ( int fpIdx = 0; fpIdx < 24; ++fpIdx )
                {
                    double scalarValue = resVals[cellIndex * 24 + fpIdx];
                    evaluateAndSetCellVisibiliy( cellIndex, scalarValue, lowerBound, upperBound, filterType, cellVisibility );
                }
            }
        }
        else
        {
#pragma omp parallel for schedule( dynamic )
            for ( int cellIndex = 0; cellIndex < elementCount; cellIndex++ )
            {
                if ( !( *cellVisibility )[cellIndex] ) continue;

                RigElementType eType        = part->elementType( cellIndex );
                int            elmNodeCount = RigFemTypes::elementNodeCount( eType );

                const int* elmNodeIndices = part->connectivities( cellIndex );
                for ( int enIdx = 0; enIdx < elmNodeCount; ++enIdx )
                {
                    size_t resultValueIndex = cvf::UNDEFINED_SIZE_T;
                    if ( resVarAddress.resultPosType == RIG_NODAL )
                    {
                        resultValueIndex = elmNodeIndices[enIdx];
                    }
                    else
                    {
                        resultValueIndex = part->elementNodeResultIdx( cellIndex, enIdx );
                    }

                    double scalarValue = resVals[resultValueIndex];
                    evaluateAndSetCellVisibiliy( cellIndex, scalarValue, lowerBound, upperBound, filterType, cellVisibility );
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivFemElmVisibilityCalculator::evaluateAndSetCellVisibiliy( int                                 cellIndex,
                                                                 double                              scalarValue,
                                                                 double                              lowerBound,
                                                                 double                              upperBound,
                                                                 const RimCellFilter::FilterModeType filterType,
                                                                 cvf::UByteArray*                    cellVisibility )
{
    if ( lowerBound <= scalarValue && scalarValue <= upperBound )
    {
        if ( filterType == RimCellFilter::EXCLUDE )
        {
            ( *cellVisibility )[cellIndex] = false;
        }
    }
    else
    {
        if ( filterType == RimCellFilter::INCLUDE )
        {
            ( *cellVisibility )[cellIndex] = false;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivFemElmVisibilityCalculator::computeOverriddenCellVisibility( cvf::UByteArray*   elmVisibilities,
                                                                     const RigFemPart*  femPart,
                                                                     RimViewController* masterViewLink )
{
    CVF_ASSERT( elmVisibilities != nullptr );
    CVF_ASSERT( femPart != nullptr );

    RimGridView* masterView = dynamic_cast<RimGridView*>( masterViewLink->ownerViewLinker()->masterView() );
    if ( !masterView ) return;

    cvf::ref<cvf::UByteArray> totCellVisibility = masterView->currentTotalCellVisibility();

    int elmCount = femPart->elementCount();
    elmVisibilities->resize( elmCount );
    elmVisibilities->setAll( false );

    const RigCaseToCaseCellMapper* cellMapper = masterViewLink->cellMapper();

    for ( int elmIdx = 0; elmIdx < elmCount; ++elmIdx )
    {
        // We are assuming that there is only one part.
        int        cellCount               = 0;
        const int* cellIndicesInMasterCase = cellMapper->masterCaseCellIndices( elmIdx, &cellCount );

        for ( int mcIdx = 0; mcIdx < cellCount; ++mcIdx )
        {
            ( *elmVisibilities )[elmIdx] |= ( *totCellVisibility )[cellIndicesInMasterCase[mcIdx]]; // If any is
                                                                                                    // visible, show
        }
    }
}
