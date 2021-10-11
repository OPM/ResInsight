/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021- Equinor ASA
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

#include "RigVisibleCategoriesCalculator.h"

#include "RiaResultNames.h"

#include "RigEclipseCaseData.h"
#include "RigEclipseNativeVisibleCellsStatCalc.h"
#include "RigFault.h"
#include "RigFlowDiagResults.h"
#include "RigFlowDiagVisibleCellsStatCalc.h"
#include "RigMainGrid.h"
#include "RigNNCData.h"
#include "RigResultAccessorFactory.h"

#include "RimBoxIntersection.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseFaultColors.h"
#include "RimEclipseView.h"
#include "RimExtrudedCurveIntersection.h"
#include "RimFaultInView.h"
#include "RimFaultInViewCollection.h"
#include "RimIntersection.h"
#include "RimIntersectionCollection.h"
#include "RimIntersectionResultsDefinitionCollection.h"
#include "RimSurface.h"
#include "RimSurfaceCollection.h"
#include "RimSurfaceInView.h"
#include "RimSurfaceInViewCollection.h"

#include "RivIntersectionGeometryGeneratorInterface.h"

#include <cmath>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<int> RigVisibleCategoriesCalculator::visibleFlowDiagCategories( RimEclipseView&     eclView,
                                                                         RigFlowDiagResults& flowDiagResults,
                                                                         const RigFlowDiagResultAddress& resVarAddr,
                                                                         size_t                          timeStepIndex )
{
    cvf::ref<cvf::UByteArray> cellVisibilities = eclView.currentTotalCellVisibility();

    cvf::ref<RigFlowDiagVisibleCellsStatCalc> calculator =
        cvf::make_ref<RigFlowDiagVisibleCellsStatCalc>( &flowDiagResults, resVarAddr, cellVisibilities.p() );

    std::set<int> visibleTracers;
    calculator->uniqueValues( timeStepIndex, visibleTracers );

    return visibleTracers;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<size_t> RigVisibleCategoriesCalculator::visibleAllanCategories( RimEclipseView* eclView )
{
    if ( !( eclView && eclView->mainGrid() ) ) return {};

    RigNNCData* nncData = eclView->mainGrid()->nncData();

    std::set<size_t> usedAllanIndices;

    auto fnAllanNncResults = nncData->staticConnectionScalarResultByName( RiaResultNames::formationAllanResultName() );
    if ( fnAllanNncResults )
    {
        auto visibleConnectionIndices = visibleNncConnectionIndices( eclView );
        for ( auto connIdx : visibleConnectionIndices )
        {
            auto allanValue = fnAllanNncResults->at( connIdx );

            usedAllanIndices.insert( allanValue );
        }
    }

    return usedAllanIndices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<int> RigVisibleCategoriesCalculator::visibleCategories( RimEclipseView* eclView )
{
    std::set<int> visibleCategoryValues;

    {
        // Visible eclipse grid cells

        RimEclipseResultDefinition* resDef = eclView->cellResult();

        RigEclipseNativeVisibleCellsStatCalc calc( resDef->currentGridCellResults(),
                                                   resDef->eclipseResultAddress(),
                                                   eclView->currentTotalCellVisibility().p() );

        calc.uniqueValues( eclView->currentTimeStep(), visibleCategoryValues );
    }

    {
        // Visible cells in faults and intersections

        std::set<size_t> visibleReservoirCells;
        RigVisibleCategoriesCalculator::appendVisibleFaultCells( eclView, visibleReservoirCells );
        RigVisibleCategoriesCalculator::appendVisibleIntersectionCells( eclView, visibleReservoirCells );

        RimEclipseResultDefinition* resDef = eclView->cellResult();

        cvf::ref<RigResultAccessor> resultAccessor =
            RigResultAccessorFactory::createFromResultDefinition( eclView->eclipseCase()->eclipseCaseData(),
                                                                  0,
                                                                  eclView->currentTimeStep(),
                                                                  resDef );

        if ( resultAccessor.notNull() )
        {
            for ( auto cIdx : visibleReservoirCells )
            {
                const auto resultVal = resultAccessor->cellScalarGlobIdx( cIdx );
                if ( resultVal != HUGE_VAL )
                {
                    visibleCategoryValues.insert( resultVal );
                }
            }
        }
    }

    return visibleCategoryValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<size_t> RigVisibleCategoriesCalculator::visibleNncConnectionIndices( RimEclipseView* eclView )
{
    if ( !eclView->faultCollection() || !eclView->faultCollection()->showFaultCollection ) return {};

    std::set<size_t> visibleConnectionIndices;

    std::vector<RimFaultInView*> visibleFaults;
    for ( auto f : eclView->faultCollection()->faults() )
    {
        if ( f->showFault() )
        {
            visibleFaults.push_back( f );

            auto nncConnectionIndices = f->faultGeometry()->connectionIndices();
            for ( const auto& c : nncConnectionIndices )
            {
                visibleConnectionIndices.insert( c );
            }
        }
    }

    return visibleConnectionIndices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigVisibleCategoriesCalculator::appendVisibleFaultCells( RimEclipseView* eclView, std::set<size_t>& visibleCells )
{
    if ( eclView->faultCollection() && eclView->faultCollection()->showFaultCollection &&
         !eclView->faultResultSettings()->showCustomFaultResult() )
    {
        for ( const auto& f : eclView->faultCollection()->faults() )
        {
            if ( f->showFault() )
            {
                for ( const auto& faultFace : f->faultGeometry()->faultFaces() )
                {
                    visibleCells.insert( faultFace.m_nativeReservoirCellIndex );
                    visibleCells.insert( faultFace.m_oppositeReservoirCellIndex );
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigVisibleCategoriesCalculator::appendVisibleIntersectionCells( RimEclipseView* eclView, std::set<size_t>& visibleCells )
{
    // Intersections
    std::vector<const RivIntersectionGeometryGeneratorInterface*> intersectionGeoGenerators;

    if ( !eclView->separateIntersectionResultsCollection()->isActive() )
    {
        if ( eclView->intersectionCollection()->isActive() )
        {
            for ( auto intersection : eclView->intersectionCollection()->intersections() )
            {
                if ( intersection->isActive() )
                {
                    auto geoGenerator = intersection->intersectionGeometryGenerator();
                    if ( geoGenerator )
                    {
                        intersectionGeoGenerators.push_back( geoGenerator );
                    }
                }
            }

            for ( auto intersection : eclView->intersectionCollection()->intersectionBoxes() )
            {
                if ( intersection->isActive() )
                {
                    auto geoGenerator = intersection->intersectionGeometryGenerator();
                    if ( geoGenerator )
                    {
                        intersectionGeoGenerators.push_back( geoGenerator );
                    }
                }
            }
        }
    }

    if ( eclView->separateSurfaceResultsCollection()->isActive() )
    {
        // Surfaces in view

        if ( eclView->surfaceInViewCollection() && eclView->surfaceInViewCollection()->isChecked() )
        {
            auto geoGenerators = eclView->surfaceInViewCollection()->intersectionGeometryGenerators();
            intersectionGeoGenerators.insert( intersectionGeoGenerators.end(), geoGenerators.begin(), geoGenerators.end() );
        }
    }

    for ( const auto geoGenerator : intersectionGeoGenerators )
    {
        for ( const auto& cIdx : geoGenerator->triangleToCellIndex() )
        {
            visibleCells.insert( cIdx );
        }
    }
}
