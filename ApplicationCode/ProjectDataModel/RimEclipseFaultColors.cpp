/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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

#include "RimEclipseFaultColors.h"

#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"
#include "RimLegendConfig.h"
#include "RimTernaryLegendConfig.h"

#include "RiuMainWindow.h"

#include "cafPdmUiTreeOrdering.h"



CAF_PDM_SOURCE_INIT(RimEclipseFaultColors, "RimFaultResultSlot");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseFaultColors::RimEclipseFaultColors()
{
    CAF_PDM_InitObject("Separate Fault Result", ":/draw_style_faults_24x24.png", "", "");

    CAF_PDM_InitField(&showCustomFaultResult,                "ShowCustomFaultResult",                 false,   "Show Custom Fault Result", "", "", "");
    showCustomFaultResult.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_customFaultResultColors, "CustomResultSlot", "Custom Fault Result", ":/CellResult.png", "", "");
    m_customFaultResultColors = new RimEclipseCellColors();

    m_customFaultResultColors.uiCapability()->setUiHidden(true);

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseFaultColors::~RimEclipseFaultColors()
{
    delete m_customFaultResultColors;
    m_customFaultResultColors = NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseFaultColors::setReservoirView(RimEclipseView* ownerReservoirView)
{
    m_reservoirView = ownerReservoirView;
    m_customFaultResultColors->setReservoirView(ownerReservoirView);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseFaultColors::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    this->updateUiIconFromToggleField();

    if (m_reservoirView) m_reservoirView->scheduleCreateDisplayModelAndRedraw();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseFaultColors::initAfterRead()
{
    m_customFaultResultColors->initAfterRead();

    this->updateUiIconFromToggleField();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseCellColors* RimEclipseFaultColors::customFaultResult()
{
    return this->m_customFaultResultColors();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseFaultColors::updateUiFieldsFromActiveResult()
{
    m_customFaultResultColors->updateUiFieldsFromActiveResult();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimEclipseFaultColors::objectToggleField()
{
    return &showCustomFaultResult;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseFaultColors::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    caf::PdmUiGroup* group1 = uiOrdering.addNewGroup("Result");
    m_customFaultResultColors->uiOrdering(uiConfigName, *group1);
}   

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimEclipseFaultColors::hasValidCustomResult()
{
    if (this->showCustomFaultResult())
    {
        if (m_customFaultResultColors->hasResult() || m_customFaultResultColors->isTernarySaturationSelected())
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseFaultColors::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/)
{
    m_customFaultResultColors()->defineUiTreeOrdering(uiTreeOrdering, uiConfigName);
}
