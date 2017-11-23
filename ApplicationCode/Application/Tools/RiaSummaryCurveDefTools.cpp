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

#include <set>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaSummaryCurveDefTools::RiaSummaryCurveDefTools(const std::vector<RiaSummaryCurveDefinition>& curveDefinitions)
    : m_curveDefinitions(curveDefinitions), m_isEvaluated(false)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::string> RiaSummaryCurveDefTools::uniqueWellNames() const
{
    computeUniqueValues();

    return std::vector<std::string>(m_wellNames.begin(), m_wellNames.end());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::string> RiaSummaryCurveDefTools::uniqueGroupNames() const
{
    computeUniqueValues();

    return std::vector<std::string>(m_groupName.begin(), m_groupName.end());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<int> RiaSummaryCurveDefTools::uniqueRegions() const
{
    computeUniqueValues();

    return std::vector<int>(m_regionNumbers.begin(), m_regionNumbers.end());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCase*> RiaSummaryCurveDefTools::uniqueSummaryCases() const
{
    computeUniqueValues();

    return std::vector<RimSummaryCase*>(m_summaryCases.begin(), m_summaryCases.end());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSummaryCurveDefTools::computeUniqueValues() const
{
    if (m_isEvaluated)
    {
        return;
    }

    // 	m_wellNames.clear();
    // 	m_groupName.clear();
    // 	m_summaryCases.clear();

    for (const auto& curveDef : m_curveDefinitions)
    {
        if (curveDef.summaryCase() != nullptr)
        {
            m_summaryCases.insert(curveDef.summaryCase());
        }

        if (!curveDef.summaryAddress().wellName().empty())
        {
            m_wellNames.insert(curveDef.summaryAddress().wellName());
        }

        if (!curveDef.summaryAddress().wellGroupName().empty())
        {
            m_groupName.insert(curveDef.summaryAddress().wellGroupName());
        }

        if (curveDef.summaryAddress().regionNumber() != -1)
        {
            m_regionNumbers.insert(curveDef.summaryAddress().regionNumber());
        }
    }

    m_isEvaluated = true;
}
