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

CAF_PDM_SOURCE_INIT(RimSummaryAddress, "SummaryAddress");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryAddress::RimSummaryAddress()
{

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

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RimSummaryAddress::address()
{
    return RifEclipseSummaryAddress("");
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
    addItem(RifEclipseSummaryAddress::SUMMARY_WELL_SEGMENT_RIVER,   "SUMMARY_SEGMENT_RIVER",       "Segment River");
    addItem(RifEclipseSummaryAddress::SUMMARY_BLOCK,                "SUMMARY_BLOCK",               "Block");
    addItem(RifEclipseSummaryAddress::SUMMARY_BLOCK_LGR,            "SUMMARY_BLOCK_LGR",           "Lgr-Block");
    setDefault(RifEclipseSummaryAddress::SUMMARY_FIELD);
}

}


namespace caf
{

template<>
void caf::AppEnum<RimSummaryCurve::SummaryFilterType>::setUp()
{
    addItem(RimSummaryCurve::SUM_FILTER_ANY,                  "SUM_FILTER_ANY",                 "Any");
    addItem(RimSummaryCurve::SUM_FILTER_FIELD,                "SUM_FILTER_FIELD",               "Field");
    addItem(RimSummaryCurve::SUM_FILTER_AQUIFER,              "SUM_FILTER_AQUIFER",             "Aquifer");
    addItem(RimSummaryCurve::SUM_FILTER_NETWORK,              "SUM_FILTER_NETWORK",             "Network");
    addItem(RimSummaryCurve::SUM_FILTER_MISC,                 "SUM_FILTER_MISC",                "Misc");
    addItem(RimSummaryCurve::SUM_FILTER_REGION,               "SUM_FILTER_REGION",              "Region");
    addItem(RimSummaryCurve::SUM_FILTER_REGION_2_REGION,      "SUM_FILTER_REGION_2_REGION",     "Region-Region");
    addItem(RimSummaryCurve::SUM_FILTER_WELL_GROUP,           "SUM_FILTER_WELL_GROUP",          "Group");
    addItem(RimSummaryCurve::SUM_FILTER_WELL,                 "SUM_FILTER_WELL",                "Well");
    addItem(RimSummaryCurve::SUM_FILTER_WELL_COMPLETION,      "SUM_FILTER_WELL_COMPLETION",     "Completion");
    addItem(RimSummaryCurve::SUM_FILTER_WELL_COMPLETION_LGR,  "SUM_FILTER_WELL_COMPLETION_LGR", "Lgr-Completion");
    addItem(RimSummaryCurve::SUM_FILTER_WELL_LGR,             "SUM_FILTER_WELL_LGR",            "Lgr-Well");
    addItem(RimSummaryCurve::SUM_FILTER_WELL_SEGMENT,         "SUM_FILTER_SEGMENT",             "Segment");
    addItem(RimSummaryCurve::SUM_FILTER_WELL_SEGMENT_RIVER,   "SUM_FILTER_SEGMENT_RIVER",       "Segment River");
    addItem(RimSummaryCurve::SUM_FILTER_BLOCK,                "SUM_FILTER_BLOCK",               "Block");
    addItem(RimSummaryCurve::SUM_FILTER_BLOCK_LGR,            "SUM_FILTER_BLOCK_LGR",           "Lgr-Block");
    setDefault(RimSummaryCurve::SUM_FILTER_FIELD);
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

    CAF_PDM_InitFieldNoDefault(&m_filterType,"SummaryVarCategory","Category","","","");
    m_filterType.xmlCapability()->setIOWritable(false);
    m_filterType.xmlCapability()->setIOReadable(false);

   
    CAF_PDM_InitFieldNoDefault(&m_filterQuantityName,"SummaryVarQuantity","Quantity","","","");
    m_filterQuantityName.xmlCapability()->setIOWritable(false);
    m_filterQuantityName.xmlCapability()->setIOReadable(false);

    CAF_PDM_InitFieldNoDefault(&m_regionNumberFilter        ,"SummaryRegionNumber","Region","","","");
    m_regionNumberFilter.xmlCapability()->setIOWritable(false);
    m_regionNumberFilter.xmlCapability()->setIOReadable(false);
    CAF_PDM_InitFieldNoDefault(&m_regionNumber2Filter       ,"SummaryRegionNumber2","Region 2","","","");
    m_regionNumber2Filter.xmlCapability()->setIOWritable(false);
    m_regionNumber2Filter.xmlCapability()->setIOReadable(false);
    CAF_PDM_InitFieldNoDefault(&m_wellGroupNameFilter, "SummaryWellGroupName", "Well Group", "", "", "");
    m_wellGroupNameFilter.xmlCapability()->setIOWritable(false);
    m_wellGroupNameFilter.xmlCapability()->setIOReadable(false);
    CAF_PDM_InitFieldNoDefault(&m_wellNameFilter            ,"SummaryWellName","Well","","","");
    m_wellNameFilter.xmlCapability()->setIOWritable(false);
    m_wellNameFilter.xmlCapability()->setIOReadable(false);
    CAF_PDM_InitFieldNoDefault(&m_wellSegmentNumberFilter   ,"SummaryWellSegmentNumber","Segment","","","");
    m_wellSegmentNumberFilter.xmlCapability()->setIOWritable(false);
    m_wellSegmentNumberFilter.xmlCapability()->setIOReadable(false);
    CAF_PDM_InitFieldNoDefault(&m_lgrNameFilter             ,"SummaryLgrName","Lgr","","","");
    m_lgrNameFilter.xmlCapability()->setIOWritable(false);
    m_lgrNameFilter.xmlCapability()->setIOReadable(false);
    CAF_PDM_InitFieldNoDefault(&m_cellIJKFilter               ,"SummaryCellIJK","I, J, K","","","");
    m_cellIJKFilter.xmlCapability()->setIOWritable(false);
    m_cellIJKFilter.xmlCapability()->setIOReadable(false);

    CAF_PDM_InitFieldNoDefault(&m_uiFilterResultSelection, "FilterResultSelection", "Vectors", "", "", "");
    m_cellIJKFilter.xmlCapability()->setIOWritable(false);
    m_cellIJKFilter.xmlCapability()->setIOReadable(false);
    m_uiFilterResultSelection.uiCapability()->setUiEditorTypeName(caf::PdmUiListEditor::uiEditorTypeName());
    m_uiFilterResultSelection.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::TOP);

