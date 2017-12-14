/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016 Statoil ASA
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

#include "RimSummaryCurve.h"

#include "RiaApplication.h"
#include "RiaDefines.h"

#include "RifReaderEclipseSummary.h"

#include "RigTimeHistoryCurveMerger.h"

#include "RimEclipseResultCase.h"
#include "RimProject.h"
#include "RimSummaryAddress.h"
#include "RimSummaryCalculationCollection.h"
#include "RimSummaryCase.h"
#include "RimSummaryCrossPlot.h"
#include "RimSummaryCurveAutoName.h"
#include "RimSummaryCurveCollection.h"
#include "RimSummaryFilter.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"
#include "RimSummaryTimeAxisProperties.h"
#include "RimTools.h"

#include "RiuLineSegmentQwtPlotCurve.h"
#include "RiuSummaryCurveDefSelectionDialog.h"
#include "RiuSummaryQwtPlot.h"

#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiListEditor.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiTreeOrdering.h"

#include "qwt_date.h"

#include <QMessageBox>


// See also corresponding fake implementations in RimSummaryCurveFilter
QTextStream& operator << (QTextStream& str, const RifEclipseSummaryAddress& sobj)
{
    CVF_ASSERT(false);
    return str;
}

QTextStream& operator >> (QTextStream& str, RifEclipseSummaryAddress& sobj)
{
    CVF_ASSERT(false);
    return str;
}




