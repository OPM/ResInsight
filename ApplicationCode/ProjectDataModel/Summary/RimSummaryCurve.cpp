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

#include "RimEclipseResultCase.h"
#include "RimProject.h"
#include "RimSummaryAddress.h"
#include "RimSummaryCalculationCollection.h"
#include "RimSummaryCase.h"
#include "RimSummaryCrossPlot.h"
#include "RimSummaryCurveAutoName.h"
#include "RimSummaryFilter.h"
#include "RimSummaryPlot.h"
#include "RimSummaryTimeAxisProperties.h"

#include "RiuLineSegmentQwtPlotCurve.h"
#include "RiuSummaryQwtPlot.h"

#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiListEditor.h"
#include "cafPdmUiTreeOrdering.h"

#include "qwt_date.h"


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
    
    CAF_PDM_InitFieldNoDefault(&m_yValuesCurveVariable, "SummaryAddress", "SummaryAddress", "", "", "");
    m_yValuesCurveVariable.uiCapability()->setUiHidden(true);
    m_yValuesCurveVariable.uiCapability()->setUiTreeChildrenHidden(true);

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

    CAF_PDM_InitFieldNoDefault(&m_xValuesCurveVariable, "x_SummaryAddress", "SummaryAddress", "", "", "");
    m_xValuesCurveVariable.uiCapability()->setUiHidden(true);
    m_xValuesCurveVariable.uiCapability()->setUiTreeChildrenHidden(true);

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
void RimSummaryCurve::setSummaryCase(RimSummaryCase* sumCase)
{
	m_yValuesSummaryCase = sumCase;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCase* RimSummaryCurve::summaryCase() const
{
    return m_yValuesSummaryCase();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RimSummaryCurve::summaryAddress()
{
    return m_yValuesCurveVariable->address();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::setSummaryAddress(const RifEclipseSummaryAddress& address)
{
    m_yValuesCurveVariable->setAddress(address);

    m_yValuesSummaryFilter->updateFromAddress(address);

    calculateCurveInterpolationFromAddress();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::string RimSummaryCurve::unitName()
{
    RifSummaryReaderInterface* reader = yValuesSummaryReader();
    if (reader) return reader->unitName(this->summaryAddress());

    return "";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<double> RimSummaryCurve::yValues() const
{
    std::vector<double> values;

    RifSummaryReaderInterface* reader = yValuesSummaryReader();

    if ( !reader ) return values;

    RifEclipseSummaryAddress addr = m_yValuesCurveVariable()->address();
    reader->values(addr, &values);

    return values;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<double> RimSummaryCurve::xValues() const
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
const std::vector<time_t>& RimSummaryCurve::timeSteps() const
{
    static std::vector<time_t> emptyVector;
    RifSummaryReaderInterface* reader = yValuesSummaryReader();

    if ( !reader ) return emptyVector;

    RifEclipseSummaryAddress addr = m_yValuesCurveVariable()->address();
    
    return reader->timeSteps(addr);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::setYAxis(RiaDefines::PlotAxis plotAxis)
{
    m_plotAxis = plotAxis;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiaDefines::PlotAxis RimSummaryCurve::yAxis() const
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
        std::vector<RimSummaryCase*> cases;

        proj->allSummaryCases(cases);

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
    return m_curveNameConfig->curveName(m_yValuesCurveVariable->address());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::updateZoomInParentPlot()
{
    RimSummaryPlot* plot = nullptr;
    firstAncestorOrThisOfType(plot);

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
    updateConnectedEditors();

    if (isCurveVisible())
    {
        std::vector<time_t> dateTimes = this->timeSteps();
        std::vector<double> yValues = this->yValues();

        RimSummaryPlot* plot = nullptr;
        firstAncestorOrThisOfType(plot);
        bool isLogCurve = plot->isLogarithmicScaleEnabled(this->yAxis());

        bool shouldPopulateViewWithEmptyData = false;

        if (isCrossPlotCurve())
        {
            std::vector<double> xValues = this->xValues();

            if (!yValues.empty() && yValues.size() == xValues.size())
            {
                m_qwtPlotCurve->setSamplesFromXValuesAndYValues(xValues, yValues, isLogCurve);
            }
            else
            {
                shouldPopulateViewWithEmptyData = true;
            }
        }
        else
        {
            if (dateTimes.size() > 0 && dateTimes.size() == yValues.size())
            {
                if (plot->timeAxisProperties()->timeMode() == RimSummaryTimeAxisProperties::DATE)
                {
                    m_qwtPlotCurve->setSamplesFromTimeTAndYValues(dateTimes, yValues, isLogCurve);
                }
                else
                {
                    double timeScale  = plot->timeAxisProperties()->fromTimeTToDisplayUnitScale();

                    std::vector<double> times;
                    if ( dateTimes.size() )
                    {
                        time_t startDate = dateTimes[0];
                        for ( time_t& date: dateTimes )
                        {
                            times.push_back(timeScale*(date - startDate));
                        }
                    }

                    m_qwtPlotCurve->setSamplesFromXValuesAndYValues(times, yValues, isLogCurve);
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

        if ( updateParentPlot && m_parentQwtPlot)
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
void RimSummaryCurve::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    RimPlotCurve::updateOptionSensitivity();

    {
        QString curveDataGroupName = "Summary Vector";
        if (isCrossPlotCurve()) curveDataGroupName += " Y";
        caf::PdmUiGroup* curveDataGroup = uiOrdering.addNewGroupWithKeyword(curveDataGroupName, "Summary Vector Y");
        curveDataGroup->add(&m_yValuesSummaryCase);
        curveDataGroup->add(&m_yValuesSelectedVariableDisplayField);

        QString curveVarSelectionGroupName = "Vector Selection";
        if (isCrossPlotCurve()) curveVarSelectionGroupName += " Y";
        caf::PdmUiGroup* curveVarSelectionGroup = curveDataGroup->addNewGroupWithKeyword(curveVarSelectionGroupName, "Vector Selection Y");
        curveVarSelectionGroup->setCollapsedByDefault(true);
        m_yValuesSummaryFilter->uiOrdering(uiConfigName, *curveVarSelectionGroup);
        curveVarSelectionGroup->add(&m_yValuesUiFilterResultSelection);
    }

    if (isCrossPlotCurve())
    {
        caf::PdmUiGroup* curveDataGroup = uiOrdering.addNewGroup("Summary Vector X");
        curveDataGroup->add(&m_xValuesSummaryCase);
        curveDataGroup->add(&m_xValuesSelectedVariableDisplayField);

        caf::PdmUiGroup* curveVarSelectionGroup = curveDataGroup->addNewGroup("Vector Selection X");
        curveVarSelectionGroup->setCollapsedByDefault(true);
        m_xValuesSummaryFilter->uiOrdering(uiConfigName, *curveVarSelectionGroup);
        curveVarSelectionGroup->add(&m_xValuesUiFilterResultSelection);
    }

    uiOrdering.add(&m_plotAxis);

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
        if (this->yAxis() == RiaDefines::PLOT_AXIS_LEFT)
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
    this->RimPlotCurve::fieldChangedByUi(changedField,oldValue,newValue);
    
    RimSummaryPlot* plot = nullptr;
    firstAncestorOrThisOfType(plot);
    CVF_ASSERT(plot);

    if(changedField == &m_yValuesUiFilterResultSelection)
    {
        m_yValuesCurveVariable->setAddress(m_yValuesUiFilterResultSelection());

        this->calculateCurveInterpolationFromAddress();
        this->loadDataAndUpdate(true);

        plot->updateAxes();
    } 
    else if (changedField == &m_xValuesUiFilterResultSelection)
    {
        m_xValuesCurveVariable->setAddress(m_xValuesUiFilterResultSelection());

        this->calculateCurveInterpolationFromAddress();
        this->loadDataAndUpdate(true);

        plot->updateAxes();
    }
    else if (&m_showCurve == changedField)
    {
        plot->updateAxes();
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
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifSummaryReaderInterface* RimSummaryCurve::yValuesSummaryReader() const
{
    if (!m_yValuesSummaryCase()) return nullptr;

    return m_yValuesSummaryCase()->summaryReader();
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

