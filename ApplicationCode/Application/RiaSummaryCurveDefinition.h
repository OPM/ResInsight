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

#include "RifEclipseSummaryAddress.h"

#include <QString>

#include <utility>
#include <vector>

class RimSummaryCase;

//==================================================================================================
/// 
//==================================================================================================
class RiaSummaryCurveDefinition
{
public:
    explicit RiaSummaryCurveDefinition(RimSummaryCase* summaryCase, const RifEclipseSummaryAddress& summaryAddress);

    RimSummaryCase*                 summaryCase() const;
    const RifEclipseSummaryAddress& summaryAddress() const;

    bool operator < (const RiaSummaryCurveDefinition& other) const;

    // TODO: Consider moving to a separate tools class
    static void                         resultValues(const RiaSummaryCurveDefinition& curveDefinition, std::vector<double>* values);
    static const std::vector<time_t>&   timeSteps(const RiaSummaryCurveDefinition& curveDefinition);

    QString                             curveDefinitionText() const;

private:
    static QString                      curveDefinitionText(RimSummaryCase* summaryCase, const RifEclipseSummaryAddress& summaryAddress);

private:
    std::pair<RimSummaryCase*, RifEclipseSummaryAddress> m_curveDefinition;
};
