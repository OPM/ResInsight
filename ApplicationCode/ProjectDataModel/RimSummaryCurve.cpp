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


namespace caf
{

template<>
void caf::AppEnum<RimSummaryCurve::SummaryFilterType>::setUp()
{
    addItem(RimSummaryCurve::SUM_FILTER_VAR_STRING,           "SUM_FILTER_VAR_STRING",          "Concatenated Variable Text");
    addItem(RimSummaryCurve::SUM_FILTER_FIELD,                "SUM_FILTER_FIELD",               "Field");
    addItem(RimSummaryCurve::SUM_FILTER_WELL,                 "SUM_FILTER_WELL",                "Well");
    addItem(RimSummaryCurve::SUM_FILTER_WELL_GROUP,           "SUM_FILTER_WELL_GROUP",          "Group");
    addItem(RimSummaryCurve::SUM_FILTER_WELL_COMPLETION,      "SUM_FILTER_WELL_COMPLETION",     "Completion");
    addItem(RimSummaryCurve::SUM_FILTER_WELL_SEGMENT,         "SUM_FILTER_SEGMENT",             "Segment");
    addItem(RimSummaryCurve::SUM_FILTER_BLOCK,                "SUM_FILTER_BLOCK",               "Block");
    addItem(RimSummaryCurve::SUM_FILTER_REGION,               "SUM_FILTER_REGION",              "Region");
    addItem(RimSummaryCurve::SUM_FILTER_REGION_2_REGION,      "SUM_FILTER_REGION_2_REGION",     "Region-Region");
    addItem(RimSummaryCurve::SUM_FILTER_WELL_LGR,             "SUM_FILTER_WELL_LGR",            "Lgr-Well");
    addItem(RimSummaryCurve::SUM_FILTER_WELL_COMPLETION_LGR,  "SUM_FILTER_WELL_COMPLETION_LGR", "Lgr-Completion");
    addItem(RimSummaryCurve::SUM_FILTER_BLOCK_LGR,            "SUM_FILTER_BLOCK_LGR",           "Lgr-Block");
    addItem(RimSummaryCurve::SUM_FILTER_MISC,                 "SUM_FILTER_MISC",                "Misc");
    addItem(RimSummaryCurve::SUM_FILTER_AQUIFER,              "SUM_FILTER_AQUIFER",             "Aquifer");
    addItem(RimSummaryCurve::SUM_FILTER_NETWORK,              "SUM_FILTER_NETWORK",             "Network");
    addItem(RimSummaryCurve::SUM_FILTER_ANY,                  "SUM_FILTER_ANY",                 "Any");
    setDefault(RimSummaryCurve::SUM_FILTER_VAR_STRING);
}

}

