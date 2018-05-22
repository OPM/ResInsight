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

#include "RimFaultInView.h"

#include "RigFault.h"

#include "RimEclipseView.h"
#include "RimIntersectionCollection.h"

CAF_PDM_SOURCE_INIT(RimFaultInView, "Fault");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFaultInView::RimFaultInView()
{
    CAF_PDM_InitObject("RimFault", ":/draw_style_faults_24x24.png", "", "");

    CAF_PDM_InitFieldNoDefault(&name,       "FaultName",             "Name", "", "", "");
    name.uiCapability()->setUiHidden(true);
    name.uiCapability()->setUiReadOnly(true);

    CAF_PDM_InitField(&showFault,         "ShowFault",      true, "Show Fault", "", "", "");
    showFault.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&faultColor,       "Color",        cvf::Color3f(0.588f, 0.588f, 0.804f), "Fault Color", "", "", "");

    m_rigFault = nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFaultInView::~RimFaultInView()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimFaultInView::userDescriptionField()
{
    return &name;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFaultInView::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    this->updateUiIconFromToggleField();

    if (&faultColor == changedField || &showFault == changedField)
    {
        RimEclipseView* reservoirView = nullptr;

        this->firstAncestorOrThisOfType(reservoirView);

        if (reservoirView) 
        {
            reservoirView->scheduleCreateDisplayModelAndRedraw();
            reservoirView->crossSectionCollection()->scheduleCreateDisplayModelAndRedraw2dIntersectionViews();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFaultInView::initAfterRead()
{
    this->updateUiIconFromToggleField();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimFaultInView::objectToggleField()
{
    return &showFault;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFaultInView::setFaultGeometry(const RigFault* faultGeometry)
{
    m_rigFault = faultGeometry;

    this->name = faultGeometry->name();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RigFault* RimFaultInView::faultGeometry() const
{
    return m_rigFault;
}

