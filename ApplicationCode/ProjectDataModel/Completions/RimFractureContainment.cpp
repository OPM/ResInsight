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

CAF_PDM_SOURCE_INIT(RimFractureContainment, "FractureContainment");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFractureContainment::RimFractureContainment()
{
    CAF_PDM_InitObject("Fracture Containment", "", "", "");

    CAF_PDM_InitField(&m_useContainment, "IsUsingFractureContainment", false, "Use Containment", "", "", "");
    CAF_PDM_InitField(&m_topKLayer, "TopKLayer", 0, "  Top Layer", "", "Do not allow fracture to grow into this layer", "");
    CAF_PDM_InitField(&m_baseKLayer, "BaseKLayer", 0, "  Base Layer", "", "Do not allow fracture to grow into this layer", "");

    CAF_PDM_InitField(&m_truncateAtFaults, "TruncateAtFaults", false, "Truncate At Faults", "", "If Fault Throw is larger than limit, truncate at fault", "");
    CAF_PDM_InitField(&m_minimumFaultThrow, "FaultThrowValue", 0.0f, "  Minimum Fault Throw", "", "If Fault Throw is larger than limit, truncate at fault", "");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFractureContainment::~RimFractureContainment() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFractureContainment::minimumFaultThrow() const
{
    if (m_truncateAtFaults())
    {
        return m_minimumFaultThrow;
    }

    return -1.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFractureContainment::isEnabled() const
{
    return (m_useContainment() || m_truncateAtFaults());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFractureContainment::isEclipseCellOpenForFlow(const RigMainGrid*      mainGrid,
                                                      size_t                  globalCellIndex,
                                                      const std::set<size_t>& reservoirCellIndicesOpenForFlow) const
{
    if (!isEnabled()) return true;

    if (m_useContainment())
    {
        CVF_ASSERT(mainGrid);

        if (globalCellIndex >= mainGrid->globalCellArray().size()) return false;

        auto cell              = mainGrid->globalCellArray()[globalCellIndex];
        auto mainGridCellIndex = cell.mainGridCellIndex();

        size_t i, j, k;
        mainGrid->ijkFromCellIndex(mainGridCellIndex, &i, &j, &k);

        if (k + 1 < static_cast<size_t>(m_topKLayer()))
        {
            return false;
        }

        if (k + 1 > static_cast<size_t>(m_baseKLayer()))
        {
            return false;
        }
    }

    if (m_truncateAtFaults())
    {
        if (reservoirCellIndicesOpenForFlow.count(globalCellIndex) > 0)
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
int RimFractureContainment::topKLayer() const
{
    return m_topKLayer;
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
int RimFractureContainment::baseKLayer() const
{
    return m_baseKLayer;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureContainment::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_useContainment);
    uiOrdering.add(&m_topKLayer);
    uiOrdering.add(&m_baseKLayer);

    m_topKLayer.uiCapability()->setUiReadOnly(!m_useContainment());
    m_baseKLayer.uiCapability()->setUiReadOnly(!m_useContainment());

    uiOrdering.add(&m_truncateAtFaults);
    uiOrdering.add(&m_minimumFaultThrow);

    m_minimumFaultThrow.uiCapability()->setUiReadOnly(!m_truncateAtFaults());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureContainment::fieldChangedByUi(const caf::PdmFieldHandle* changedField,
                                              const QVariant&            oldValue,
                                              const QVariant&            newValue)
{
    {
        RimProject* proj;
        this->firstAncestorOrThisOfType(proj);
        if (proj)
        {
            proj->reloadCompletionTypeResultsInAllViews();
        }
    }

    if (changedField == &m_useContainment || changedField == &m_truncateAtFaults)
    {
        RimFractureTemplate* fractureTemplate = nullptr;
        this->firstAncestorOrThisOfType(fractureTemplate);
        if (fractureTemplate)
        {
            fractureTemplate->updateConnectedEditors();
        }
    }
}
