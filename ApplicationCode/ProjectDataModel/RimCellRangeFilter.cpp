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

#include "RIStdInclude.h"

#include "RimCellRangeFilter.h"
#include "RimCellRangeFilterCollection.h"
#include "RimReservoirView.h"
#include "RigReservoir.h"


CAF_PDM_SOURCE_INIT(RimCellRangeFilter, "CellRangeFilter");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCellRangeFilter::RimCellRangeFilter()
    : m_parentContainer(NULL)
{
    CAF_PDM_InitObject("Cell Range Filter", ":/CellFilter_Range.png", "", "");

    CAF_PDM_InitField(&startIndexI, "StartIndexI",  1,  "Start index I", "", "","");
    CAF_PDM_InitField(&cellCountI,  "CellCountI",   1,  "Cell Count I", "", "","");
    CAF_PDM_InitField(&startIndexJ, "StartIndexJ",  1,  "Start index J", "", "","");
    CAF_PDM_InitField(&cellCountJ,  "CellCountJ",   1,  "Cell Count J", "", "","");
    CAF_PDM_InitField(&startIndexK, "StartIndexK",  1,  "Start index K", "", "","");
    CAF_PDM_InitField(&cellCountK,  "CellCountK",   1,  "Cell Count K", "", "","");
    updateIconState();

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCellRangeFilter::~RimCellRangeFilter()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCellRangeFilter::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField != &name)
    {
        computeAndSetValidValues();
    
        CVF_ASSERT(m_parentContainer);
        m_parentContainer->fieldChangedByUi(changedField, oldValue, newValue);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCellRangeFilter::setParentContainer(RimCellRangeFilterCollection* parentContainer)
{
    m_parentContainer = parentContainer;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCellRangeFilter::computeAndSetValidValues()
{
    CVF_ASSERT(m_parentContainer);

    RigMainGrid* mainGrid = m_parentContainer->mainGrid();
    if (mainGrid && mainGrid->cellCount() > 0 )
    {
        cellCountI = cvf::Math::clamp(cellCountI.v(),   1, static_cast<int>(mainGrid->cellCountI()));
        startIndexI = cvf::Math::clamp(startIndexI.v(), 1, static_cast<int>(mainGrid->cellCountI()));

        cellCountJ = cvf::Math::clamp(cellCountJ.v(),   1, static_cast<int>(mainGrid->cellCountJ()));
        startIndexJ = cvf::Math::clamp(startIndexJ.v(), 1, static_cast<int>(mainGrid->cellCountJ()));

        cellCountK = cvf::Math::clamp(cellCountK.v(),   1, static_cast<int>(mainGrid->cellCountK()));
        startIndexK = cvf::Math::clamp(startIndexK.v(), 1, static_cast<int>(mainGrid->cellCountK()));
    }
    this->updateIconState();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCellRangeFilter::setDefaultValues()
{
    CVF_ASSERT(m_parentContainer);

    RigMainGrid* mainGrid = m_parentContainer->mainGrid();
    if (mainGrid)
    {
        cvf::Vec3st min, max;
        mainGrid->activeCellsBoundingBox(min, max);

        // Adjust to Eclipse indexing
        min.x() = min.x() + 1;
        min.y() = min.y() + 1;
        min.z() = min.z() + 1;

        max.x() = max.x() + 1;
        max.y() = max.y() + 1;
        max.z() = max.z() + 1;

        startIndexI = min.x();
        startIndexI.setUiName(QString("I Start (%1)").arg(min.x()));

        startIndexJ = min.y();
        startIndexJ.setUiName(QString("J Start (%1)").arg(min.y()));

        startIndexK = min.z();
        startIndexK.setUiName(QString("K Start (%1)").arg(min.z()));

        cellCountI = max.x() - min.x() + 1;
        cellCountI.setUiName(QString("  Width (%1)").arg(max.x() - min.x() + 1));

        cellCountJ = max.y() - min.y() + 1;
        cellCountJ.setUiName(QString("  Width (%1)").arg(max.y() - min.y() + 1));

        cellCountK = max.z() - min.z() + 1;
        cellCountK.setUiName(QString("  Width (%1)").arg(max.z() - min.z() + 1));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCellRangeFilterCollection* RimCellRangeFilter::parentContainer()
{
    return m_parentContainer;
}

