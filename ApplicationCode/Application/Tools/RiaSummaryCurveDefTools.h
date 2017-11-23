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

class RimSummaryCurveCollection;
class RimSummaryCase;

//==================================================================================================
//
//==================================================================================================
class RiaSummaryCurveDefTools
{
public:
    RiaSummaryCurveDefTools();

    void findIdentifiers(RimSummaryCurveCollection* sumCurveCollection);

    std::set<std::string> quantities() const;
    std::set<std::string> wellNames() const;
    std::set<std::string> wellGroupNames() const;

    std::set<int>             regionNumbers() const;
    std::set<RimSummaryCase*> summaryCases() const;

private:
    void clearAllSets();

private:
    std::set<std::string>     m_quantities;
    std::set<std::string>     m_wellNames;
    std::set<std::string>     m_wellGroupNames;
    std::set<int>             m_regionNumbers;
    std::set<RimSummaryCase*> m_summaryCases;
};
