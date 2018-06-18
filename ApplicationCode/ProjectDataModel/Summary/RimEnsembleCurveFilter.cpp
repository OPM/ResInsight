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
#include "RimSummaryCase.h"

#include "cafPdmUiDoubleSliderEditor.h"
#include "cafPdmUiPushButtonEditor.h"

#include <algorithm>


CAF_PDM_SOURCE_INIT(RimEnsembleCurveFilter, "RimEnsembleCurveFilter");


//--------------------------------------------------------------------------------------------------
/// Internal constants
//--------------------------------------------------------------------------------------------------
#define DOUBLE_INF  std::numeric_limits<double>::infinity()

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEnsembleCurveFilter::RimEnsembleCurveFilter() : m_lowerLimit(DOUBLE_INF), m_upperLimit(DOUBLE_INF)
{
    CAF_PDM_InitObject("Ensemble Curve Filter", ":/EnsembleCurveSet16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_active, "Active", "Active", "", "", "");
    m_active = true;

    CAF_PDM_InitFieldNoDefault(&m_ensembleParameterName, "EnsembleParameter", "Ensemble Parameter", "", "", "");
    
    CAF_PDM_InitFieldNoDefault(&m_minValue, "MinValue", "Min", "", "", "");
    m_minValue.uiCapability()->setUiEditorTypeName(caf::PdmUiDoubleSliderEditor::uiEditorTypeName());

    CAF_PDM_InitFieldNoDefault(&m_maxValue, "MaxValue", "Max", "", "", "");
    m_maxValue.uiCapability()->setUiEditorTypeName(caf::PdmUiDoubleSliderEditor::uiEditorTypeName());

    CAF_PDM_InitFieldNoDefault(&m_categories, "Categories", "Categories", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_deleteButton, "DeleteEnsembleFilter", "Delete Filter", "", "", "");
    m_deleteButton = false;
    m_deleteButton.uiCapability()->setUiEditorTypeName(caf::PdmUiPushButtonEditor::uiEditorTypeName());
    m_deleteButton.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEnsembleCurveFilter::RimEnsembleCurveFilter(const QString& ensembleParameterName) : RimEnsembleCurveFilter()
{
    m_ensembleParameterName = ensembleParameterName;
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
QString RimEnsembleCurveFilter::ensembleParameterName() const
{
    return m_ensembleParameterName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimEnsembleCurveFilter::filterId() const
{
    return QString("%1").arg((int64_t)this);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimEnsembleCurveFilter::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    if (fieldNeedingOptions == &m_ensembleParameterName)
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
        auto eParam = ensemble ? ensemble->ensembleParameter(m_ensembleParameterName) : EnsembleParameter();

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

    if (changedField == &m_ensembleParameterName)
    {
        auto eParam = selectedEnsembleParameter();
        if (eParam.isNumeric())
        {
            setDefaultValues();
        }
        else if (eParam.isText())
        {
            m_categories.v().clear();
            for (const auto& val : eParam.values)
            {
                m_categories.v().push_back(val.toString());
            }
        }
        curveSet->updateAllCurves();

        auto collection = parentCurveFilterCollection();
        if (collection) collection->updateConnectedEditors();
    }
    else if (changedField == &m_active ||
             changedField == &m_minValue ||
             changedField == &m_maxValue ||
             changedField == &m_categories)
    {
        if (curveSet)
        {
            curveSet->updateAllCurves();
            curveSet->filterCollection()->updateConnectedEditors();
        }
    }
    else if (changedField == &m_deleteButton)
    {
        m_deleteButton = false;

        if (!curveSet) return;

        curveSet->filterCollection()->removeFilter(this);
        curveSet->filterCollection()->updateConnectedEditors();
        curveSet->updateAllCurves();

        // Must be last statement before return
        delete this;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveFilter::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    auto eParam = selectedEnsembleParameter();

    uiOrdering.add(&m_active);
    uiOrdering.add(&m_ensembleParameterName);

    if (eParam.isNumeric())
    {
        uiOrdering.add(&m_minValue);
        uiOrdering.add(&m_maxValue);
    }
    else if(eParam.isText())
    {
        uiOrdering.add(&m_categories);
    }
    uiOrdering.add(&m_deleteButton);

    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveFilter::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute)
{
    if (field == &m_minValue || field == &m_maxValue)
    {
        caf::PdmUiDoubleSliderEditorAttribute* myAttr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>(attribute);
        if (!myAttr)
        {
            return;
        }

        myAttr->m_minimum = m_lowerLimit;
        myAttr->m_maximum = m_upperLimit;
    }
    else if (field == &m_deleteButton)
    {
        caf::PdmUiPushButtonEditorAttribute* attr = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>(attribute);
        if (!attr) return;

        attr->m_buttonText = "Delete";
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCase*> RimEnsembleCurveFilter::applyFilter(const std::vector<RimSummaryCase*>& allSumCases)
{
    auto curveSet = parentCurveSet();
    auto ensemble = curveSet ? curveSet->summaryCaseCollection() : nullptr;
    if (!ensemble || !isActive()) return allSumCases;

    std::set<RimSummaryCase*> casesToRemove;
    for (const auto& sumCase : allSumCases)
    {
        auto eParam = ensemble->ensembleParameter(m_ensembleParameterName());
        if (!eParam.isValid()) continue;
        if (!sumCase->caseRealizationParameters()) continue;

        auto crpValue = sumCase->caseRealizationParameters()->parameterValue(m_ensembleParameterName());

        if (eParam.isNumeric())
        {
            if (!crpValue.isNumeric() ||
                crpValue.numericValue() < m_minValue() ||
                crpValue.numericValue() > m_maxValue())
            {
                casesToRemove.insert(sumCase);
            }
        }
        else if (eParam.isText())
        {
            const auto& filterCategories = categories();
            if (!crpValue.isText() ||
                std::count(filterCategories.begin(), filterCategories.end(), crpValue.textValue()) == 0)
            {
                casesToRemove.insert(sumCase);
            }
        }
    }

    std::vector<RimSummaryCase*> filteredCases;
    std::set<RimSummaryCase*> allCasesSet(allSumCases.begin(), allSumCases.end());
    std::set_difference(allCasesSet.begin(), allCasesSet.end(),
                        casesToRemove.begin(), casesToRemove.end(),
                        std::inserter(filteredCases, filteredCases.end()));
    return filteredCases;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveFilter::loadDataAndUpdate()
{
    setDefaultValues();
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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEnsembleCurveFilterCollection* RimEnsembleCurveFilter::parentCurveFilterCollection() const
{
    RimEnsembleCurveFilterCollection* coll;
    firstAncestorOrThisOfType(coll);
    return coll;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveFilter::setDefaultValues()
{
    auto eParam = selectedEnsembleParameter();
    if (eParam.isValid() && eParam.isNumeric())
    {
        m_lowerLimit = m_minValue = eParam.minValue;
        m_upperLimit = m_maxValue = eParam.maxValue;

        m_minValue.uiCapability()->setUiName(QString("Min (%1)").arg(m_lowerLimit));
        m_maxValue.uiCapability()->setUiName(QString("Max (%1)").arg(m_upperLimit));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
EnsembleParameter RimEnsembleCurveFilter::selectedEnsembleParameter() const
{
    auto curveSet = parentCurveSet();
    auto ensemble = curveSet ? curveSet->summaryCaseCollection() : nullptr;
    return ensemble ? ensemble->ensembleParameter(m_ensembleParameterName) : EnsembleParameter();
}
