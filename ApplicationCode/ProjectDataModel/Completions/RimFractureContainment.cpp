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

#include "RigMainGrid.h"

#include "RimFractureTemplate.h"
#include "RimProject.h"

#include "cafPdmUiSliderEditor.h"

CAF_PDM_SOURCE_INIT(RimFractureContainment, "FractureContainment");

namespace caf
{
template<>
void caf::AppEnum<RimFractureContainment::FaultTruncType>::setUp()
{
    addItem(RimFractureContainment::DISABLED, "DISABLED", "Continue Across");
    addItem(RimFractureContainment::TRUNCATE_AT_FAULT, "TRUNCATE_AT_FAULT", "Truncate At Faults");
    addItem(RimFractureContainment::CONTINUE_IN_CONTAINMENT_ZONE, "CONTINUE_IN_CONTAINMENT_ZONE", "Continue in Containment Zone");

    setDefault(RimFractureContainment::DISABLED);
}
} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFractureContainment::RimFractureContainment()
{
    CAF_PDM_InitObject("Fracture Containment", "", "", "");

    CAF_PDM_InitField(
        &m_isUsingFractureContainment_OBSOLETE, "IsUsingFractureContainment", false, "Fracture Containment", "", "", "");
    m_isUsingFractureContainment_OBSOLETE.xmlCapability()->setIOWritable(false);
    m_isUsingFractureContainment_OBSOLETE.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&m_topKLayer, "TopKLayer", 0, "Top Layer", "", "", "");
    // m_topKLayer.uiCapability()->setUiEditorTypeName(caf::PdmUiSliderEditor::uiEditorTypeName());
    CAF_PDM_InitField(&m_baseKLayer, "BaseKLayer", 0, "Base Layer", "", "", "");
    // m_topKLayer.uiCapability()->setUiEditorTypeName(caf::PdmUiSliderEditor::uiEditorTypeName());

    CAF_PDM_InitFieldNoDefault(&m_faultTruncation, "FaultTruncationType", "Fault Truncation", "", "", "");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFractureContainment::~RimFractureContainment() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimFractureContainment::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                            bool*                      useOptionsOnly)
{
    // TODO: Remove this
    return caf::PdmObject::calculateValueOptions(fieldNeedingOptions, useOptionsOnly);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureContainment::initAfterRead()
{
    if (m_isUsingFractureContainment_OBSOLETE())
    {
        m_faultTruncation = CONTINUE_IN_CONTAINMENT_ZONE;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFractureContainment::isEnabled() const
{
    return m_faultTruncation() != DISABLED;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFractureContainment::isEclipseCellWithinContainment(const RigMainGrid*      mainGrid,
                                                            size_t                  anchorEclipseCell,
                                                            size_t                  globalCellIndex,
                                                            const std::set<size_t>& containmentCells) const
{
    if (!isEnabled()) return true;

    if (m_faultTruncation() == CONTINUE_IN_CONTAINMENT_ZONE || m_faultTruncation() == TRUNCATE_AT_FAULT)
    {
        CVF_ASSERT(mainGrid);

        size_t i, j, k;
        if (globalCellIndex >= mainGrid->globalCellArray().size()) return false;

        mainGrid->ijkFromCellIndex(globalCellIndex, &i, &j, &k);

        if (k + 1 < static_cast<size_t>(m_topKLayer()))
        {
            return false;
        }

        if (k + 1 > static_cast<size_t>(m_baseKLayer()))
        {
            return false;
        }
    }

    if (m_faultTruncation() == TRUNCATE_AT_FAULT)
    {
        if (containmentCells.count(globalCellIndex) > 0)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureContainment::setTopKLayer(int topKLayer)
{
    m_topKLayer = topKLayer;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureContainment::setBaseKLayer(int baseKLayer)
{
    m_baseKLayer = baseKLayer;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureContainment::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_faultTruncation);

    if (m_faultTruncation() == CONTINUE_IN_CONTAINMENT_ZONE || m_faultTruncation() == TRUNCATE_AT_FAULT)
    {
        uiOrdering.add(&m_topKLayer);
        uiOrdering.add(&m_baseKLayer);
    }

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureContainment::fieldChangedByUi(const caf::PdmFieldHandle* changedField,
                                              const QVariant&            oldValue,
                                              const QVariant&            newValue)
{
    if (changedField == &m_faultTruncation || changedField == &m_topKLayer || changedField == &m_baseKLayer)
    {
        RimProject* proj;
        this->firstAncestorOrThisOfType(proj);
        if (proj)
        {
            proj->reloadCompletionTypeResultsInAllViews();
        }
    }

    if (changedField == &m_faultTruncation)
    {
        RimFractureTemplate* fractureTemplate = nullptr;
        this->firstAncestorOrThisOfType(fractureTemplate);
        if (fractureTemplate)
        {
            fractureTemplate->updateConnectedEditors();
        }
    }
}
