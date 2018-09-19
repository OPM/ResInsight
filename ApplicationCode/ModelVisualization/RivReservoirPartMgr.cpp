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

#include "RivReservoirPartMgr.h"

#include "RigEclipseCaseData.h"

#include "RimEclipseCase.h"

#include "RivGridPartMgr.h"
#include "RivReservoirFaultsPartMgr.h"

#include "cvfStructGrid.h"
#include "cvfModelBasicList.h"
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirPartMgr::clearAndSetReservoir(RimEclipseCase* eclipseCase, RimEclipseView* reservoirView)
{
    m_allGrids.clear();


    if (eclipseCase && eclipseCase->eclipseCaseData())
    {
        RigEclipseCaseData* eclipseCaseData = eclipseCase->eclipseCaseData();

        std::vector<const RigGridBase*> grids;
        eclipseCaseData->allGrids(&grids);
        for (size_t i = 0; i < grids.size() ; ++i)
        {
            m_allGrids.push_back(new RivGridPartMgr(eclipseCase, grids[i], i));
        }

        if (eclipseCase->mainGrid())
        {
            // Faults read from file are present only on main grid
            m_faultsPartMgr = new RivReservoirFaultsPartMgr(eclipseCase->mainGrid(), reservoirView);
        }
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

    if (m_faultsPartMgr.notNull())
    {
        m_faultsPartMgr->setTransform(scaleTransform);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirPartMgr::setCellVisibility(size_t gridIndex, cvf::UByteArray* cellVisibilities)
{
    CVF_ASSERT(gridIndex < m_allGrids.size());
    m_allGrids[gridIndex]->setCellVisibility(cellVisibilities);

    if (gridIndex == 0)
    {
        CVF_ASSERT(m_faultsPartMgr.notNull());
        m_faultsPartMgr->setCellVisibility(cellVisibilities);
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

    if (m_faultsPartMgr.notNull())
    {
        m_faultsPartMgr->setOpacityLevel(color.a());
        m_faultsPartMgr->applySingleColorEffect();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirPartMgr::updateCellResultColor(size_t timeStepIndex, RimEclipseCellColors* cellResultColors)
{
    for (size_t i = 0; i < m_allGrids.size() ; ++i)
    {
        m_allGrids[i]->updateCellResultColor(timeStepIndex, cellResultColors);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirPartMgr::updateCellEdgeResultColor(size_t timeStepIndex, RimEclipseCellColors* cellResultColors, RimCellEdgeColors* cellEdgeResultColors)
{
    for (size_t i = 0; i < m_allGrids.size() ; ++i)
    {
        m_allGrids[i]->updateCellEdgeResultColor(timeStepIndex, cellResultColors, cellEdgeResultColors);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirPartMgr::appendGridPartsToModel(cvf::ModelBasicList* model)
{
    for (size_t i = 0; i < m_allGrids.size() ; ++i)
    {
        m_allGrids[i]->appendPartsToModel(model);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirPartMgr::appendGridPartsToModel(cvf::ModelBasicList* model, const std::vector<size_t>& gridIndices)
{
    for (size_t i = 0; i < gridIndices.size() ; ++i)
    {
        if (gridIndices[i] < m_allGrids.size())
        {
            m_allGrids[gridIndices[i]]->appendPartsToModel(model);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirPartMgr::updateFaultColors(size_t timeStepIndex, RimEclipseCellColors* cellResultColors)
{
    if (m_faultsPartMgr.notNull())
    {
        m_faultsPartMgr->updateColors(timeStepIndex, cellResultColors);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirPartMgr::appendFaultPartsToModel(cvf::ModelBasicList* model)
{
    if (m_faultsPartMgr.notNull())
    {
        m_faultsPartMgr->appendPartsToModel(model);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirPartMgr::appendFaultLabelPartsToModel(cvf::ModelBasicList* model)
{
    if (m_faultsPartMgr.notNull())
    {
        m_faultsPartMgr->appendLabelPartsToModel(model);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirPartMgr::forceWatertightGeometryOn()
{
    if (m_faultsPartMgr.notNull())
    {
        m_faultsPartMgr->forceWatertightGeometryOn();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirPartMgr::clearWatertightGeometryFlag()
{
    if (m_faultsPartMgr.notNull())
    {
        m_faultsPartMgr->clearWatertightGeometryFlag();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirPartMgr::updateFaultCellEdgeResultColor(size_t timeStepIndex, RimEclipseCellColors* cellResultColors, RimCellEdgeColors* cellEdgeResultColors)
{
    if (m_faultsPartMgr.notNull())
    {
        m_faultsPartMgr->updateCellEdgeResultColor(timeStepIndex, cellResultColors, cellEdgeResultColors);
    }

}

