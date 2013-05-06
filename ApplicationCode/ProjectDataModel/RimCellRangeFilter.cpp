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

#include "RimCellRangeFilter.h"
#include "RimCellRangeFilterCollection.h"
#include "RimReservoirView.h"
#include "RigCaseData.h"
#include "RimCase.h"

#include "cafPdmFieldCvfMat4d.h"
#include "cafPdmFieldCvfColor.h"
#include "RimResultSlot.h"
#include "RimCellEdgeResultSlot.h"
#include "RimCellRangeFilterCollection.h"
#include "RimCellPropertyFilterCollection.h"
#include "RimWellCollection.h"
#include "Rim3dOverlayInfoConfig.h"
#include "RimReservoirCellResultsCacher.h"

#include "cafPdmUiSliderEditor.h"


CAF_PDM_SOURCE_INIT(RimCellRangeFilter, "CellRangeFilter");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCellRangeFilter::RimCellRangeFilter()
    : m_parentContainer(NULL)
{
    CAF_PDM_InitObject("Cell Range Filter", ":/CellFilter_Range.png", "", "");

    CAF_PDM_InitField(&gridIndex, "GridIndex",  0,  "Grid", "", "","");
    CAF_PDM_InitField(&propagateToSubGrids, "PropagateToSubGrids",  true,  "Apply to Subgrids", "", "","");

    CAF_PDM_InitField(&startIndexI, "StartIndexI",  1,  "Start index I", "", "","");
    startIndexI.setUiEditorTypeName(caf::PdmUiSliderEditor::uiEditorTypeName());
    
    CAF_PDM_InitField(&cellCountI,  "CellCountI",   1,  "Cell Count I", "", "","");
    cellCountI.setUiEditorTypeName(caf::PdmUiSliderEditor::uiEditorTypeName());

    CAF_PDM_InitField(&startIndexJ, "StartIndexJ",  1,  "Start index J", "", "","");
    startIndexJ.setUiEditorTypeName(caf::PdmUiSliderEditor::uiEditorTypeName());

    CAF_PDM_InitField(&cellCountJ,  "CellCountJ",   1,  "Cell Count J", "", "","");
    cellCountJ.setUiEditorTypeName(caf::PdmUiSliderEditor::uiEditorTypeName());

    CAF_PDM_InitField(&startIndexK, "StartIndexK",  1,  "Start index K", "", "","");
    startIndexK.setUiEditorTypeName(caf::PdmUiSliderEditor::uiEditorTypeName());

    CAF_PDM_InitField(&cellCountK,  "CellCountK",   1,  "Cell Count K", "", "","");
    cellCountK.setUiEditorTypeName(caf::PdmUiSliderEditor::uiEditorTypeName());
    
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

    RigGridBase* grid = selectedGrid();
    if (grid && grid->cellCount() > 0 )
    {
        cellCountI = cvf::Math::clamp(cellCountI.v(),   1, static_cast<int>(grid->cellCountI()));
        startIndexI = cvf::Math::clamp(startIndexI.v(), 1, static_cast<int>(grid->cellCountI()));

        cellCountJ = cvf::Math::clamp(cellCountJ.v(),   1, static_cast<int>(grid->cellCountJ()));
        startIndexJ = cvf::Math::clamp(startIndexJ.v(), 1, static_cast<int>(grid->cellCountJ()));

        cellCountK = cvf::Math::clamp(cellCountK.v(),   1, static_cast<int>(grid->cellCountK()));
        startIndexK = cvf::Math::clamp(startIndexK.v(), 1, static_cast<int>(grid->cellCountK()));
    }
    this->updateIconState();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCellRangeFilter::setDefaultValues()
{
    CVF_ASSERT(m_parentContainer);

    RigGridBase* grid = selectedGrid();

    RigActiveCellInfo* actCellInfo = m_parentContainer->activeCellInfo();
    if (grid == m_parentContainer->mainGrid() && actCellInfo)
    {
        cvf::Vec3st min, max;
        actCellInfo->IJKBoundingBox(min, max);

        // Adjust to Eclipse indexing
        min.x() = min.x() + 1;
        min.y() = min.y() + 1;
        min.z() = min.z() + 1;

        max.x() = max.x() + 1;
        max.y() = max.y() + 1;
        max.z() = max.z() + 1;

        startIndexI = static_cast<int>(min.x());
        startIndexJ = static_cast<int>(min.y());
        startIndexK = static_cast<int>(min.z());
        cellCountI = static_cast<int>(max.x() - min.x() + 1);
        cellCountJ = static_cast<int>(max.y() - min.y() + 1);
        cellCountK = static_cast<int>(max.z() - min.z() + 1);
    }
    else
    {
        startIndexI = 1;
        startIndexJ = 1;
        startIndexK = 1;
        cellCountI = static_cast<int>(grid->cellCountI() );
        cellCountJ = static_cast<int>(grid->cellCountJ() );
        cellCountK = static_cast<int>(grid->cellCountK() );
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCellRangeFilterCollection* RimCellRangeFilter::parentContainer()
{
    return m_parentContainer;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCellRangeFilter::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute * attribute)
{
    caf::PdmUiSliderEditorAttribute* myAttr = static_cast<caf::PdmUiSliderEditorAttribute*>(attribute);
    if (!myAttr || !m_parentContainer)
    {
        return;
    }

    RigGridBase* grid = selectedGrid();

    if (field == &startIndexI || field == &cellCountI)
    {
        myAttr->m_minimum = 1;
        myAttr->m_maximum = static_cast<int>(grid->cellCountI());
    }
    else if (field == &startIndexJ  || field == &cellCountJ)
    {
        myAttr->m_minimum = 1;
        myAttr->m_maximum = static_cast<int>(grid->cellCountJ());
    }
    else if (field == &startIndexK || field == &cellCountK)
    {
        myAttr->m_minimum = 1;
        myAttr->m_maximum = static_cast<int>(grid->cellCountK());
    }

    RigActiveCellInfo* actCellInfo = m_parentContainer->activeCellInfo();
    if (grid == m_parentContainer->mainGrid() && actCellInfo)
    {
        cvf::Vec3st min, max;
        actCellInfo->IJKBoundingBox(min, max);

        // Adjust to Eclipse indexing
        min.x() = min.x() + 1;
        min.y() = min.y() + 1;
        min.z() = min.z() + 1;

        max.x() = max.x() + 1;
        max.y() = max.y() + 1;
        max.z() = max.z() + 1;

        startIndexI.setUiName(QString("I Start (%1)").arg(min.x()));
        startIndexJ.setUiName(QString("J Start (%1)").arg(min.y()));
        startIndexK.setUiName(QString("K Start (%1)").arg(min.z()));
        cellCountI.setUiName(QString("  Width (%1)").arg(max.x() - min.x() + 1));
        cellCountJ.setUiName(QString("  Width (%1)").arg(max.y() - min.y() + 1));
        cellCountK.setUiName(QString("  Width (%1)").arg(max.z() - min.z() + 1));
    }
    else
    {
        startIndexI.setUiName(QString("I Start"));
        startIndexJ.setUiName(QString("J Start"));
        startIndexK.setUiName(QString("K Start"));
        cellCountI.setUiName(QString("  Width"));
        cellCountJ.setUiName(QString("  Width"));
        cellCountK.setUiName(QString("  Width"));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimCellRangeFilter::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    if (useOptionsOnly) (*useOptionsOnly) = true;

    if (&gridIndex == fieldNeedingOptions)
    { 
        RigMainGrid * mainGrid = NULL;

        if (parentContainer() && parentContainer()->reservoirView() && parentContainer()->reservoirView()->eclipseCase() &&  parentContainer()->reservoirView()->eclipseCase()->reservoirData())
            mainGrid = parentContainer()->reservoirView()->eclipseCase()->reservoirData()->mainGrid();

        for (size_t gIdx = 0; gIdx < mainGrid->gridCount(); ++gIdx)
        {
            RigGridBase* grid = mainGrid->gridByIndex(gIdx);
            QString gridName;

            gridName += grid->gridName().c_str();
            if (gIdx == 0)
            {
                if (gridName.isEmpty())
                    gridName += "Main Grid";
                else
                    gridName += " (Main Grid)";
            }

            caf::PdmOptionItemInfo item(gridName, (int)gIdx);
            options.push_back(item);
        }
    }
    return options;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigGridBase* RimCellRangeFilter::selectedGrid()
{
    RigMainGrid* mainGrid = m_parentContainer->mainGrid();
    CVF_ASSERT(mainGrid);

    RigGridBase* grid = NULL;
    if (gridIndex() >= mainGrid->gridCount())
    {
        gridIndex = 0;
    }

    grid = mainGrid->gridByIndex(gridIndex());
    CVF_ASSERT(grid);
    return grid;
}

