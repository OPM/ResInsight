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

#include "RimSummaryCurveAutoName.h"

#include "RifEclipseSummaryAddress.h"

#include "RimSummaryCurve.h"
#include "RimSummaryCase.h"
#include "RimSummaryCurveFilter.h"

#include "cafPdmUiPushButtonEditor.h"



CAF_PDM_SOURCE_INIT(RimSummaryCurveAutoName, "SummaryCurveAutoName");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCurveAutoName::RimSummaryCurveAutoName()
{
    CAF_PDM_InitObject("RimSummaryCurveAutoName", "", "", "");

    CAF_PDM_InitField(&m_vectorName,        "VectorName",         true, "Vector Name", "", "", "");
    CAF_PDM_InitField(&m_unit,              "Unit",               false,"Unit", "", "", "");
    CAF_PDM_InitField(&m_regionNumber,      "RegionNumber",       true, "Region Number", "", "", "");
    CAF_PDM_InitField(&m_wellGroupName,     "WellGroupName",      true, "Group Name", "", "", "");
    CAF_PDM_InitField(&m_wellName,          "WellName",           true, "Well Name", "", "", "");
    CAF_PDM_InitField(&m_wellSegmentNumber, "WellSegmentNumber",  true, "Well Segment Number", "", "", "");
    CAF_PDM_InitField(&m_lgrName,           "LgrName",            true, "Lgr Name", "", "", "");
    CAF_PDM_InitField(&m_completion,        "Completion",         true, "I, J, K", "", "", "");
    
    CAF_PDM_InitField(&m_caseName,          "CaseName",           true, "Case Name", "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimSummaryCurveAutoName::curveName(const RifEclipseSummaryAddress& summaryAddress) const
{
    std::string text;

    RimSummaryCurve* summaryCurve = nullptr;
    this->firstAncestorOrThisOfType(summaryCurve);

    if (m_vectorName)
    {
        text += summaryAddress.quantityName();

        if (m_unit && summaryCurve)
        {
            text += "[" + summaryCurve->unitName() + "]";
        }
    }

    switch (summaryAddress.category())
    {
        case RifEclipseSummaryAddress::SUMMARY_REGION:
        {
            if (m_regionNumber)
            {
                if (text.size() > 0) text +=":";
                text += std::to_string(summaryAddress.regionNumber());
            }
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_REGION_2_REGION:
        {
            if (m_regionNumber)
            {
                if (text.size() > 0) text += ":";
                text += std::to_string(summaryAddress.regionNumber());
                text += "-" + std::to_string(summaryAddress.regionNumber2());
            }
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_WELL_GROUP:
        {
            if (m_wellGroupName)
            {
                if (text.size() > 0) text += ":";
                text += summaryAddress.wellGroupName();
            }
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_WELL:
        {
            appendWellName(text, summaryAddress);
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION:
        {
            appendWellName(text, summaryAddress);

            if (m_completion)
            {
                if (text.size() > 0) text += ":";
                text += std::to_string(summaryAddress.cellI()) + ", "
                    + std::to_string(summaryAddress.cellJ()) + ", "
                    + std::to_string(summaryAddress.cellK());
            }
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_WELL_LGR:
        {
            appendLgrName(text, summaryAddress);
            appendWellName(text, summaryAddress);
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION_LGR:
        {
            appendLgrName(text, summaryAddress);
            appendWellName(text, summaryAddress);

            if (m_completion)
            {
                if (text.size() > 0) text += ":";
                text += std::to_string(summaryAddress.cellI()) + ", "
                    + std::to_string(summaryAddress.cellJ()) + ", "
                    + std::to_string(summaryAddress.cellK());
            }
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_WELL_SEGMENT:
        {
            appendWellName(text, summaryAddress);

            if (m_wellSegmentNumber)
            {
                if (text.size() > 0) text += ":";
                text += ":" + summaryAddress.wellSegmentNumber();
            }
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_BLOCK:
        {
            if (m_completion)
            {
                if (text.size() > 0) text += ":";
                text += std::to_string(summaryAddress.cellI()) + ", "
                    + std::to_string(summaryAddress.cellJ()) + ", "
                    + std::to_string(summaryAddress.cellK());
            }
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_BLOCK_LGR:
        {
            appendLgrName(text, summaryAddress);

            if (m_completion)
            {
                if (text.size() > 0) text += ":";
                text += std::to_string(summaryAddress.cellI()) + ", "
                    + std::to_string(summaryAddress.cellJ()) + ", "
                    + std::to_string(summaryAddress.cellK());
            }
        }
        break;
    }


    if (summaryCurve)
    {
        if (m_caseName)
        {
            if (summaryCurve && summaryCurve->summaryCase())
            {
                if (text.size() > 0) text += ", ";
                text += summaryCurve->summaryCase()->caseName().toStdString();
            }
        }
    }

    return QString::fromStdString(text);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveAutoName::applySettings(const RimSummaryCurveAutoName& other)
{
    m_caseName          = other.m_caseName;
    m_vectorName        = other.m_vectorName;
    m_unit              = other.m_unit;
    m_regionNumber      = other.m_regionNumber;
    m_wellGroupName     = other.m_wellGroupName;
    m_wellName          = other.m_wellName;
    m_wellSegmentNumber = other.m_wellSegmentNumber;
    m_lgrName           = other.m_lgrName;
    m_completion        = other.m_completion;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveAutoName::appendWellName(std::string& text, const RifEclipseSummaryAddress& summaryAddress) const
{
    if (m_wellName)
    {
        if (text.size() > 0) text += ":";
        text += summaryAddress.wellName();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveAutoName::appendLgrName(std::string& text, const RifEclipseSummaryAddress& summaryAddress) const
{
    if (m_lgrName)
    {
        if (text.size() > 0) text += ":";
        text += ":" + summaryAddress.lgrName();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveAutoName::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    // NOTE: The curve filter is parent object of a summary curve, and the update is supposed to update
    // the first parent, not the grandparent. This is the reason for not using firstAncestorOrThisOfType()

    RimSummaryCurve* summaryCurve = dynamic_cast<RimSummaryCurve*>(this->parentField()->ownerObject());
    if (summaryCurve)
    {
        summaryCurve->updateCurveName();
        summaryCurve->updateConnectedEditors();
    }

    RimSummaryCurveFilter* summaryCurveFilter = dynamic_cast<RimSummaryCurveFilter*>(this->parentField()->ownerObject());
    if (summaryCurveFilter)
    {
        summaryCurveFilter->updateCurveNames();
        summaryCurveFilter->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveAutoName::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_caseName);
    uiOrdering.add(&m_vectorName);
    uiOrdering.add(&m_wellGroupName);
    uiOrdering.add(&m_wellName);
    
    caf::PdmUiGroup& advanced = *(uiOrdering.addNewGroup("Advanced"));
    advanced.setCollapsedByDefault(true);
    advanced.add(&m_regionNumber);
    advanced.add(&m_lgrName);
    advanced.add(&m_completion);
    advanced.add(&m_wellSegmentNumber);
    advanced.add(&m_unit);

    uiOrdering.skipRemainingFields();
}

