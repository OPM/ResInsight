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

#include "RimFaultResultSlot.h"

#include "RimReservoirView.h"
#include "RimResultSlot.h"
#include "RiuMainWindow.h"
#include "RimUiTreeModelPdm.h"


namespace caf
{
    template<>
    void AppEnum< RimFaultResultSlot::FaultVisualizationMode >::setUp()
    {
        addItem(RimFaultResultSlot::FAULT_COLOR,            "FAULT_COLOR",              "Fault Colors");
        addItem(RimFaultResultSlot::CELL_RESULT_MAPPING,    "CELL_RESULT_MAPPING",      "Grid Cell Results");
        addItem(RimFaultResultSlot::CUSTOM_RESULT_MAPPING,  "CUSTOM_RESULT_MAPPING",    "Custom Cell Results");
        setDefault(RimFaultResultSlot::CELL_RESULT_MAPPING);
    }
}

CAF_PDM_SOURCE_INIT(RimFaultResultSlot, "RimFaultResultSlot");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFaultResultSlot::RimFaultResultSlot()
{
    CAF_PDM_InitObject("Fault Result Slot", "", "", "");

    CAF_PDM_InitField(&visualizationMode, "VisualizationMode", caf::AppEnum<RimFaultResultSlot::FaultVisualizationMode>(RimFaultResultSlot::CELL_RESULT_MAPPING), "Fault Color Mapping", "", "", "");

     CAF_PDM_InitFieldNoDefault(&customResultSlot, "CustomResultSlot", "Custom Cell Result", ":/CellResult.png", "", "");
     customResultSlot = new RimResultSlot();

     updateVisibility();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFaultResultSlot::~RimFaultResultSlot()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFaultResultSlot::setReservoirView(RimReservoirView* ownerReservoirView)
{
    customResultSlot->setReservoirView(ownerReservoirView);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFaultResultSlot::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &visualizationMode)
    {
        updateVisibility();

        RiuMainWindow::instance()->uiPdmModel()->updateUiSubTree(this);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFaultResultSlot::initAfterRead()
{
    updateVisibility();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFaultResultSlot::updateVisibility()
{
    if (this->visualizationMode() == FAULT_COLOR || this->visualizationMode() == CELL_RESULT_MAPPING)
    {
        this->customResultSlot.setUiHidden(true);
        this->customResultSlot.setUiChildrenHidden(true);
    }
    else
    {
        this->customResultSlot.setUiHidden(false);
        this->customResultSlot.setUiChildrenHidden(false);
    }
}

