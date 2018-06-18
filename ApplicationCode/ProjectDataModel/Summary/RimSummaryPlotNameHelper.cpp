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
    m_ensembleCases.clear();
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
void RimSummaryPlotNameHelper::setSummaryCases(const std::vector<RimSummaryCase*>& summaryCases)
{
    m_summaryCases.clear();

    m_summaryCases.resize(summaryCases.size());

    for (size_t i = 0; i < summaryCases.size(); i++)
    {
        m_summaryCases[i] = summaryCases[i];
    }

    extractPlotTitleSubStrings();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotNameHelper::setEnsembleCases(const std::vector<RimSummaryCaseCollection*>& ensembleCases)
{
    m_ensembleCases.clear();

    m_ensembleCases.resize(ensembleCases.size());

    for (size_t i = 0; i < ensembleCases.size(); i++)
    {
        m_ensembleCases[i] = ensembleCases[i];
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
    auto categories     = m_analyzer.categories();

    if (categories.size() == 1)
    {
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
    }

    auto summaryCases = setOfSummaryCases();
    auto ensembleCases = setOfEnsembleCases();

    if (summaryCases.size() == 1 && ensembleCases.empty())
    {
        auto summaryCase = *(summaryCases.begin());

        if (summaryCase)
        {
            m_titleCaseName = summaryCase->caseName();
        }
    }
    else if (ensembleCases.size() == 1 && summaryCases.empty())
    {
        auto ensembleCase = *(ensembleCases.begin());
        if (ensembleCase)
        {
            m_titleCaseName = ensembleCase->name();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::set<RimSummaryCase*> RimSummaryPlotNameHelper::setOfSummaryCases() const
{
    std::set<RimSummaryCase*> summaryCases;

    for (const auto& sumCase : m_summaryCases)
    {
        if (sumCase) summaryCases.insert(sumCase);
    }

    return summaryCases;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::set<RimSummaryCaseCollection*> RimSummaryPlotNameHelper::setOfEnsembleCases() const
{
    std::set<RimSummaryCaseCollection*> ensembleCases;

    for (const auto& ensemble : m_ensembleCases)
    {
        if (ensemble) ensembleCases.insert(ensemble);
    }

    return ensembleCases;
}
