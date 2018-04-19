/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 Statoil ASA
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

#include "RimVirtualPerforationResults.h"

#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimRegularLegendConfig.h"

CAF_PDM_SOURCE_INIT(RimVirtualPerforationResults, "RimVirtualPerforationResults");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimVirtualPerforationResults::RimVirtualPerforationResults()
{
    // clang-format off

    QString connectionFactorUiName = "Well Connection Factors";

    CAF_PDM_InitObject(connectionFactorUiName, ":/CellResult.png", "", "");

    CAF_PDM_InitField(&m_isActive,              "ShowConnectionFactors",    false,   "", "", "", "");
    CAF_PDM_InitField(&m_showClosedConnections, "ShowClosedConnections",    true,   "Show On Closed Connections", "", "", "");
    CAF_PDM_InitField(&m_geometryScaleFactor,   "GeometryScaleFactor",      2.0,    "Geometry Scale Factor", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_legendConfig, "LegendDefinition", "Legend Definition", "", "", "");
    m_legendConfig.uiCapability()->setUiHidden(true);

    m_legendConfig = new RimRegularLegendConfig();
    m_legendConfig->setTitle(connectionFactorUiName);

    // clang-format on
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimVirtualPerforationResults::~RimVirtualPerforationResults() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimVirtualPerforationResults::showConnectionFactors() const
{
    return m_isActive();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimVirtualPerforationResults::showConnectionFactorsOnClosedConnections() const
{
    return m_showClosedConnections();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimVirtualPerforationResults::geometryScaleFactor() const
{
    return m_geometryScaleFactor();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimRegularLegendConfig* RimVirtualPerforationResults::legendConfig() const
{
    return m_legendConfig();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVirtualPerforationResults::loadData()
{
    RimEclipseCase* eclipseCase = nullptr;
    this->firstAncestorOrThisOfType(eclipseCase);
    if (eclipseCase)
    {
        eclipseCase->computeAndGetVirtualPerforationTransmissibilities();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVirtualPerforationResults::fieldChangedByUi(const caf::PdmFieldHandle* changedField,
                                                    const QVariant&            oldValue,
                                                    const QVariant&            newValue)
{
    if (changedField == &m_isActive)
    {
        updateUiIconFromToggleField();

        loadData();
    }

    RimEclipseView* eclView = nullptr;
    this->firstAncestorOrThisOfTypeAsserted(eclView);

    eclView->scheduleCreateDisplayModelAndRedraw();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimVirtualPerforationResults::objectToggleField()
{
    return &m_isActive;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVirtualPerforationResults::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_geometryScaleFactor);
    uiOrdering.add(&m_showClosedConnections);

    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVirtualPerforationResults::initAfterRead()
{
    updateUiIconFromToggleField();
}
