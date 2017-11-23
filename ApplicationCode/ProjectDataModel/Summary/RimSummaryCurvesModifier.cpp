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

#include "RimSummaryCurvesModifier.h"

#include "RiaSummaryCurveDefTools.h"
#include "RiaSummaryCurveDefinition.h"

#include "RimSummaryCase.h"
#include "RimSummaryCurve.h"
#include "RimSummaryCurveCollection.h"

#include "RifSummaryReaderInterface.h"
#include "RimSummaryPlot.h"

#include "cafPdmUiItem.h"
#include "cafPdmUiListEditor.h"

CAF_PDM_SOURCE_INIT(RimSummaryCurvesModifier, "RimSummaryCurveCollectionModifier");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCurvesModifier::RimSummaryCurvesModifier()
{
    // clang-format off
    CAF_PDM_InitObject("Summary Curves Modifier", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_summaryCase,  "CurveCase",    "Case", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_wellName,     "WellName",     "Well Name", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_groupName,    "GroupName",    "Group Name", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_region,       "Region",       "Region", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_wellNameProxy, "WellNameProxy", "WellNameProxy", "", "", "");
    m_wellNameProxy.registerGetMethod(this, &RimSummaryCurvesModifier::wellName);
    m_wellNameProxy.registerSetMethod(this, &RimSummaryCurvesModifier::setWellName);
    m_wellNameProxy.uiCapability()->setUiEditorTypeName(caf::PdmUiListEditor::uiEditorTypeName());
    m_wellNameProxy.uiCapability()->setUiHidden(false);
    // clang-format on
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<QString> findAvailableIdentifierTexts(const std::vector<RifEclipseSummaryAddress>& allAddresses,
                                               RifEclipseSummaryAddress::SummaryVarCategory category)
{
    std::set<QString> mySet;

    for (const auto& adr : allAddresses)
    {
        if (adr.category() == category)
        {
            if (category == RifEclipseSummaryAddress::SUMMARY_REGION)
            {
                mySet.insert(QString::number(adr.regionNumber()));
            }
            else if (category == RifEclipseSummaryAddress::SUMMARY_WELL)
            {
                mySet.insert(QString::fromStdString(adr.wellName()));
            }
            else if (category == RifEclipseSummaryAddress::SUMMARY_WELL_GROUP)
            {
                mySet.insert(QString::fromStdString(adr.wellGroupName()));
            }
        }
    }

    return mySet;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimSummaryCurvesModifier::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                              bool*                      useOptionsOnly)
{
    std::set<QString> identifierTexts;

    RifSummaryReaderInterface* reader = summaryReader();
    if (reader)
    {
        const std::vector<RifEclipseSummaryAddress> allAddresses = reader->allResultAddresses();

        if (fieldNeedingOptions == &m_wellName || fieldNeedingOptions == &m_wellNameProxy)
        {
            identifierTexts = findAvailableIdentifierTexts(allAddresses, RifEclipseSummaryAddress::SUMMARY_WELL);
        }
        else if (fieldNeedingOptions == &m_region)
        {
            identifierTexts = findAvailableIdentifierTexts(allAddresses, RifEclipseSummaryAddress::SUMMARY_REGION);
        }
        else if (fieldNeedingOptions == &m_groupName)
        {
            identifierTexts = findAvailableIdentifierTexts(allAddresses, RifEclipseSummaryAddress::SUMMARY_WELL_GROUP);
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
RifSummaryReaderInterface* RimSummaryCurvesModifier::summaryReader() const
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
RimSummaryCase* RimSummaryCurvesModifier::singleSummaryCase() const
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
QString RimSummaryCurvesModifier::wellName() const
{
    return m_wellName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurvesModifier::setWellName(const QString& wellName)
{
    m_wellName.setValueWithFieldChanged(wellName);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurvesModifier::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue,
                                                const QVariant& newValue)
{
    RimSummaryCurveCollection* curveCollection = nullptr;
    this->firstAncestorOrThisOfTypeAsserted(curveCollection);

    bool triggerLoadDataAndUpdate = false;
    if (changedField == &m_wellName || changedField == &m_wellNameProxy)
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
    else if (changedField == &m_groupName)
    {
        for (auto curve : curveCollection->curves())
        {
            RifEclipseSummaryAddress adr = curve->summaryAddressY();
            if (adr.category() == RifEclipseSummaryAddress::SUMMARY_WELL_GROUP)
            {
                adr.setWellGroupName(m_groupName().toStdString());

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
void RimSummaryCurvesModifier::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    RimSummaryCurveCollection* curveCollection = nullptr;
    this->firstAncestorOrThisOfTypeAsserted(curveCollection);

    std::vector<RiaSummaryCurveDefinition> curveDefinitions;
    for (auto curve : curveCollection->curves())
    {
        curveDefinitions.push_back(RiaSummaryCurveDefinition(curve->summaryCaseY(), curve->summaryAddressY()));
    }

    //     m_summaryCase.uiCapability()->setUiReadOnly(true);
    //     m_wellName.uiCapability()->setUiReadOnly(true);
    //     m_groupName.uiCapability()->setUiReadOnly(true);
    //     m_region.uiCapability()->setUiReadOnly(true);

    /*
        if (tools.uniqueSummaryCases().size() > 1)
        {
            m_summaryCase.uiCapability()->setUiReadOnly(true);
        }
        else
        {
            m_summaryCase.uiCapability()->setUiReadOnly(false);
        }

        if (tools.uniqueWellNames().size() < 2)
        {
            m_wellName.uiCapability()->setUiReadOnly(true);
        }
        else
        {
            m_wellName.uiCapability()->setUiReadOnly(false);
        }

        if (tools.uniqueGroupNames().size() < 2)
        {
            m_groupName.uiCapability()->setUiReadOnly(true);
        }
        else
        {
            m_groupName.uiCapability()->setUiReadOnly(false);
        }

        if (tools.uniqueRegions().size() < 2)
        {
            m_region.uiCapability()->setUiReadOnly(true);
        }
        else
        {
            m_region.uiCapability()->setUiReadOnly(false);
        }
    */
}
