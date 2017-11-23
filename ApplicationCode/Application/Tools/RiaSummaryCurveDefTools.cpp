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

#include "RiaSummaryCurveDefTools.h"

#include "RiaSummaryCurveDefinition.h"

#include "RimSummaryCurve.h"
#include "RimSummaryCurveCollection.h"

#include <set>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaSummaryCurveDefTools::RiaSummaryCurveDefTools() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSummaryCurveDefTools::findIdentifiers(RimSummaryCurveCollection* sumCurveCollection)
{
    if (!sumCurveCollection) return;

    for (auto curve : sumCurveCollection->curves())
    {
        m_summaryCases.insert(curve->summaryCaseY());

        auto adr = curve->summaryAddressY();

        if (!adr.wellName().empty())
        {
            m_wellNames.insert(adr.wellName());
        }

        if (!adr.quantityName().empty())
        {
            m_quantities.insert(adr.quantityName());
        }

        if (!adr.wellGroupName().empty())
        {
            m_wellGroupNames.insert(adr.wellGroupName());
        }

        if (adr.regionNumber() != -1)
        {
            m_regionNumbers.insert(adr.regionNumber());
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<std::string> RiaSummaryCurveDefTools::quantities() const
{
    return m_quantities;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<std::string> RiaSummaryCurveDefTools::wellNames() const
{
    return m_wellNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<std::string> RiaSummaryCurveDefTools::wellGroupNames() const
{
    return m_wellGroupNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<int> RiaSummaryCurveDefTools::regionNumbers() const
{
    return m_regionNumbers;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RimSummaryCase*> RiaSummaryCurveDefTools::summaryCases() const
{
    return m_summaryCases;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaSummaryCurveDefTools::clearAllSets()
{
    m_quantities.clear();
    m_wellNames.clear();
    m_wellGroupNames.clear();
    m_regionNumbers.clear();
    m_summaryCases.clear();
}
