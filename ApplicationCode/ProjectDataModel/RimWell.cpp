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


#include "cafAppEnum.h"
#include "cafPdmField.h"
#include "RimWell.h"
#include "RivReservoirViewPartMgr.h"
#include "RimReservoirView.h"


CAF_PDM_SOURCE_INIT(RimWell, "Well");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWell::RimWell()
{
    CAF_PDM_InitObject("Well", ":/Well.png", "", "");

    CAF_PDM_InitFieldNoDefault(&name,       "WellName",             "Name", "", "", "");

    CAF_PDM_InitField(&showWellLabel,         "ShowWellLabel",      true, "Show well label", "", "", "");

    CAF_PDM_InitField(&showWellPipes,       "ShowWellPipe",         true,   "Show well pipe", "", "", "");
    CAF_PDM_InitField(&pipeRadiusScaleFactor, "WellPipeRadiusScale",1.0,    "    Pipe radius scale", "", "", "");
    CAF_PDM_InitField(&wellPipeColor,       "WellPipeColor",        cvf::Color3f(0.588f, 0.588f, 0.804f), "    Well pipe color", "", "", "");

    CAF_PDM_InitField(&showWellCells,       "ShowWellCells",        true,   "Add cells to range filter", "", "", "");
    CAF_PDM_InitField(&showWellCellFence,   "ShowWellCellFence",    false,  "    Use well fence", "", "", "");
    //CAF_PDM_InitField(&wellCellColor,       "WellCellColor",      cvf::Color3f(cvf::Color3f::BROWN), "Well cell color", "", "", "");



    name.setUiHidden(true);
    name.setUiReadOnly(true);

    m_reservoirView = NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWell::~RimWell()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimWell::userDescriptionField()
{
    return &name;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWell::setReservoirView(RimReservoirView* ownerReservoirView)
{
    m_reservoirView = ownerReservoirView;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWell::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (&showWellLabel == changedField)
    {
        if (m_reservoirView) 
        {
            m_reservoirView->createDisplayModelAndRedraw();
        }
    }
    else if (&showWellCells == changedField)
    {
        if (m_reservoirView)
        {
            m_reservoirView->scheduleGeometryRegen(RivReservoirViewPartMgr::VISIBLE_WELL_CELLS);
            m_reservoirView->createDisplayModelAndRedraw();
        }

    }
    else if (&showWellCellFence == changedField)
    {
        if (m_reservoirView)
        {
            m_reservoirView->scheduleGeometryRegen(RivReservoirViewPartMgr::VISIBLE_WELL_CELLS);
            m_reservoirView->createDisplayModelAndRedraw();
        }

    }
    else if (&showWellPipes == changedField)
    {
        if (m_reservoirView) m_reservoirView->createDisplayModelAndRedraw();
    }
    else if (&wellPipeColor == changedField)
    {
        if (m_reservoirView) m_reservoirView->createDisplayModelAndRedraw();
    }
    else if (&pipeRadiusScaleFactor == changedField)
    {
        if (m_reservoirView)
        {
            m_reservoirView->schedulePipeGeometryRegen();
            m_reservoirView->createDisplayModelAndRedraw();
        }
    }
}

