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

#include "cafPdmUiSliderEditor.h"


CAF_PDM_SOURCE_INIT(RimCellRangeFilter, "CellRangeFilter");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCellRangeFilter::RimCellRangeFilter()
    : m_parentContainer(NULL)
{
    CAF_PDM_InitObject("Cell Range Filter", ":/CellFilter_Range.png", "", "");

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

        startIndexI = static_cast<int>(min.x());
        startIndexJ = static_cast<int>(min.y());
        startIndexK = static_cast<int>(min.z());
        cellCountI = static_cast<int>(max.x() - min.x() + 1);
        cellCountJ = static_cast<int>(max.y() - min.y() + 1);
        cellCountK = static_cast<int>(max.z() - min.z() + 1);
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

        startIndexI.setUiName(QString("I Start (%1)").arg(min.x()));
        startIndexJ.setUiName(QString("J Start (%1)").arg(min.y()));
        startIndexK.setUiName(QString("K Start (%1)").arg(min.z()));
        cellCountI.setUiName(QString("  Width (%1)").arg(max.x() - min.x() + 1));
        cellCountJ.setUiName(QString("  Width (%1)").arg(max.y() - min.y() + 1));
        cellCountK.setUiName(QString("  Width (%1)").arg(max.z() - min.z() + 1));

        if (field == &startIndexI || field == &cellCountI)
        {
            myAttr->m_minimum = 1;
            myAttr->m_maximum = static_cast<int>(mainGrid->cellCountI());
        }
        else if (field == &startIndexJ  || field == &cellCountJ)
        {
            myAttr->m_minimum = 1;
            myAttr->m_maximum = static_cast<int>(mainGrid->cellCountJ());
        }
        else if (field == &startIndexK || field == &cellCountK)
        {
            myAttr->m_minimum = 1;
            myAttr->m_maximum = static_cast<int>(mainGrid->cellCountK());
        }
    }
}

