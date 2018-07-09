/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017- Statoil ASA
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

#include "RimEnsembleStatistics.h"

#include "RifSummaryReaderInterface.h"

#include "RigStatisticsMath.h"
#include "RiaTimeHistoryCurveMerger.h"

#include "RimEnsembleCurveSet.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"


CAF_PDM_SOURCE_INIT(RimEnsembleStatistics, "RimEnsembleStatistics");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEnsembleStatistics::RimEnsembleStatistics()
{
    CAF_PDM_InitObject("Ensemble Curve Filter", ":/EnsembleCurveSet16x16.png", "", "");

    CAF_PDM_InitField(&m_active, "Active", true, "Show statistics curves", "", "", "");
    CAF_PDM_InitField(&m_hideEnsembleCurves, "HideEnsembleCurves", false, "Hide Ensemble Curves", "", "", "");
    CAF_PDM_InitField(&m_basedOnFilteredCases, "BasedOnFilteredCases", false, "Based on Filtered Cases", "", "", "");
    CAF_PDM_InitField(&m_showP10Curve, "ShowP10Curve", true, "P90", "", "", "");    // Yes, P90
    CAF_PDM_InitField(&m_showP50Curve, "ShowP50Curve", true, "P50", "", "", "");
    CAF_PDM_InitField(&m_showP90Curve, "ShowP90Curve", true, "P10", "", "", "");    // Yes, P10
    CAF_PDM_InitField(&m_showMeanCurve, "ShowMeanCurve", true, "Mean", "", "", "");
    CAF_PDM_InitField(&m_showCurveLabels, "ShowCurveLabels", true, "Show Curve Labels", "", "", "");
    CAF_PDM_InitField(&m_color, "Color", cvf::Color3f(cvf::Color3::BLACK), "Color", "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimEnsembleStatistics::isActive() const
{
    return m_active;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsembleStatistics::disableP10Curve(bool disable)
{
    m_showP10Curve.uiCapability()->setUiReadOnly(disable);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsembleStatistics::disableP50Curve(bool disable)
{
    m_showP50Curve.uiCapability()->setUiReadOnly(disable);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsembleStatistics::disableP90Curve(bool disable)
{
    m_showP90Curve.uiCapability()->setUiReadOnly(disable);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsembleStatistics::disableMeanCurve(bool disable)
{
    m_showMeanCurve.uiCapability()->setUiReadOnly(disable);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsembleStatistics::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &m_active ||
        changedField == &m_basedOnFilteredCases ||
        changedField == &m_showP10Curve ||
        changedField == &m_showP50Curve ||
        changedField == &m_showP90Curve ||
        changedField == &m_showMeanCurve ||
        changedField == &m_showCurveLabels ||
        changedField == &m_color)
    {
        auto curveSet = parentCurveSet();
        if (!curveSet) return;

        curveSet->updateStatisticsCurves();

        if (changedField == &m_active || changedField == &m_basedOnFilteredCases) curveSet->updateConnectedEditors();
    }


    if (changedField == &m_hideEnsembleCurves)
    {
        auto curveSet = parentCurveSet();
        if (!curveSet) return;

        curveSet->updateAllCurves();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsembleStatistics::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    auto curveSet = parentCurveSet();

    uiOrdering.add(&m_active);
    uiOrdering.add(&m_hideEnsembleCurves);
    uiOrdering.add(&m_basedOnFilteredCases);
    uiOrdering.add(&m_showCurveLabels);
    uiOrdering.add(&m_color);

    auto group = uiOrdering.addNewGroup("Curves");
    group->add(&m_showP90Curve);
    group->add(&m_showP50Curve);
    group->add(&m_showMeanCurve);
    group->add(&m_showP10Curve);

    disableP10Curve(!m_active || !curveSet->hasP10Data());
    disableP50Curve(!m_active || !curveSet->hasP50Data());
    disableP90Curve(!m_active || !curveSet->hasP90Data());
    disableMeanCurve(!m_active);
    m_showCurveLabels.uiCapability()->setUiReadOnly(!m_active);
    m_color.uiCapability()->setUiReadOnly(!m_active);

    m_showP10Curve.uiCapability()->setUiName(curveSet->hasP10Data() ? "P90" : "P90 (Needs > 8 curves)");
    m_showP90Curve.uiCapability()->setUiName(curveSet->hasP90Data() ? "P10" : "P10 (Needs > 8 curves)");

    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEnsembleCurveSet* RimEnsembleStatistics::parentCurveSet() const
{
    RimEnsembleCurveSet* curveSet;
    firstAncestorOrThisOfType(curveSet);
    return curveSet;
}
