/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RimSummaryCaseCollection.h"

#include "RimEnsembleCurveSet.h"
#include "RimGridSummaryCase.h"
#include "RimProject.h"
#include "RimSummaryCase.h"

#include "RifSummaryReaderInterface.h"

#include <cmath>

CAF_PDM_SOURCE_INIT(RimSummaryCaseCollection, "SummaryCaseSubCollection");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCaseCollection::RimSummaryCaseCollection()
{
    CAF_PDM_InitObject("Summary Case Group", ":/SummaryGroup16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_cases, "SummaryCases", "", "", "", "");
    m_cases.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&m_name, "SummaryCollectionName", QString("Group"), "Name", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_nameAndItemCount, "NameCount", "Name", "", "", "");
    m_nameAndItemCount.registerGetMethod(this, &RimSummaryCaseCollection::nameAndItemCount);
    m_nameAndItemCount.uiCapability()->setUiHidden(true);
    m_nameAndItemCount.xmlCapability()->setIOWritable(false);

    CAF_PDM_InitField(&m_isEnsemble, "IsEnsemble", false, "Is Ensemble", "", "", "");
    m_isEnsemble.uiCapability()->setUiHidden(true);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCaseCollection::~RimSummaryCaseCollection()
{
    m_cases.deleteAllChildObjectsAsync();
    updateReferringCurveSets();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseCollection::removeCase(RimSummaryCase* summaryCase)
{
    m_cases.removeChildObject(summaryCase);
    updateReferringCurveSets();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseCollection::addCase(RimSummaryCase* summaryCase)
{
    m_cases.push_back(summaryCase);
    updateReferringCurveSets();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCase*> RimSummaryCaseCollection::allSummaryCases() const
{
    return m_cases.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseCollection::setName(const QString& name)
{
    m_name = name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryCaseCollection::name() const
{
    return m_name;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimSummaryCaseCollection::isEnsemble() const
{
    return m_isEnsemble();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseCollection::setAsEnsemble(bool isEnsemble)
{
    m_isEnsemble = isEnsemble;
    updateIcon();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseSummaryAddress> RimSummaryCaseCollection::calculateUnionOfSummaryAddresses() const
{
   std::set<RifEclipseSummaryAddress> addressUnion;

   for (RimSummaryCase* currCase: m_cases)
   {
       if ( !currCase ) continue;
       
       RifSummaryReaderInterface* reader = currCase->summaryReader();

       if ( !reader ) continue;

       const std::vector<RifEclipseSummaryAddress>& readerAddresses = reader->allResultAddresses();
       addressUnion.insert(readerAddresses.begin(), readerAddresses.end());

    }

    return addressUnion;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
EnsembleParameter RimSummaryCaseCollection::ensembleParameter(const QString& paramName) const
{
    if (!isEnsemble() || paramName.isEmpty()) return EnsembleParameter();

    EnsembleParameter eParam;
    eParam.name = paramName;

    bool numericValuesCount = 0;
    bool textValuesCount = 0;

    // Prepare case realization params, and check types
    for (const auto& rimCase : allSummaryCases())
    {
        auto crp = rimCase->caseRealizationParameters();
        if (!crp) continue;

        auto value = crp->parameterValue(paramName);
        if (!value.isValid()) continue;

        if (value.isNumeric())
        {
            double numVal = value.numericValue();
            eParam.values.push_back(QVariant(numVal));
            if (numVal < eParam.minValue) eParam.minValue = numVal;
            if (numVal > eParam.maxValue) eParam.maxValue = numVal;
            numericValuesCount++;
        }
        else if (value.isText())
        {
            eParam.values.push_back(QVariant(value.textValue()));
            textValuesCount++;
        }
    }

    if (numericValuesCount && !textValuesCount)
    {
        eParam.type = EnsembleParameter::TYPE_NUMERIC;
    }
    else if (textValuesCount && !numericValuesCount)
    {
        eParam.type = EnsembleParameter::TYPE_TEXT;
    }
    if (numericValuesCount && textValuesCount)
    {
        // A mix of types have been added to parameter values
        if (numericValuesCount > textValuesCount)
        {
            // Use numeric type
            for (auto& val : eParam.values)
            {
                if (val.type() == QVariant::String)
                {
                    val.setValue(std::numeric_limits<double>::infinity());
                }
            }
            eParam.type = EnsembleParameter::TYPE_NUMERIC;
        }
        else
        {
            // Use text type
            for (auto& val : eParam.values)
            {
                if (val.type() == QVariant::Double)
                {
                    val.setValue(QString::number(val.value<double>()));
                }
            }
            eParam.type = EnsembleParameter::TYPE_TEXT;
            eParam.minValue = std::numeric_limits<double>::infinity();
            eParam.maxValue = -std::numeric_limits<double>::infinity();
        }
    }

    if (eParam.isText())
    {
        // Remove duplicate texts
        std::set<QString> valueSet;
        for (const auto& val : eParam.values)
        {
            valueSet.insert(val.toString());
        }
        eParam.values.clear();
        for (const auto& val : valueSet)
        {
            eParam.values.push_back(QVariant(val));
        }
    }
    return eParam;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimSummaryCaseCollection::userDescriptionField()
{
    return &m_name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseCollection::updateReferringCurveSets() const
{
    // Update curve set referring to this group
    std::vector<PdmObjectHandle*> referringObjects;
    objectsWithReferringPtrFields(referringObjects);

    for (PdmObjectHandle* obj : referringObjects)
    {
        RimEnsembleCurveSet* curveSet = dynamic_cast<RimEnsembleCurveSet*>(obj);
        if (curveSet) curveSet->updateAllCurves();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryCaseCollection::nameAndItemCount() const
{
    size_t itemCount = m_cases.size();
    if (itemCount > 20)
    {
        return QString("%1 (%2)").arg(m_name()).arg(itemCount);
    }

    return m_name();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseCollection::updateIcon()
{
    if (m_isEnsemble) setUiIcon(QIcon(":/SummaryEnsemble16x16.png"));
    else              setUiIcon(QIcon(":/SummaryGroup16x16.png"));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseCollection::initAfterRead()
{
    updateIcon();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseCollection::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &m_isEnsemble)
    {
        updateIcon();
    }
}
