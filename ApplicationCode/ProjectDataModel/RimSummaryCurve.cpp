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
#include "RimDefines.h"
#include "RimEclipseResultCase.h"
#include "RimProject.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"
#include "RiuSummaryQwtPlot.h"

#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiListEditor.h"
#include "cafPdmUiTreeOrdering.h"
#include "RiuLineSegmentQwtPlotCurve.h"
#include "qwt_date.h"
#include "RimSummaryCase.h"
#include "RigSummaryCaseData.h"

namespace caf
{

template<>
void caf::AppEnum<RifEclipseSummaryAddress::SummaryVarCategory>::setUp()
{
    addItem(RifEclipseSummaryAddress::SUMMARY_WELL,            "SUMMARY_WELL",           "Well");
    addItem(RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION, "SUMMARY_WELL_COMPLETION","Completion");
    addItem(RifEclipseSummaryAddress::SUMMARY_GROUP,           "SUMMARY_GROUP",          "Group");
    addItem(RifEclipseSummaryAddress::SUMMARY_FIELD,           "SUMMARY_FIELD",          "Field");
    addItem(RifEclipseSummaryAddress::SUMMARY_REGION,          "SUMMARY_REGION",         "Region");
    addItem(RifEclipseSummaryAddress::SUMMARY_MISC,            "SUMMARY_MISC",           "Misc");
    addItem(RifEclipseSummaryAddress::SUMMARY_BLOCK,           "SUMMARY_BLOCK",          "Block");
    addItem(RifEclipseSummaryAddress::SUMMARY_BLOCK_LGR,       "SUMMARY_BLOCK_LGR",      "LGR Block");
    addItem(RifEclipseSummaryAddress::SUMMARY_AQUIFIER,        "SUMMARY_AQUIFIER",       "Aquifier");
    addItem(RifEclipseSummaryAddress::SUMMARY_SEGMENT,         "SUMMARY_SEGMENT",        "Segment");
    addItem(RifEclipseSummaryAddress::SUMMARY_SEGMENT_RIVER,   "SUMMARY_SEGMENT_RIVER",  "Segment River");
    setDefault(RifEclipseSummaryAddress::SUMMARY_FIELD);
}

}

