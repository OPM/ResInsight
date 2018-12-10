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

class RimSummaryCaseCollection;

//==================================================================================================
/// 
//==================================================================================================
class RiaCurveSetDefinition
{
public:
    RiaCurveSetDefinition();
    explicit RiaCurveSetDefinition(RimSummaryCaseCollection* emsemble,
                                   const RifEclipseSummaryAddress& summaryAddress);

    RimSummaryCaseCollection*       ensemble() const;
    const RifEclipseSummaryAddress& summaryAddress() const;

    bool operator < (const RiaCurveSetDefinition& other) const;

private:
    RimSummaryCaseCollection * m_ensemble;
    RifEclipseSummaryAddress   m_summaryAddress;
};
