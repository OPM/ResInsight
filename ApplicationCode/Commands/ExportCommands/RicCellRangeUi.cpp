/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RicCellRangeUi.h"

#include "RifReaderInterface.h"

#include "RigActiveCellInfo.h"
#include "RigEclipseCaseData.h"
#include "RigReservoirGridTools.h"

#include "RimCase.h"
#include "RimEclipseCase.h"

#include "cafPdmUiSliderEditor.h"

#include "cvfStructGrid.h"

CAF_PDM_SOURCE_INIT(RicCellRangeUi, "RicCellRangeUi");


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicCellRangeUi::RicCellRangeUi()
{
    CAF_PDM_InitObject("Cell Range", "", "", "");

    CAF_PDM_InitField(&m_gridIndex, "GridIndex", 0, "Grid", "", "", "");

    CAF_PDM_InitField(&m_startIndexI, "StartIndexI", 1, "Start index I", "", "", "");
    m_startIndexI.uiCapability()->setUiEditorTypeName(caf::PdmUiSliderEditor::uiEditorTypeName());

    CAF_PDM_InitField(&m_cellCountI, "CellCountI", 1, "Cell Count I", "", "", "");
    m_cellCountI.uiCapability()->setUiEditorTypeName(caf::PdmUiSliderEditor::uiEditorTypeName());

    CAF_PDM_InitField(&m_startIndexJ, "StartIndexJ", 1, "Start index J", "", "", "");
    m_startIndexJ.uiCapability()->setUiEditorTypeName(caf::PdmUiSliderEditor::uiEditorTypeName());

    CAF_PDM_InitField(&m_cellCountJ, "CellCountJ", 1, "Cell Count J", "", "", "");
    m_cellCountJ.uiCapability()->setUiEditorTypeName(caf::PdmUiSliderEditor::uiEditorTypeName());

    CAF_PDM_InitField(&m_startIndexK, "StartIndexK", 1, "Start index K", "", "", "");
    m_startIndexK.uiCapability()->setUiEditorTypeName(caf::PdmUiSliderEditor::uiEditorTypeName());

    CAF_PDM_InitField(&m_cellCountK, "CellCountK", 1, "Cell Count K", "", "", "");
    m_cellCountK.uiCapability()->setUiEditorTypeName(caf::PdmUiSliderEditor::uiEditorTypeName());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicCellRangeUi::setCase(RimCase* rimCase)
{
    m_case = rimCase;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicCellRangeUi::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute)
{
    caf::PdmUiSliderEditorAttribute* myAttr = dynamic_cast<caf::PdmUiSliderEditorAttribute*>(attribute);
    if (!myAttr)
    {
        return;
    }

    const cvf::StructGridInterface* grid = RigReservoirGridTools::gridByIndex(m_case, m_gridIndex());

    if (field == &m_startIndexI || field == &m_cellCountI)
    {
        myAttr->m_minimum = 1;
        myAttr->m_maximum = static_cast<int>(grid->cellCountI());
    }
    else if (field == &m_startIndexJ || field == &m_cellCountJ)
    {
        myAttr->m_minimum = 1;
        myAttr->m_maximum = static_cast<int>(grid->cellCountJ());
    }
    else if (field == &m_startIndexK || field == &m_cellCountK)
    {
        myAttr->m_minimum = 1;
        myAttr->m_maximum = static_cast<int>(grid->cellCountK());
    }

    const cvf::StructGridInterface* mainGrid = RigReservoirGridTools::mainGrid(m_case);
    RigActiveCellInfo* actCellInfo = activeCellInfo();

    if (grid == mainGrid && actCellInfo)
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

        m_startIndexI.uiCapability()->setUiName(QString("I Start (%1)").arg(min.x()));
        m_startIndexJ.uiCapability()->setUiName(QString("J Start (%1)").arg(min.y()));
        m_startIndexK.uiCapability()->setUiName(QString("K Start (%1)").arg(min.z()));
        m_cellCountI.uiCapability()->setUiName( QString("  Width (%1)").arg(max.x() - min.x() + 1));
        m_cellCountJ.uiCapability()->setUiName( QString("  Width (%1)").arg(max.y() - min.y() + 1));
        m_cellCountK.uiCapability()->setUiName( QString("  Width (%1)").arg(max.z() - min.z() + 1));
    }
    else
    {
        m_startIndexI.uiCapability()->setUiName(QString("I Start"));
        m_startIndexJ.uiCapability()->setUiName(QString("J Start"));
        m_startIndexK.uiCapability()->setUiName(QString("K Start"));
        m_cellCountI.uiCapability()->setUiName( QString("  Width"));
        m_cellCountJ.uiCapability()->setUiName( QString("  Width"));
        m_cellCountK.uiCapability()->setUiName( QString("  Width"));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RicCellRangeUi::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    if (useOptionsOnly) (*useOptionsOnly) = true;

    if (&m_gridIndex == fieldNeedingOptions)
    {
        for (int gIdx = 0; gIdx < RigReservoirGridTools::gridCount(m_case); ++gIdx)
        {
            QString gridName;

            gridName += RigReservoirGridTools::gridName(m_case, gIdx);
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
void RicCellRangeUi::clampValues()
{
    if (!m_case) return;

    const cvf::StructGridInterface* grid = RigReservoirGridTools::gridByIndex(m_case, m_gridIndex());
    if (grid)
    {
        m_cellCountI = cvf::Math::clamp(m_cellCountI.v(), 1, static_cast<int>(grid->cellCountI()));
        m_startIndexI = cvf::Math::clamp(m_startIndexI.v(), 1, static_cast<int>(grid->cellCountI()));

        m_cellCountJ = cvf::Math::clamp(m_cellCountJ.v(), 1, static_cast<int>(grid->cellCountJ()));
        m_startIndexJ = cvf::Math::clamp(m_startIndexJ.v(), 1, static_cast<int>(grid->cellCountJ()));

        m_cellCountK = cvf::Math::clamp(m_cellCountK.v(), 1, static_cast<int>(grid->cellCountK()));
        m_startIndexK = cvf::Math::clamp(m_startIndexK.v(), 1, static_cast<int>(grid->cellCountK()));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicCellRangeUi::setDefaultValues()
{
    const cvf::StructGridInterface* grid = RigReservoirGridTools::gridByIndex(m_case, m_gridIndex());

    RigActiveCellInfo* actCellInfo = this->activeCellInfo();

    const cvf::StructGridInterface* mainGrid = RigReservoirGridTools::mainGrid(m_case);

    if (grid == mainGrid && actCellInfo)
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

        m_startIndexI = static_cast<int>(min.x());
        m_startIndexJ = static_cast<int>(min.y());
        m_startIndexK = static_cast<int>(min.z());
        m_cellCountI = static_cast<int>(max.x() - min.x() + 1);
        m_cellCountJ = static_cast<int>(max.y() - min.y() + 1);
        m_cellCountK = static_cast<int>(max.z() - min.z() + 1);
    }
    else
    {
        m_startIndexI = 1;
        m_startIndexJ = 1;
        m_startIndexK = 1;
        m_cellCountI = static_cast<int>(grid->cellCountI());
        m_cellCountJ = static_cast<int>(grid->cellCountJ());
        m_cellCountK = static_cast<int>(grid->cellCountK());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigActiveCellInfo* RicCellRangeUi::activeCellInfo() const
{
    RimEclipseCase* rimEclipeCase = dynamic_cast<RimEclipseCase*>(m_case.p());
    if (rimEclipeCase)
    {
        return rimEclipeCase->eclipseCaseData()->activeCellInfo(RifReaderInterface::MATRIX_RESULTS);
    }

    return nullptr;
}
