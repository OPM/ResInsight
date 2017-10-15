/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RivReservoirViewPartMgr.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigCaseToCaseCellMapper.h"
#include "RigCell.h"
#include "RigGridBase.h"
#include "RigResultAccessorFactory.h"

#include "Rim3dOverlayInfoConfig.h"
#include "RimCellEdgeColors.h"
#include "RimCellRangeFilterCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipsePropertyFilter.h"
#include "RimEclipsePropertyFilterCollection.h"
#include "RimEclipseResultDefinition.h"
#include "RimEclipseView.h"
#include "RimFaultInViewCollection.h"
#include "RimReservoirCellResultsStorage.h"
#include "RimSimWellInViewCollection.h"
#include "RimViewController.h"
#include "RimViewLinker.h"

#include "RivGridPartMgr.h"
#include "RivReservoirFaultsPartMgr.h"

#include <QDebug>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivReservoirViewPartMgr::RivReservoirViewPartMgr(RimEclipseView * resv) :
m_reservoirView(resv)
{
    m_scaleTransform = new cvf::Transform();
    clearGeometryCache();
}


//--------------------------------------------------------------------------------------------------
/// Clears the geometry cache for the given, and the dependent geometryTypes (from a visibility standpoint)
//--------------------------------------------------------------------------------------------------
void RivReservoirViewPartMgr::scheduleGeometryRegen(RivCellSetEnum geometryType)
{
    switch (geometryType)
    {
    case OVERRIDDEN_CELL_VISIBILITY:
        clearGeometryCache(OVERRIDDEN_CELL_VISIBILITY);
    case INACTIVE:
        clearGeometryCache(INACTIVE);
        clearGeometryCache(RANGE_FILTERED_INACTIVE);
        break;  
    case RANGE_FILTERED_INACTIVE:
        clearGeometryCache(RANGE_FILTERED_INACTIVE);
        break;
    case ACTIVE:
        clearGeometryCache(ACTIVE);
        clearGeometryCache(ALL_WELL_CELLS);
        clearGeometryCache(VISIBLE_WELL_CELLS);
        clearGeometryCache(VISIBLE_WELL_FENCE_CELLS);
        clearGeometryCache(RANGE_FILTERED);
        clearGeometryCache(RANGE_FILTERED_WELL_CELLS);
        clearGeometryCache(VISIBLE_WELL_CELLS_OUTSIDE_RANGE_FILTER);
        clearGeometryCache(VISIBLE_WELL_FENCE_CELLS_OUTSIDE_RANGE_FILTER);
        clearGeometryCache(PROPERTY_FILTERED);
        clearGeometryCache(PROPERTY_FILTERED_WELL_CELLS);
        break;
    case ALL_WELL_CELLS:
        clearGeometryCache(ALL_WELL_CELLS);
        clearGeometryCache(VISIBLE_WELL_CELLS);
        clearGeometryCache(VISIBLE_WELL_FENCE_CELLS);
        clearGeometryCache(RANGE_FILTERED);
        clearGeometryCache(RANGE_FILTERED_WELL_CELLS);
        clearGeometryCache(VISIBLE_WELL_CELLS_OUTSIDE_RANGE_FILTER);
        clearGeometryCache(VISIBLE_WELL_FENCE_CELLS_OUTSIDE_RANGE_FILTER);
        clearGeometryCache(PROPERTY_FILTERED_WELL_CELLS);
        break;
    case VISIBLE_WELL_CELLS:
        clearGeometryCache(VISIBLE_WELL_CELLS);
        clearGeometryCache(VISIBLE_WELL_FENCE_CELLS);
        clearGeometryCache(VISIBLE_WELL_CELLS_OUTSIDE_RANGE_FILTER);
        clearGeometryCache(VISIBLE_WELL_FENCE_CELLS_OUTSIDE_RANGE_FILTER);
        clearGeometryCache(PROPERTY_FILTERED);
        clearGeometryCache(PROPERTY_FILTERED_WELL_CELLS);
        break;
    case VISIBLE_WELL_FENCE_CELLS:
        clearGeometryCache(VISIBLE_WELL_FENCE_CELLS);
        clearGeometryCache(VISIBLE_WELL_FENCE_CELLS_OUTSIDE_RANGE_FILTER);
        clearGeometryCache(PROPERTY_FILTERED_WELL_CELLS);
        break;
    case RANGE_FILTERED:
        clearGeometryCache(RANGE_FILTERED);
        clearGeometryCache(RANGE_FILTERED_INACTIVE);
        clearGeometryCache(RANGE_FILTERED_WELL_CELLS);
        clearGeometryCache(VISIBLE_WELL_CELLS_OUTSIDE_RANGE_FILTER);
        clearGeometryCache(VISIBLE_WELL_FENCE_CELLS_OUTSIDE_RANGE_FILTER);
        clearGeometryCache(PROPERTY_FILTERED);
        clearGeometryCache(PROPERTY_FILTERED_WELL_CELLS);
        break;
    case RANGE_FILTERED_WELL_CELLS:
        clearGeometryCache(RANGE_FILTERED_WELL_CELLS);
        clearGeometryCache(RANGE_FILTERED);
        clearGeometryCache(VISIBLE_WELL_CELLS_OUTSIDE_RANGE_FILTER);
        clearGeometryCache(VISIBLE_WELL_FENCE_CELLS_OUTSIDE_RANGE_FILTER);
        clearGeometryCache(PROPERTY_FILTERED_WELL_CELLS);
        break;
    case VISIBLE_WELL_CELLS_OUTSIDE_RANGE_FILTER:
        clearGeometryCache(VISIBLE_WELL_CELLS_OUTSIDE_RANGE_FILTER);
        clearGeometryCache(VISIBLE_WELL_FENCE_CELLS_OUTSIDE_RANGE_FILTER);
        clearGeometryCache(PROPERTY_FILTERED_WELL_CELLS);
        break;
    case VISIBLE_WELL_FENCE_CELLS_OUTSIDE_RANGE_FILTER:
        clearGeometryCache(VISIBLE_WELL_FENCE_CELLS_OUTSIDE_RANGE_FILTER);
        clearGeometryCache(PROPERTY_FILTERED_WELL_CELLS);
        break;
    case PROPERTY_FILTERED:
        clearGeometryCache(PROPERTY_FILTERED);
        clearGeometryCache(PROPERTY_FILTERED_WELL_CELLS);
        break;
    case PROPERTY_FILTERED_WELL_CELLS:
        clearGeometryCache(PROPERTY_FILTERED_WELL_CELLS);
        break;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirViewPartMgr::clearGeometryCache(RivCellSetEnum geomType)
{
    RigEclipseCaseData* eclipseCase = NULL;
    if (m_reservoirView != NULL && m_reservoirView->eclipseCase())
    {
        eclipseCase = m_reservoirView->eclipseCase()->eclipseCaseData();
    }

    if (geomType == PROPERTY_FILTERED)
    {
        for (size_t i = 0; i < m_propFilteredGeometryFramesNeedsRegen.size(); ++i)
        {
            m_propFilteredGeometryFramesNeedsRegen[i] = true;
            if (m_propFilteredGeometryFrames[i].notNull())
            {
                m_propFilteredGeometryFrames[i]->clearAndSetReservoir(eclipseCase, m_reservoirView);
                m_propFilteredGeometryFrames[i]->setTransform(m_scaleTransform.p());
            }
        }
    }
    else if (geomType == PROPERTY_FILTERED_WELL_CELLS)
    {
        for (size_t i = 0; i < m_propFilteredWellGeometryFramesNeedsRegen.size(); ++i)
        {
            m_propFilteredWellGeometryFramesNeedsRegen[i] = true;
            if (m_propFilteredWellGeometryFrames[i].notNull())
            {
                m_propFilteredWellGeometryFrames[i]->clearAndSetReservoir(eclipseCase, m_reservoirView);
                m_propFilteredWellGeometryFrames[i]->setTransform(m_scaleTransform.p());
            }
        }
    }
    else
    {
        m_geometriesNeedsRegen[geomType] = true;
        m_geometries[geomType].clearAndSetReservoir(eclipseCase, m_reservoirView);
        m_geometries[geomType].setTransform(m_scaleTransform.p());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirViewPartMgr::clearGeometryCache()
{
    clearGeometryCache(OVERRIDDEN_CELL_VISIBILITY);
    clearGeometryCache(ACTIVE);
    clearGeometryCache(ALL_WELL_CELLS);
    clearGeometryCache(VISIBLE_WELL_CELLS);
    clearGeometryCache(VISIBLE_WELL_FENCE_CELLS);
    clearGeometryCache(INACTIVE);
    clearGeometryCache(RANGE_FILTERED);
    clearGeometryCache(RANGE_FILTERED_WELL_CELLS);
    clearGeometryCache(RANGE_FILTERED_INACTIVE);
    clearGeometryCache(VISIBLE_WELL_CELLS_OUTSIDE_RANGE_FILTER);
    clearGeometryCache(VISIBLE_WELL_FENCE_CELLS_OUTSIDE_RANGE_FILTER);
    clearGeometryCache(PROPERTY_FILTERED);
    clearGeometryCache(PROPERTY_FILTERED_WELL_CELLS);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirViewPartMgr::appendStaticGeometryPartsToModel(cvf::ModelBasicList* model, RivCellSetEnum geometryType, 
                                                                const std::vector<size_t>& gridIndices)
{
    if (m_geometriesNeedsRegen[geometryType])
    {
        createGeometry( geometryType);
    }
    m_geometries[geometryType].appendGridPartsToModel(model, gridIndices);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirViewPartMgr::appendDynamicGeometryPartsToModel(cvf::ModelBasicList* model, RivCellSetEnum geometryType, 
                                                                 size_t frameIndex, const std::vector<size_t>& gridIndices)
{
    if (geometryType == PROPERTY_FILTERED)
    {
        if (frameIndex >= m_propFilteredGeometryFramesNeedsRegen.size() || m_propFilteredGeometryFramesNeedsRegen[frameIndex])
        {
            createPropertyFilteredNoneWellCellGeometry(frameIndex);
        }
        m_propFilteredGeometryFrames[frameIndex]->appendGridPartsToModel(model, gridIndices);
    }
    else if (geometryType == PROPERTY_FILTERED_WELL_CELLS)
    {
        if (frameIndex >= m_propFilteredWellGeometryFramesNeedsRegen.size() || m_propFilteredWellGeometryFramesNeedsRegen[frameIndex])
        {
            createPropertyFilteredWellGeometry(frameIndex);
        }
        m_propFilteredWellGeometryFrames[frameIndex]->appendGridPartsToModel(model, gridIndices);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirViewPartMgr::createGeometry(RivCellSetEnum geometryType)
{
    RigEclipseCaseData* res = m_reservoirView->eclipseCase()->eclipseCaseData();
    m_geometries[geometryType].clearAndSetReservoir(res, m_reservoirView);
    m_geometries[geometryType].setTransform(m_scaleTransform.p());
    
    std::vector<RigGridBase*> grids;
    res->allGrids(&grids);

    for (size_t i = 0; i < grids.size(); ++i)
    {
        cvf::ref<cvf::UByteArray> cellVisibility = m_geometries[geometryType].cellVisibility(i); 
        computeVisibility(cellVisibility.p(), geometryType, grids[i], i);
        
        m_geometries[geometryType].setCellVisibility(i, cellVisibility.p());
    }

    m_geometriesNeedsRegen[geometryType] = false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirViewPartMgr::computeVisibility(cvf::UByteArray* cellVisibility, RivCellSetEnum geometryType, RigGridBase* grid, size_t gridIdx)
{
    RigEclipseCaseData* eclipseCase = m_reservoirView->eclipseCase()->eclipseCaseData();
    RigActiveCellInfo* activeCellInfo = m_reservoirView->currentActiveCellInfo();

    switch (geometryType)
    {
    case OVERRIDDEN_CELL_VISIBILITY:
        computeOverriddenCellVisibility(cellVisibility, grid);
    break;
    case ACTIVE:
        computeNativeVisibility(cellVisibility, grid, activeCellInfo, eclipseCase->wellCellsInGrid(gridIdx), false, false, true, m_reservoirView->showMainGrid() );
        break;
    case ALL_WELL_CELLS:
        copyByteArray(cellVisibility, eclipseCase->wellCellsInGrid(gridIdx));
        break;
    case VISIBLE_WELL_CELLS:
        {
            cvf::ref<cvf::UByteArray> allWellCellsVisibility;
            if (m_geometriesNeedsRegen[ALL_WELL_CELLS]) createGeometry(ALL_WELL_CELLS);

            allWellCellsVisibility = m_geometries[ALL_WELL_CELLS].cellVisibility(gridIdx);

            m_reservoirView->calculateVisibleWellCellsIncFence(cellVisibility, grid);

#pragma omp parallel for
            for (int cellIdx = 0; cellIdx < static_cast<int>(cellVisibility->size()); ++cellIdx)
            {
                (*cellVisibility)[cellIdx] = (*allWellCellsVisibility)[cellIdx] && (*cellVisibility)[cellIdx];
            }
        }
        break;
    case VISIBLE_WELL_FENCE_CELLS:
        {
            cvf::ref<cvf::UByteArray> allWellCellsVisibility;
            if (m_geometriesNeedsRegen[ALL_WELL_CELLS]) createGeometry(ALL_WELL_CELLS);

            allWellCellsVisibility = m_geometries[ALL_WELL_CELLS].cellVisibility(gridIdx);

            m_reservoirView->calculateVisibleWellCellsIncFence(cellVisibility, grid);

#pragma omp parallel for
            for (int cellIdx = 0; cellIdx < static_cast<int>(cellVisibility->size()); ++cellIdx)
            {
                (*cellVisibility)[cellIdx] = !(*allWellCellsVisibility)[cellIdx] && (*cellVisibility)[cellIdx];
            }
        }
        break;
    case INACTIVE:
        computeNativeVisibility(cellVisibility, grid, activeCellInfo, eclipseCase->wellCellsInGrid(gridIdx),  m_reservoirView->showInvalidCells(), true, false, m_reservoirView->showMainGrid());
        break;
    case RANGE_FILTERED:
        {
            cvf::ref<cvf::UByteArray> nativeVisibility;
            if (m_geometriesNeedsRegen[ACTIVE]) createGeometry(ACTIVE);

            nativeVisibility = m_geometries[ACTIVE].cellVisibility(gridIdx);
            computeRangeVisibility(geometryType, cellVisibility, grid, nativeVisibility.p(), m_reservoirView->rangeFilterCollection());
        }
        break;
    case RANGE_FILTERED_INACTIVE:
        {
            cvf::ref<cvf::UByteArray> nativeVisibility;
            if (m_geometriesNeedsRegen[INACTIVE]) createGeometry(INACTIVE);

            nativeVisibility = m_geometries[INACTIVE].cellVisibility(gridIdx);
            computeRangeVisibility(geometryType, cellVisibility, grid, nativeVisibility.p(), m_reservoirView->rangeFilterCollection());
        }
        break;
    case RANGE_FILTERED_WELL_CELLS:
        {
            cvf::ref<cvf::UByteArray> nativeVisibility;
            if (m_geometriesNeedsRegen[ALL_WELL_CELLS]) createGeometry(ALL_WELL_CELLS);

            nativeVisibility = m_geometries[ALL_WELL_CELLS].cellVisibility(gridIdx);
            computeRangeVisibility(geometryType, cellVisibility, grid, nativeVisibility.p(), m_reservoirView->rangeFilterCollection());
        }
        break;
    case VISIBLE_WELL_CELLS_OUTSIDE_RANGE_FILTER:
        {
            cvf::ref<cvf::UByteArray> visibleWellCells;
            cvf::ref<cvf::UByteArray> rangeFilteredWellCells;

            if (m_geometriesNeedsRegen[VISIBLE_WELL_CELLS]) createGeometry(VISIBLE_WELL_CELLS);
            if (m_geometriesNeedsRegen[RANGE_FILTERED_WELL_CELLS]) createGeometry(RANGE_FILTERED_WELL_CELLS);

            visibleWellCells = m_geometries[VISIBLE_WELL_CELLS].cellVisibility(gridIdx);
            rangeFilteredWellCells = m_geometries[RANGE_FILTERED_WELL_CELLS].cellVisibility(gridIdx);

            cellVisibility->resize(visibleWellCells->size());

#pragma omp parallel for
            for (int cellIdx = 0; cellIdx < static_cast<int>(cellVisibility->size()); ++cellIdx)
            {
                (*cellVisibility)[cellIdx] = (*visibleWellCells)[cellIdx] && !(*rangeFilteredWellCells)[cellIdx];
            }
        }
        break;
    case VISIBLE_WELL_FENCE_CELLS_OUTSIDE_RANGE_FILTER:
        {
            cvf::ref<cvf::UByteArray> visibleWellCells;
            cvf::ref<cvf::UByteArray> rangeFilteredWellCells;

            if (m_geometriesNeedsRegen[VISIBLE_WELL_FENCE_CELLS]) createGeometry(VISIBLE_WELL_FENCE_CELLS);
            if (m_geometriesNeedsRegen[RANGE_FILTERED]) createGeometry(RANGE_FILTERED);

            visibleWellCells = m_geometries[VISIBLE_WELL_FENCE_CELLS].cellVisibility(gridIdx);
            rangeFilteredWellCells = m_geometries[RANGE_FILTERED].cellVisibility(gridIdx);

            cellVisibility->resize(visibleWellCells->size());

#pragma omp parallel for
            for (int cellIdx = 0; cellIdx < static_cast<int>(cellVisibility->size()); ++cellIdx)
            {
                (*cellVisibility)[cellIdx] = (*visibleWellCells)[cellIdx] && !(*rangeFilteredWellCells)[cellIdx];
            }
        }
        break;
    default:
        CVF_ASSERT(false); // Call special function for property filtered stuff
        break;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirViewPartMgr::createPropertyFilteredNoneWellCellGeometry(size_t frameIndex)
{
    RigEclipseCaseData* res = m_reservoirView->eclipseCase()->eclipseCaseData();

    if ( frameIndex >= m_propFilteredGeometryFrames.size())
    { 
        m_propFilteredGeometryFrames.resize(frameIndex + 1);
        m_propFilteredGeometryFramesNeedsRegen.resize(frameIndex + 1, true);
    }

    if ( m_propFilteredGeometryFrames[frameIndex].isNull())  m_propFilteredGeometryFrames[frameIndex] = new RivReservoirPartMgr;

    m_propFilteredGeometryFrames[frameIndex]->clearAndSetReservoir(res, m_reservoirView);
    m_propFilteredGeometryFrames[frameIndex]->setTransform(m_scaleTransform.p());

    std::vector<RigGridBase*> grids;
    res->allGrids(&grids);

    bool hasActiveRangeFilters = m_reservoirView->rangeFilterCollection()->hasActiveFilters();
    bool hasVisibleWellCells = m_reservoirView->wellCollection()->hasVisibleWellCells();

    for (size_t gIdx = 0; gIdx < grids.size(); ++gIdx)
    {
        cvf::ref<cvf::UByteArray> cellVisibility = m_propFilteredGeometryFrames[frameIndex]->cellVisibility(gIdx); 
        cvf::ref<cvf::UByteArray> rangeVisibility; 
        cvf::ref<cvf::UByteArray> fenceVisibility; 
        cvf::cref<cvf::UByteArray> isWellCell = res->wellCellsInGrid(gIdx); 

        if (hasActiveRangeFilters && hasVisibleWellCells)
        {
            if (m_geometriesNeedsRegen[RANGE_FILTERED]) createGeometry(RANGE_FILTERED);
            if (m_geometriesNeedsRegen[VISIBLE_WELL_FENCE_CELLS_OUTSIDE_RANGE_FILTER]) createGeometry(VISIBLE_WELL_FENCE_CELLS_OUTSIDE_RANGE_FILTER);

            rangeVisibility = m_geometries[RANGE_FILTERED].cellVisibility(gIdx);
            fenceVisibility = m_geometries[VISIBLE_WELL_FENCE_CELLS_OUTSIDE_RANGE_FILTER].cellVisibility(gIdx);
        }
        else if (hasActiveRangeFilters && !hasVisibleWellCells)
        {
            if (m_geometriesNeedsRegen[RANGE_FILTERED]) createGeometry(RANGE_FILTERED);

            rangeVisibility = m_geometries[RANGE_FILTERED].cellVisibility(gIdx);
            fenceVisibility = m_geometries[RANGE_FILTERED].cellVisibility(gIdx);
        }
        else if (!hasActiveRangeFilters && hasVisibleWellCells)
        {
            if (m_geometriesNeedsRegen[VISIBLE_WELL_FENCE_CELLS]) createGeometry(VISIBLE_WELL_FENCE_CELLS);

            rangeVisibility = m_geometries[VISIBLE_WELL_FENCE_CELLS].cellVisibility(gIdx);
            fenceVisibility = m_geometries[VISIBLE_WELL_FENCE_CELLS].cellVisibility(gIdx);
        }
        else if (!hasActiveRangeFilters && !hasVisibleWellCells)
        {
            if (m_geometriesNeedsRegen[ACTIVE]) createGeometry(ACTIVE);

            rangeVisibility = m_geometries[ACTIVE].cellVisibility(gIdx);
            fenceVisibility = m_geometries[ACTIVE].cellVisibility(gIdx);
        }

        cellVisibility->resize(rangeVisibility->size());

#pragma omp parallel for
        for (int cellIdx = 0; cellIdx < static_cast<int>(cellVisibility->size()); ++cellIdx)
        {
            (*cellVisibility)[cellIdx] = (*rangeVisibility)[cellIdx] || (*fenceVisibility)[cellIdx];
        }

        computePropertyVisibility(cellVisibility.p(), grids[gIdx], frameIndex, cellVisibility.p(), m_reservoirView->eclipsePropertyFilterCollection()); 

        m_propFilteredGeometryFrames[frameIndex]->setCellVisibility(gIdx, cellVisibility.p());
    } 

    m_propFilteredGeometryFramesNeedsRegen[frameIndex] = false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirViewPartMgr::createPropertyFilteredWellGeometry(size_t frameIndex)
{
    RigEclipseCaseData* res = m_reservoirView->eclipseCase()->eclipseCaseData();

    if ( frameIndex >= m_propFilteredWellGeometryFrames.size())
    { 
        m_propFilteredWellGeometryFrames.resize(frameIndex + 1);
        m_propFilteredWellGeometryFramesNeedsRegen.resize(frameIndex + 1, true);
    }

    if ( m_propFilteredWellGeometryFrames[frameIndex].isNull())  m_propFilteredWellGeometryFrames[frameIndex] = new RivReservoirPartMgr;

    m_propFilteredWellGeometryFrames[frameIndex]->clearAndSetReservoir(res, m_reservoirView);
    m_propFilteredWellGeometryFrames[frameIndex]->setTransform(m_scaleTransform.p());

    std::vector<RigGridBase*> grids;
    res->allGrids(&grids);

    bool hasActiveRangeFilters = m_reservoirView->rangeFilterCollection()->hasActiveFilters();
    bool hasVisibleWellCells = m_reservoirView->wellCollection()->hasVisibleWellCells();

    for (size_t gIdx = 0; gIdx < grids.size(); ++gIdx)
    {
        cvf::ref<cvf::UByteArray> cellVisibility = m_propFilteredWellGeometryFrames[frameIndex]->cellVisibility(gIdx); 
        cvf::ref<cvf::UByteArray> rangeVisibility; 
        cvf::ref<cvf::UByteArray> wellCellsOutsideRange; 
        cvf::ref<cvf::UByteArray> wellFenceCells;  

        if (hasActiveRangeFilters && hasVisibleWellCells)
        {
            if (m_geometriesNeedsRegen[RANGE_FILTERED_WELL_CELLS]) createGeometry(RANGE_FILTERED_WELL_CELLS);
            rangeVisibility = m_geometries[RANGE_FILTERED_WELL_CELLS].cellVisibility(gIdx);

            if (m_geometriesNeedsRegen[VISIBLE_WELL_CELLS_OUTSIDE_RANGE_FILTER]) createGeometry(VISIBLE_WELL_CELLS_OUTSIDE_RANGE_FILTER);
            wellCellsOutsideRange = m_geometries[VISIBLE_WELL_CELLS_OUTSIDE_RANGE_FILTER].cellVisibility(gIdx);

            if (m_geometriesNeedsRegen[VISIBLE_WELL_FENCE_CELLS_OUTSIDE_RANGE_FILTER]) createGeometry(VISIBLE_WELL_FENCE_CELLS_OUTSIDE_RANGE_FILTER);
            wellFenceCells = m_geometries[VISIBLE_WELL_FENCE_CELLS_OUTSIDE_RANGE_FILTER].cellVisibility(gIdx);

        }
        else if (hasActiveRangeFilters && !hasVisibleWellCells)
        {
            if (m_geometriesNeedsRegen[RANGE_FILTERED_WELL_CELLS]) createGeometry(RANGE_FILTERED_WELL_CELLS);
            rangeVisibility = m_geometries[RANGE_FILTERED_WELL_CELLS].cellVisibility(gIdx);
            wellCellsOutsideRange = rangeVisibility;
            wellFenceCells = rangeVisibility;
        }
        else if (!hasActiveRangeFilters && hasVisibleWellCells)
        {
            if (m_geometriesNeedsRegen[VISIBLE_WELL_CELLS]) createGeometry(VISIBLE_WELL_CELLS);
            wellCellsOutsideRange = m_geometries[VISIBLE_WELL_CELLS].cellVisibility(gIdx);

            if (m_geometriesNeedsRegen[VISIBLE_WELL_FENCE_CELLS]) createGeometry(VISIBLE_WELL_FENCE_CELLS);
            wellFenceCells = m_geometries[VISIBLE_WELL_FENCE_CELLS].cellVisibility(gIdx);

            rangeVisibility = wellCellsOutsideRange;
        }
        else if (!hasActiveRangeFilters && !hasVisibleWellCells)
        {
            if (m_geometriesNeedsRegen[ALL_WELL_CELLS]) createGeometry(ALL_WELL_CELLS);
            wellFenceCells = m_geometries[ALL_WELL_CELLS].cellVisibility(gIdx);
            wellCellsOutsideRange = wellFenceCells;
            rangeVisibility = wellFenceCells;
        }

        cellVisibility->resize(rangeVisibility->size());

#pragma omp parallel for
        for (int cellIdx = 0; cellIdx < static_cast<int>(cellVisibility->size()); ++cellIdx)
        {
            (*cellVisibility)[cellIdx] =  (*wellFenceCells)[cellIdx] || (*rangeVisibility)[cellIdx] || (*wellCellsOutsideRange)[cellIdx];
        }

        computePropertyVisibility(cellVisibility.p(), grids[gIdx], frameIndex, cellVisibility.p(), m_reservoirView->eclipsePropertyFilterCollection()); 

        m_propFilteredWellGeometryFrames[frameIndex]->setCellVisibility(gIdx, cellVisibility.p());
    }

    m_propFilteredWellGeometryFramesNeedsRegen[frameIndex] = false;
}

//--------------------------------------------------------------------------------------------------
/// Evaluate visibility based on cell state
//--------------------------------------------------------------------------------------------------
void RivReservoirViewPartMgr::computeNativeVisibility(cvf::UByteArray* cellVisibility, const RigGridBase* grid, const RigActiveCellInfo* activeCellInfo, const cvf::UByteArray* cellIsInWellStatuses,
    bool invalidCellsIsVisible, 
    bool inactiveCellsIsVisible, 
    bool activeCellsIsVisible,
    bool mainGridIsVisible) 
{
    CVF_ASSERT(cellVisibility != NULL);
    CVF_ASSERT(grid != NULL);
    CVF_ASSERT(activeCellInfo != NULL);
    CVF_ASSERT(cellIsInWellStatuses != NULL);
    CVF_ASSERT(cellIsInWellStatuses->size() >= grid->cellCount());

    cellVisibility->resize(grid->cellCount());

#pragma omp parallel for 
    for (int cellIndex = 0; cellIndex < static_cast<int>(grid->cellCount()); cellIndex++)
    {
        const RigCell& cell = grid->cell(cellIndex);
        size_t reservoirCellIndex = cell.mainGridCellIndex();

        if (   !invalidCellsIsVisible && cell.isInvalid() 
            || !inactiveCellsIsVisible && !activeCellInfo->isActive(reservoirCellIndex)
            || !activeCellsIsVisible && activeCellInfo->isActive(reservoirCellIndex)
            //|| mainGridIsVisible && (cell.subGrid() != NULL) // this is handled on global level instead
            || (*cellIsInWellStatuses)[cellIndex]
            )
        {
            (*cellVisibility)[cellIndex] = false;
        }
        else
        {
            (*cellVisibility)[cellIndex] = true;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirViewPartMgr::computeOverriddenCellVisibility(cvf::UByteArray* cellVisibility, const RigGridBase* grid)
{

    RimViewController* masterViewLink = m_reservoirView->viewController();
    
    CVF_ASSERT(masterViewLink);

    RimView* masterView = masterViewLink->ownerViewLinker()->masterView();

    // get cell visibility
    #if 1
    cvf::ref<cvf::UByteArray> totCellVisibility =  masterView->currentTotalCellVisibility();
    #else
    // Could get something more like 
    std::vector<std::vector<cvf::UByteArray*> > gridsWithCellSetVisibility = masterView->getAllGridsCurrentCellSetsCellVisibility();
    #endif    
    
    CVF_ASSERT(cellVisibility != NULL);
    CVF_ASSERT(grid != NULL);

    size_t gridCellCount = grid->cellCount();
    cellVisibility->resize(gridCellCount);
    cellVisibility->setAll(false);

    const RigCaseToCaseCellMapper* cellMapper = masterViewLink->cellMapper();

    for (size_t lcIdx = 0; lcIdx < gridCellCount; ++lcIdx)
    {
        #if 1
        int reservoirCellIdx = static_cast<int>(grid->reservoirCellIndex(lcIdx));
        int cellCount = 0;
        const int* cellIndicesInMasterCase = cellMapper->masterCaseCellIndices(reservoirCellIdx, &cellCount);
        
        for (int mcIdx = 0; mcIdx < cellCount; ++mcIdx)
        {
            (*cellVisibility)[lcIdx] |=  (*totCellVisibility)[cellIndicesInMasterCase[mcIdx]]; // If any is visible, show
        }

        #else
        
        const RigGridCells& masterCaseCells = cellMapper->masterCaseGridAndLocalCellIndex(grid->gridIndex, lcIdx);

        for (int mcIdx = 0; mcIdx < masterCaseCells.cellCount(); ++mcIdx)
        {
            int cellSetCount = gridsWithCellSetVisibility[ masterCaseCells.gridIndex[mcIdx] ].size();
            for (int csIdx = 0; csIdx < cellSetCount; ++csIdx)
            {
                (*cellVisibility)[lcIdx] |=  gridsWithCellSetVisibility[masterCaseCells.gridIndex[mcIdx]][masterCaseCells.cellIndex[mcIdx]];
            }
        }
        #endif
    }
}




//--------------------------------------------------------------------------------------------------
/// Copy the data from source into destination. This is not trivial to do using cvf::Array ...
/// using parallelized operator [] and not memcopy. Do not know what is faster. 
//--------------------------------------------------------------------------------------------------
void RivReservoirViewPartMgr::copyByteArray(cvf::UByteArray* destination, const cvf::UByteArray* source ) 
{
    CVF_ASSERT(destination != NULL);
    CVF_ASSERT(source != NULL);

    if (destination->size() != source->size())
    {
        destination->resize(source->size());
    }

#pragma omp parallel for 
    for (int cellIndex = 0; cellIndex < static_cast<int>(source->size()); cellIndex++)
    {
        (*destination)[cellIndex] = (*source)[cellIndex];
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirViewPartMgr::computeRangeVisibility(RivCellSetEnum geometryType, 
                                                     cvf::UByteArray* cellVisibility, 
                                                     const RigGridBase* grid, 
                                                     const cvf::UByteArray* nativeVisibility, 
                                                     const RimCellRangeFilterCollection* rangeFilterColl) 
{
    CVF_ASSERT(cellVisibility != NULL);
    CVF_ASSERT(nativeVisibility != NULL);
    CVF_ASSERT(rangeFilterColl != NULL);

    CVF_ASSERT(grid != NULL);
    CVF_ASSERT(nativeVisibility->size() == grid->cellCount());

    // Initialize range filter with native visibility
    if (cellVisibility != nativeVisibility) (*cellVisibility) = (*nativeVisibility);

    if (rangeFilterColl->hasActiveFilters() || m_reservoirView->wellCollection()->hasVisibleWellCells())
    {
        // Build range filter for current grid
        cvf::CellRangeFilter gridCellRangeFilter;
        rangeFilterColl->compoundCellRangeFilter(&gridCellRangeFilter, grid->gridIndex());

        const RigLocalGrid* lgr = NULL;
        cvf::ref<cvf::UByteArray> parentGridVisibilities;

        if (!grid->isMainGrid())
        {
            lgr = static_cast<const RigLocalGrid*>(grid);

            size_t parentGridIndex = lgr->parentGrid()->gridIndex();
            CVF_ASSERT(parentGridIndex < grid->gridIndex());

            if (geometryType == RANGE_FILTERED_WELL_CELLS)
            {
                geometryType = RANGE_FILTERED; // Use the range filtering in the parent grid, not the well cells in the parent grid
            }

            RivReservoirPartMgr* reservoirGridPartMgr = &m_geometries[geometryType];

            parentGridVisibilities = reservoirGridPartMgr->cellVisibility(parentGridIndex);
        }

        bool hasAdditiveRangeFilters = rangeFilterColl->hasActiveIncludeFilters() || m_reservoirView->wellCollection()->hasVisibleWellCells();

#pragma omp parallel for 
        for (int cellIndex = 0; cellIndex < static_cast<int>(grid->cellCount()); cellIndex++)
        {
            if ( (*nativeVisibility)[cellIndex] )
            {
                const RigCell& cell = grid->cell(cellIndex);
                bool visibleDueToParentGrid = false;
                if (lgr)
                {
                    size_t parentGridCellIndex = cell.parentCellIndex();
                    visibleDueToParentGrid = parentGridVisibilities->get(parentGridCellIndex);
                }

                // Normal grid visibility
                size_t mainGridI;
                size_t mainGridJ;
                size_t mainGridK;

                bool isInSubGridArea = cell.subGrid() != NULL;
                grid->ijkFromCellIndex(cellIndex, &mainGridI, &mainGridJ, &mainGridK);
                
                bool nativeRangeVisibility = false;
                   
                if (hasAdditiveRangeFilters)
                {
                    nativeRangeVisibility = gridCellRangeFilter.isCellVisible(mainGridI, mainGridJ, mainGridK, isInSubGridArea);
                }
                else
                {
                    // Special handling when no include filters are present. Use native visibility 
                    nativeRangeVisibility = (*nativeVisibility)[cellIndex];
                }
                
                (*cellVisibility)[cellIndex] = (visibleDueToParentGrid || nativeRangeVisibility) 
                                                && !gridCellRangeFilter.isCellExcluded(mainGridI, mainGridJ, mainGridK, isInSubGridArea);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirViewPartMgr::computePropertyVisibility(cvf::UByteArray* cellVisibility, const RigGridBase* grid, size_t timeStepIndex, 
    const cvf::UByteArray* rangeFilterVisibility, RimEclipsePropertyFilterCollection* propFilterColl)
{
    CVF_ASSERT(cellVisibility != NULL);
    CVF_ASSERT(rangeFilterVisibility != NULL);
    CVF_ASSERT(propFilterColl != NULL);

    CVF_ASSERT(grid->cellCount() > 0);
    CVF_ASSERT(rangeFilterVisibility->size() == grid->cellCount());

    // Copy if not equal
    if (cellVisibility != rangeFilterVisibility ) (*cellVisibility) = *rangeFilterVisibility;

    if (propFilterColl->hasActiveFilters())
    {
        for (size_t i = 0; i < propFilterColl->propertyFilters().size(); i++)
        {
            RimEclipsePropertyFilter* propertyFilter = propFilterColl->propertyFilters()[i];

            if (propertyFilter->isActive() && propertyFilter->resultDefinition->hasResult())
            {
                const RimCellFilter::FilterModeType filterType = propertyFilter->filterMode();

                RigEclipseCaseData* eclipseCase = propFilterColl->reservoirView()->eclipseCase()->eclipseCaseData();

                cvf::ref<RigResultAccessor> resultAccessor = RigResultAccessorFactory::createFromResultDefinition(eclipseCase, grid->gridIndex(),  timeStepIndex, propertyFilter->resultDefinition);

                CVF_ASSERT(resultAccessor.notNull());

                if (propertyFilter->isCategorySelectionActive())
                {
                    std::vector<int> integerVector = propertyFilter->selectedCategoryValues();
                    std::set<int> integerSet;
                    for (auto val : integerVector)
                    {
                        integerSet.insert(val);
                    }

                    for (int cellIndex = 0; cellIndex < static_cast<int>(grid->cellCount()); cellIndex++)
                    {
                        if ((*cellVisibility)[cellIndex])
                        {
                            size_t resultValueIndex = cellIndex;

                            double scalarValue = resultAccessor->cellScalar(resultValueIndex);
                            if (integerSet.find(scalarValue) != integerSet.end())
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
                    double lowerBound = 0.0;
                    double upperBound = 0.0;
                    propertyFilter->rangeValues(&lowerBound, &upperBound);

                    for (int cellIndex = 0; cellIndex < static_cast<int>(grid->cellCount()); cellIndex++)
                    {
                        if ((*cellVisibility)[cellIndex])
                        {
                            size_t resultValueIndex = cellIndex;

                            double scalarValue = resultAccessor->cellScalar(resultValueIndex);
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
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirViewPartMgr::updateCellColor(RivCellSetEnum geometryType, size_t timeStepIndex, cvf::Color4f color)
{
    RivReservoirPartMgr * pmgr = reservoirPartManager( geometryType,  timeStepIndex );
    pmgr->updateCellColor(color);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirViewPartMgr::updateCellResultColor(RivCellSetEnum geometryType, size_t timeStepIndex, RimEclipseCellColors* cellResultColors)
{
    RivReservoirPartMgr * pmgr = reservoirPartManager( geometryType,  timeStepIndex );
    pmgr->updateCellResultColor(timeStepIndex, cellResultColors);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirViewPartMgr::updateCellEdgeResultColor(RivCellSetEnum geometryType, size_t timeStepIndex, RimEclipseCellColors* cellResultColors, RimCellEdgeColors* cellEdgeResultColors)
{
    RivReservoirPartMgr * pmgr = reservoirPartManager( geometryType,  timeStepIndex );
    pmgr->updateCellEdgeResultColor(timeStepIndex, cellResultColors, cellEdgeResultColors);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirViewPartMgr::updateFaultCellEdgeResultColor(RivCellSetEnum geometryType, size_t timeStepIndex, RimEclipseCellColors* cellResultColors, RimCellEdgeColors* cellEdgeResultColors)
{
    RivReservoirPartMgr * pmgr = reservoirPartManager(geometryType, timeStepIndex);
    pmgr->updateFaultCellEdgeResultColor(timeStepIndex, cellResultColors, cellEdgeResultColors);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const cvf::UByteArray* RivReservoirViewPartMgr::cellVisibility(RivCellSetEnum geometryType, size_t gridIndex, size_t timeStepIndex)
{
    if (geometryType == PROPERTY_FILTERED)
    {
        if (timeStepIndex >= m_propFilteredGeometryFramesNeedsRegen.size() || m_propFilteredGeometryFramesNeedsRegen[timeStepIndex])
        {
            createPropertyFilteredNoneWellCellGeometry(timeStepIndex);
        }
    }
    else if (geometryType == PROPERTY_FILTERED_WELL_CELLS)
    {
        if (timeStepIndex >= m_propFilteredWellGeometryFramesNeedsRegen.size() || m_propFilteredWellGeometryFramesNeedsRegen[timeStepIndex])
        {
            createPropertyFilteredWellGeometry(timeStepIndex);
        }
    }
    else
    {
        if (m_geometriesNeedsRegen[geometryType])
        {
            createGeometry(geometryType);
        }
    }
    RivReservoirPartMgr * pmgr = (const_cast<RivReservoirViewPartMgr*>(this))->reservoirPartManager( geometryType,  timeStepIndex );
    return pmgr->cellVisibility(gridIndex).p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivReservoirPartMgr* RivReservoirViewPartMgr::reservoirPartManager(RivCellSetEnum geometryType, size_t timeStepIndex)
{
    if (geometryType == PROPERTY_FILTERED)
    {
        return m_propFilteredGeometryFrames[timeStepIndex].p();
    }
    else if (geometryType == PROPERTY_FILTERED_WELL_CELLS)
    {
        return m_propFilteredWellGeometryFrames[timeStepIndex].p();
    }
    else
    {
        return &m_geometries[geometryType];
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirViewPartMgr::updateFaultColors(RivCellSetEnum geometryType, size_t timeStepIndex, RimEclipseCellColors* cellResultColors)
{
    if (geometryType == PROPERTY_FILTERED && timeStepIndex >= m_propFilteredGeometryFrames.size())
    {
        return;
    }

    if (geometryType == PROPERTY_FILTERED_WELL_CELLS && timeStepIndex >= m_propFilteredWellGeometryFrames.size())
    {
        return;
    }

    RivReservoirPartMgr* pmgr = reservoirPartManager(geometryType, timeStepIndex);
    pmgr->updateFaultColors(timeStepIndex, cellResultColors);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirViewPartMgr::appendFaultsStaticGeometryPartsToModel(cvf::ModelBasicList* model, RivCellSetEnum geometryType)
{
    //CVF_ASSERT(geometryType < PROPERTY_FILTERED);
    if (geometryType >= PROPERTY_FILTERED) return;

    if (m_geometriesNeedsRegen[geometryType])
    {
        createGeometry(geometryType);
    }

/*
    QString text;
    switch (geometryType)
    {
    case OVERRIDDEN_CELL_VISIBILITY:
        text = "OVERRIDDEN_CELL_VISIBILITY";
        break;
    case ALL_CELLS:
        text = "ALL_CELLS";
        break;
    case ACTIVE:
        text = "ACTIVE";
        break;
    case ALL_WELL_CELLS:
        text = "ALL_WELL_CELLS";
        break;
    case VISIBLE_WELL_CELLS:
        text = "VISIBLE_WELL_CELLS";
        break;
    case VISIBLE_WELL_FENCE_CELLS:
        text = "VISIBLE_WELL_FENCE_CELLS";
        break;
    case INACTIVE:
        text = "INACTIVE";
        break;
    case RANGE_FILTERED:
        text = "RANGE_FILTERED";
        break;
    case RANGE_FILTERED_INACTIVE:
        text = "RANGE_FILTERED_INACTIVE";
        break;
    case RANGE_FILTERED_WELL_CELLS:
        text = "RANGE_FILTERED_WELL_CELLS";
        break;
    case VISIBLE_WELL_CELLS_OUTSIDE_RANGE_FILTER:
        text = "VISIBLE_WELL_CELLS_OUTSIDE_RANGE_FILTER";
        break;
    case VISIBLE_WELL_FENCE_CELLS_OUTSIDE_RANGE_FILTER:
        text = "VISIBLE_WELL_FENCE_CELLS_OUTSIDE_RANGE_FILTER";
        break;
    case PROPERTY_FILTERED:
        text = "PROPERTY_FILTERED";
        break;
    case PROPERTY_FILTERED_WELL_CELLS:
        text = "PROPERTY_FILTERED_WELL_CELLS";
        break;
    default:
        break;
    }

    qDebug() << text;
*/

    m_geometries[geometryType].appendFaultPartsToModel(model);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirViewPartMgr::appendFaultsDynamicGeometryPartsToModel(cvf::ModelBasicList* model, RivCellSetEnum geometryType, size_t frameIndex)
{
    if (geometryType == PROPERTY_FILTERED)
    {
        //qDebug() << "PROPERTY_FILTERED";

        m_propFilteredGeometryFrames[frameIndex]->appendFaultPartsToModel(model);
    }
    else if (geometryType == PROPERTY_FILTERED_WELL_CELLS)
    {
        //qDebug() << "PROPERTY_FILTERED_WELL_CELLS";

        m_propFilteredWellGeometryFrames[frameIndex]->appendFaultPartsToModel(model);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivCellSetEnum RivReservoirViewPartMgr::geometryTypeForFaultLabels(const std::set<RivCellSetEnum>& geometryTypes, bool showFaultsOutsideFilters) const
{
    for (RivCellSetEnum cellSetType : geometryTypes)
    {
        if (!showFaultsOutsideFilters)
        {
            if (cellSetType == PROPERTY_FILTERED)
            {
                return PROPERTY_FILTERED;
            }

            if (cellSetType == RANGE_FILTERED)
            {
                return RANGE_FILTERED;
            }
        }
    }

    return ACTIVE;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirViewPartMgr::appendFaultLabelsStaticGeometryPartsToModel(cvf::ModelBasicList* model, RivCellSetEnum geometryType)
{
    if (m_geometriesNeedsRegen[geometryType])
    {
        createGeometry(geometryType);
    }
    m_geometries[geometryType].appendFaultLabelPartsToModel(model);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirViewPartMgr::appendFaultLabelsDynamicGeometryPartsToModel(cvf::ModelBasicList* model, RivCellSetEnum geometryType, size_t frameIndex)
{
    m_propFilteredGeometryFrames[frameIndex]->appendFaultLabelPartsToModel(model);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirViewPartMgr::forceWatertightGeometryOnForType(RivCellSetEnum geometryType)
{
    if (geometryType == PROPERTY_FILTERED)
    {
        for (size_t i = 0; i < m_propFilteredGeometryFrames.size(); ++i)
        {
            if (m_propFilteredGeometryFrames[i].p()) m_propFilteredGeometryFrames[i]->forceWatertightGeometryOn();
        }
    }
    else if (geometryType == PROPERTY_FILTERED_WELL_CELLS)
    {
        for (size_t i = 0; i < m_propFilteredWellGeometryFrames.size(); ++i)
        {
            if (m_propFilteredWellGeometryFrames[i].p()) m_propFilteredWellGeometryFrames[i]->forceWatertightGeometryOn();
        }
    }
    else
    {
        if (m_geometriesNeedsRegen[geometryType])
        {
            createGeometry(geometryType);
        }
        m_geometries[geometryType].forceWatertightGeometryOn();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirViewPartMgr::clearWatertightGeometryFlags()
{
    for (size_t i = 0; i < m_propFilteredGeometryFrames.size(); ++i)
    {
        if (m_propFilteredGeometryFrames[i].p()) m_propFilteredGeometryFrames[i]->clearWatertightGeometryFlag();
    }

    for (size_t i = 0; i < m_propFilteredWellGeometryFrames.size(); ++i)
    {
        if (m_propFilteredWellGeometryFrames[i].p()) m_propFilteredWellGeometryFrames[i]->clearWatertightGeometryFlag();
    }

    for (int i = 0; i < PROPERTY_FILTERED; i++)
    {
        m_geometries[RivCellSetEnum(i)].clearWatertightGeometryFlag();
    }
}
