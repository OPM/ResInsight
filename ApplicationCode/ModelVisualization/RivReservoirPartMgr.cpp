/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "RiaStdInclude.h"
#include "RivReservoirPartMgr.h"

#include "cvfStructGrid.h"
#include "cvfModelBasicList.h"

#include "RigCaseData.h"
#include "RivGridPartMgr.h"
#include "RivFaultPartMgr.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirPartMgr::clearAndSetReservoir(const RigCaseData* eclipseCase, const RimFaultCollection* faultCollection)
{
    m_allGrids.clear();
    m_faults.clear();

    if (eclipseCase)
    {
        std::vector<const RigGridBase*> grids;
        eclipseCase->allGrids(&grids);
        for (size_t i = 0; i < grids.size() ; ++i)
        {
            m_allGrids.push_back(new RivGridPartMgr(grids[i], i, faultCollection));
        }

        // Faults read from file are present only on main grid
        m_faults.push_back(new RivFaultPartMgr(eclipseCase->mainGrid(), 0, faultCollection));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirPartMgr::setTransform(cvf::Transform* scaleTransform)
{
    for (size_t i = 0; i < m_allGrids.size() ; ++i)
    {
        m_allGrids[i]->setTransform(scaleTransform);
    }

    for (size_t i = 0; i < m_faults.size() ; ++i)
    {
        m_faults[i]->setTransform(scaleTransform);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirPartMgr::setCellVisibility(size_t gridIndex, cvf::UByteArray* cellVisibilities)
{
    CVF_ASSERT(gridIndex < m_allGrids.size());
    m_allGrids[gridIndex]->setCellVisibility(cellVisibilities);

    if (gridIndex < m_faults.size())
    {
        m_faults[gridIndex]->setCellVisibility(cellVisibilities);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::UByteArray> RivReservoirPartMgr::cellVisibility(size_t gridIdx)
{
    CVF_ASSERT(gridIdx < m_allGrids.size()); 
    return  m_allGrids[gridIdx]->cellVisibility();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirPartMgr::updateCellColor(cvf::Color4f color)
{
    for (size_t i = 0; i < m_allGrids.size() ; ++i)
    {
        m_allGrids[i]->updateCellColor(color);
    }

    for (size_t i = 0; i < m_faults.size() ; ++i)
    {
        m_faults[i]->updateCellColor(color);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirPartMgr::updateCellResultColor(size_t timeStepIndex, RimResultSlot* cellResultSlot)
{
    for (size_t i = 0; i < m_allGrids.size() ; ++i)
    {
        m_allGrids[i]->updateCellResultColor(timeStepIndex, cellResultSlot);
    }

    for (size_t i = 0; i < m_faults.size() ; ++i)
    {
        m_faults[i]->updateCellResultColor(timeStepIndex, cellResultSlot);
    }

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirPartMgr::updateCellEdgeResultColor(size_t timeStepIndex, RimResultSlot* cellResultSlot, RimCellEdgeResultSlot* cellEdgeResultSlot)
{
    for (size_t i = 0; i < m_allGrids.size() ; ++i)
    {
        m_allGrids[i]->updateCellEdgeResultColor(timeStepIndex, cellResultSlot, cellEdgeResultSlot);
    }

    for (size_t i = 0; i < m_faults.size() ; ++i)
    {
        m_faults[i]->updateCellEdgeResultColor(timeStepIndex, cellResultSlot, cellEdgeResultSlot);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirPartMgr::appendPartsToModel(cvf::ModelBasicList* model)
{
    for (size_t i = 0; i < m_allGrids.size() ; ++i)
    {
        m_allGrids[i]->appendPartsToModel(model);
    }

    for (size_t i = 0; i < m_faults.size() ; ++i)
    {
        m_faults[i]->appendPartsToModel(model);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirPartMgr::appendPartsToModel(cvf::ModelBasicList* model, const std::vector<size_t>& gridIndices)
{
    for (size_t i = 0; i < gridIndices.size() ; ++i)
    {
        if (gridIndices[i] < m_allGrids.size())
        {
            m_allGrids[gridIndices[i]]->appendPartsToModel(model);
        }

        if (gridIndices[i] < m_faults.size())
        {
            m_faults[gridIndices[i]]->appendPartsToModel(model);
        }
    }
}