CAF_PDM_SOURCE_INIT(RimSummaryCurve, "SummaryCurve");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCurve::RimSummaryCurve()
{
    CAF_PDM_InitObject("Summary Curve", ":/WellLogCurve16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_summaryCase, "SummaryCase", "Case", "", "", "");
    m_summaryCase.uiCapability()->setUiChildrenHidden(true);
    
    CAF_PDM_InitFieldNoDefault(&m_selectedVariableDisplayField, "SelectedVariableDisplayVar", "Variable", "", "", "");
    m_selectedVariableDisplayField.xmlCapability()->setIOWritable(false);
    m_selectedVariableDisplayField.xmlCapability()->setIOReadable(false);
    m_selectedVariableDisplayField.uiCapability()->setUiReadOnly(true);

    CAF_PDM_InitFieldNoDefault(&m_filterType,"SummaryFilterType","Filter Type","","","");
    m_filterType.xmlCapability()->setIOWritable(false);
    m_filterType.xmlCapability()->setIOReadable(false);

    CAF_PDM_InitFieldNoDefault(&m_completeVarStringFilter, "SummaryCompleteVarStringFilter", "Filter", "", "", "");
    m_completeVarStringFilter.xmlCapability()->setIOWritable(false);
    m_completeVarStringFilter.xmlCapability()->setIOReadable(false);

    CAF_PDM_InitFieldNoDefault(&m_filterQuantityName,"SummaryVarQuantityFilter","Quantity","","","");
    m_filterQuantityName.xmlCapability()->setIOWritable(false);
    m_filterQuantityName.xmlCapability()->setIOReadable(false);

    CAF_PDM_InitFieldNoDefault(&m_regionNumberFilter        ,"SummaryRegionNumberFilter","Region","","","");
    m_regionNumberFilter.xmlCapability()->setIOWritable(false);
    m_regionNumberFilter.xmlCapability()->setIOReadable(false);
    CAF_PDM_InitFieldNoDefault(&m_regionNumber2Filter       ,"SummaryRegionNumber2Filter","Region 2","","","");
    m_regionNumber2Filter.xmlCapability()->setIOWritable(false);
    m_regionNumber2Filter.xmlCapability()->setIOReadable(false);
    CAF_PDM_InitFieldNoDefault(&m_wellGroupNameFilter, "SummaryWellGroupNameFilter", "Well Group", "", "", "");
    m_wellGroupNameFilter.xmlCapability()->setIOWritable(false);
    m_wellGroupNameFilter.xmlCapability()->setIOReadable(false);
    CAF_PDM_InitFieldNoDefault(&m_wellNameFilter            ,"SummaryWellNameFilter","Well","","","");
    m_wellNameFilter.xmlCapability()->setIOWritable(false);
    m_wellNameFilter.xmlCapability()->setIOReadable(false);
    CAF_PDM_InitFieldNoDefault(&m_wellSegmentNumberFilter   ,"SummaryWellSegmentNumberFilter","Segment","","","");
    m_wellSegmentNumberFilter.xmlCapability()->setIOWritable(false);
    m_wellSegmentNumberFilter.xmlCapability()->setIOReadable(false);
    CAF_PDM_InitFieldNoDefault(&m_lgrNameFilter             ,"SummaryLgrNameFilter","Lgr","","","");
    m_lgrNameFilter.xmlCapability()->setIOWritable(false);
    m_lgrNameFilter.xmlCapability()->setIOReadable(false);
    CAF_PDM_InitFieldNoDefault(&m_cellIJKFilter               ,"SummaryCellIJKFilter","I, J, K","","","");
    m_cellIJKFilter.xmlCapability()->setIOWritable(false);
    m_cellIJKFilter.xmlCapability()->setIOReadable(false);

    CAF_PDM_InitFieldNoDefault(&m_uiFilterResultSelection, "FilterResultSelection", "Filter Result", "", "", "");
    m_cellIJKFilter.xmlCapability()->setIOWritable(false);
    m_cellIJKFilter.xmlCapability()->setIOReadable(false);
    m_uiFilterResultSelection.uiCapability()->setUiEditorTypeName(caf::PdmUiListEditor::uiEditorTypeName());
    m_uiFilterResultSelection.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);
    m_uiFilterResultSelection.uiCapability()->setAutoAddingOptionFromValue(false);
    

    CAF_PDM_InitFieldNoDefault(&m_curveVariable, "SummaryAddress", "SummaryAddress", "", "", "");
    m_curveVariable.uiCapability()->setUiHidden(true);
    m_curveVariable.uiCapability()->setUiChildrenHidden(true);

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
    m_curveVariable->setAddress(RifEclipseSummaryAddress::fieldVarAddress(varName.toStdString()));
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
                    if (!isIncludedByFilter(allAddresses[i] )) continue;
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
    return QString::fromStdString( m_curveVariable->address().uiText());
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

        zoomAllParentPlot();

        if (m_parentQwtPlot) m_parentQwtPlot->replot();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    caf::PdmUiGroup* curveDataGroup = uiOrdering.addNewGroup("Summary Variable");
    curveDataGroup->add(&m_summaryCase);
    curveDataGroup->add(&m_selectedVariableDisplayField);

    caf::PdmUiGroup* curveVarSelectionGroup = curveDataGroup->addNewGroup("Variable Selection");
    curveVarSelectionGroup->add(&m_filterType);

    caf::PdmUiGroup* curveVarFilterGroup = nullptr;

    if (m_filterType() == SUM_FILTER_VAR_STRING)
    {
        curveVarSelectionGroup->add(&m_completeVarStringFilter);
    }
    else
    {
        caf::PdmUiGroup* curveVarFilterGroup = curveVarSelectionGroup->addNewGroup("Filter Settings");

        curveVarFilterGroup->add(&m_filterQuantityName);
    
    switch (m_filterType())
    {
        case SUM_FILTER_ANY:
        {
            curveVarFilterGroup->add(&m_wellNameFilter);
            curveVarFilterGroup->add(&m_wellGroupNameFilter);
            curveVarFilterGroup->add(&m_regionNumberFilter);
            curveVarFilterGroup->add(&m_regionNumber2Filter);
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
    }
    curveVarSelectionGroup->add(&m_uiFilterResultSelection);

    caf::PdmUiGroup* appearanceGroup = uiOrdering.addNewGroup("Appearance");
    appearanceGroup->add(&m_curveColor);
    appearanceGroup->add(&m_curveThickness);
    appearanceGroup->add(&m_pointSymbol);
    appearanceGroup->add(&m_lineStyle);
    appearanceGroup->add(&m_curveName);
    appearanceGroup->add(&m_isUsingAutoName);

    uiOrdering.setForgetRemainingFields(true); // For now. 
}

bool isNumberMatch(QString numericalFilterString, int number)
{
    if(numericalFilterString.isEmpty()) return true;

    // Todo: Ranges, and lists
    int filterNumber = numericalFilterString.toInt();
    return number == filterNumber;
}

bool isStringMatch(QString filterString, std::string value)
{
    if(filterString.isEmpty()) return true;

    QRegExp searcher(filterString, Qt::CaseInsensitive, QRegExp::WildcardUnix);
    QString qstrValue = QString::fromStdString(value);
    return searcher.exactMatch(qstrValue);
}

bool isIJKMatch(QString filterString, int cellI, int cellJ, int cellK)
{
    if(filterString.isEmpty()) return true;

    // Todo: Ranges, and lists
    int filterNumber = filterString.toInt();
    return cellI == filterNumber;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimSummaryCurve::isIncludedByFilter(const RifEclipseSummaryAddress& addr)
{
    if (!isSumVarTypeMatchingFilterType(m_filterType(), addr.category())) return false;

    if(m_filterType() == SUM_FILTER_VAR_STRING)
    {
        return isStringMatch(m_completeVarStringFilter(), addr.uiText());
    }

    if (!isStringMatch(m_filterQuantityName(), addr.quantityName())) return false;

    if(m_filterType() == SUM_FILTER_ANY)
    {
        return (isNumberMatch(m_regionNumberFilter(), addr.regionNumber())
            &&  isNumberMatch(m_regionNumber2Filter(), addr.regionNumber2())
            &&  isStringMatch(m_wellGroupNameFilter(), addr.wellGroupName())
            &&  isStringMatch(m_wellNameFilter(), addr.wellName())
            &&  isStringMatch(m_lgrNameFilter(), addr.lgrName())
            &&  isNumberMatch(m_wellSegmentNumberFilter(), addr.wellSegmentNumber())
            &&  isIJKMatch(m_cellIJKFilter(), addr.cellI(), addr.cellJ(), addr.cellK()));
    }

    switch (addr.category())
    {
        case RifEclipseSummaryAddress::SUMMARY_REGION:
        { 
            return isNumberMatch(m_regionNumberFilter(), addr.regionNumber());
        } 
        break;
        case RifEclipseSummaryAddress::SUMMARY_REGION_2_REGION:
        {
            return  isNumberMatch(m_regionNumberFilter(), addr.regionNumber()) 
                 && isNumberMatch(m_regionNumber2Filter(), addr.regionNumber2()); 
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_WELL_GROUP:
        {
            return  isStringMatch(m_wellGroupNameFilter(), addr.wellGroupName());
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_WELL:
        {
            return  isStringMatch(m_wellNameFilter(), addr.wellName());
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION:
        {
            return  isStringMatch(m_wellNameFilter(), addr.wellName()) 
                 && isIJKMatch(m_cellIJKFilter(), addr.cellI(), addr.cellJ(), addr.cellK());
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_WELL_LGR:
        {
            return  isStringMatch(m_wellNameFilter(), addr.wellName())
                 && isStringMatch(m_lgrNameFilter(), addr.lgrName());
        }
        break;

        case RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION_LGR:
        {
            return  isStringMatch(m_wellNameFilter(), addr.wellName()) 
                 && isStringMatch(m_lgrNameFilter(), addr.lgrName()) 
                 && isIJKMatch(m_cellIJKFilter(), addr.cellI(), addr.cellJ(), addr.cellK());
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_WELL_SEGMENT:
        {
            return  isStringMatch(m_wellNameFilter(), addr.wellName())
                 && isNumberMatch(m_wellSegmentNumberFilter(), addr.wellSegmentNumber());
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_BLOCK:
        {
            return  isIJKMatch(m_cellIJKFilter(), addr.cellI(), addr.cellJ(), addr.cellK());
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_BLOCK_LGR:
        {
            return  isStringMatch(m_lgrNameFilter(), addr.lgrName()) 
                 && isIJKMatch(m_cellIJKFilter(), addr.cellI(), addr.cellJ(),addr.cellK());
        }
        break;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimSummaryCurve::isSumVarTypeMatchingFilterType(SummaryFilterType sumFilterType, RifEclipseSummaryAddress::SummaryVarCategory sumVarType)
{
    if (sumVarType == RifEclipseSummaryAddress::SUMMARY_INVALID) return false;
    if (sumFilterType == SUM_FILTER_ANY || sumFilterType == SUM_FILTER_VAR_STRING) return true;

    switch(sumVarType)
    {
        case RifEclipseSummaryAddress::SUMMARY_FIELD:               { return (sumFilterType == SUM_FILTER_FIELD); } break;
        case RifEclipseSummaryAddress::SUMMARY_AQUIFER:             { return (sumFilterType == SUM_FILTER_AQUIFER);  } break;
        case RifEclipseSummaryAddress::SUMMARY_NETWORK:             { return (sumFilterType == SUM_FILTER_NETWORK);  } break;
        case RifEclipseSummaryAddress::SUMMARY_MISC:                { return (sumFilterType == SUM_FILTER_MISC);  } break;
        case RifEclipseSummaryAddress::SUMMARY_REGION:              { return (sumFilterType == SUM_FILTER_REGION);  } break;
        case RifEclipseSummaryAddress::SUMMARY_REGION_2_REGION:     { return (sumFilterType == SUM_FILTER_REGION_2_REGION);  } break;
        case RifEclipseSummaryAddress::SUMMARY_WELL_GROUP:          { return (sumFilterType == SUM_FILTER_WELL_GROUP);  } break;
        case RifEclipseSummaryAddress::SUMMARY_WELL:                { return (sumFilterType == SUM_FILTER_WELL);  } break;
        case RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION:     { return (sumFilterType == SUM_FILTER_WELL_COMPLETION);  } break;
        case RifEclipseSummaryAddress::SUMMARY_WELL_LGR:            { return (sumFilterType == SUM_FILTER_WELL_LGR);  } break;
        case RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION_LGR: { return (sumFilterType == SUM_FILTER_FIELD);  } break;
        case RifEclipseSummaryAddress::SUMMARY_WELL_SEGMENT:        { return (sumFilterType == SUM_FILTER_WELL_SEGMENT);  } break;
        case RifEclipseSummaryAddress::SUMMARY_BLOCK:               { return (sumFilterType == SUM_FILTER_BLOCK);  } break;
        case RifEclipseSummaryAddress::SUMMARY_BLOCK_LGR:           { return (sumFilterType == SUM_FILTER_BLOCK_LGR);  } break;
    }

    return false;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    this->RimPlotCurve::fieldChangedByUi(changedField,oldValue,newValue);

    if(changedField = &m_uiFilterResultSelection)
    {
        if (0 <= m_uiFilterResultSelection() && m_uiFilterResultSelection() < summaryReader()->allResultAddresses().size())
        {
            m_curveVariable->setAddress(summaryReader()->allResultAddresses()[m_uiFilterResultSelection()]);
        }
        else
        {
            m_curveVariable->setAddress(RifEclipseSummaryAddress());
        }

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
