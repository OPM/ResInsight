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

#include "RiaSummaryCurveAnalyzer.h"

#include "RiaSummaryCurveDefinition.h"

#include "RimSummaryCurve.h"
#include "RimSummaryCurveCollection.h"

#include <QString>

#include <set>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaSummaryCurveAnalyzer::RiaSummaryCurveAnalyzer() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSummaryCurveAnalyzer::analyzeCurves(const RimSummaryCurveCollection* sumCurveCollection)
{
    clearAllSets();

    if (!sumCurveCollection)
        return;

    for (auto curve : sumCurveCollection->curves())
    {
        m_summaryCases.insert(curve->summaryCaseY());

        auto adr = curve->summaryAddressY();
        analyzeAddress(adr);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSummaryCurveAnalyzer::analyzeAdresses(const std::vector<RifEclipseSummaryAddress>& allAddresses)
{
    clearAllSets();

    for (const auto& adr : allAddresses)
    {
        analyzeAddress(adr);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<std::string> RiaSummaryCurveAnalyzer::quantities() const
{
    return m_quantities;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<std::string> RiaSummaryCurveAnalyzer::wellNames() const
{
    return m_wellNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<std::string> RiaSummaryCurveAnalyzer::wellGroupNames() const
{
    return m_wellGroupNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<int> RiaSummaryCurveAnalyzer::regionNumbers() const
{
    return m_regionNumbers;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RimSummaryCase*> RiaSummaryCurveAnalyzer::summaryCases() const
{
    return m_summaryCases;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseSummaryAddress::SummaryVarCategory> RiaSummaryCurveAnalyzer::categories() const
{
    return m_categories;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<QString> RiaSummaryCurveAnalyzer::identifierTexts(RifEclipseSummaryAddress::SummaryVarCategory category) const
{
    std::set<QString> stringSet;

    if (category == RifEclipseSummaryAddress::SUMMARY_REGION)
    {
        for (const auto& regionNumber : m_regionNumbers)
        {
            stringSet.insert(QString::number(regionNumber));
        }
    }
    else if (category == RifEclipseSummaryAddress::SUMMARY_WELL)
    {
        for (const auto& wellName : m_wellNames)
        {
            stringSet.insert(QString::fromStdString(wellName));
        }
    }
    else if (category == RifEclipseSummaryAddress::SUMMARY_WELL_GROUP)
    {
        for (const auto& wellGroupName : m_wellGroupNames)
        {
            stringSet.insert(QString::fromStdString(wellGroupName));
        }
    }

    return stringSet;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RifEclipseSummaryAddress> RiaSummaryCurveAnalyzer::addressesForCategory(const std::vector<RifEclipseSummaryAddress>& addresses, RifEclipseSummaryAddress::SummaryVarCategory category)
{
    std::vector<RifEclipseSummaryAddress> filteredAddresses;

    for (const auto& adr : addresses)
    {
        if (adr.category() == category)
        {
            filteredAddresses.push_back(adr);
        }
    }

    return filteredAddresses;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSummaryCurveAnalyzer::clearAllSets()
{
    m_quantities.clear();
    m_wellNames.clear();
    m_wellGroupNames.clear();
    m_regionNumbers.clear();
    m_summaryCases.clear();
    m_categories.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSummaryCurveAnalyzer::analyzeAddress(const RifEclipseSummaryAddress& address)
{
    if (!address.wellName().empty())
    {
        m_wellNames.insert(address.wellName());
    }

    if (!address.quantityName().empty())
    {
        m_quantities.insert(address.quantityName());
    }

    if (!address.wellGroupName().empty())
    {
        m_wellGroupNames.insert(address.wellGroupName());
    }

    if (address.regionNumber() != -1)
    {
        m_regionNumbers.insert(address.regionNumber());
    }

    if (address.category() != RifEclipseSummaryAddress::SUMMARY_INVALID)
    {
        m_categories.insert(address.category());
    }
}
