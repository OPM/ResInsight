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

#include "RimFaultResultSettings.h"

#include "RimReservoirView.h"
#include "RimResultSlot.h"
#include "RiuMainWindow.h"
#include "RimUiTreeModelPdm.h"


namespace caf
{
    template<>
    void AppEnum< RimFaultResultSettings::FaultVisualizationMode >::setUp()
    {
        addItem(RimFaultResultSettings::FAULT_COLOR,            "FAULT_COLOR",              "Fault Colors");
        addItem(RimFaultResultSettings::CELL_RESULT_MAPPING,    "CELL_RESULT_MAPPING",      "Grid Cell Results");
        addItem(RimFaultResultSettings::CUSTOM_RESULT_MAPPING,  "CUSTOM_RESULT_MAPPING",    "Custom Cell Results");
        setDefault(RimFaultResultSettings::CELL_RESULT_MAPPING);
    }
}

CAF_PDM_SOURCE_INIT(RimFaultResultSettings, "RimFaultResultSlot");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFaultResultSettings::RimFaultResultSettings()
{
    CAF_PDM_InitObject("Fault Result Slot", "", "", "");

    CAF_PDM_InitField(&visualizationMode, "VisualizationMode", caf::AppEnum<RimFaultResultSettings::FaultVisualizationMode>(RimFaultResultSettings::CELL_RESULT_MAPPING), "Fault Color Mapping", "", "", "");

     CAF_PDM_InitFieldNoDefault(&m_customFaultResult, "CustomResultSlot", "Custom Fault Result", ":/CellResult.png", "", "");
     m_customFaultResult = new RimResultSlot();

     updateVisibility();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFaultResultSettings::~RimFaultResultSettings()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFaultResultSettings::setReservoirView(RimReservoirView* ownerReservoirView)
{
    m_reservoirView = ownerReservoirView;
    m_customFaultResult->setReservoirView(ownerReservoirView);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFaultResultSettings::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &visualizationMode)
    {
        updateVisibility();

        RiuMainWindow::instance()->uiPdmModel()->updateUiSubTree(this);
    }

    if (m_reservoirView) m_reservoirView->createDisplayModelAndRedraw();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFaultResultSettings::initAfterRead()
{
    updateVisibility();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFaultResultSettings::updateVisibility()
{
    if (this->visualizationMode() == FAULT_COLOR || this->visualizationMode() == CELL_RESULT_MAPPING)
    {
        this->m_customFaultResult.setUiHidden(true);
        this->m_customFaultResult.setUiChildrenHidden(true);
    }
    else
    {
        this->m_customFaultResult.setUiHidden(false);
        this->m_customFaultResult.setUiChildrenHidden(false);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimResultSlot* RimFaultResultSettings::customFaultResult()
{
    if (this->visualizationMode() == CUSTOM_RESULT_MAPPING)
    {
        return this->m_customFaultResult();
    }

    return NULL;
}

