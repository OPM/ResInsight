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

#include "RimSummaryPlotSourceStepping.h"

#include "RiaApplication.h"
#include "RiaSummaryCurveAnalyzer.h"
#include "RiaSummaryCurveDefinition.h"

#include "RifSummaryReaderInterface.h"

#include "RimOilField.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryCurve.h"
#include "RimSummaryCurveCollection.h"
#include "RimSummaryPlot.h"

#include "cafPdmUiItem.h"
#include "cafPdmUiListEditor.h"

CAF_PDM_SOURCE_INIT(RimSummaryPlotSourceStepping, "RimSummaryCurveCollectionModifier");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlotSourceStepping::RimSummaryPlotSourceStepping()
{
    // clang-format off
    CAF_PDM_InitObject("Summary Curves Modifier", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_summaryCase,      "CurveCase",    "Case", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_wellName,         "WellName",     "Well Name", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_wellGroupName,    "GroupName",    "Group Name", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_region,           "Region",       "Region", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_quantity,         "Quantities",   "Quantity", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_wellNameProxy,    "WellNameProxy", "WellNameProxy", "", "", "");
    m_wellNameProxy.registerGetMethod(this, &RimSummaryPlotSourceStepping::wellName);
    m_wellNameProxy.registerSetMethod(this, &RimSummaryPlotSourceStepping::setWellName);
    m_wellNameProxy.uiCapability()->setUiEditorTypeName(caf::PdmUiListEditor::uiEditorTypeName());
    m_wellNameProxy.uiCapability()->setUiHidden(true);
    // clang-format on
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotSourceStepping::applyNextIdentifier()
{
    updateUiFromCurves();

    caf::PdmValueField* valueField = valueFieldToModify();
    if (valueField)
    {
        bool useOptionsOnly = true;

        QList<caf::PdmOptionItemInfo> options = calculateValueOptions(valueField, nullptr);
        if (options.isEmpty())
        {
            return;
        }

        auto uiVariant = valueField->uiCapability()->toUiBasedQVariant();

        int currentIndex = -1;
        for (int i = 0; i < options.size(); i++)
        {
            if (uiVariant == options[i].optionUiText())
            {
                currentIndex = i;
            }
        }

        if (currentIndex == -1)
        {
            currentIndex = 0;
        }

        int nextIndex = currentIndex + 1;
        if (nextIndex >= options.size() - 1)
        {
            nextIndex = 0;
        }

        auto optionValue = options[nextIndex].value();

        QVariant currentValue = valueField->toQVariant();

        valueField->setFromQVariant(optionValue);

        valueField->uiCapability()->notifyFieldChanged(currentValue, optionValue);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotSourceStepping::applyPreviousIdentifier()
{
    updateUiFromCurves();

    caf::PdmValueField* valueField = valueFieldToModify();
    if (valueField)
    {
        bool useOptionsOnly = true;

        QList<caf::PdmOptionItemInfo> options = calculateValueOptions(valueField, nullptr);
        if (options.isEmpty())
        {
            return;
        }

        auto uiVariant = valueField->uiCapability()->toUiBasedQVariant();

        int currentIndex = -1;
        for (int i = 0; i < options.size(); i++)
        {
            if (uiVariant == options[i].optionUiText())
            {
                currentIndex = i;
            }
        }

        if (currentIndex == -1)
        {
            currentIndex = 0;
        }

        int nextIndex = currentIndex - 1;
        if (nextIndex < 0)
        {
            nextIndex = options.size() - 1;
        }

        auto optionValue = options[nextIndex].value();

        QVariant currentValue = valueField->toQVariant();

        valueField->setFromQVariant(optionValue);

        valueField->uiCapability()->notifyFieldChanged(currentValue, optionValue);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotSourceStepping::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    updateUiFromCurves();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimSummaryPlotSourceStepping::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                                  bool*                      useOptionsOnly)
{
    if (fieldNeedingOptions == &m_summaryCase)
    {
        QList<caf::PdmOptionItemInfo> options;

        RimProject* proj = RiaApplication::instance()->project();

        RimSummaryCaseMainCollection* sumCaseColl =
            proj->activeOilField() ? proj->activeOilField()->summaryCaseMainCollection() : nullptr;
        if (sumCaseColl)
        {
            for (auto sumCase : sumCaseColl->allSummaryCases())
            {
                options.append(caf::PdmOptionItemInfo(sumCase->caseName(), sumCase));
            }
        }

        return options;
    }

    std::set<QString> identifierTexts;

    RifSummaryReaderInterface* reader = summaryReader();
    if (reader)
    {
        const std::vector<RifEclipseSummaryAddress> allAddresses = reader->allResultAddresses();

        RiaSummaryCurveAnalyzer analyzer;
        analyzer.analyzeAdresses(allAddresses);

        if (fieldNeedingOptions == &m_wellName || fieldNeedingOptions == &m_wellNameProxy)
        {
            identifierTexts = analyzer.identifierTexts(RifEclipseSummaryAddress::SUMMARY_WELL);
        }
        else if (fieldNeedingOptions == &m_region)
        {
            identifierTexts = analyzer.identifierTexts(RifEclipseSummaryAddress::SUMMARY_REGION);
        }
        else if (fieldNeedingOptions == &m_wellGroupName)
        {
            identifierTexts = analyzer.identifierTexts(RifEclipseSummaryAddress::SUMMARY_WELL_GROUP);
        }
        else if (fieldNeedingOptions == &m_quantity)
        {
            RimSummaryCurveCollection* curveCollection = nullptr;
            this->firstAncestorOrThisOfTypeAsserted(curveCollection);

            RifEclipseSummaryAddress::SummaryVarCategory category = RifEclipseSummaryAddress::SUMMARY_FIELD;

            if (curveCollection->curves().size() > 0)
            {
                category = curveCollection->curves()[0]->summaryAddressY().category();
            }

            RiaSummaryCurveAnalyzer quantityAnalyzer;

            auto subset = RiaSummaryCurveAnalyzer::addressesForCategory(allAddresses, category);
            quantityAnalyzer.analyzeAdresses(subset);

            for (const auto& quantity : quantityAnalyzer.quantities())
            {
                identifierTexts.insert(QString::fromStdString(quantity));
            }
        }
    }

    QList<caf::PdmOptionItemInfo> options;
    if (identifierTexts.size() > 0)
    {
        for (const auto& text : identifierTexts)
        {
            options.append(caf::PdmOptionItemInfo(text, text));
        }
    }
    else
    {
        options.push_back(caf::PdmOptionItemInfo("None", "None"));
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotSourceStepping::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue,
                                                    const QVariant& newValue)
{
    RimSummaryCurveCollection* curveCollection = nullptr;
    this->firstAncestorOrThisOfTypeAsserted(curveCollection);

    bool triggerLoadDataAndUpdate = false;

    if (changedField == &m_summaryCase)
    {
        if (m_summaryCase())
        {
            for (auto curve : curveCollection->curves())
            {
                curve->setSummaryCaseY(m_summaryCase);
            }

            triggerLoadDataAndUpdate = true;
        }
    }
    else if (changedField == &m_wellName || changedField == &m_wellNameProxy)
    {
        for (auto curve : curveCollection->curves())
        {
            RifEclipseSummaryAddress adr = curve->summaryAddressY();
            if (adr.category() == RifEclipseSummaryAddress::SUMMARY_WELL)
            {
                adr.setWellName(m_wellName().toStdString());

                curve->setSummaryAddressY(adr);
            }
        }

        triggerLoadDataAndUpdate = true;
    }
    else if (changedField == &m_region)
    {
        for (auto curve : curveCollection->curves())
        {
            RifEclipseSummaryAddress adr = curve->summaryAddressY();
            if (adr.category() == RifEclipseSummaryAddress::SUMMARY_REGION)
            {
                adr.setRegion(m_region());

                curve->setSummaryAddressY(adr);
            }
        }

        triggerLoadDataAndUpdate = true;
    }
    else if (changedField == &m_quantity)
    {
        for (auto curve : curveCollection->curves())
        {
            RifEclipseSummaryAddress adr = curve->summaryAddressY();
            adr.setQuantityName(m_quantity().toStdString());
            curve->setSummaryAddressY(adr);
        }

        triggerLoadDataAndUpdate = true;
    }
    else if (changedField == &m_wellGroupName)
    {
        for (auto curve : curveCollection->curves())
        {
            RifEclipseSummaryAddress adr = curve->summaryAddressY();
            if (adr.category() == RifEclipseSummaryAddress::SUMMARY_WELL_GROUP)
            {
                adr.setWellGroupName(m_wellGroupName().toStdString());

                curve->setSummaryAddressY(adr);
            }
        }

        triggerLoadDataAndUpdate = true;
    }

    if (triggerLoadDataAndUpdate)
    {
        RimSummaryPlot* summaryPlot = nullptr;
        this->firstAncestorOrThisOfTypeAsserted(summaryPlot);

        summaryPlot->updatePlotTitle();

        summaryPlot->loadDataAndUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifSummaryReaderInterface* RimSummaryPlotSourceStepping::summaryReader() const
{
    RimSummaryCase* sumCase = singleSummaryCase();
    if (sumCase)
    {
        return sumCase->summaryReader();
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCase* RimSummaryPlotSourceStepping::singleSummaryCase() const
{
    RimSummaryCurveCollection* curveCollection = nullptr;
    this->firstAncestorOrThisOfTypeAsserted(curveCollection);

    std::set<RimSummaryCase*> cases;
    for (auto curve : curveCollection->curves())
    {
        cases.insert(curve->summaryCaseY());
    }

    if (cases.size() == 1)
    {
        return *(cases.begin());
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryPlotSourceStepping::wellName() const
{
    return m_wellName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotSourceStepping::setWellName(const QString& wellName)
{
    m_wellName.setValueWithFieldChanged(wellName);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotSourceStepping::updateUiFromCurves()
{
    m_summaryCase.uiCapability()->setUiHidden(true);
    m_wellName.uiCapability()->setUiHidden(true);
    m_wellGroupName.uiCapability()->setUiHidden(true);
    m_region.uiCapability()->setUiHidden(true);
    m_quantity.uiCapability()->setUiHidden(true);

    RimSummaryCurveCollection* curveCollection = nullptr;
    this->firstAncestorOrThisOfTypeAsserted(curveCollection);

    RiaSummaryCurveAnalyzer analyzer;
    analyzer.analyzeCurves(curveCollection);

    if (analyzer.summaryCases().size() == 1)
    {
        m_summaryCase = *(analyzer.summaryCases().begin());
        m_summaryCase.uiCapability()->setUiHidden(false);
    }

    RifEclipseSummaryAddress::SummaryVarCategory category = RifEclipseSummaryAddress::SUMMARY_INVALID;
    {
        if (analyzer.categories().size() == 1)
        {
            category = *(analyzer.categories().begin());
        }
    }

    if (category != RifEclipseSummaryAddress::SUMMARY_INVALID)
    {
        if (analyzer.wellNames().size() == 1)
        {
            QString txt = QString::fromStdString(*(analyzer.wellNames().begin()));
            m_wellName  = txt;
            m_wellName.uiCapability()->setUiHidden(false);
        }

        if (analyzer.wellGroupNames().size() == 1)
        {
            QString txt     = QString::fromStdString(*(analyzer.wellGroupNames().begin()));
            m_wellGroupName = txt;
            m_wellGroupName.uiCapability()->setUiHidden(false);
        }

        if (analyzer.regionNumbers().size() == 1)
        {
            m_region = *(analyzer.regionNumbers().begin());
            m_region.uiCapability()->setUiHidden(false);
        }

        if (analyzer.quantities().size() == 1)
        {
            QString txt = QString::fromStdString(*(analyzer.quantities().begin()));
            m_quantity  = txt;
            m_quantity.uiCapability()->setUiHidden(false);
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimSummaryPlotSourceStepping::fieldToModify()
{
    RimSummaryCurveCollection* curveCollection = nullptr;
    this->firstAncestorOrThisOfTypeAsserted(curveCollection);

    RiaSummaryCurveAnalyzer analyzer;
    analyzer.analyzeCurves(curveCollection);

    if (analyzer.wellNames().size() == 1)
    {
        return &m_wellName;
    }

    if (analyzer.wellGroupNames().size() == 1)
    {
        return &m_wellName;
    }

    if (analyzer.regionNumbers().size() == 1)
    {
        return &m_region;
    }

    if (analyzer.quantities().size() == 1)
    {
        return &m_quantity;
    }

    // A pointer field is no a value field, so this must be improved
    // to be able to step between summary cases
    if (analyzer.summaryCases().size() == 1)
    {
        return &m_summaryCase;
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmValueField* RimSummaryPlotSourceStepping::valueFieldToModify()
{
    // This will return a null pointer for summary case modifier

    return dynamic_cast<caf::PdmValueField*>(fieldToModify());
}
