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

#include "RimDataSourceSteppingTools.h"
#include "RimEnsembleCurveSet.h"
#include "RimEnsembleCurveSetCollection.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryCrossPlot.h"
#include "RimSummaryCurve.h"
#include "RimSummaryCurveCollection.h"
#include "RimSummaryPlot.h"

#include "RiuPlotMainWindow.h"

#include "RiaStdStringTools.h"
#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiItem.h"
#include "cafPdmUiListEditor.h"

CAF_PDM_SOURCE_INIT(RimSummaryPlotSourceStepping, "RimSummaryCurveCollectionModifier");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlotSourceStepping::RimSummaryPlotSourceStepping()
    : m_sourceSteppingType(Y_AXIS)
{
    // clang-format off
    CAF_PDM_InitObject("Summary Curves Modifier", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_summaryCase,      "CurveCase",    "Case", "", "", "");

    CAF_PDM_InitField(&m_includeEnsembleCasesForCaseStepping,
                      "IncludeEnsembleCasesForCaseStepping",
                      false,
                      "Allow Stepping on Ensemble cases",
                      "",
                      "",
                      "");

    CAF_PDM_InitFieldNoDefault(&m_wellName,         "WellName",     "Well Name", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_wellGroupName,    "GroupName",    "Group Name", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_region,           "Region",       "Region", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_quantity,         "Quantities",   "Quantity", "", "", "");
    
    CAF_PDM_InitFieldNoDefault(&m_ensemble,         "Ensemble",     "Ensemble", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_placeholderForLabel, "Placeholder",   "", "", "", "");
    m_placeholderForLabel = "No common identifiers detected";
    m_placeholderForLabel.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::TOP);
    m_placeholderForLabel.uiCapability()->setUiReadOnly(true);

    // clang-format on
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotSourceStepping::setSourceSteppingType(SourceSteppingType sourceSteppingType)
{
    m_sourceSteppingType = sourceSteppingType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotSourceStepping::applyNextCase()
{
    modifyCurrentIndex(&m_summaryCase, 1);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotSourceStepping::applyPrevCase()
{
    modifyCurrentIndex(&m_summaryCase, -1);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotSourceStepping::applyNextQuantity()
{
    if (!m_quantity.uiCapability()->isUiHidden())
    {
        modifyCurrentIndex(&m_quantity, 1);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotSourceStepping::applyPrevQuantity()
{
    if (!m_quantity.uiCapability()->isUiHidden())
    {
        modifyCurrentIndex(&m_quantity, -1);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotSourceStepping::applyNextOtherIdentifier()
{
    caf::PdmValueField* valueField = fieldToModify();
    if (!valueField) return;

    modifyCurrentIndex(valueField, 1);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotSourceStepping::applyPrevOtherIdentifier()
{
    caf::PdmValueField* valueField = fieldToModify();
    if (!valueField) return;

    modifyCurrentIndex(valueField, -1);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<caf::PdmFieldHandle*> RimSummaryPlotSourceStepping::fieldsToShowInToolbar()
{
    return computeVisibleFieldsAndSetFieldVisibility();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotSourceStepping::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    auto visible = computeVisibleFieldsAndSetFieldVisibility();
    if (visible.size() == 0)
    {
        m_placeholderForLabel.uiCapability()->setUiHidden(false);
    }
    else
    {
        m_placeholderForLabel.uiCapability()->setUiHidden(true);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimSummaryPlotSourceStepping::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                                  bool*                      useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    if (fieldNeedingOptions == &m_includeEnsembleCasesForCaseStepping)
    {
        return caf::PdmObject::calculateValueOptions(fieldNeedingOptions, useOptionsOnly);
    }
    else if (fieldNeedingOptions == &m_placeholderForLabel)
    {
        options;
    }
    else if (fieldNeedingOptions == &m_summaryCase)
    {
        auto summaryCases = RimSummaryPlotSourceStepping::summaryCasesForSourceStepping();
        for (auto sumCase : summaryCases)
        {
            options.append(caf::PdmOptionItemInfo(sumCase->caseName(), sumCase));
        }

        return options;
    }
    else if (fieldNeedingOptions == &m_ensemble)
    {
        RimProject* proj = RiaApplication::instance()->project();
        for (auto ensemble : proj->summaryGroups())
        {
            if (ensemble->isEnsemble())
            {
                options.append(caf::PdmOptionItemInfo(ensemble->name(), ensemble));
            }
        }

        return options;
    }

    std::vector<RifSummaryReaderInterface*> readers = summaryReadersForCurves();
    if (!readers.empty())
    {
        if (fieldNeedingOptions == &m_quantity)
        {
            RifEclipseSummaryAddress::SummaryVarCategory category = RifEclipseSummaryAddress::SUMMARY_FIELD;

            auto addresses = addressesCurveCollection();
            if (!addresses.empty())
            {
                category = addresses.begin()->category();
            }

            std::map<QString, QString> displayAndValueStrings;

            {
                RiaSummaryCurveAnalyzer quantityAnalyzer;

                for (auto reader : readers)
                {
                    if (reader != nullptr)
                    {
                        auto subset = RiaSummaryCurveAnalyzer::addressesForCategory(reader->allResultAddresses(), category);
                        quantityAnalyzer.appendAdresses(subset);
                    }
                }

                RiaSummaryCurveAnalyzer analyzerForCurves;
                analyzerForCurves.appendAdresses(addressesCurveCollection());

                if (analyzerForCurves.quantityNamesWithHistory().empty())
                {
                    auto quantities = quantityAnalyzer.quantities();
                    for (const auto& s : quantities)
                    {
                        QString valueString = QString::fromStdString(s);

                        displayAndValueStrings[valueString] = valueString;
                    }
                }
                else
                {
                    auto quantitiesWithHistory = quantityAnalyzer.quantityNamesWithHistory();
                    for (const auto& s : quantitiesWithHistory)
                    {
                        QString valueString   = QString::fromStdString(s);
                        QString displayString = valueString + " (H)";

                        displayAndValueStrings[displayString] = valueString;
                    }

                    auto quantitiesNoHistory = quantityAnalyzer.quantityNamesNoHistory();
                    for (const auto& s : quantitiesNoHistory)
                    {
                        QString valueString = QString::fromStdString(s);

                        displayAndValueStrings[valueString] = valueString;
                    }
                }
            }

            for (const auto& displayAndValue : displayAndValueStrings)
            {
                options.append(caf::PdmOptionItemInfo(displayAndValue.first, displayAndValue.second));
            }

            if (options.isEmpty())
            {
                options.push_back(caf::PdmOptionItemInfo("None", "None"));
            }
        }
        else
        {
            RifEclipseSummaryAddress::SummaryVarCategory category = RifEclipseSummaryAddress::SUMMARY_INVALID;

            if (fieldNeedingOptions == &m_wellName)
            {
                category = RifEclipseSummaryAddress::SUMMARY_WELL;
            }
            else if (fieldNeedingOptions == &m_region)
            {
                category = RifEclipseSummaryAddress::SUMMARY_REGION;
            }
            else if (fieldNeedingOptions == &m_wellGroupName)
            {
                category = RifEclipseSummaryAddress::SUMMARY_WELL_GROUP;
            }

            std::set<QString> identifierTexts;

            if (category != RifEclipseSummaryAddress::SUMMARY_INVALID)
            {
                for (auto reader : readers)
                {
                    auto analyzer = analyzerForReader(reader);

                    if (analyzer)
                    {
                        for (const auto& t : analyzer->identifierTexts(category))
                        {
                            identifierTexts.insert(t);
                        }
                    }
                }
            }

            if (!identifierTexts.empty())
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
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotSourceStepping::fieldChangedByUi(const caf::PdmFieldHandle* changedField,
                                                    const QVariant&            oldValue,
                                                    const QVariant&            newValue)
{
    std::vector<RimSummaryCurve*> curves;

    RimSummaryCurveCollection* curveCollection = nullptr;
    this->firstAncestorOrThisOfType(curveCollection);
    if (curveCollection)
    {
        curves = curveCollection->curves();
    }

    RimEnsembleCurveSetCollection* ensembleCurveColl = nullptr;
    this->firstAncestorOrThisOfType(ensembleCurveColl);

    if (changedField == &m_includeEnsembleCasesForCaseStepping)
    {
        if (curveCollection)
        {
            curveCollection->updateConnectedEditors();
        }

        if (ensembleCurveColl)
        {
            ensembleCurveColl->updateConnectedEditors();
        }

        RiuPlotMainWindow* mainPlotWindow = RiaApplication::instance()->getOrCreateMainPlotWindow();
        bool forceUpdateOfFieldsInToolbar = true;
        mainPlotWindow->updateSummaryPlotToolBar(forceUpdateOfFieldsInToolbar);

        return;
    }

    bool triggerLoadDataAndUpdate = false;

    if (changedField == &m_summaryCase)
    {
        if (m_summaryCase())
        {
            caf::PdmPointer<caf::PdmObjectHandle> variantHandle = oldValue.value<caf::PdmPointer<caf::PdmObjectHandle>>();
            RimSummaryCase*                       previousCase  = dynamic_cast<RimSummaryCase*>(variantHandle.p());

            for (auto curve : curves)
            {
                if (isYAxisStepping())
                {
                    if (previousCase == curve->summaryCaseY())
                    {
                        bool doSetAppearance = curve->summaryCaseY()->isObservedData() != m_summaryCase->isObservedData();
                        curve->setSummaryCaseY(m_summaryCase);
                        if (doSetAppearance) curve->forceUpdateCurveAppearanceFromCaseType();
                    }
                }

                if (isXAxisStepping())
                {
                    if (previousCase == curve->summaryCaseX())
                    {
                        curve->setSummaryCaseX(m_summaryCase);
                    }
                }
            }

            triggerLoadDataAndUpdate = true;
        }

        m_wellName.uiCapability()->updateConnectedEditors();
        m_wellGroupName.uiCapability()->updateConnectedEditors();
        m_region.uiCapability()->updateConnectedEditors();
        m_quantity.uiCapability()->updateConnectedEditors();
    }
    else if (changedField == &m_ensemble)
    {
        if (m_ensemble() && ensembleCurveColl)
        {
            caf::PdmPointer<caf::PdmObjectHandle> variantHandle      = oldValue.value<caf::PdmPointer<caf::PdmObjectHandle>>();
            RimSummaryCaseCollection*             previousCollection = dynamic_cast<RimSummaryCaseCollection*>(variantHandle.p());

            for (auto curveSet : ensembleCurveColl->curveSets())
            {
                if (curveSet->summaryCaseCollection() == previousCollection)
                {
                    curveSet->setSummaryCaseCollection(m_ensemble);
                }
            }

            triggerLoadDataAndUpdate = true;
        }

        m_wellName.uiCapability()->updateConnectedEditors();
        m_wellGroupName.uiCapability()->updateConnectedEditors();
        m_region.uiCapability()->updateConnectedEditors();
        m_quantity.uiCapability()->updateConnectedEditors();
    }
    else if (changedField == &m_wellName)
    {
        for (auto curve : curves)
        {
            if (isYAxisStepping())
            {
                RifEclipseSummaryAddress adr = curve->summaryAddressY();
                updateAddressIfMatching(oldValue, newValue, RifEclipseSummaryAddress::SUMMARY_WELL, &adr);
                curve->setSummaryAddressY(adr);
            }

            if (isXAxisStepping())
            {
                RifEclipseSummaryAddress adr = curve->summaryAddressX();
                updateAddressIfMatching(oldValue, newValue, RifEclipseSummaryAddress::SUMMARY_WELL, &adr);
                curve->setSummaryAddressX(adr);
            }
        }

        if (ensembleCurveColl)
        {
            for (auto curveSet : ensembleCurveColl->curveSets())
            {
                auto adr = curveSet->summaryAddress();
                updateAddressIfMatching(oldValue, newValue, RifEclipseSummaryAddress::SUMMARY_WELL, &adr);
                curveSet->setSummaryAddress(adr);
            }
        }

        triggerLoadDataAndUpdate = true;
    }
    else if (changedField == &m_region)
    {
        for (auto curve : curves)
        {
            if (isYAxisStepping())
            {
                RifEclipseSummaryAddress adr = curve->summaryAddressY();
                updateAddressIfMatching(oldValue, newValue, RifEclipseSummaryAddress::SUMMARY_REGION, &adr);
                curve->setSummaryAddressY(adr);
            }

            if (isXAxisStepping())
            {
                RifEclipseSummaryAddress adr = curve->summaryAddressX();
                updateAddressIfMatching(oldValue, newValue, RifEclipseSummaryAddress::SUMMARY_REGION, &adr);
                curve->setSummaryAddressX(adr);
            }
        }

        if (ensembleCurveColl)
        {
            for (auto curveSet : ensembleCurveColl->curveSets())
            {
                auto adr = curveSet->summaryAddress();
                updateAddressIfMatching(oldValue, newValue, RifEclipseSummaryAddress::SUMMARY_REGION, &adr);
                curveSet->setSummaryAddress(adr);
            }
        }

        triggerLoadDataAndUpdate = true;
    }
    else if (changedField == &m_quantity)
    {
        for (auto curve : curves)
        {
            if (isYAxisStepping())
            {
                auto adr = curve->summaryAddressY();
                updateHistoryAndSummaryQuantityIfMatching(oldValue, newValue, &adr);
                curve->setSummaryAddressY(adr);
            }

            if (isXAxisStepping())
            {
                auto adr = curve->summaryAddressX();
                updateHistoryAndSummaryQuantityIfMatching(oldValue, newValue, &adr);
                curve->setSummaryAddressX(adr);
            }
        }

        if (ensembleCurveColl)
        {
            for (auto curveSet : ensembleCurveColl->curveSets())
            {
                auto adr = curveSet->summaryAddress();
                updateHistoryAndSummaryQuantityIfMatching(oldValue, newValue, &adr);
                curveSet->setSummaryAddress(adr);
            }
        }

        triggerLoadDataAndUpdate = true;
    }
    else if (changedField == &m_wellGroupName)
    {
        for (auto curve : curves)
        {
            if (isYAxisStepping())
            {
                RifEclipseSummaryAddress adr = curve->summaryAddressY();
                updateAddressIfMatching(oldValue, newValue, RifEclipseSummaryAddress::SUMMARY_WELL_GROUP, &adr);
                curve->setSummaryAddressY(adr);
            }

            if (isXAxisStepping())
            {
                RifEclipseSummaryAddress adr = curve->summaryAddressX();
                updateAddressIfMatching(oldValue, newValue, RifEclipseSummaryAddress::SUMMARY_WELL_GROUP, &adr);
                curve->setSummaryAddressX(adr);
            }
        }

        if (ensembleCurveColl)
        {
            for (auto curveSet : ensembleCurveColl->curveSets())
            {
                auto adr = curveSet->summaryAddress();
                updateAddressIfMatching(oldValue, newValue, RifEclipseSummaryAddress::SUMMARY_WELL_GROUP, &adr);
                curveSet->setSummaryAddress(adr);
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

        if (ensembleCurveColl)
        {
            ensembleCurveColl->updateConnectedEditors();
        }

        RimSummaryCrossPlot* summaryCrossPlot = dynamic_cast<RimSummaryCrossPlot*>(summaryPlot);
        if (summaryCrossPlot)
        {
            // Trigger update of curve collection (and summary toolbar in main window), as the visibility of combo boxes might
            // have been changed due to the updates in this function
            if (curveCollection)
            {
                curveCollection->updateConnectedEditors();
            }

            RiuPlotMainWindow* mainPlotWindow = RiaApplication::instance()->mainPlotWindow();
            mainPlotWindow->updateSummaryPlotToolBar();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RifSummaryReaderInterface*> RimSummaryPlotSourceStepping::summaryReadersForCurves() const
{
    std::vector<RifSummaryReaderInterface*> readers;
    RimSummaryCurveCollection*              curveCollection = nullptr;
    this->firstAncestorOrThisOfType(curveCollection);

    if (curveCollection)
    {
        for (auto curve : curveCollection->curves())
        {
            if (isYAxisStepping() && curve->summaryCaseY())
            {
                readers.push_back(curve->summaryCaseY()->summaryReader());
            }

            if (isXAxisStepping() && curve->summaryCaseX())
            {
                readers.push_back(curve->summaryCaseX()->summaryReader());
            }
        }
    }

    RimEnsembleCurveSetCollection* ensembleCollection = nullptr;
    this->firstAncestorOrThisOfType(ensembleCollection);
    if (ensembleCollection)
    {
        auto curveSets = ensembleCollection->curveSets();
        for (const RimEnsembleCurveSet* curveSet : curveSets)
        {
            for (auto curve : curveSet->curves())
            {
                if (isYAxisStepping() && curve->summaryCaseY())
                {
                    readers.push_back(curve->summaryCaseY()->summaryReader());
                }
            }
        }
    }

    return readers;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmValueField* RimSummaryPlotSourceStepping::fieldToModify()
{
    RiaSummaryCurveAnalyzer analyzer;
    analyzer.appendAdresses(addressesCurveCollection());

    if (analyzer.wellNames().size() == 1)
    {
        return &m_wellName;
    }

    if (analyzer.wellGroupNames().size() == 1)
    {
        return &m_wellGroupName;
    }

    if (analyzer.regionNumbers().size() == 1)
    {
        return &m_region;
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseSummaryAddress> RimSummaryPlotSourceStepping::addressesCurveCollection() const
{
    std::set<RifEclipseSummaryAddress> addresses;

    RimSummaryCurveCollection* curveCollection = nullptr;
    this->firstAncestorOrThisOfType(curveCollection);

    if (curveCollection)
    {
        auto curves = curveCollection->curvesForSourceStepping(m_sourceSteppingType);
        for (auto c : curves)
        {
            if (isYAxisStepping())
            {
                addresses.insert(c->summaryAddressY());
            }

            if (isXAxisStepping())
            {
                addresses.insert(c->summaryAddressX());
            }
        }
    }

    RimEnsembleCurveSetCollection* ensembleCollection = nullptr;
    this->firstAncestorOrThisOfType(ensembleCollection);
    if (ensembleCollection)
    {
        auto curveSets = ensembleCollection->curveSetsForSourceStepping();
        for (const RimEnsembleCurveSet* curveSet : curveSets)
        {
            addresses.insert(curveSet->summaryAddress());
        }
    }

    return addresses;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RimSummaryCase*> RimSummaryPlotSourceStepping::summaryCasesCurveCollection() const
{
    std::set<RimSummaryCase*> sumCases;

    RimSummaryCurveCollection* curveCollection = nullptr;
    this->firstAncestorOrThisOfType(curveCollection);

    if (!curveCollection) return sumCases;

    auto curves = curveCollection->curvesForSourceStepping(m_sourceSteppingType);
    for (auto c : curves)
    {
        if (isYAxisStepping())
        {
            sumCases.insert(c->summaryCaseY());
        }

        if (isXAxisStepping())
        {
            sumCases.insert(c->summaryCaseX());
        }
    }

    return sumCases;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<caf::PdmFieldHandle*> RimSummaryPlotSourceStepping::computeVisibleFieldsAndSetFieldVisibility()
{
    m_summaryCase.uiCapability()->setUiHidden(true);
    m_includeEnsembleCasesForCaseStepping.uiCapability()->setUiHidden(true);
    m_wellName.uiCapability()->setUiHidden(true);
    m_wellGroupName.uiCapability()->setUiHidden(true);
    m_region.uiCapability()->setUiHidden(true);
    m_quantity.uiCapability()->setUiHidden(true);
    m_ensemble.uiCapability()->setUiHidden(true);

    std::vector<caf::PdmFieldHandle*> fields;

    auto sumCases = summaryCasesCurveCollection();
    if (sumCases.size() == 1)
    {
        RimProject* proj = RiaApplication::instance()->project();
        if (proj->allSummaryCases().size() > 1)
        {
            m_summaryCase = *(sumCases.begin());

            m_summaryCase.uiCapability()->setUiHidden(false);

            fields.push_back(&m_summaryCase);

            m_includeEnsembleCasesForCaseStepping.uiCapability()->setUiHidden(false);
        }
    }

    auto ensembleColl = ensembleCollection();
    if (ensembleColl.size() == 1)
    {
        RimProject* proj = RiaApplication::instance()->project();

        if (proj->summaryGroups().size() > 1)
        {
            m_ensemble = *(ensembleColl.begin());

            m_ensemble.uiCapability()->setUiHidden(false);

            fields.push_back(&m_ensemble);
        }
    }

    std::vector<caf::PdmFieldHandle*> fieldsCommonForAllCurves;

    {
        RiaSummaryCurveAnalyzer analyzer;
        analyzer.appendAdresses(addressesCurveCollection());

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

                fieldsCommonForAllCurves.push_back(&m_wellName);
            }

            if (analyzer.wellGroupNames().size() == 1)
            {
                QString txt     = QString::fromStdString(*(analyzer.wellGroupNames().begin()));
                m_wellGroupName = txt;
                m_wellGroupName.uiCapability()->setUiHidden(false);

                fieldsCommonForAllCurves.push_back(&m_wellGroupName);
            }

            if (analyzer.regionNumbers().size() == 1)
            {
                m_region = *(analyzer.regionNumbers().begin());
                m_region.uiCapability()->setUiHidden(false);

                fieldsCommonForAllCurves.push_back(&m_region);
            }

            if (!analyzer.quantityNameForTitle().empty())
            {
                QString txt = QString::fromStdString(analyzer.quantityNameForTitle());
                m_quantity  = txt;
                m_quantity.uiCapability()->setUiHidden(false);

                fieldsCommonForAllCurves.push_back(&m_quantity);
            }
        }
    }

    for (const auto& f : fieldsCommonForAllCurves)
    {
        fields.push_back(f);
    }

    return fields;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RimSummaryCaseCollection*> RimSummaryPlotSourceStepping::ensembleCollection() const
{
    std::set<RimSummaryCaseCollection*> sumCases;

    RimEnsembleCurveSetCollection* curveCollection = nullptr;
    this->firstAncestorOrThisOfType(curveCollection);

    if (!curveCollection) return sumCases;

    auto curves = curveCollection->curveSets();
    for (auto c : curves)
    {
        sumCases.insert(c->summaryCaseCollection());
    }

    return sumCases;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryPlotSourceStepping::isXAxisStepping() const
{
    if (m_sourceSteppingType == UNION_X_Y_AXIS) return true;

    if (m_sourceSteppingType == X_AXIS) return true;

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryPlotSourceStepping::isYAxisStepping() const
{
    if (m_sourceSteppingType == UNION_X_Y_AXIS) return true;

    if (m_sourceSteppingType == Y_AXIS) return true;

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaSummaryCurveAnalyzer* RimSummaryPlotSourceStepping::analyzerForReader(RifSummaryReaderInterface* reader)
{
    if (!reader) return nullptr;

    if (m_curveAnalyzerForReader.first != reader)
    {
        RiaSummaryCurveAnalyzer analyzer;
        m_curveAnalyzerForReader = std::make_pair(reader, analyzer);
    }

    m_curveAnalyzerForReader.second.appendAdresses(reader->allResultAddresses());

    return &m_curveAnalyzerForReader.second;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotSourceStepping::modifyCurrentIndex(caf::PdmValueField* valueField, int indexOffset)
{
    bool                          useOptionsOnly;
    QList<caf::PdmOptionItemInfo> options = calculateValueOptions(valueField, &useOptionsOnly);
    RimDataSourceSteppingTools::modifyCurrentIndex(valueField, options, indexOffset);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryPlotSourceStepping::updateAddressIfMatching(const QVariant&                              oldValue,
                                                           const QVariant&                              newValue,
                                                           RifEclipseSummaryAddress::SummaryVarCategory category,
                                                           RifEclipseSummaryAddress*                    adr)
{
    if (!adr) return false;

    if (category == RifEclipseSummaryAddress::SUMMARY_REGION)
    {
        int oldInt = oldValue.toInt();
        int newInt = newValue.toInt();

        if (adr->regionNumber() == oldInt)
        {
            adr->setRegion(newInt);

            return true;
        }
    }
    else if (category == RifEclipseSummaryAddress::SUMMARY_WELL_GROUP)
    {
        std::string oldString = oldValue.toString().toStdString();
        std::string newString = newValue.toString().toStdString();

        if (adr->wellGroupName() == oldString)
        {
            adr->setWellGroupName(newString);

            return true;
        }
    }
    else if (RifEclipseSummaryAddress::isDependentOnWellName(category))
    {
        std::string oldString = oldValue.toString().toStdString();
        std::string newString = newValue.toString().toStdString();

        if (adr->wellName() == oldString)
        {
            adr->setWellName(newString);

            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryPlotSourceStepping::updateHistoryAndSummaryQuantityIfMatching(const QVariant&           oldValue,
                                                                             const QVariant&           newValue,
                                                                             RifEclipseSummaryAddress* adr)
{
    if (!adr) return false;

    std::string oldString = oldValue.toString().toStdString();
    std::string newString = newValue.toString().toStdString();

    if (adr->quantityName() == oldString)
    {
        adr->setQuantityName(newString);

        return true;
    }

    std::string correspondingOldString = RiaSummaryCurveAnalyzer::correspondingHistorySummaryCurveName(oldString);
    std::string correspondingNewString = RiaSummaryCurveAnalyzer::correspondingHistorySummaryCurveName(newString);

    if (adr->quantityName() == correspondingOldString)
    {
        adr->setQuantityName(correspondingNewString);

        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCase*> RimSummaryPlotSourceStepping::summaryCasesForSourceStepping()
{
    std::vector<RimSummaryCase*> cases;

    RimProject* proj = RiaApplication::instance()->project();
    for (auto sumCase : proj->allSummaryCases())
    {
        if (sumCase->isObservedData()) continue;

        RimSummaryCaseCollection* sumCaseColl = nullptr;
        sumCase->firstAncestorOrThisOfType(sumCaseColl);

        if (sumCaseColl && sumCaseColl->isEnsemble())
        {
            if (m_includeEnsembleCasesForCaseStepping())
            {
                cases.push_back(sumCase);
            }
        }
        else
        {
            cases.push_back(sumCase);
        }
    }

    return cases;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotSourceStepping::defineEditorAttribute(const caf::PdmFieldHandle* field,
                                                         QString                    uiConfigName,
                                                         caf::PdmUiEditorAttribute* attribute)
{
    caf::PdmUiComboBoxEditorAttribute* myAttr = dynamic_cast<caf::PdmUiComboBoxEditorAttribute*>(attribute);
    if (myAttr)
    {
        myAttr->showPreviousAndNextButtons = true;

        QString modifierText;

        if (field == &m_summaryCase)
        {
            modifierText = ("(Shift+");
        }
        else if (field == &m_wellName || field == &m_wellGroupName || field == &m_region)
        {
            modifierText = ("(Ctrl+");
        }
        else if (field == &m_quantity)
        {
            modifierText = ("(");
        }

        if (!modifierText.isEmpty())
        {
            myAttr->nextButtonText = "Next " + modifierText + "PgDown)";
            myAttr->prevButtonText = "Previous " + modifierText + "PgUp)";
        }
    }
}