CAF_PDM_SOURCE_INIT(RimSummaryCurve, "SummaryCurve");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCurve::RimSummaryCurve()
{
    CAF_PDM_InitObject("Summary Curve", ":/WellLogCurve16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_summaryCase, "SummaryCase", "Summary Case", "", "", "");
    m_summaryCase.uiCapability()->setUiChildrenHidden(true);
    
    // TODO: Implement setUiTreeHidden 
    //m_eclipseCase.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_variableName, "SummaryVariableName", "Variable Name", "", "", "");
    m_variableName.uiCapability()->setUiEditorTypeName(caf::PdmUiComboBoxEditor::uiEditorTypeName());

    CAF_PDM_InitFieldNoDefault(&m_category,"SummaryVarCategory","Category","","","");
    m_category.xmlCapability()->setIOWritable(false);
    m_category.xmlCapability()->setIOReadable(false);

    CAF_PDM_InitFieldNoDefault(&m_simulationItemName,"SummaryVarItem","Item","","","");
    m_simulationItemName.xmlCapability()->setIOWritable(false);
    m_simulationItemName.xmlCapability()->setIOReadable(false);
    
    CAF_PDM_InitFieldNoDefault(&m_quantityName,"SummaryVarQuantity","Quantity","","","");
    m_quantityName.xmlCapability()->setIOWritable(false);
    m_quantityName.xmlCapability()->setIOReadable(false);

    updateOptionSensitivity();
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
void RimSummaryCurve::setVariable(QString varName)
{
    m_variableName = varName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimSummaryCurve::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> optionList = this->RimPlotCurve::calculateValueOptions(fieldNeedingOptions,useOptionsOnly);
    if (!optionList.isEmpty()) return optionList;

    if (fieldNeedingOptions == &m_variableName)
    {
        if (m_summaryCase)
        {
            RifReaderEclipseSummary* reader = summaryReader();
            if (reader)
            {
                std::vector<std::string> varNames = reader->variableNames();

                for (size_t i = 0; i < varNames.size(); i++)
                {
                    std::string name = varNames[i];

                    QString s = QString::fromStdString(name);
                    optionList.push_back(caf::PdmOptionItemInfo(s, s));
                }
            }

            optionList.push_front(caf::PdmOptionItemInfo(RimDefines::undefinedResultName(), RimDefines::undefinedResultName()));

            if (useOptionsOnly) *useOptionsOnly = true;
        }
    }
    else if (fieldNeedingOptions == &m_summaryCase)
    {
        RimProject* proj = RiaApplication::instance()->project();
        std::vector<RimSummaryCase*> cases;

        proj->allSummaryCases(cases);

        for (size_t i = 0; i < cases.size(); i++)
        {
            RimSummaryCase* rimCase = cases[i];

            optionList.push_back(caf::PdmOptionItemInfo(rimCase->caseName(), QVariant::fromValue(caf::PdmPointer<caf::PdmObjectHandle>(rimCase))));
        }

        if (optionList.size() > 0)
        {
            optionList.push_front(caf::PdmOptionItemInfo("None", QVariant::fromValue(caf::PdmPointer<caf::PdmObjectHandle>(NULL))));
        }
    }
    else if (fieldNeedingOptions == &m_simulationItemName)
    {
        RifReaderEclipseSummary* reader = summaryReader();
        if(reader)
        {
            const std::vector<RifEclipseSummaryAddress>& addrs = reader->allResultAddresses();
        }
    }

    return optionList;

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimSummaryCurve::createCurveAutoName()
{
    return m_variableName();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::zoomAllParentPlot()
{
    // Todo

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::onLoadDataAndUpdate()
{
    this->RimPlotCurve::updateCurvePresentation();

    if (isCurveVisible())
    {
        std::vector<QDateTime> dateTimes;
        std::vector<double> values;

        this->curveData(&dateTimes, &values);

        m_qwtPlotCurve->setSamplesFromDateAndValues(dateTimes, values);

        zoomAllParentPlot();

        if (m_parentQwtPlot) m_parentQwtPlot->replot();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    caf::PdmUiGroup* curveDataGroup = uiOrdering.addNewGroup("Curve Data");
    curveDataGroup->add(&m_summaryCase);
    curveDataGroup->add(&m_variableName);
    //curveDataGroup->add(&m_category);
    //curveDataGroup->add(&m_simulationItemName);
    //curveDataGroup->add(&m_quantityName);

    caf::PdmUiGroup* appearanceGroup = uiOrdering.addNewGroup("Appearance");
    appearanceGroup->add(&m_curveColor);
    appearanceGroup->add(&m_curveThickness);
    appearanceGroup->add(&m_pointSymbol);
    appearanceGroup->add(&m_lineStyle);
    appearanceGroup->add(&m_curveName);
    appearanceGroup->add(&m_isUsingAutoName);

    uiOrdering.setForgetRemainingFields(true); // For now. 
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    this->RimPlotCurve::fieldChangedByUi(changedField,oldValue,newValue);

    if (changedField == &m_variableName)
    {
        this->loadDataAndUpdate();
    }
    else if (changedField = &m_summaryCase)
    {
        this->loadDataAndUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifReaderEclipseSummary* RimSummaryCurve::summaryReader()
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
void RimSummaryCurve::curveData(std::vector<QDateTime>* timeSteps, std::vector<double>* values)
{
    RifReaderEclipseSummary* reader = summaryReader();
    
    if (!reader) return;

    if (timeSteps)
    {
        std::vector<time_t> times = reader->timeSteps();
        *timeSteps = RifReaderEclipseSummary::fromTimeT(times);
    }

    if (values)
    {
        std::string keyword = m_variableName().toStdString();
        reader->values(keyword, values);
    }
}
