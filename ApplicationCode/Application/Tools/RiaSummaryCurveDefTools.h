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

#pragma once

#include <set>
#include <string>
#include <vector>

class RiaSummaryCurveDefinition;
class RimSummaryCase;

//==================================================================================================
//
//==================================================================================================
class RiaSummaryCurveDefTools
{
public:
    RiaSummaryCurveDefTools(const std::vector<RiaSummaryCurveDefinition>& curveDefinitions);

    std::vector<std::string> uniqueWellNames() const;
    std::vector<std::string> uniqueGroupNames() const;
    std::vector<int> uniqueRegions() const;
    std::vector<RimSummaryCase*> uniqueSummaryCases() const;

private:
    void computeUniqueValues() const;

private:
    const std::vector<RiaSummaryCurveDefinition>& m_curveDefinitions;

    mutable bool m_isEvaluated;
    mutable std::set<std::string> m_wellNames;
    mutable std::set<std::string> m_groupName;
    mutable std::set<int> m_regionNumbers;
    mutable std::set<RimSummaryCase*> m_summaryCases;
};
