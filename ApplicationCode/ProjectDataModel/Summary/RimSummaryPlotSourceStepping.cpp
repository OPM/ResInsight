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

#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryCrossPlot.h"
#include "RimSummaryCurve.h"
#include "RimSummaryCurveCollection.h"
#include "RimSummaryPlot.h"

#include "RiuMainPlotWindow.h"

#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiItem.h"
#include "cafPdmUiListEditor.h"

CAF_PDM_SOURCE_INIT(RimSummaryPlotSourceStepping, "RimSummaryCurveCollectionModifier");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlotSourceStepping::RimSummaryPlotSourceStepping() : m_sourceSteppingType(Y_AXIS)
{
    // clang-format off
    CAF_PDM_InitObject("Summary Curves Modifier", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_summaryCase,      "CurveCase",    "Case", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_wellName,         "WellName",     "Well Name", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_wellGroupName,    "GroupName",    "Group Name", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_region,           "Region",       "Region", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_quantity,         "Quantities",   "Quantity", "", "", "");
    
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
    RimProject* proj = RiaApplication::instance()->project();

    auto summaryCases = proj->allSummaryCases();
    if (summaryCases.size() < 1) return;

    auto currentCase = std::find(summaryCases.begin(), summaryCases.end(), m_summaryCase());

    if (currentCase != summaryCases.end())
    {
        currentCase++;
        if (currentCase != summaryCases.end())
        {
            m_summaryCase = *currentCase;
        }
    }
    else
    {
        m_summaryCase = summaryCases[0];
    }

    fieldChangedByUi(&m_summaryCase, QVariant(), QVariant());
    m_summaryCase.uiCapability()->updateConnectedEditors();

    RimSummaryCurveCollection* curveCollection = nullptr;
    this->firstAncestorOrThisOfTypeAsserted(curveCollection);

    curveCollection->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotSourceStepping::applyPrevCase()
{
    RimProject* proj = RiaApplication::instance()->project();

    auto summaryCases = proj->allSummaryCases();
    if (summaryCases.size() < 1) return;

    auto currentCase = std::find(summaryCases.begin(), summaryCases.end(), m_summaryCase());

    if (currentCase != summaryCases.end() && currentCase != summaryCases.begin())
    {
        currentCase--;
        m_summaryCase = *currentCase;
    }
    else
    {
        m_summaryCase = summaryCases[0];
    }

    fieldChangedByUi(&m_summaryCase, QVariant(), QVariant());
    m_summaryCase.uiCapability()->updateConnectedEditors();

    RimSummaryCurveCollection* curveCollection = nullptr;
    this->firstAncestorOrThisOfTypeAsserted(curveCollection);

    curveCollection->updateConnectedEditors();
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
    if (fieldNeedingOptions == &m_placeholderForLabel)
    {
        return QList<caf::PdmOptionItemInfo>();
    }

    if (fieldNeedingOptions == &m_summaryCase)
    {
        QList<caf::PdmOptionItemInfo> options;

        RimProject* proj = RiaApplication::instance()->project();
        for (auto sumCase : proj->allSummaryCases())
        {
            options.append(caf::PdmOptionItemInfo(sumCase->caseName(), sumCase));
        }

        return options;
    }

    std::vector<QString> identifierTexts;

    RifSummaryReaderInterface* reader = firstSummaryReaderForVisibleCurves();
    if (reader)
    {
        RiaSummaryCurveAnalyzer* analyzer = analyzerForReader(reader);

        if (fieldNeedingOptions == &m_wellName)
        {
            identifierTexts = analyzer->identifierTexts(RifEclipseSummaryAddress::SUMMARY_WELL);
        }
        else if (fieldNeedingOptions == &m_region)
        {
            identifierTexts = analyzer->identifierTexts(RifEclipseSummaryAddress::SUMMARY_REGION);
        }
        else if (fieldNeedingOptions == &m_wellGroupName)
        {
            identifierTexts = analyzer->identifierTexts(RifEclipseSummaryAddress::SUMMARY_WELL_GROUP);
        }
        else if (fieldNeedingOptions == &m_quantity)
        {
            RimSummaryCurveCollection* curveCollection = nullptr;
            this->firstAncestorOrThisOfTypeAsserted(curveCollection);

            RifEclipseSummaryAddress::SummaryVarCategory category = RifEclipseSummaryAddress::SUMMARY_FIELD;

            if (curveCollection->visibleCurves().size() > 0)
            {
                category = curveCollection->visibleCurves()[0]->summaryAddressY().category();
            }

            RiaSummaryCurveAnalyzer quantityAnalyzer;

            auto subset = RiaSummaryCurveAnalyzer::addressesForCategory(reader->allResultAddresses(), category);

            quantityAnalyzer.appendAdresses(subset);
            for (const auto& quantity : quantityAnalyzer.quantities())
            {
                identifierTexts.push_back(QString::fromStdString(quantity));
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
            for (auto curve : curveCollection->visibleCurves())
            {
                if (isYAxisStepping())
                {
                    curve->setSummaryCaseY(m_summaryCase);
                }

                if (isXAxisStepping())
                {
                    curve->setSummaryCaseX(m_summaryCase);
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
        for (auto curve : curveCollection->visibleCurves())
        {
            if (isYAxisStepping())
            {
                RifEclipseSummaryAddress adr = curve->summaryAddressY();
                if (adr.category() == RifEclipseSummaryAddress::SUMMARY_WELL)
                {
                    adr.setWellName(m_wellName().toStdString());

                    curve->setSummaryAddressY(adr);
                }
            }

            if (isXAxisStepping())
            {
                RifEclipseSummaryAddress adr = curve->summaryAddressX();
                if (adr.category() == RifEclipseSummaryAddress::SUMMARY_WELL)
                {
                    adr.setWellName(m_wellName().toStdString());

                    curve->setSummaryAddressX(adr);
                }
            }
        }

        triggerLoadDataAndUpdate = true;
    }
    else if (changedField == &m_region)
    {
        for (auto curve : curveCollection->visibleCurves())
        {
            if (isYAxisStepping())
            {
                RifEclipseSummaryAddress adr = curve->summaryAddressY();
                if (adr.category() == RifEclipseSummaryAddress::SUMMARY_REGION)
                {
                    adr.setRegion(m_region());

                    curve->setSummaryAddressY(adr);
                }
            }

            if (isXAxisStepping())
            {
                RifEclipseSummaryAddress adr = curve->summaryAddressX();
                if (adr.category() == RifEclipseSummaryAddress::SUMMARY_REGION)
                {
                    adr.setRegion(m_region());

                    curve->setSummaryAddressX(adr);
                }
            }
        }

        triggerLoadDataAndUpdate = true;
    }
    else if (changedField == &m_quantity)
    {
        for (auto curve : curveCollection->visibleCurves())
        {
            if (isYAxisStepping())
            {
                RifEclipseSummaryAddress adr = curve->summaryAddressY();
                adr.setQuantityName(m_quantity().toStdString());

                curve->setSummaryAddressY(adr);
            }

            if (isXAxisStepping())
            {
                RifEclipseSummaryAddress adr = curve->summaryAddressX();
                adr.setQuantityName(m_quantity().toStdString());

                curve->setSummaryAddressX(adr);
            }
        }

        triggerLoadDataAndUpdate = true;
    }
    else if (changedField == &m_wellGroupName)
    {
        for (auto curve : curveCollection->visibleCurves())
        {
            if (isYAxisStepping())
            {
                RifEclipseSummaryAddress adr = curve->summaryAddressY();
                if (adr.category() == RifEclipseSummaryAddress::SUMMARY_WELL_GROUP)
                {
                    adr.setWellGroupName(m_wellGroupName().toStdString());

                    curve->setSummaryAddressY(adr);
                }
            }

            if (isXAxisStepping())
            {
                RifEclipseSummaryAddress adr = curve->summaryAddressX();
                if (adr.category() == RifEclipseSummaryAddress::SUMMARY_WELL_GROUP)
                {
                    adr.setWellGroupName(m_wellGroupName().toStdString());

                    curve->setSummaryAddressX(adr);
                }
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

        RimSummaryCrossPlot* summaryCrossPlot = dynamic_cast<RimSummaryCrossPlot*>(summaryPlot);
        if (summaryCrossPlot)
        {
            // Trigger update of curve collection (and summary toolbar in main window), as the visibility of combo boxes might
            // have been changed due to the updates in this function
            curveCollection->updateConnectedEditors();

            RiuMainPlotWindow* mainPlotWindow = RiaApplication::instance()->mainPlotWindow();
            mainPlotWindow->updateSummaryPlotToolBar();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifSummaryReaderInterface* RimSummaryPlotSourceStepping::firstSummaryReaderForVisibleCurves() const
{
    RimSummaryCase* sumCase = firstSummaryCaseForVisibleCurves();
    if (sumCase)
    {
        return sumCase->summaryReader();
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCase* RimSummaryPlotSourceStepping::firstSummaryCaseForVisibleCurves() const
{
    RimSummaryCurveCollection* curveCollection = nullptr;
    this->firstAncestorOrThisOfTypeAsserted(curveCollection);

    for (auto curve : curveCollection->visibleCurves())
    {
        if (isYAxisStepping() && curve->summaryCaseY())
        {
            return curve->summaryCaseY();
        }

        if (isXAxisStepping() && curve->summaryCaseX())
        {
            return curve->summaryCaseX();
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmValueField* RimSummaryPlotSourceStepping::fieldToModify()
{
    RiaSummaryCurveAnalyzer analyzer;
    analyzer.appendAdresses(visibleAddressesCurveCollection());

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
std::set<RifEclipseSummaryAddress> RimSummaryPlotSourceStepping::visibleAddressesCurveCollection() const
{
    std::set<RifEclipseSummaryAddress> addresses;

    RimSummaryCurveCollection* curveCollection = nullptr;
    this->firstAncestorOrThisOfType(curveCollection);

    if (!curveCollection) return addresses;

    auto curves = curveCollection->visibleCurves();
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

    return addresses;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RimSummaryCase*> RimSummaryPlotSourceStepping::visibleSummaryCasesCurveCollection() const
{
    std::set<RimSummaryCase*> sumCases;

    RimSummaryCurveCollection* curveCollection = nullptr;
    this->firstAncestorOrThisOfType(curveCollection);

    if (!curveCollection) return sumCases;

    auto curves = curveCollection->visibleCurves();
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
    m_wellName.uiCapability()->setUiHidden(true);
    m_wellGroupName.uiCapability()->setUiHidden(true);
    m_region.uiCapability()->setUiHidden(true);
    m_quantity.uiCapability()->setUiHidden(true);

    std::vector<caf::PdmFieldHandle*> fields;

    auto sumCases = visibleSummaryCasesCurveCollection();
    if (sumCases.size() == 1)
    {
        RimProject* proj = RiaApplication::instance()->project();
        if (proj->allSummaryCases().size() > 1)
        {
            m_summaryCase = *(sumCases.begin());

            m_summaryCase.uiCapability()->setUiHidden(false);

            fields.push_back(&m_summaryCase);
        }
    }

    RiaSummaryCurveAnalyzer analyzer;
    analyzer.appendAdresses(visibleAddressesCurveCollection());

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

            fields.push_back(&m_wellName);
        }

        if (analyzer.wellGroupNames().size() == 1)
        {
            QString txt     = QString::fromStdString(*(analyzer.wellGroupNames().begin()));
            m_wellGroupName = txt;
            m_wellGroupName.uiCapability()->setUiHidden(false);

            fields.push_back(&m_wellGroupName);
        }

        if (analyzer.regionNumbers().size() == 1)
        {
            m_region = *(analyzer.regionNumbers().begin());
            m_region.uiCapability()->setUiHidden(false);

            fields.push_back(&m_region);
        }

        if (analyzer.quantities().size() == 1)
        {
            QString txt = QString::fromStdString(*(analyzer.quantities().begin()));
            m_quantity  = txt;
            m_quantity.uiCapability()->setUiHidden(false);

            fields.push_back(&m_quantity);
        }
    }

    return fields;
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

        int nextIndex = currentIndex + indexOffset;
        if (nextIndex < options.size() && nextIndex > -1)
        {
            auto optionValue = options[nextIndex].value();

            QVariant currentValue = valueField->toQVariant();

            valueField->setFromQVariant(optionValue);

            valueField->uiCapability()->notifyFieldChanged(currentValue, optionValue);
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotSourceStepping::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName,
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
