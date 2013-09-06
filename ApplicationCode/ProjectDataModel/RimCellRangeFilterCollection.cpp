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

//#include "RiaStdInclude.h"

#include "RimCellRangeFilterCollection.h"
#include "RimCase.h"
#include "RimReservoirView.h"
#include "RigCaseData.h"
#include "cafPdmFieldCvfMat4d.h"
#include "cafPdmFieldCvfColor.h"
#include "RimResultSlot.h"
#include "RimCellEdgeResultSlot.h"
#include "Rim3dOverlayInfoConfig.h"
#include "RimCellPropertyFilterCollection.h"
#include "RimReservoirCellResultsCacher.h"
#include "RimWellCollection.h"


CAF_PDM_SOURCE_INIT(RimCellRangeFilterCollection, "CellRangeFilterCollection");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCellRangeFilterCollection::RimCellRangeFilterCollection()
{
    CAF_PDM_InitObject("Cell Range Filters", ":/CellFilter_Range.png", "", "");

    CAF_PDM_InitFieldNoDefault(&rangeFilters,   "RangeFilters", "Range Filters", "", "", "");
    CAF_PDM_InitField(&isActive,                  "Active", true, "Active", "", "", "");
    isActive.setUiHidden(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCellRangeFilterCollection::~RimCellRangeFilterCollection()
{
    std::list< caf::PdmPointer< RimCellRangeFilter > >::const_iterator it;
    for (it = rangeFilters.v().begin(); it != rangeFilters.v().end(); ++it)
    {
        delete it->p();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCellRangeFilterCollection::setReservoirView(RimReservoirView* reservoirView)
{
    m_reservoirView = reservoirView;
}


//--------------------------------------------------------------------------------------------------
/// RimCellRangeFilter is using Eclipse 1-based indexing, adjust filter values before 
//  populating cvf::CellRangeFilter (which is 0-based)
//--------------------------------------------------------------------------------------------------
void RimCellRangeFilterCollection::compoundCellRangeFilter(cvf::CellRangeFilter* cellRangeFilter, const RigGridBase* grid) const
{
    CVF_ASSERT(cellRangeFilter);

    std::list< caf::PdmPointer<RimCellRangeFilter> >::const_iterator it;
    for (it = rangeFilters.v().begin(); it != rangeFilters.v().end(); it++)
    {
        RimCellRangeFilter* rangeFilter = *it;

        if (rangeFilter && rangeFilter->isActive() && static_cast<size_t>(rangeFilter->gridIndex()) == grid->gridIndex())
        {
            if (rangeFilter->filterMode == RimCellFilter::INCLUDE)
            {
                cellRangeFilter->addCellIncludeRange(
                    rangeFilter->startIndexI - 1,
                    rangeFilter->startIndexJ - 1,
                    rangeFilter->startIndexK - 1,
                    rangeFilter->startIndexI - 1 + rangeFilter->cellCountI,
                    rangeFilter->startIndexJ - 1 + rangeFilter->cellCountJ,
                    rangeFilter->startIndexK - 1 + rangeFilter->cellCountK,
                    rangeFilter->propagateToSubGrids());
            }
            else
            {
                cellRangeFilter->addCellExcludeRange(
                    rangeFilter->startIndexI - 1,
                    rangeFilter->startIndexJ - 1,
                    rangeFilter->startIndexK - 1,
                    rangeFilter->startIndexI - 1 + rangeFilter->cellCountI,
                    rangeFilter->startIndexJ - 1 + rangeFilter->cellCountJ,
                    rangeFilter->startIndexK - 1 + rangeFilter->cellCountK, 
                    rangeFilter->propagateToSubGrids());
            }
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigMainGrid* RimCellRangeFilterCollection::mainGrid() const
{
    if (m_reservoirView &&
        m_reservoirView->eclipseCase() &&
        m_reservoirView->eclipseCase()->reservoirData() &&
        m_reservoirView->eclipseCase()->reservoirData()->mainGrid())
    {

        return m_reservoirView->eclipseCase()->reservoirData()->mainGrid();
    }

    return NULL;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigActiveCellInfo* RimCellRangeFilterCollection::activeCellInfo() const
{
    if (m_reservoirView )
    {
        return m_reservoirView->currentActiveCellInfo();
    }

    return NULL;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCellRangeFilterCollection::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    updateUiIconFromState(isActive);

    CVF_ASSERT(m_reservoirView);

    m_reservoirView->scheduleGeometryRegen(RivReservoirViewPartMgr::RANGE_FILTERED);
    m_reservoirView->scheduleGeometryRegen(RivReservoirViewPartMgr::RANGE_FILTERED_INACTIVE);

    m_reservoirView->createDisplayModelAndRedraw();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCellRangeFilter* RimCellRangeFilterCollection::createAndAppendRangeFilter()
{
    RimCellRangeFilter* rangeFilter = new RimCellRangeFilter();
    rangeFilter->setParentContainer(this);
    rangeFilter->setDefaultValues();

    rangeFilters.v().push_back(rangeFilter);

    rangeFilter->name = QString("New Filter (%1)").arg(rangeFilters().size());

    return rangeFilter;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimReservoirView* RimCellRangeFilterCollection::reservoirView()
{
    return m_reservoirView;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCellRangeFilterCollection::initAfterRead()
{
    std::list< caf::PdmPointer<RimCellRangeFilter> >::iterator it;
    for (it = rangeFilters.v().begin(); it != rangeFilters.v().end(); it++)
    {
        RimCellRangeFilter* rangeFilter = *it;
        rangeFilter->setParentContainer(this);
        rangeFilter->updateIconState();

    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCellRangeFilterCollection::remove(RimCellRangeFilter* rangeFilter)
{
    rangeFilters.v().remove(rangeFilter);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimCellRangeFilterCollection::hasActiveFilters() const
{
    if (!isActive()) return false; 

    std::list< caf::PdmPointer< RimCellRangeFilter > >::const_iterator it;
    for (it = rangeFilters.v().begin(); it != rangeFilters.v().end(); ++it)
    {
        if ((*it)->isActive()) return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimCellRangeFilterCollection::objectToggleField()
{
    return &isActive;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimCellRangeFilterCollection::hasActiveIncludeFilters() const
{
    if (!isActive) return false; 

    std::list< caf::PdmPointer< RimCellRangeFilter > >::const_iterator it;
    for (it = rangeFilters.v().begin(); it != rangeFilters.v().end(); ++it)
    {
        if ((*it)->isActive() && (*it)->filterMode() == RimCellFilter::INCLUDE) return true;
    }

    return false;

}

