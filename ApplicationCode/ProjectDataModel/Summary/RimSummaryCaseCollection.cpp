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

#include "RimDerivedEnsembleCaseCollection.h"
#include "RimEnsembleCurveSet.h"
#include "RimGridSummaryCase.h"
#include "RimProject.h"
#include "RimSummaryCase.h"

#include "RifSummaryReaderInterface.h"

#include <QFileDialog>
#include <QMessageBox>

#include <cmath>
#include <algorithm>

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
    size_t caseCountBeforeRemove = m_cases.size();
    m_cases.removeChildObject(summaryCase);
    updateReferringCurveSets();

    if (m_isEnsemble && m_cases.size() != caseCountBeforeRemove)
    {
        if(dynamic_cast<RimDerivedEnsembleCase*>(summaryCase) == nullptr)
            calculateEnsembleParametersIntersectionHash();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseCollection::deleteAllCases()
{
    m_cases.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseCollection::addCase(RimSummaryCase* summaryCase, bool updateCurveSets)
{
    m_cases.push_back(summaryCase);

    // Update derived ensemble cases (if any)
    std::vector<caf::PdmObjectHandle*> referringObjects;
    objectsWithReferringPtrFields(referringObjects);
    for (auto refObj : referringObjects)
    {
        auto derEnsemble = dynamic_cast<RimDerivedEnsembleCaseCollection*>(refObj);
        if (!derEnsemble) continue;

        derEnsemble->updateDerivedEnsembleCases();
        if (updateCurveSets) derEnsemble->updateReferringCurveSets();
    }

    if (m_isEnsemble)
    {
        validateEnsembleCases({ summaryCase });
        calculateEnsembleParametersIntersectionHash();
    }

    if(updateCurveSets) updateReferringCurveSets();
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
    if (isEnsemble != m_isEnsemble)
    {
        m_isEnsemble = isEnsemble;
        updateIcon();

        if (m_isEnsemble && dynamic_cast<RimDerivedEnsembleCaseCollection*>(this) == nullptr)
        {
            validateEnsembleCases(allSummaryCases());
            calculateEnsembleParametersIntersectionHash();
        }
    }
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

       const std::set<RifEclipseSummaryAddress>& readerAddresses = reader->allResultAddresses();
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
void RimSummaryCaseCollection::calculateEnsembleParametersIntersectionHash()
{
    clearEnsembleParametersHashes();

    // Find ensemble parameters intersection
    std::set<QString> paramNames;
    auto sumCases = allSummaryCases();

    for (int i = 0; i < sumCases.size(); i++)
    {
        auto crp = sumCases[i]->caseRealizationParameters();
        if (!crp) continue;

        auto caseParamNames = crp->parameterNames();
        
        if (i == 0) paramNames = caseParamNames;
        else
        {
            std::set<QString> newIntersection;
            std::set_intersection(paramNames.begin(), paramNames.end(),
                                  caseParamNames.begin(), caseParamNames.end(),
                                  std::inserter(newIntersection, newIntersection.end()));

            if(paramNames.size() != newIntersection.size()) paramNames = newIntersection;
        }
    }

    for (auto sumCase : sumCases)
    {
        auto crp = sumCase->caseRealizationParameters();
        if(crp) crp->calculateParametersHash(paramNames);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseCollection::clearEnsembleParametersHashes()
{
    for (auto sumCase : allSummaryCases())
    {
        auto crp = sumCase->caseRealizationParameters();
        if (crp) crp->clearParametersHash();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseCollection::loadDataAndUpdate()
{
    onLoadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimSummaryCaseCollection::validateEnsembleCases(const std::vector<RimSummaryCase*> cases)
{
    // Validate ensemble parameters
    try
    {
        QString errors;
        std::hash<std::string> paramsHasher;
        size_t paramsHash = 0;

        for (RimSummaryCase* rimCase : cases)
        {
            if (rimCase->caseRealizationParameters() == nullptr || rimCase->caseRealizationParameters()->parameters().empty())
            {
                errors.append(QString("The case %1 has no ensemble parameters\n").arg(QFileInfo(rimCase->summaryHeaderFilename()).fileName()));
            }
            else
            {
                QString paramNames;
                for (std::pair<QString, RigCaseRealizationParameters::Value> paramPair : rimCase->caseRealizationParameters()->parameters())
                {
                    paramNames.append(paramPair.first);
                }

                size_t currHash = paramsHasher(paramNames.toStdString());
                if (paramsHash == 0)
                {
                    paramsHash = currHash;
                }
                else if (paramsHash != currHash)
                {
                    throw QString("Ensemble parameters differ between cases");
                }
            }
        }


        if (!errors.isEmpty())
        {
            errors.append("\n");
            errors.append("No parameters file (parameters.txt or runspecification.xml) was found in \n");
            errors.append("the searched folders. ResInsight searches the home folder of the summary \n");
            errors.append("case file and the three folder levels above that.\n");

            throw errors;
        }
        return true;
    }
    catch (QString errorMessage)
    {
        QMessageBox mbox;
        mbox.setIcon(QMessageBox::Icon::Warning);
        mbox.setInformativeText(errorMessage);
        mbox.exec();
        return false;
    }
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
void RimSummaryCaseCollection::onLoadDataAndUpdate()
{
    if (m_isEnsemble) calculateEnsembleParametersIntersectionHash();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseCollection::updateReferringCurveSets()
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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseCollection::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_name);
    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseCollection::setNameAsReadOnly()
{
    m_name.uiCapability()->setUiReadOnly(true);
}
