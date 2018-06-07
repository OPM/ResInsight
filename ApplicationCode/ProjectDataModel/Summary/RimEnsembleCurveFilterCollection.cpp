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

#include "RimEnsembleCurveFilterCollection.h"

#include "RiaApplication.h"

#include "RimEnsembleCurveFilter.h"
#include "RimEnsembleCurveSet.h"

#include <algorithm>


CAF_PDM_SOURCE_INIT(RimEnsembleCurveFilterCollection, "RimEnsembleCurveFilterCollection");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEnsembleCurveFilterCollection::RimEnsembleCurveFilterCollection()
{
    CAF_PDM_InitObject("Curve Filters", ":/SummaryCurveFilter16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_active, "Active", "Active", "", "", "");
    m_active.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_filters, "CurveFilters", "", "", "", "");
    m_filters.uiCapability()->setUiHidden(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEnsembleCurveFilter* RimEnsembleCurveFilterCollection::addFilter()
{
    auto newFilter = new RimEnsembleCurveFilter();
    m_filters.push_back(newFilter);
    return newFilter;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveFilterCollection::removeFilter(RimEnsembleCurveFilter* filter)
{
    std::remove_if(m_filters.begin(), m_filters.end(), [&](RimEnsembleCurveFilter* f) { return f == filter; });
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimEnsembleCurveFilter*> RimEnsembleCurveFilterCollection::filters() const
{
    return m_filters.childObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimEnsembleCurveFilterCollection::isActive() const
{
    return m_active;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimEnsembleCurveFilterCollection::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    return options;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveFilterCollection::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &m_active)
    {
        RimEnsembleCurveSet* curveSet;
        firstAncestorOrThisOfType(curveSet);
        if (curveSet) curveSet->updateAllCurves();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveFilterCollection::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveFilterCollection::loadDataAndUpdate()
{
    for (auto& filter : m_filters) filter->loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimEnsembleCurveFilterCollection::objectToggleField()
{
    return &m_active;
}
