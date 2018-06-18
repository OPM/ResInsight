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

#include <cafPdmUiTableViewEditor.h>
#include <cafPdmUiTreeOrdering.h>
#include <cafPdmUiPushButtonEditor.h>

#include <algorithm>


CAF_PDM_SOURCE_INIT(RimEnsembleCurveFilterCollection, "RimEnsembleCurveFilterCollection");

//--------------------------------------------------------------------------------------------------
/// Internal variables
//--------------------------------------------------------------------------------------------------
static std::vector<RimEnsembleCurveFilter*> _removedFilters;

static void garbageCollectFilters();

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEnsembleCurveFilterCollection::RimEnsembleCurveFilterCollection()
{
    CAF_PDM_InitObject("Curve Filters", ":/SummaryCurveFilter16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_active, "Active", "Active", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_filters, "CurveFilters", "", "", "", "");
    m_filters.uiCapability()->setUiTreeChildrenHidden(true);
    //m_filters.uiCapability()->setUiEditorTypeName(caf::PdmUiTableViewEditor::uiEditorTypeName());
    m_filters.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);

    CAF_PDM_InitFieldNoDefault(&m_newFilterButton, "NewEnsembleFilter", "New Filter", "", "", "");
    m_newFilterButton = false;
    m_newFilterButton.uiCapability()->setUiEditorTypeName(caf::PdmUiPushButtonEditor::uiEditorTypeName());
    m_newFilterButton.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEnsembleCurveFilter* RimEnsembleCurveFilterCollection::addFilter(const QString& ensembleParameterName)
{
    garbageCollectFilters();

    auto newFilter = new RimEnsembleCurveFilter(ensembleParameterName);
    m_filters.push_back(newFilter);
    return newFilter;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveFilterCollection::removeFilter(RimEnsembleCurveFilter* filter)
{
    garbageCollectFilters();

    size_t sizeBefore = m_filters.size();
    m_filters.removeChildObject(filter);
    size_t sizeAfter = m_filters.size();

    if(sizeAfter < sizeBefore) _removedFilters.push_back(filter);
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
    RimEnsembleCurveSet* curveSet;
    firstAncestorOrThisOfType(curveSet);
    if (!curveSet) return;

    if (changedField == &m_active)
    {
        curveSet->updateAllCurves();
    }
    else if (changedField == &m_newFilterButton)
    {
        m_newFilterButton = false;

        addFilter();
        updateConnectedEditors();
        curveSet->updateAllCurves();

    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveFilterCollection::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_newFilterButton);

    for (auto& filter : m_filters)
    {
        QString groupTitle;
        auto selEnsembleParam = filter->selectedEnsembleParameter();
        if (selEnsembleParam.isNumeric())
        {
            if (!filter->isActive()) groupTitle = "DISABLED - ";
            groupTitle += QString("%1. Min: %2, Max: %3")
                .arg(filter->ensembleParameterName())
                .arg(QString::number(filter->minValue()))
                .arg(QString::number(filter->maxValue()));
        }
        else if (selEnsembleParam.isText())
        {
            if (!filter->isActive()) groupTitle = "DISABLED - ";
            groupTitle += QString("%1. Categories: ")
                .arg(filter->ensembleParameterName());

            bool first = true;
            for (auto cat : filter->categories())
            {
                if (!first) groupTitle += ", ";
                groupTitle += cat;
                first = false;
            }
        }
        caf::PdmUiGroup* filterGroup = uiOrdering.addNewGroupWithKeyword(groupTitle, QString("EnsembleFilter_") + filter->filterId());
        filter->defineUiOrdering(uiConfigName, *filterGroup);
    }

    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveFilterCollection::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /* = "" */)
{
    uiTreeOrdering.skipRemainingChildren(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveFilterCollection::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute)
{
    if (field == &m_newFilterButton)
    {
        caf::PdmUiPushButtonEditorAttribute* attr = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>(attribute);
        if (!attr) return;

        attr->m_buttonText = "Add Ensemble Curve Filter";
    }
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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void garbageCollectFilters()
{
    for (auto filter : _removedFilters)
    {
        delete filter;
    }
    _removedFilters.clear();
}
