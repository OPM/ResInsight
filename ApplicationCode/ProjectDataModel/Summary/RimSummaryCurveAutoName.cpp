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

#include "RimEnsembleCurveSet.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryCurve.h"
#include "RimSummaryPlotNameHelper.h"

#include "SummaryPlotCommands/RicSummaryCurveCreator.h"

#include "cafPdmUiPushButtonEditor.h"

CAF_PDM_SOURCE_INIT(RimSummaryCurveAutoName, "SummaryCurveAutoName");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCurveAutoName::RimSummaryCurveAutoName()
{
    // clang-format off
    CAF_PDM_InitObject("RimSummaryCurveAutoName", "", "", "");

    CAF_PDM_InitField(&m_vectorName,        "VectorName",         true, "Vector Name", "", "", "");
    CAF_PDM_InitField(&m_unit,              "Unit",               false,"Unit", "", "", "");
    CAF_PDM_InitField(&m_regionNumber,      "RegionNumber",       true, "Region Number", "", "", "");
    CAF_PDM_InitField(&m_wellGroupName,     "WellGroupName",      true, "Group Name", "", "", "");
    CAF_PDM_InitField(&m_wellName,          "WellName",           true, "Well Name", "", "", "");
    CAF_PDM_InitField(&m_wellSegmentNumber, "WellSegmentNumber",  true, "Well Segment Number", "", "", "");
    CAF_PDM_InitField(&m_lgrName,           "LgrName",            true, "Lgr Name", "", "", "");
    CAF_PDM_InitField(&m_completion,        "Completion",         true, "I, J, K", "", "", "");
    CAF_PDM_InitField(&m_aquiferNumber,     "Aquifer",            true, "Aquifer Number", "", "", "");
    
    CAF_PDM_InitField(&m_caseName,          "CaseName",           true, "Case/Ensemble Name", "", "", "");

    // clang-format on
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryCurveAutoName::curveNameY(const RifEclipseSummaryAddress& summaryAddress,
                                            const RimSummaryPlotNameHelper* nameHelper) const
{
    std::string text;

    RimSummaryCurve* summaryCurve = nullptr;
    this->firstAncestorOrThisOfType(summaryCurve);

    if (m_vectorName)
    {
        bool skipSubString = nameHelper && nameHelper->isQuantityInTitle();
        if (!skipSubString)
        {
            text += summaryAddress.quantityName();

            if (m_unit && summaryCurve && !summaryCurve->unitNameY().empty())
            {
                text += "[" + summaryCurve->unitNameY() + "]";
            }
        }
    }

    appendAddressDetails(text, summaryAddress, nameHelper);

    QString caseName;

    {
        RimEnsembleCurveSet* ensembleCurveSet = nullptr;
        this->firstAncestorOrThisOfType(ensembleCurveSet);
        if (ensembleCurveSet && ensembleCurveSet->summaryCaseCollection())
        {
            caseName = ensembleCurveSet->summaryCaseCollection()->name();
        }
    }

    if (caseName.isEmpty() && summaryCurve && summaryCurve->summaryCaseY())
    {
        caseName = summaryCurve->summaryCaseY()->caseName();
    }

    if (!caseName.isEmpty())
    {
        bool skipSubString = nameHelper && nameHelper->isCaseInTitle();

        if (m_caseName && !skipSubString)
        {
            if (!text.empty()) text += ", ";
            text += caseName.toStdString();
        }
    }

    return QString::fromStdString(text);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryCurveAutoName::curveNameX(const RifEclipseSummaryAddress& summaryAddress,
                                            const RimSummaryPlotNameHelper* nameHelper) const
{
    std::string text;

    RimSummaryCurve* summaryCurve = nullptr;
    this->firstAncestorOrThisOfType(summaryCurve);

    if (m_vectorName)
    {
        bool skipSubString = nameHelper && nameHelper->isQuantityInTitle();
        if (!skipSubString)
        {
            text += summaryAddress.quantityName();

            if (m_unit && summaryCurve && !summaryCurve->unitNameX().empty())
            {
                text += "[" + summaryCurve->unitNameX() + "]";
            }
        }
    }

    appendAddressDetails(text, summaryAddress, nameHelper);

    if (summaryCurve && summaryCurve->summaryCaseX())
    {
        QString caseName = summaryCurve->summaryCaseX()->caseName();

        bool skipSubString = nameHelper && nameHelper->isCaseInTitle();

        if (m_caseName && !skipSubString)
        {
            if (summaryCurve && summaryCurve->summaryCaseX())
            {
                if (!text.empty()) text += ", ";
                text += caseName.toStdString();
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
    m_aquiferNumber     = other.m_aquiferNumber;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveAutoName::appendWellName(std::string&                    text,
                                             const RifEclipseSummaryAddress& summaryAddress,
                                             const RimSummaryPlotNameHelper* nameHelper) const
{
    bool skipSubString = nameHelper && nameHelper->isWellNameInTitle();
    if (skipSubString) return;

    if (m_wellName)
    {
        if (!text.empty()) text += ":";
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
        if (!text.empty()) text += ":";
        text += ":" + summaryAddress.lgrName();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveAutoName::appendAddressDetails(std::string&                    text,
                                                   const RifEclipseSummaryAddress& summaryAddress,
                                                   const RimSummaryPlotNameHelper* nameHelper) const
{
    switch (summaryAddress.category())
    {
        case RifEclipseSummaryAddress::SUMMARY_AQUIFER:
        {
            if (m_aquiferNumber)
            {
                if (!text.empty()) text += ":";
                text += std::to_string(summaryAddress.aquiferNumber());
            }
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_REGION:
        {
            if (m_regionNumber)
            {
                bool skipSubString = nameHelper && nameHelper->isRegionInTitle();
                if (!skipSubString)
                {
                    if (!text.empty()) text += ":";
                    text += std::to_string(summaryAddress.regionNumber());
                }
            }
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_REGION_2_REGION:
        {
            if (m_regionNumber)
            {
                if (!text.empty()) text += ":";
                text += std::to_string(summaryAddress.regionNumber());
                text += "-" + std::to_string(summaryAddress.regionNumber2());
            }
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_WELL_GROUP:
        {
            if (m_wellGroupName)
            {
                bool skipSubString = nameHelper && nameHelper->isWellGroupNameInTitle();
                if (!skipSubString)
                {
                    if (!text.empty()) text += ":";
                    text += summaryAddress.wellGroupName();
                }
            }
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_WELL:
        {
            appendWellName(text, summaryAddress, nameHelper);
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION:
        {
            appendWellName(text, summaryAddress, nameHelper);

            if (m_completion)
            {
                if (!text.empty()) text += ":";
                text += std::to_string(summaryAddress.cellI()) + ", " + std::to_string(summaryAddress.cellJ()) + ", " +
                        std::to_string(summaryAddress.cellK());
            }
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_WELL_LGR:
        {
            appendLgrName(text, summaryAddress);
            appendWellName(text, summaryAddress, nameHelper);
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION_LGR:
        {
            appendLgrName(text, summaryAddress);
            appendWellName(text, summaryAddress, nameHelper);

            if (m_completion)
            {
                if (!text.empty()) text += ":";
                text += std::to_string(summaryAddress.cellI()) + ", " + std::to_string(summaryAddress.cellJ()) + ", " +
                        std::to_string(summaryAddress.cellK());
            }
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_WELL_SEGMENT:
        {
            appendWellName(text, summaryAddress, nameHelper);

            if (m_wellSegmentNumber)
            {
                if (!text.empty()) text += ":";
                text += ":" + summaryAddress.wellSegmentNumber();
            }
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_BLOCK:
        {
            if (m_completion)
            {
                if (!text.empty()) text += ":";
                text += std::to_string(summaryAddress.cellI()) + ", " + std::to_string(summaryAddress.cellJ()) + ", " +
                        std::to_string(summaryAddress.cellK());
            }
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_BLOCK_LGR:
        {
            appendLgrName(text, summaryAddress);

            if (m_completion)
            {
                if (!text.empty()) text += ":";
                text += std::to_string(summaryAddress.cellI()) + ", " + std::to_string(summaryAddress.cellJ()) + ", " +
                        std::to_string(summaryAddress.cellK());
            }
        }
        break;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveAutoName::fieldChangedByUi(const caf::PdmFieldHandle* changedField,
                                               const QVariant&            oldValue,
                                               const QVariant&            newValue)
{
    // NOTE: The curve filter is parent object of a summary curve, and the update is supposed to update
    // the first parent, not the grandparent. This is the reason for not using firstAncestorOrThisOfType()

    RimSummaryCurve* summaryCurve = dynamic_cast<RimSummaryCurve*>(this->parentField()->ownerObject());
    if (summaryCurve)
    {
        summaryCurve->updateCurveNameAndUpdatePlotLegend();
        summaryCurve->updateConnectedEditors();

        return;
    }

    RicSummaryCurveCreator* curveCreator = dynamic_cast<RicSummaryCurveCreator*>(this->parentField()->ownerObject());
    if (curveCreator)
    {
        curveCreator->updateCurveNames();
        curveCreator->updateConnectedEditors();

        return;
    }

    {
        auto ensembleCurveSet = dynamic_cast<RimEnsembleCurveSet*>(this->parentField()->ownerObject());
        if (ensembleCurveSet)
        {
            ensembleCurveSet->updateAllTextInPlot();
            ensembleCurveSet->updateConnectedEditors();

            return;
        }
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
    advanced.add(&m_aquiferNumber);
    advanced.add(&m_unit);

    uiOrdering.skipRemainingFields();
}
