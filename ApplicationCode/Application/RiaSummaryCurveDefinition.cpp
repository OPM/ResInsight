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

#include "RiaSummaryCurveDefinition.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiaSummaryCurveDefinition::RiaSummaryCurveDefinition(RimSummaryCase* summaryCase, const RifEclipseSummaryAddress& summaryAddress)
{
    m_curveDefinition = std::make_pair(summaryCase, summaryAddress);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCase* RiaSummaryCurveDefinition::summaryCase() const
{
    return m_curveDefinition.first;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RifEclipseSummaryAddress& RiaSummaryCurveDefinition::summaryAddress() const
{
    return m_curveDefinition.second;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiaSummaryCurveDefinition::operator<(const RiaSummaryCurveDefinition& other) const
{
    if (m_curveDefinition.first == other.summaryCase())
    {
        return (m_curveDefinition.second < other.summaryAddress());
    }

    return (m_curveDefinition.first < other.summaryCase());
}

