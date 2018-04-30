/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RimSummaryPlotNameHelper.h"

#include "RifEclipseSummaryAddress.h"

#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"

#include "RiuSummaryVectorDescriptionMap.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlotNameHelper::RimSummaryPlotNameHelper() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotNameHelper::clear()
{
    m_summaryCases.clear();

    m_analyzer.clear();

    clearTitleSubStrings();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotNameHelper::appendAddresses(const std::vector<RifEclipseSummaryAddress>& addresses)
{
    m_analyzer.appendAdresses(addresses);

    extractPlotTitleSubStrings();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotNameHelper::appendSummaryCases(const std::vector<RimSummaryCase*>& summaryCases)
{
    m_summaryCases.clear();

    for (auto c : summaryCases)
    {
        m_summaryCases.insert(c);
    }

    extractPlotTitleSubStrings();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotNameHelper::appendEnsembleCases(const std::vector<RimSummaryCaseCollection*>& ensembleCases)
{
    m_ensembleCases.clear();

    for (auto c : ensembleCases)
    {
        m_ensembleCases.insert(c);
    }

    extractPlotTitleSubStrings();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryPlotNameHelper::plotTitle() const
{
    QString title;

    if (!m_titleCaseName.isEmpty())
    {
        if (!title.isEmpty()) title += ", ";
        title += m_titleCaseName;
    }

    if (!m_titleWellName.empty())
    {
        if (!title.isEmpty()) title += ", ";
        title += QString::fromStdString(m_titleWellName);
    }

    if (!m_titleWellGroupName.empty())
    {
        if (!title.isEmpty()) title += ", ";
        title += QString::fromStdString(m_titleWellGroupName);
    }

    if (!m_titleRegion.empty())
    {
        if (!title.isEmpty()) title += ", ";
        title += "Region : " + QString::fromStdString(m_titleRegion);
    }

    if (!m_titleQuantity.empty())
    {
        if (!title.isEmpty()) title += ", ";
        title += QString::fromStdString(RiuSummaryVectorDescriptionMap::instance()->fieldInfo(m_titleQuantity));
    }

    if (title.isEmpty())
    {
        title = "Composed Plot";
    }

    return title;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryPlotNameHelper::isQuantityInTitle() const
{
    return !m_titleQuantity.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryPlotNameHelper::isWellNameInTitle() const
{
    return !m_titleWellName.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryPlotNameHelper::isWellGroupNameInTitle() const
{
    return !m_titleWellGroupName.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryPlotNameHelper::isRegionInTitle() const
{
    return !m_titleRegion.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryPlotNameHelper::isCaseInTitle() const
{
    return !m_titleCaseName.isEmpty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotNameHelper::clearTitleSubStrings()
{
    m_titleQuantity.clear();
    m_titleRegion.clear();
    m_titleWellName.clear();
    m_titleRegion.clear();

    m_titleCaseName.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotNameHelper::extractPlotTitleSubStrings()
{
    clearTitleSubStrings();

    auto quantities     = m_analyzer.quantities();
    auto wellNames      = m_analyzer.wellNames();
    auto wellGroupNames = m_analyzer.wellGroupNames();
    auto regions        = m_analyzer.regionNumbers();

    if (quantities.size() == 1)
    {
        m_titleQuantity = *(quantities.begin());
    }

    if (wellNames.size() == 1)
    {
        m_titleWellName = *(wellNames.begin());
    }

    if (wellGroupNames.size() == 1)
    {
        m_titleWellGroupName = *(wellGroupNames.begin());
    }

    if (regions.size() == 1)
    {
        m_titleRegion = std::to_string(*(regions.begin()));
    }

    {
        if (m_summaryCases.size() == 1 && m_ensembleCases.empty())
        {
            auto summaryCase = *(m_summaryCases.begin());

            m_titleCaseName = summaryCase->caseName();
        }
        else if (m_ensembleCases.size() == 1 && m_summaryCases.empty())
        {
            auto ensembleCase = *(m_ensembleCases.begin());

            m_titleCaseName = ensembleCase->name();
        }
    }
}