CAF_PDM_SOURCE_INIT(RimSummaryCurve, "SummaryCurve");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCurve::RimSummaryCurve()
{
    CAF_PDM_InitObject("Summary Curve", ":/SummaryCurve16x16.png", "", "");

    // Y Values

    CAF_PDM_InitFieldNoDefault(&m_yValuesSummaryCase, "SummaryCase", "Case", "", "", "");
    m_yValuesSummaryCase.uiCapability()->setUiTreeChildrenHidden(true);
    m_yValuesSummaryCase.uiCapability()->setAutoAddingOptionFromValue(false);

    CAF_PDM_InitFieldNoDefault(&m_yValuesSelectedVariableDisplayField, "SelectedVariableDisplayVar", "Vector", "", "", "");
    m_yValuesSelectedVariableDisplayField.xmlCapability()->setIOWritable(false);
    m_yValuesSelectedVariableDisplayField.xmlCapability()->setIOReadable(false);
    m_yValuesSelectedVariableDisplayField.uiCapability()->setUiReadOnly(true);

    CAF_PDM_InitFieldNoDefault(&m_yValuesSummaryFilter, "VarListFilter", "Filter", "", "", "");
    m_yValuesSummaryFilter.uiCapability()->setUiTreeChildrenHidden(true);
    m_yValuesSummaryFilter.uiCapability()->setUiHidden(true);

    m_yValuesSummaryFilter = new RimSummaryFilter;

    CAF_PDM_InitFieldNoDefault(&m_yValuesUiFilterResultSelection, "FilterResultSelection", "Filter Result", "", "", "");
    m_yValuesUiFilterResultSelection.xmlCapability()->setIOWritable(false);
    m_yValuesUiFilterResultSelection.xmlCapability()->setIOReadable(false);
    m_yValuesUiFilterResultSelection.uiCapability()->setUiEditorTypeName(caf::PdmUiListEditor::uiEditorTypeName());
    m_yValuesUiFilterResultSelection.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);
    m_yValuesUiFilterResultSelection.uiCapability()->setAutoAddingOptionFromValue(false);
    
    CAF_PDM_InitFieldNoDefault(&m_yValuesCurveVariable, "SummaryAddress", "Summary Address", "", "", "");
    m_yValuesCurveVariable.uiCapability()->setUiHidden(true);
    m_yValuesCurveVariable.uiCapability()->setUiTreeChildrenHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_yPushButtonSelectSummaryAddress, "SelectAddress", "", "", "", "");
    caf::PdmUiPushButtonEditor::configureEditorForField(&m_yPushButtonSelectSummaryAddress);
    m_yPushButtonSelectSummaryAddress = false;

    m_yValuesCurveVariable = new RimSummaryAddress;


    // X Values

    CAF_PDM_InitFieldNoDefault(&m_xValuesSummaryCase, "x_SummaryCase", "Case", "", "", "");
    m_xValuesSummaryCase.uiCapability()->setUiTreeChildrenHidden(true);
    m_xValuesSummaryCase.uiCapability()->setAutoAddingOptionFromValue(false);

    CAF_PDM_InitFieldNoDefault(&m_xValuesSelectedVariableDisplayField, "x_SelectedVariableDisplayVar", "Vector", "", "", "");
    m_xValuesSelectedVariableDisplayField.xmlCapability()->setIOWritable(false);
    m_xValuesSelectedVariableDisplayField.xmlCapability()->setIOReadable(false);
    m_xValuesSelectedVariableDisplayField.uiCapability()->setUiReadOnly(true);

    CAF_PDM_InitFieldNoDefault(&m_xValuesSummaryFilter, "x_VarListFilter", "Filter", "", "", "");
    m_xValuesSummaryFilter.uiCapability()->setUiTreeChildrenHidden(true);
    m_xValuesSummaryFilter.uiCapability()->setUiHidden(true);

    m_xValuesSummaryFilter = new RimSummaryFilter;

    CAF_PDM_InitFieldNoDefault(&m_xValuesUiFilterResultSelection, "x_FilterResultSelection", "Filter Result", "", "", "");
    m_xValuesUiFilterResultSelection.xmlCapability()->setIOWritable(false);
    m_xValuesUiFilterResultSelection.xmlCapability()->setIOReadable(false);
    m_xValuesUiFilterResultSelection.uiCapability()->setUiEditorTypeName(caf::PdmUiListEditor::uiEditorTypeName());
    m_xValuesUiFilterResultSelection.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);
    m_xValuesUiFilterResultSelection.uiCapability()->setAutoAddingOptionFromValue(false);

    CAF_PDM_InitFieldNoDefault(&m_xValuesCurveVariable, "x_SummaryAddress", "Summary Address", "", "", "");
    m_xValuesCurveVariable.uiCapability()->setUiHidden(true);
    m_xValuesCurveVariable.uiCapability()->setUiTreeChildrenHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_xPushButtonSelectSummaryAddress, "x_SelectAddress", "", "", "", "");
    caf::PdmUiPushButtonEditor::configureEditorForField(&m_xPushButtonSelectSummaryAddress);
    m_xPushButtonSelectSummaryAddress = false;

    m_xValuesCurveVariable = new RimSummaryAddress;

    
    // Other members

    CAF_PDM_InitFieldNoDefault(&m_plotAxis, "PlotAxis", "Axis", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_curveNameConfig, "SummaryCurveNameConfig", "SummaryCurveNameConfig", "", "", "");
    m_curveNameConfig.uiCapability()->setUiHidden(true);
    m_curveNameConfig.uiCapability()->setUiTreeChildrenHidden(true);

    m_curveNameConfig = new RimSummaryCurveAutoName;

    m_symbolSkipPixelDistance = 10.0f;
    m_curveThickness = 2;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCurve::~RimSummaryCurve()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::setSummaryCaseY(RimSummaryCase* sumCase)
{
	m_yValuesSummaryCase = sumCase;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCase* RimSummaryCurve::summaryCaseY() const
{
    return m_yValuesSummaryCase();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RimSummaryCurve::summaryAddressX() const
{
    return m_xValuesCurveVariable->address();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::setSummaryAddressX(const RifEclipseSummaryAddress& address)
{
    m_xValuesCurveVariable->setAddress(address);

    // TODO: Should interpolation be computed similar to RimSummaryCurve::setSummaryAddressY
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RimSummaryCurve::summaryAddressY() const
{
    return m_yValuesCurveVariable->address();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::setSummaryAddressY(const RifEclipseSummaryAddress& address)
{
    m_yValuesCurveVariable->setAddress(address);

    m_yValuesSummaryFilter->updateFromAddress(address);

    calculateCurveInterpolationFromAddress();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::string RimSummaryCurve::unitNameY() const
{
    RifSummaryReaderInterface* reader = valuesSummaryReaderY();
    if (reader) return reader->unitName(this->summaryAddressY());

    return "";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::string RimSummaryCurve::unitNameX() const
{
    RifSummaryReaderInterface* reader = valuesSummaryReaderX();
    if (reader) return reader->unitName(this->summaryAddressX());

    return "";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<double> RimSummaryCurve::valuesY() const
{
    std::vector<double> values;

    RifSummaryReaderInterface* reader = valuesSummaryReaderY();

    if ( !reader ) return values;

    RifEclipseSummaryAddress addr = m_yValuesCurveVariable()->address();
    reader->values(addr, &values);

    return values;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<double> RimSummaryCurve::valuesX() const
{
    std::vector<double> values;

    if (m_xValuesSummaryCase() && m_xValuesSummaryCase()->summaryReader())
    {
        RifSummaryReaderInterface* reader = m_xValuesSummaryCase()->summaryReader();

        RifEclipseSummaryAddress addr = m_xValuesCurveVariable()->address();
        reader->values(addr, &values);
    }

    return values;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<time_t>& RimSummaryCurve::timeStepsY() const
{
    static std::vector<time_t> emptyVector;
    RifSummaryReaderInterface* reader = valuesSummaryReaderY();

    if ( !reader ) return emptyVector;

    RifEclipseSummaryAddress addr = m_yValuesCurveVariable()->address();
    
    return reader->timeSteps(addr);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::setSummaryCaseX(RimSummaryCase* sumCase)
{
    m_xValuesSummaryCase = sumCase;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCase* RimSummaryCurve::summaryCaseX() const
{
    return m_xValuesSummaryCase();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::setLeftOrRightAxisY(RiaDefines::PlotAxis plotAxis)
{
    m_plotAxis = plotAxis;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiaDefines::PlotAxis RimSummaryCurve::axisY() const
{
    return m_plotAxis();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimSummaryCurve::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options = this->RimPlotCurve::calculateValueOptions(fieldNeedingOptions, useOptionsOnly);
    if (!options.isEmpty()) return options;

    if (fieldNeedingOptions == &m_yValuesSummaryCase ||
        fieldNeedingOptions == &m_xValuesSummaryCase)
    {
        RimProject* proj = RiaApplication::instance()->project();

        std::vector<RimSummaryCase*> cases = proj->allSummaryCases();

        cases.push_back(proj->calculationCollection->calculationSummaryCase());

        for (RimSummaryCase* rimCase : cases)
        {
            options.push_back(caf::PdmOptionItemInfo(rimCase->caseName(), rimCase));
        }

        if (options.size() > 0)
        {
            options.push_front(caf::PdmOptionItemInfo("None", nullptr));
        }
    }
    else if(fieldNeedingOptions == &m_yValuesUiFilterResultSelection)
    {
        appendOptionItemsForSummaryAddresses(&options, m_yValuesSummaryCase(), m_yValuesSummaryFilter());
    }
    else if (fieldNeedingOptions == &m_xValuesUiFilterResultSelection)
    {
        appendOptionItemsForSummaryAddresses(&options, m_xValuesSummaryCase(), m_xValuesSummaryFilter());
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimSummaryCurve::createCurveAutoName()
{
    RimSummaryPlot* plot = nullptr;
    firstAncestorOrThisOfTypeAsserted(plot);

    QString name = m_curveNameConfig->curveName(m_yValuesCurveVariable->address(), plot->activePlotTitleHelper());
    if (isCrossPlotCurve())
    {
        QString xCurveName = m_curveNameConfig->curveName(m_xValuesCurveVariable->address(), plot->activePlotTitleHelper());

        name += " | " + xCurveName;
    }

    return name;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::updateZoomInParentPlot()
{
    RimSummaryPlot* plot = nullptr;
    firstAncestorOrThisOfTypeAsserted(plot);

    plot->updateZoomInQwt(); 
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::onLoadDataAndUpdate(bool updateParentPlot)
{
    this->RimPlotCurve::updateCurvePresentation(updateParentPlot);

    m_yValuesSelectedVariableDisplayField = QString::fromStdString(m_yValuesCurveVariable->address().uiText());
    m_yValuesUiFilterResultSelection = m_yValuesCurveVariable->address();

    m_xValuesSelectedVariableDisplayField = QString::fromStdString(m_xValuesCurveVariable->address().uiText());
    m_xValuesUiFilterResultSelection = m_xValuesCurveVariable->address();

    updateConnectedEditors();

    if (isCurveVisible())
    {
        std::vector<double> curveValuesY = this->valuesY();

        RimSummaryPlot* plot = nullptr;
        firstAncestorOrThisOfTypeAsserted(plot);
        bool isLogCurve = plot->isLogarithmicScaleEnabled(this->axisY());

        bool shouldPopulateViewWithEmptyData = false;

        if (isCrossPlotCurve())
        {
            auto curveValuesX = this->valuesX();
            auto curveTimeStepsX = timeStepsX();

            auto curveTimeStepsY = timeStepsY();

            if (curveValuesY.empty() || curveValuesX.empty())
            {
                shouldPopulateViewWithEmptyData = true;
            }
            else
            {
                RigTimeHistoryCurveMerger curveMerger;
                curveMerger.addCurveData(curveValuesX, curveTimeStepsX);
                curveMerger.addCurveData(curveValuesY, curveTimeStepsY);
                curveMerger.computeInterpolatedValues();

                if (curveMerger.allTimeSteps().size() > 0)
                {
                    m_qwtPlotCurve->setSamplesFromXValuesAndYValues(curveMerger.interpolatedCurveValuesForAllTimeSteps(0), 
                                                                    curveMerger.interpolatedCurveValuesForAllTimeSteps(1), 
                                                                    isLogCurve);
                }
                else
                {
                    shouldPopulateViewWithEmptyData = true;
                }
            }
        }
        else
        {
            std::vector<time_t> curveTimeStepsY = this->timeStepsY();
            if (curveTimeStepsY.size() > 0 && curveTimeStepsY.size() == curveValuesY.size())
            {
                if (plot->timeAxisProperties()->timeMode() == RimSummaryTimeAxisProperties::DATE)
                {
                    m_qwtPlotCurve->setSamplesFromTimeTAndYValues(curveTimeStepsY, curveValuesY, isLogCurve);
                }
                else
                {
                    double timeScale = plot->timeAxisProperties()->fromTimeTToDisplayUnitScale();

                    std::vector<double> timeFromSimulationStart;
                    if (curveTimeStepsY.size())
                    {
                        time_t startDate = curveTimeStepsY[0];
                        for (const auto& date : curveTimeStepsY)
                        {
                            timeFromSimulationStart.push_back(timeScale*(date - startDate));
                        }
                    }

                    m_qwtPlotCurve->setSamplesFromXValuesAndYValues(timeFromSimulationStart, curveValuesY, isLogCurve);
                }
            }
            else
            {
                shouldPopulateViewWithEmptyData = true;
            }
        }

        if (shouldPopulateViewWithEmptyData)
        {
            m_qwtPlotCurve->setSamplesFromXValuesAndYValues(std::vector<double>(), std::vector<double>(), isLogCurve);
        }

        if (updateParentPlot && m_parentQwtPlot)
        {
            updateZoomInParentPlot();
            m_parentQwtPlot->replot();
        }
    }

    if (updateParentPlot) updateQwtPlotAxis();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute)
{
    if (&m_yPushButtonSelectSummaryAddress == field ||
        &m_xPushButtonSelectSummaryAddress == field)
    {
        caf::PdmUiPushButtonEditorAttribute* attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*> (attribute);
        if (attrib)
        {
            attrib->m_buttonText = "Vector Selection Dialog";
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    RimPlotCurve::updateOptionSensitivity();

    {
        QString curveDataGroupName = "Summary Vector";
        if (isCrossPlotCurve()) curveDataGroupName += " Y";
        caf::PdmUiGroup* curveDataGroup = uiOrdering.addNewGroupWithKeyword(curveDataGroupName, "Summary Vector Y");
        curveDataGroup->add(&m_yValuesSummaryCase);
        curveDataGroup->add(&m_yValuesSelectedVariableDisplayField);
        curveDataGroup->add(&m_plotAxis);
        curveDataGroup->add(&m_yPushButtonSelectSummaryAddress);

        QString curveVarSelectionGroupName = "Vector Selection Filter Y";
        caf::PdmUiGroup* curveVarSelectionGroup = curveDataGroup->addNewGroupWithKeyword("Vector Selection Filter", curveVarSelectionGroupName);
        curveVarSelectionGroup->setCollapsedByDefault(true);
        m_yValuesSummaryFilter->uiOrdering(uiConfigName, *curveVarSelectionGroup);
        curveVarSelectionGroup->add(&m_yValuesUiFilterResultSelection);

    }

    if (isCrossPlotCurve())
    {
        caf::PdmUiGroup* curveDataGroup = uiOrdering.addNewGroup("Summary Vector X");
        curveDataGroup->add(&m_xValuesSummaryCase);
        curveDataGroup->add(&m_xValuesSelectedVariableDisplayField);
        curveDataGroup->add(&m_xPushButtonSelectSummaryAddress);

        caf::PdmUiGroup* curveVarSelectionGroup = curveDataGroup->addNewGroupWithKeyword("Vector Selection Filter", "Vector Selection Filter X");
        curveVarSelectionGroup->setCollapsedByDefault(true);
        m_xValuesSummaryFilter->uiOrdering(uiConfigName, *curveVarSelectionGroup);
        curveVarSelectionGroup->add(&m_xValuesUiFilterResultSelection);
    }

    caf::PdmUiGroup* appearanceGroup = uiOrdering.addNewGroup("Appearance");
    RimPlotCurve::appearanceUiOrdering(*appearanceGroup);

    caf::PdmUiGroup* nameGroup = uiOrdering.addNewGroup("Curve Name");
    nameGroup->setCollapsedByDefault(true);
    nameGroup->add(&m_showLegend);
    RimPlotCurve::curveNameUiOrdering(*nameGroup);

    if (m_isUsingAutoName)
    {
        m_curveNameConfig->uiOrdering(uiConfigName, *nameGroup);
    }

    uiOrdering.skipRemainingFields(); // For now. 
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::appendOptionItemsForSummaryAddresses(QList<caf::PdmOptionItemInfo>* options,
                                                           RimSummaryCase* summaryCase,
                                                           RimSummaryFilter* summaryFilter) 
{
    if (summaryCase)
    {
        RifSummaryReaderInterface* reader = summaryCase->summaryReader();
        if (reader)
        {
            const std::vector<RifEclipseSummaryAddress> allAddresses = reader->allResultAddresses();

            for (auto& address : allAddresses)
            {
                if (summaryFilter && !summaryFilter->isIncludedByFilter(address)) continue;

                std::string name = address.uiText();
                QString s = QString::fromStdString(name);
                options->push_back(caf::PdmOptionItemInfo(s, QVariant::fromValue(address)));
            }
        }

        options->push_front(caf::PdmOptionItemInfo(RiaDefines::undefinedResultName(), QVariant::fromValue(RifEclipseSummaryAddress())));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimSummaryCurve::isCrossPlotCurve() const
{
    RimSummaryCrossPlot* crossPlot = nullptr;
    this->firstAncestorOrThisOfType(crossPlot);
    if (crossPlot) return true;

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::updateQwtPlotAxis()
{
    if (m_qwtPlotCurve)
    {
        if (this->axisY() == RiaDefines::PLOT_AXIS_LEFT)
        {
            m_qwtPlotCurve->setYAxis(QwtPlot::yLeft);
        }
        else
        {
            m_qwtPlotCurve->setYAxis(QwtPlot::yRight);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::applyCurveAutoNameSettings(const RimSummaryCurveAutoName& autoNameSettings)
{
    m_curveNameConfig->applySettings(autoNameSettings);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    this->RimPlotCurve::fieldChangedByUi(changedField, oldValue, newValue);
    
    RimSummaryPlot* plot = nullptr;
    firstAncestorOrThisOfType(plot);
    CVF_ASSERT(plot);

    bool loadAndUpdate = false;
    bool crossPlotTestForMatchingTimeSteps = false;

    if(changedField == &m_yValuesUiFilterResultSelection)
    {
        m_yValuesCurveVariable->setAddress(m_yValuesUiFilterResultSelection());

        this->calculateCurveInterpolationFromAddress();

        loadAndUpdate = true;
    } 
    else if (changedField == &m_xValuesUiFilterResultSelection)
    {
        m_xValuesCurveVariable->setAddress(m_xValuesUiFilterResultSelection());

        this->calculateCurveInterpolationFromAddress();

        loadAndUpdate = true;
    }
    else if (&m_showCurve == changedField)
    {
        plot->updateAxes();
        plot->updatePlotTitle();
    }
    else if (changedField == &m_plotAxis)
    {
        updateQwtPlotAxis();

        plot->updateAxes();
    }
    else if (changedField == &m_yValuesSummaryCase)
    {
        plot->updateCaseNameHasChanged();
        this->onLoadDataAndUpdate(true);
    }
    else if (changedField == &m_yPushButtonSelectSummaryAddress)
    {
        RiuSummaryCurveDefSelectionDialog dlg(nullptr);
        RimSummaryCase* candidateCase = m_yValuesSummaryCase();
        RifEclipseSummaryAddress candicateAddress = m_yValuesCurveVariable->address();

        if (candidateCase == nullptr)
        {
            candidateCase = m_xValuesSummaryCase();
        }

        if (!candicateAddress.isValid())
        {
            candicateAddress = m_xValuesCurveVariable->address();
        }

        dlg.setCaseAndAddress(candidateCase, candicateAddress);
        
        if (dlg.exec() == QDialog::Accepted)
        {
            auto curveSelection = dlg.curveSelection();
            if (curveSelection.size() > 0)
            {
                m_yValuesSummaryCase = curveSelection[0].summaryCase();
                m_yValuesCurveVariable->setAddress(curveSelection[0].summaryAddress());

                crossPlotTestForMatchingTimeSteps = true;
                loadAndUpdate = true;
            }
        }

        m_yPushButtonSelectSummaryAddress = false;
    }
    else if (changedField == &m_xPushButtonSelectSummaryAddress)
    {
        RiuSummaryCurveDefSelectionDialog dlg(nullptr);
        RimSummaryCase* candidateCase = m_xValuesSummaryCase();
        RifEclipseSummaryAddress candicateAddress = m_xValuesCurveVariable->address();

        if (candidateCase == nullptr)
        {
            candidateCase = m_yValuesSummaryCase();
        }

        if (!candicateAddress.isValid())
        {
            candicateAddress = m_yValuesCurveVariable->address();
        }

        dlg.setCaseAndAddress(candidateCase, candicateAddress);

        if (dlg.exec() == QDialog::Accepted)
        {
            auto curveSelection = dlg.curveSelection();
            if (curveSelection.size() > 0)
            {
                m_xValuesSummaryCase = curveSelection[0].summaryCase();
                m_xValuesCurveVariable->setAddress(curveSelection[0].summaryAddress());

                crossPlotTestForMatchingTimeSteps = true;
                loadAndUpdate = true;
            }
        }

        m_xPushButtonSelectSummaryAddress = false;
    }

    if (crossPlotTestForMatchingTimeSteps)
    {
        auto curveValuesX = this->valuesX();
        auto curveTimeStepsX = timeStepsX();

        auto curveValuesY = this->valuesY();
        auto curveTimeStepsY = timeStepsY();

        if (!curveValuesX.empty() && !curveValuesY.empty())
        {
            RigTimeHistoryCurveMerger curveMerger;
            curveMerger.addCurveData(curveValuesX, curveTimeStepsX);
            curveMerger.addCurveData(curveValuesY, curveTimeStepsY);
            curveMerger.computeInterpolatedValues();

            if (curveMerger.validIntervalsForAllTimeSteps().size() == 0)
            {
                QString description;

                {
                    QDateTime first = QDateTime::fromTime_t(curveTimeStepsX.front());
                    QDateTime last = QDateTime::fromTime_t(curveTimeStepsX.back());

                    std::vector<QDateTime> timeSteps;
                    timeSteps.push_back(first);
                    timeSteps.push_back(last);

                    QString formatString = RimTools::createTimeFormatStringFromDates(timeSteps);

                    description += QString("Time step range for X : '%1' - '%2'")
                        .arg(first.toString(formatString))
                        .arg(last.toString(formatString));
                }

                {
                    QDateTime first = QDateTime::fromTime_t(curveTimeStepsY.front());
                    QDateTime last = QDateTime::fromTime_t(curveTimeStepsY.back());

                    std::vector<QDateTime> timeSteps;
                    timeSteps.push_back(first);
                    timeSteps.push_back(last);

                    QString formatString = RimTools::createTimeFormatStringFromDates(timeSteps);

                    description += "\n";
                    description += QString("Time step range for Y : '%1' - '%2'")
                        .arg(first.toString(formatString))
                        .arg(last.toString(formatString));
                }

                QMessageBox::warning(NULL, "Detected no overlapping time steps", description);
            }
        }
    }

    if (loadAndUpdate)
    {
        this->loadDataAndUpdate(true);

        plot->updateAxes();
        plot->updatePlotTitle();
    }

    if (&m_showCurve == changedField)
    {
        // If no plot collection is found, we assume that we are inside a curve creator
        // Update the summary curve collection to make sure the curve names are updated in curve creator UI

        RimSummaryPlotCollection* plotCollection = nullptr;
        this->firstAncestorOrThisOfType(plotCollection);
        if (!plotCollection)
        {
            RimSummaryCurveCollection* curveColl = nullptr;
            this->firstAncestorOrThisOfType(curveColl);
            if (curveColl)
            {
                curveColl->updateConnectedEditors();
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifSummaryReaderInterface* RimSummaryCurve::valuesSummaryReaderX() const
{
    if (!m_xValuesSummaryCase()) return nullptr;

    return m_xValuesSummaryCase()->summaryReader();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifSummaryReaderInterface* RimSummaryCurve::valuesSummaryReaderY() const
{
    if (!m_yValuesSummaryCase()) return nullptr;

    return m_yValuesSummaryCase()->summaryReader();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<time_t>& RimSummaryCurve::timeStepsX() const
{
    static std::vector<time_t> emptyVector;
    RifSummaryReaderInterface* reader = valuesSummaryReaderX();

    if (!reader) return emptyVector;

    RifEclipseSummaryAddress addr = m_xValuesCurveVariable()->address();

    return reader->timeSteps(addr);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::calculateCurveInterpolationFromAddress()
{
    if (m_yValuesCurveVariable())
    {
        QString quantityName = QString::fromUtf8(m_yValuesCurveVariable()->address().quantityName().c_str());
        if (quantityName.endsWith("T"))
        {
            m_curveInterpolation = INTERPOLATION_POINT_TO_POINT;
        }
        else
        {
            m_curveInterpolation = INTERPOLATION_STEP_LEFT;
        }
    }
}

