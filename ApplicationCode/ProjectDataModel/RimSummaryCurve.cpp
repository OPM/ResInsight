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
#include "RimSummaryFilter.h"

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
    m_summaryCase.uiCapability()->setUiChildrenHidden(true);
    
    CAF_PDM_InitFieldNoDefault(&m_selectedVariableDisplayField, "SelectedVariableDisplayVar", "Vector", "", "", "");
    m_selectedVariableDisplayField.xmlCapability()->setIOWritable(false);
    m_selectedVariableDisplayField.xmlCapability()->setIOReadable(false);
    m_selectedVariableDisplayField.uiCapability()->setUiReadOnly(true);

    CAF_PDM_InitFieldNoDefault(&m_summaryFilter, "VarListFilter", "Filter", "", "", "");
    m_summaryFilter.xmlCapability()->setIOWritable(false);
    m_summaryFilter.xmlCapability()->setIOReadable(false);
    m_summaryFilter.uiCapability()->setUiChildrenHidden(true);
    m_summaryFilter.uiCapability()->setUiHidden(true);

    m_summaryFilter = new RimSummaryFilter();

    CAF_PDM_InitFieldNoDefault(&m_uiFilterResultSelection, "FilterResultSelection", "Filter Result", "", "", "");
    m_uiFilterResultSelection.xmlCapability()->setIOWritable(false);
    m_uiFilterResultSelection.xmlCapability()->setIOReadable(false);
    m_uiFilterResultSelection.uiCapability()->setUiEditorTypeName(caf::PdmUiListEditor::uiEditorTypeName());
    m_uiFilterResultSelection.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);
    m_uiFilterResultSelection.uiCapability()->setAutoAddingOptionFromValue(false);
    

    CAF_PDM_InitFieldNoDefault(&m_curveVariable, "SummaryAddress", "SummaryAddress", "", "", "");
    m_curveVariable.uiCapability()->setUiHidden(true);
    m_curveVariable.uiCapability()->setUiChildrenHidden(true);

    m_curveVariable = new RimSummaryAddress;

    // Add some space before name to indicate these belong to the Auto Name field
    CAF_PDM_InitField(&m_addCaseNameToCurveName, "AddCaseNameToCurveName", true, "   Case Name", "", "", "");
    
    m_symbolSkipPixelDistance = 10.0f;
    m_curveThickness = 2;
    updateOptionSensitivity();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCurve::~RimSummaryCurve()
{
    delete m_curveVariable();
    delete m_summaryFilter();
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
void RimSummaryCurve::setVariable(QString varName)
{
    m_curveVariable->setAddress(RifEclipseSummaryAddress::fieldVarAddress(varName.toStdString()));
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
QList<caf::PdmOptionItemInfo> RimSummaryCurve::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> optionList = this->RimPlotCurve::calculateValueOptions(fieldNeedingOptions,useOptionsOnly);
    if (!optionList.isEmpty()) return optionList;


    if (fieldNeedingOptions == &m_summaryCase)
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
                    optionList.push_back(caf::PdmOptionItemInfo(s, addrIntPair.second));
                }
            }

            optionList.push_front(caf::PdmOptionItemInfo(RimDefines::undefinedResultName(), addressCount));

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
    QString generatedCurveName = QString::fromStdString( m_curveVariable->address().uiText()) + "["+ this->unitName().c_str() + "]";

    if (m_addCaseNameToCurveName && m_summaryCase())
    {
        if (!generatedCurveName.isEmpty())
        {
            generatedCurveName += ", ";
        }

        generatedCurveName += m_summaryCase->shortName();
    }

    return generatedCurveName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::updateZoomInParentPlot()
{
    RimSummaryPlot* plot = nullptr;
    firstAnchestorOrThisOfType(plot);

    plot->updateZoom(); 
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::onLoadDataAndUpdate()
{
    this->RimPlotCurve::updateCurvePresentation();

    m_selectedVariableDisplayField = QString::fromStdString(m_curveVariable->address().uiText());

    if (isCurveVisible())
    {
        std::vector<QDateTime> dateTimes;
        std::vector<double> values;

        if(this->curveData(&dateTimes, &values))
        {
            m_qwtPlotCurve->setSamplesFromDateAndValues(dateTimes, values);
        }
        else
        {
            m_qwtPlotCurve->setSamplesFromDateAndValues(std::vector<QDateTime>(), std::vector<double>());
        }

        updateZoomInParentPlot();

        if (m_parentQwtPlot) m_parentQwtPlot->replot();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    caf::PdmUiGroup* curveDataGroup = uiOrdering.addNewGroup("Summary Vector");
    curveDataGroup->add(&m_summaryCase);
    curveDataGroup->add(&m_selectedVariableDisplayField);

    caf::PdmUiGroup* curveVarSelectionGroup = curveDataGroup->addNewGroup("Vector Selection");
    m_summaryFilter->defineUiOrdering(uiConfigName, *curveVarSelectionGroup);

    curveVarSelectionGroup->add(&m_uiFilterResultSelection);

    caf::PdmUiGroup* appearanceGroup = uiOrdering.addNewGroup("Appearance");
    appearanceGroup->add(&m_curveColor);
    appearanceGroup->add(&m_curveThickness);
    appearanceGroup->add(&m_pointSymbol);
    appearanceGroup->add(&m_symbolSkipPixelDistance);
    appearanceGroup->add(&m_lineStyle);
    appearanceGroup->add(&m_curveName);
    appearanceGroup->add(&m_isUsingAutoName);
    if (m_isUsingAutoName)
    {
        appearanceGroup->add(&m_addCaseNameToCurveName);
    }

    uiOrdering.setForgetRemainingFields(true); // For now. 
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    this->RimPlotCurve::fieldChangedByUi(changedField,oldValue,newValue);
    
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

        RimSummaryPlot* plot = nullptr;
        firstAnchestorOrThisOfType(plot);
        plot->updateYAxisUnit();
    } 
    else if (&m_showCurve == changedField)
    {
        RimSummaryPlot* plot = nullptr;
        firstAnchestorOrThisOfType(plot);
        plot->updateYAxisUnit();
    }
    else if (changedField == &m_addCaseNameToCurveName)
    {
        this->uiCapability()->updateConnectedEditors();
        updateCurveName();
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
bool RimSummaryCurve::curveData(std::vector<QDateTime>* timeSteps, std::vector<double>* values)
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
