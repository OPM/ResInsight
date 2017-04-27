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

#include "RifReaderEclipseSummary.h"

#include "RigSummaryCaseData.h"

#include "RimDefines.h"
#include "RimEclipseResultCase.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryCurveAutoName.h"
#include "RimSummaryCurveFilter.h"
#include "RimSummaryFilter.h"
#include "RimSummaryPlot.h"
#include "RimSummaryTimeAxisProperties.h"

#include "RiuLineSegmentQwtPlotCurve.h"
#include "RiuSummaryQwtPlot.h"

#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiListEditor.h"
#include "cafPdmUiTreeOrdering.h"

#include "qwt_date.h"


CAF_PDM_SOURCE_INIT(RimSummaryAddress, "SummaryAddress");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryAddress::RimSummaryAddress()
{

    CAF_PDM_InitFieldNoDefault(&m_category,          "SummaryVarType",      "Type", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_quantityName,      "SummaryQuantityName", "Quantity", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_regionNumber,      "SummaryRegion",       "Region", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_regionNumber2,     "SummaryRegion2",      "Region2", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_wellGroupName,     "SummaryWellGroup",    "Group", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_wellName,          "SummaryWell",         "Well", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_wellSegmentNumber, "SummaryWellSegment",  "Well Segment", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_lgrName,           "SummaryLgr",          "Grid", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_cellI,             "SummaryCellI",        "I", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_cellJ,             "SummaryCellJ",        "J", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_cellK,             "SummaryCellK",        "K", "", "", "");

    m_category = RifEclipseSummaryAddress::SUMMARY_INVALID;
    m_regionNumber = m_regionNumber2 = m_wellSegmentNumber = m_cellI = m_cellJ = m_cellK = -1;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryAddress::~RimSummaryAddress()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryAddress::setAddress(const RifEclipseSummaryAddress& addr)
{
    m_category                      = addr.category();
    m_quantityName                  = addr.quantityName().c_str();
    m_regionNumber                  = addr.regionNumber();
    m_regionNumber2                 = addr.regionNumber2();
    m_wellGroupName                 = addr.wellGroupName().c_str();
    m_wellName                      = addr.wellName().c_str();
    m_wellSegmentNumber             = addr.wellSegmentNumber();
    m_lgrName                       = addr.lgrName().c_str();

    m_cellI = addr.cellI(); m_cellJ = addr.cellJ(); m_cellK = addr.cellK();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RimSummaryAddress::address()
{
    return RifEclipseSummaryAddress( m_category(),
                                     m_quantityName().toStdString(),
                                     m_regionNumber(),
                                     m_regionNumber2(),
                                     m_wellGroupName().toStdString(),
                                     m_wellName().toStdString(),
                                     m_wellSegmentNumber(),
                                     m_lgrName().toStdString(),
                                     m_cellI(), m_cellJ(), m_cellK());
}

namespace caf
{

template<>
void caf::AppEnum<RifEclipseSummaryAddress::SummaryVarCategory>::setUp()
{
    addItem(RifEclipseSummaryAddress::SUMMARY_FIELD,                "SUMMARY_FIELD",               "Field");
    addItem(RifEclipseSummaryAddress::SUMMARY_AQUIFER,              "SUMMARY_AQUIFER",             "Aquifer");
    addItem(RifEclipseSummaryAddress::SUMMARY_NETWORK,              "SUMMARY_NETWORK",             "Network");
    addItem(RifEclipseSummaryAddress::SUMMARY_MISC,                 "SUMMARY_MISC",                "Misc");
    addItem(RifEclipseSummaryAddress::SUMMARY_REGION,               "SUMMARY_REGION",              "Region");
    addItem(RifEclipseSummaryAddress::SUMMARY_REGION_2_REGION,      "SUMMARY_REGION_2_REGION",     "Region-Region");
    addItem(RifEclipseSummaryAddress::SUMMARY_WELL_GROUP,           "SUMMARY_WELL_GROUP",          "Group");
    addItem(RifEclipseSummaryAddress::SUMMARY_WELL,                 "SUMMARY_WELL",                "Well");
    addItem(RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION,      "SUMMARY_WELL_COMPLETION",     "Completion");
    addItem(RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION_LGR,  "SUMMARY_WELL_COMPLETION_LGR", "Lgr-Completion");
    addItem(RifEclipseSummaryAddress::SUMMARY_WELL_LGR,             "SUMMARY_WELL_LGR",            "Lgr-Well");
    addItem(RifEclipseSummaryAddress::SUMMARY_WELL_SEGMENT,         "SUMMARY_SEGMENT",             "Segment");
    addItem(RifEclipseSummaryAddress::SUMMARY_BLOCK,                "SUMMARY_BLOCK",               "Block");
    addItem(RifEclipseSummaryAddress::SUMMARY_BLOCK_LGR,            "SUMMARY_BLOCK_LGR",           "Lgr-Block");
    setDefault(RifEclipseSummaryAddress::SUMMARY_FIELD);
}

}


CAF_PDM_SOURCE_INIT(RimSummaryCurve, "SummaryCurve");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCurve::RimSummaryCurve()
{
    CAF_PDM_InitObject("Summary Curve", ":/SummaryCurve16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_summaryCase, "SummaryCase", "Case", "", "", "");
    m_summaryCase.uiCapability()->setUiTreeChildrenHidden(true);
    
    CAF_PDM_InitFieldNoDefault(&m_selectedVariableDisplayField, "SelectedVariableDisplayVar", "Vector", "", "", "");
    m_selectedVariableDisplayField.xmlCapability()->setIOWritable(false);
    m_selectedVariableDisplayField.xmlCapability()->setIOReadable(false);
    m_selectedVariableDisplayField.uiCapability()->setUiReadOnly(true);

    CAF_PDM_InitFieldNoDefault(&m_summaryFilter, "VarListFilter", "Filter", "", "", "");
    m_summaryFilter.uiCapability()->setUiTreeChildrenHidden(true);
    m_summaryFilter.uiCapability()->setUiHidden(true);

    m_summaryFilter = new RimSummaryFilter;

    CAF_PDM_InitFieldNoDefault(&m_uiFilterResultSelection, "FilterResultSelection", "Filter Result", "", "", "");
    m_uiFilterResultSelection.xmlCapability()->setIOWritable(false);
    m_uiFilterResultSelection.xmlCapability()->setIOReadable(false);
    m_uiFilterResultSelection.uiCapability()->setUiEditorTypeName(caf::PdmUiListEditor::uiEditorTypeName());
    m_uiFilterResultSelection.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);
    m_uiFilterResultSelection.uiCapability()->setAutoAddingOptionFromValue(false);
    
    CAF_PDM_InitFieldNoDefault(&m_curveVariable, "SummaryAddress", "SummaryAddress", "", "", "");
    m_curveVariable.uiCapability()->setUiHidden(true);
    m_curveVariable.uiCapability()->setUiTreeChildrenHidden(true);

    m_curveVariable = new RimSummaryAddress;

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
    m_summaryCase = sumCase;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCase* RimSummaryCurve::summaryCase()
{
    return m_summaryCase();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RimSummaryCurve::summaryAddress()
{
    return m_curveVariable->address();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::setSummaryAddress(const RifEclipseSummaryAddress& address)
{
    m_curveVariable->setAddress(address);

    m_summaryFilter->updateFromAddress(address);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::string RimSummaryCurve::unitName()
{
    RifReaderEclipseSummary* reader = summaryReader();
    if (reader) return reader->unitName(this->summaryAddress());
    return "";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<double> RimSummaryCurve::yValues() const
{
    std::vector<double> values;

    RifReaderEclipseSummary* reader = summaryReader();

    if ( !reader ) return values;

    RifEclipseSummaryAddress addr = m_curveVariable()->address();
    reader->values(addr, &values);

    return values;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<time_t>& RimSummaryCurve::timeSteps() const
{
    static std::vector<time_t> emptyVector;
    RifReaderEclipseSummary* reader = summaryReader();

    if ( !reader ) return emptyVector;

    return reader->timeSteps();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::setYAxis(RimDefines::PlotAxis plotAxis)
{
    m_plotAxis = plotAxis;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimDefines::PlotAxis RimSummaryCurve::yAxis() const
{
    return m_plotAxis();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimSummaryCurve::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options = this->RimPlotCurve::calculateValueOptions(fieldNeedingOptions,useOptionsOnly);
    if (!options.isEmpty()) return options;


    if (fieldNeedingOptions == &m_summaryCase)
    {
        RimProject* proj = RiaApplication::instance()->project();
        std::vector<RimSummaryCase*> cases;

        proj->allSummaryCases(cases);

        for (RimSummaryCase* rimCase : cases)
        {
            options.push_back(caf::PdmOptionItemInfo(rimCase->caseName(), rimCase));
        }

        if (options.size() > 0)
        {
            options.push_front(caf::PdmOptionItemInfo("None", nullptr));
        }
    }
    else if(fieldNeedingOptions == &m_uiFilterResultSelection)
    {
        if(m_summaryCase)
        {
            RifReaderEclipseSummary* reader = summaryReader();
            int addressCount = 0;
            if(reader)
            {
                const std::vector<RifEclipseSummaryAddress> allAddresses = reader->allResultAddresses();
                addressCount = static_cast<int>(allAddresses.size());
                std::map<RifEclipseSummaryAddress, int> addrToIdxMap;
                for(int i = 0; i <addressCount; i++)
                {
                    if (!m_summaryFilter->isIncludedByFilter(allAddresses[i] )) continue;
                    addrToIdxMap[allAddresses[i]] = i;
                }

                for (const auto& addrIntPair: addrToIdxMap)
                {
                    std::string name = addrIntPair.first.uiText();
                    QString s = QString::fromStdString(name);
                    options.push_back(caf::PdmOptionItemInfo(s, addrIntPair.second));
                }
            }

            options.push_front(caf::PdmOptionItemInfo(RimDefines::undefinedResultName(), addressCount));

            if(useOptionsOnly) *useOptionsOnly = true;
        }
    }
    return options;

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimSummaryCurve::createCurveAutoName()
{
    return m_curveNameConfig->curveName(m_curveVariable->address());
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
void RimSummaryCurve::onLoadDataAndUpdate()
{
    this->RimPlotCurve::updateCurvePresentation();

    m_selectedVariableDisplayField = QString::fromStdString(m_curveVariable->address().uiText());

    RifReaderEclipseSummary* reader = summaryReader();
    if (reader)
    {
        const std::vector<RifEclipseSummaryAddress> allAddresses = reader->allResultAddresses();

        for (size_t i = 0; i < allAddresses.size(); i++)
        {
            if (allAddresses[i].uiText() == m_curveVariable->address().uiText())
            {
                m_uiFilterResultSelection = static_cast<int>(i);
                updateConnectedEditors();
            }
        }
    }

    if (isCurveVisible())
    {
        std::vector<time_t> dateTimes = this->timeSteps();
        std::vector<double> values = this->yValues();

        RimSummaryPlot* plot = nullptr;
        firstAncestorOrThisOfType(plot);
        bool isLogCurve = plot->isLogarithmicScaleEnabled(this->yAxis());

        if (dateTimes.size() > 0 && dateTimes.size() == values.size())
        {
            if (plot->timeAxisProperties()->timeMode() == RimSummaryTimeAxisProperties::DATE)
            {
                m_qwtPlotCurve->setSamplesFromTimeTAndValues(dateTimes, values, isLogCurve);
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

                m_qwtPlotCurve->setSamplesFromTimeAndValues(times, values, isLogCurve);
            }
           
        }
        else
        {
            m_qwtPlotCurve->setSamplesFromTimeTAndValues(std::vector<time_t>(), std::vector<double>(), isLogCurve);
        }

        updateZoomInParentPlot();

        if (m_parentQwtPlot) m_parentQwtPlot->replot();
    }

    updateQwtPlotAxis();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    RimPlotCurve::updateOptionSensitivity();

    caf::PdmUiGroup* curveDataGroup = uiOrdering.addNewGroup("Summary Vector");
    curveDataGroup->add(&m_summaryCase);
    curveDataGroup->add(&m_selectedVariableDisplayField);

    caf::PdmUiGroup* curveVarSelectionGroup = curveDataGroup->addNewGroup("Vector Selection");
    curveVarSelectionGroup->setCollapsedByDefault(true);
    m_summaryFilter->uiOrdering(uiConfigName, *curveVarSelectionGroup);
    curveVarSelectionGroup->add(&m_uiFilterResultSelection);

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
void RimSummaryCurve::updateQwtPlotAxis()
{
    if (m_qwtPlotCurve)
    {
        if (this->yAxis() == RimDefines::PLOT_AXIS_LEFT)
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

    if(changedField == &m_uiFilterResultSelection)
    {
        if (0 <= m_uiFilterResultSelection() && static_cast<size_t>(m_uiFilterResultSelection()) < summaryReader()->allResultAddresses().size())
        {
            m_curveVariable->setAddress(summaryReader()->allResultAddresses()[m_uiFilterResultSelection()]);
        }
        else
        {
            m_curveVariable->setAddress(RifEclipseSummaryAddress());
        }

        this->loadDataAndUpdate();

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
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifReaderEclipseSummary* RimSummaryCurve::summaryReader() const
{
    if (!m_summaryCase()) return nullptr;

    if (!m_summaryCase->caseData()) return nullptr;

    return m_summaryCase()->caseData()->summaryReader();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/)
{
    
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimSummaryCurve::curveData(std::vector<QDateTime>* timeSteps, std::vector<double>* values) const
{
    CVF_ASSERT(timeSteps && values);

    RifReaderEclipseSummary* reader = summaryReader();
   
    if (!reader) return false;

    std::vector<time_t> times = reader->timeSteps();
    *timeSteps = RifReaderEclipseSummary::fromTimeT(times);

    if (!times.size()) return false;

    RifEclipseSummaryAddress addr = m_curveVariable()->address();
    return reader->values(addr, values);
}

