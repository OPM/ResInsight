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


#include "RimCellRangeFilter.h"

#include "RigActiveCellInfo.h"
#include "RigGridBase.h"
#include "RigMainGrid.h"

#include "RimEclipseCase.h"
#include "RimCellRangeFilterCollection.h"
#include "RimEclipseView.h"
#include "RimViewController.h"

#include "cafPdmUiSliderEditor.h"
#include "cvfAssert.h"

CAF_PDM_SOURCE_INIT(RimCellRangeFilter, "CellRangeFilter");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCellRangeFilter::RimCellRangeFilter()
{
    CAF_PDM_InitObject("Cell Range Filter", ":/CellFilter_Range.png", "", "");

    CAF_PDM_InitField(&gridIndex, "GridIndex",  0,  "Grid", "", "","");
    CAF_PDM_InitField(&propagateToSubGrids, "PropagateToSubGrids",  true,  "Apply to Subgrids", "", "","");

    CAF_PDM_InitField(&startIndexI, "StartIndexI",  1,  "Start index I", "", "","");
    startIndexI.uiCapability()->setUiEditorTypeName(caf::PdmUiSliderEditor::uiEditorTypeName());
    
    CAF_PDM_InitField(&cellCountI,  "CellCountI",   1,  "Cell Count I", "", "","");
    cellCountI.uiCapability()->setUiEditorTypeName(caf::PdmUiSliderEditor::uiEditorTypeName());

    CAF_PDM_InitField(&startIndexJ, "StartIndexJ",  1,  "Start index J", "", "","");
    startIndexJ.uiCapability()->setUiEditorTypeName(caf::PdmUiSliderEditor::uiEditorTypeName());

    CAF_PDM_InitField(&cellCountJ,  "CellCountJ",   1,  "Cell Count J", "", "","");
    cellCountJ.uiCapability()->setUiEditorTypeName(caf::PdmUiSliderEditor::uiEditorTypeName());

    CAF_PDM_InitField(&startIndexK, "StartIndexK",  1,  "Start index K", "", "","");
    startIndexK.uiCapability()->setUiEditorTypeName(caf::PdmUiSliderEditor::uiEditorTypeName());

    CAF_PDM_InitField(&cellCountK,  "CellCountK",   1,  "Cell Count K", "", "","");
    cellCountK.uiCapability()->setUiEditorTypeName(caf::PdmUiSliderEditor::uiEditorTypeName());
    
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
    
        parentContainer()->updateDisplayModeNotifyManagedViews(this);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCellRangeFilter::computeAndSetValidValues()
{
    CVF_ASSERT(parentContainer());

    const cvf::StructGridInterface* grid = selectedGrid();
    if (grid && grid->cellCountI() > 0 && grid->cellCountJ() > 0 && grid->cellCountK() > 0)
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
void RimCellRangeFilter::updateActiveState()
{
    isActive.uiCapability()->setUiReadOnly(isRangeFilterControlled());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCellRangeFilter::setDefaultValues()
{
    CVF_ASSERT(parentContainer());

    const cvf::StructGridInterface* grid = selectedGrid();

    RigActiveCellInfo* actCellInfo = parentContainer()->activeCellInfo();
    if (grid == parentContainer()->gridByIndex(0) && actCellInfo)
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
    return dynamic_cast<RimCellRangeFilterCollection*>(this->parentField()->ownerObject());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCellRangeFilter::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute * attribute)
{
    caf::PdmUiSliderEditorAttribute* myAttr = dynamic_cast<caf::PdmUiSliderEditorAttribute*>(attribute);
    if (!myAttr || !parentContainer())
    {
        return;
    }

    const cvf::StructGridInterface* grid = selectedGrid();

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

    RigActiveCellInfo* actCellInfo = parentContainer()->activeCellInfo();
    if (grid == parentContainer()->gridByIndex(0) && actCellInfo)
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

        startIndexI.uiCapability()->setUiName(QString("I Start (%1)").arg(min.x()));
        startIndexJ.uiCapability()->setUiName(QString("J Start (%1)").arg(min.y()));
        startIndexK.uiCapability()->setUiName(QString("K Start (%1)").arg(min.z()));
        cellCountI.uiCapability()->setUiName(QString("  Width (%1)").arg(max.x() - min.x() + 1));
        cellCountJ.uiCapability()->setUiName(QString("  Width (%1)").arg(max.y() - min.y() + 1));
        cellCountK.uiCapability()->setUiName(QString("  Width (%1)").arg(max.z() - min.z() + 1));
    }
    else
    {
        startIndexI.uiCapability()->setUiName(QString("I Start"));
        startIndexJ.uiCapability()->setUiName(QString("J Start"));
        startIndexK.uiCapability()->setUiName(QString("K Start"));
        cellCountI.uiCapability()->setUiName(QString("  Width"));
        cellCountJ.uiCapability()->setUiName(QString("  Width"));
        cellCountK.uiCapability()->setUiName(QString("  Width"));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCellRangeFilter::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    bool readOnlyState = isRangeFilterControlled();

    std::vector<caf::PdmFieldHandle*> objFields;
    this->fields(objFields);
    for (size_t i = 0; i < objFields.size(); i ++)
    {
        objFields[i]->uiCapability()->setUiReadOnly(readOnlyState);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCellRangeFilter::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName)
{
    RimCellFilter::defineUiTreeOrdering(uiTreeOrdering, uiConfigName);

    updateActiveState();
    updateIconState();
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
        for (int gIdx = 0; gIdx < parentContainer()->gridCount(); ++gIdx)
        {
            QString gridName;

            gridName += parentContainer()->gridName(gIdx);
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
bool RimCellRangeFilter::isRangeFilterControlled()
{
    RimView* rimView = NULL;
    firstAncestorOrThisOfType(rimView);
    CVF_ASSERT(rimView);

    bool isRangeFilterControlled = false;
    if (rimView && rimView->viewController() && rimView->viewController()->isRangeFiltersControlled())
    {
        isRangeFilterControlled = true;
    }

    return isRangeFilterControlled;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const cvf::StructGridInterface* RimCellRangeFilter::selectedGrid()
{
    if (gridIndex() >= parentContainer()->gridCount())
    {
        gridIndex = 0;
    }

    const cvf::StructGridInterface* grid = parentContainer()->gridByIndex(gridIndex());

    CVF_ASSERT(grid);

    return grid;
}

