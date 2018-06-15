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

    CAF_PDM_InitFieldNoDefault(&m_active, "Active", "Show statistics curves", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_showP10Curve, "ShowP10Curve", "P10", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_showP50Curve, "ShowP50Curve", "P50", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_showP90Curve, "ShowP90Curve", "P90", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_showMeanCurve, "ShowPMeanCurve", "Mean", "", "", "");
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
QList<caf::PdmOptionItemInfo> RimEnsembleStatistics::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;


    return options;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsembleStatistics::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &m_active ||
        changedField == &m_showP10Curve ||
        changedField == &m_showP50Curve ||
        changedField == &m_showP90Curve ||
        changedField == &m_showMeanCurve ||
        changedField == &m_color)
    {
        auto curveSet = parentCurveSet();
        if (!curveSet) return;

        curveSet->updateStatisticsCurves(false);

        if (changedField == &m_active) curveSet->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsembleStatistics::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_active);

    auto group = uiOrdering.addNewGroup("Curves");
    group->add(&m_showP10Curve);
    group->add(&m_showP50Curve);
    group->add(&m_showP90Curve);
    group->add(&m_showMeanCurve);
    group->add(&m_color);

    m_showP10Curve.uiCapability()->setUiReadOnly(!m_active);
    m_showP50Curve.uiCapability()->setUiReadOnly(!m_active);
    m_showP90Curve.uiCapability()->setUiReadOnly(!m_active);
    m_showMeanCurve.uiCapability()->setUiReadOnly(!m_active);
    m_color.uiCapability()->setUiReadOnly(!m_active);

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
