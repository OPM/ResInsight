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

#include "RimEnsembleCurveFilter.h"
#include "RimEnsembleCurveFilterCollection.h"
#include "RimEnsembleCurveSet.h"

#include "cafPdmUiDoubleSliderEditor.h"


CAF_PDM_SOURCE_INIT(RimEnsembleCurveFilter, "RimEnsembleCurveFilter");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEnsembleCurveFilter::RimEnsembleCurveFilter()
{
    CAF_PDM_InitObject("Ensemble Curve Filter", ":/EnsembleCurveSet16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_active, "Active", "Active", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_ensembleParameter, "EnsembleParameter", "Ensemble Parameter", "", "", "");
    
    CAF_PDM_InitFieldNoDefault(&m_minValue, "MinValue", "Min", "", "", "");
    m_minValue.uiCapability()->setUiEditorTypeName(caf::PdmUiDoubleSliderEditor::uiEditorTypeName());

    CAF_PDM_InitFieldNoDefault(&m_maxValue, "MaxValue", "Max", "", "", "");
    m_maxValue.uiCapability()->setUiEditorTypeName(caf::PdmUiDoubleSliderEditor::uiEditorTypeName());

    CAF_PDM_InitFieldNoDefault(&m_categories, "Categories", "Categories", "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimEnsembleCurveFilter::isActive() const
{
    RimEnsembleCurveFilterCollection* coll;
    firstAncestorOrThisOfType(coll);

    return (!coll || coll->isActive()) && m_active;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimEnsembleCurveFilter::ensembleParameter() const
{
    return m_ensembleParameter;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RimEnsembleCurveFilter::minValue() const
{
    return m_minValue;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RimEnsembleCurveFilter::maxValue() const
{
    return m_maxValue;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::set<QString> RimEnsembleCurveFilter::categories() const
{
    const auto cs = m_categories();
    return std::set<QString>(cs.begin(), cs.end());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimEnsembleCurveFilter::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    if (fieldNeedingOptions == &m_ensembleParameter)
    {
        auto curveSet = parentCurveSet();
        if (curveSet)
        {
             auto names = curveSet->ensembleParameterNames();
             for (auto& name : names)
             {
                 options.push_back(caf::PdmOptionItemInfo(name, name));
             }
        }
    }
    else if (fieldNeedingOptions == &m_categories)
    {
        auto curveSet = parentCurveSet();
        auto ensemble = curveSet ? curveSet->summaryCaseCollection() : nullptr;
        auto eParam = ensemble ? ensemble->ensembleParameter(m_ensembleParameter) : EnsembleParameter();

        if (eParam.isText())
        {
            for (const auto& val : eParam.values)
            {
                options.push_back(caf::PdmOptionItemInfo(val.toString(), val.toString()));
            }
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveFilter::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    auto curveSet = parentCurveSet();

    if (changedField == &m_ensembleParameter)
    {
        auto ensemble = curveSet ? curveSet->summaryCaseCollection() : nullptr;
        auto eParam = ensemble ? ensemble->ensembleParameter(m_ensembleParameter) : EnsembleParameter();

        if (eParam.isNumeric())
        {
            m_minValue = eParam.minValue;
            m_maxValue = eParam.maxValue;
        }
        else if (eParam.isText())
        {
            m_categories.v().clear();
            for (const auto val : eParam.values)
            {
                m_categories.v().push_back(val.toString());
            }
        }
        curveSet->updateAllCurves();
    }
    else if (changedField == &m_active ||
             changedField == &m_minValue ||
             changedField == &m_maxValue ||
             changedField == &m_categories)
    {
        if (curveSet) curveSet->updateAllCurves();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveFilter::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    auto curveSet = parentCurveSet();
    auto ensemble = curveSet ? curveSet->summaryCaseCollection() : nullptr;
    auto eParam = ensemble ? ensemble->ensembleParameter(m_ensembleParameter) : EnsembleParameter();

    uiOrdering.add(&m_ensembleParameter);

    if (eParam.isNumeric())
    {
        uiOrdering.add(&m_minValue);
        uiOrdering.add(&m_maxValue);
    }
    else if (eParam.isText())
    {
        uiOrdering.add(&m_categories);
    }

    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimEnsembleCurveFilter::objectToggleField()
{
    return &m_active;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEnsembleCurveSet * RimEnsembleCurveFilter::parentCurveSet() const
{
    RimEnsembleCurveSet* curveSet;
    firstAncestorOrThisOfType(curveSet);
    return curveSet;
}