    CAF_PDM_InitFieldNoDefault(&m_curveVariable, "SummaryAddress", "SummaryAddress", "", "", "");
    m_curveVariable.uiCapability()->setUiHidden(true);
    m_curveVariable = new RimSummaryAddress;

    updateOptionSensitivity();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCurve::~RimSummaryCurve()
{
    delete m_curveVariable();
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
    else if(fieldNeedingOptions == &m_uiFilterResultSelection)
    {
        if(m_summaryCase)
        {
            RifReaderEclipseSummary* reader = summaryReader();
            if(reader)
            {
                std::vector<std::string> varNames = reader->variableNames();

                for(size_t i = 0; i < varNames.size(); i++)
                {
                    std::string name = varNames[i];

                    QString s = QString::fromStdString(name);
                    optionList.push_back(caf::PdmOptionItemInfo(s, i));
                }
            }

            optionList.push_front(caf::PdmOptionItemInfo(RimDefines::undefinedResultName(), -1));

            if(useOptionsOnly) *useOptionsOnly = true;
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
    //curveDataGroup->add(&m_variableName);
    caf::PdmUiGroup* curveVarFilterGroup = curveDataGroup->addNewGroup("Filter");

    curveVarFilterGroup->add(&m_filterType);
    curveVarFilterGroup->add(&m_filterQuantityName);
    switch (m_filterType())
    {
        case SUM_FILTER_ANY:
        {
            curveVarFilterGroup->add(&m_regionNumberFilter);
            curveVarFilterGroup->add(&m_regionNumber2Filter);
            curveVarFilterGroup->add(&m_wellGroupNameFilter);
            curveVarFilterGroup->add(&m_wellNameFilter);
            curveVarFilterGroup->add(&m_wellSegmentNumberFilter);
            curveVarFilterGroup->add(&m_lgrNameFilter);
            curveVarFilterGroup->add(&m_cellIJKFilter);
        }
        break;
        case SUM_FILTER_REGION:
        {
            curveVarFilterGroup->add(&m_regionNumberFilter);
        }
        break;
        case SUM_FILTER_REGION_2_REGION:
        {
            curveVarFilterGroup->add(&m_regionNumberFilter);
            curveVarFilterGroup->add(&m_regionNumber2Filter);

        }
        break;
        case SUM_FILTER_WELL_GROUP:
        {
            curveVarFilterGroup->add(&m_wellGroupNameFilter);

        }
        break;
        case SUM_FILTER_WELL:
        {
            curveVarFilterGroup->add(&m_wellNameFilter);

        }
        break;
        case SUM_FILTER_WELL_COMPLETION:
        {
            curveVarFilterGroup->add(&m_wellNameFilter);
            curveVarFilterGroup->add(&m_cellIJKFilter);
            
        }
        break;
        case SUM_FILTER_WELL_LGR:
        {
            curveVarFilterGroup->add(&m_wellNameFilter);
            curveVarFilterGroup->add(&m_lgrNameFilter);
        }
        break;
        case SUM_FILTER_WELL_COMPLETION_LGR:
        {
            curveVarFilterGroup->add(&m_wellNameFilter);
            curveVarFilterGroup->add(&m_lgrNameFilter);
            curveVarFilterGroup->add(&m_cellIJKFilter);
        }
        break;
        case SUM_FILTER_WELL_SEGMENT:
        {
            curveVarFilterGroup->add(&m_wellNameFilter);
            curveVarFilterGroup->add(&m_wellSegmentNumberFilter);
        }
        break;
        case SUM_FILTER_BLOCK:
        {
            curveVarFilterGroup->add(&m_cellIJKFilter);
        }
        break;
        case SUM_FILTER_BLOCK_LGR:
        {
            curveVarFilterGroup->add(&m_lgrNameFilter);
            curveVarFilterGroup->add(&m_cellIJKFilter);
        }
        break;

    }
    curveDataGroup->add(&m_uiFilterResultSelection);

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
