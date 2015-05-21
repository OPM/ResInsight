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

#include "RimFaultResultSlot.h"

#include "RigCaseData.h"
#include "RigMainGrid.h"

#include "RimEclipseCase.h"
#include "RimReservoirView.h"
#include "RimResultSlot.h"
#include "RimUiTreeModelPdm.h"
#include "RiuMainWindow.h"



CAF_PDM_SOURCE_INIT(RimFaultResultSlot, "RimFaultResultSlot");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFaultResultSlot::RimFaultResultSlot()
{
    CAF_PDM_InitObject("Fault Result Slot", ":/draw_style_faults_24x24.png", "", "");

    CAF_PDM_InitField(&showCustomFaultResult,                "ShowCustomFaultResult",                 false,   "Show Custom Fault Result", "", "", "");
    showCustomFaultResult.setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_customFaultResult, "CustomResultSlot", "Custom Fault Result", ":/CellResult.png", "", "");
    m_customFaultResult = new RimResultSlot();
    m_customFaultResult.setOwnerObject(this);
    m_customFaultResult.setUiHidden(true);

    // Take ownership of the fields in RimResultDefinition to be able to trap fieldChangedByUi in this class
    m_customFaultResult->m_resultTypeUiField.setOwnerObject(this);
    m_customFaultResult->m_porosityModelUiField.setOwnerObject(this);
    m_customFaultResult->m_resultVariableUiField.setOwnerObject(this);


    updateFieldVisibility();
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
void RimFaultResultSlot::setReservoirView(RimEclipseView* ownerReservoirView)
{
    m_reservoirView = ownerReservoirView;
    m_customFaultResult->setReservoirView(ownerReservoirView);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFaultResultSlot::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    this->updateUiIconFromToggleField();

    m_customFaultResult->fieldChangedByUi(changedField, oldValue, newValue);

    if (changedField == &m_customFaultResult->m_resultVariableUiField)
    {
        RiuMainWindow::instance()->uiPdmModel()->updateUiSubTree(this);
    }

    if (m_reservoirView) m_reservoirView->scheduleCreateDisplayModelAndRedraw();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFaultResultSlot::initAfterRead()
{
    m_customFaultResult->initAfterRead();
    updateFieldVisibility();

    this->updateUiIconFromToggleField();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFaultResultSlot::updateFieldVisibility()
{
    m_customFaultResult->updateFieldVisibility();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimResultSlot* RimFaultResultSlot::customFaultResult()
{
    return this->m_customFaultResult();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimFaultResultSlot::objectToggleField()
{
    return &showCustomFaultResult;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFaultResultSlot::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    caf::PdmUiGroup* group1 = uiOrdering.addNewGroup("Result");
    group1->add(&(m_customFaultResult->m_resultTypeUiField));
    group1->add(&(m_customFaultResult->m_porosityModelUiField));
    group1->add(&(m_customFaultResult->m_resultVariableUiField));
}   

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimFaultResultSlot::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly)
{
    return m_customFaultResult->calculateValueOptionsForSpecifiedDerivedListPosition(true, fieldNeedingOptions, useOptionsOnly);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimFaultResultSlot::hasValidCustomResult()
{
    if (this->showCustomFaultResult())
    {
        if (m_customFaultResult->hasResult() || m_customFaultResult->isTernarySaturationSelected())
        {
            return true;
        }
    }

    return false;
}
