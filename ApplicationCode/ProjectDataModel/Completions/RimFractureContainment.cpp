/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 -     Statoil ASA
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
#include "RimFractureContainment.h"
#include "cafPdmUiSliderEditor.h"


CAF_PDM_SOURCE_INIT(RimFractureContainment, "FractureContainment");

namespace caf
{
template<>
void caf::AppEnum< RimFractureContainment::FaultTruncType>::setUp()
{
    addItem(RimFractureContainment::DISABLED, "DISABLED", "Disable");
    addItem(RimFractureContainment::TRUNCATE_AT_FAULT, "TRUNCATE_AT_FAULT", "Truncate At Faults");
    addItem(RimFractureContainment::CONTINUE_IN_CONTAINMENT_ZONE, "CONTINUE_IN_CONTAINMENT_ZONE", "Continue in Containment Zone");

    setDefault(RimFractureContainment::DISABLED);
}
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFractureContainment::RimFractureContainment()
{
    CAF_PDM_InitObject("Fracture Containment", "", "", "");

    CAF_PDM_InitField(&m_isUsingFractureContainment,  "IsUsingFractureContainment",  false, "Fracture Containment", "", "", "");
    CAF_PDM_InitField(&m_topKLayer, "topKLayer", 0, "Top Layer", "", "", "");
    //m_topKLayer.uiCapability()->setUiEditorTypeName(caf::PdmUiSliderEditor::uiEditorTypeName());
    CAF_PDM_InitField(&m_bottomKLayer, "bottomKLayer", 0, "Bottom Layer", "", "", "");
    //m_topKLayer.uiCapability()->setUiEditorTypeName(caf::PdmUiSliderEditor::uiEditorTypeName());

    CAF_PDM_InitFieldNoDefault(&m_faultTruncation, "m_faultTruncation", "Fault Truncation", "", "", ""); 
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimFractureContainment::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, 
                                                                            bool* useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;
    if (fieldNeedingOptions == &m_faultTruncation)
    {
        options.push_back(caf::PdmOptionItemInfo(caf::AppEnum< FaultTruncType >::uiText(DISABLED), DISABLED));
        options.push_back(caf::PdmOptionItemInfo(caf::AppEnum< FaultTruncType >::uiText(TRUNCATE_AT_FAULT), TRUNCATE_AT_FAULT));
        if (m_isUsingFractureContainment())
        {
            options.push_back(caf::PdmOptionItemInfo(caf::AppEnum< FaultTruncType >::uiText(CONTINUE_IN_CONTAINMENT_ZONE), CONTINUE_IN_CONTAINMENT_ZONE));
        }
    }
    return options;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFractureContainment::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_isUsingFractureContainment);
    uiOrdering.add(&m_topKLayer);
    uiOrdering.add(&m_bottomKLayer);
    uiOrdering.add(&m_faultTruncation);
}

