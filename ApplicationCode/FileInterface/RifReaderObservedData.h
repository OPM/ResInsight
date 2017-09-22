/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017- Statoil ASA
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

#include "RifSummaryReaderInterface.h"

#include <string>
#include <vector>
#include <map>

#include "cvfObject.h"

struct AsciiData;
class QDateTime;
class RifColumnBasedAsciiParser;
class RifEclipseSummaryAddress;

//==================================================================================================
//
//
//==================================================================================================
class RifReaderObservedData : public RifSummaryReaderInterface
{
public:
    RifReaderObservedData();
    ~RifReaderObservedData();

    bool                                                open(const std::string& headerFileName);

    virtual const std::vector<time_t>&                   timeSteps(const RifEclipseSummaryAddress& resultAddress) const override;

    virtual bool                                         values(const RifEclipseSummaryAddress& resultAddress, std::vector<double>* values) override;
    std::string                                          unitName(const RifEclipseSummaryAddress& resultAddress) override;

private:
    RifEclipseSummaryAddress                             address(const AsciiData& asciiData, std::string identifierName, RifEclipseSummaryAddress::SummaryVarCategory summaryCategor);
private:
    RifColumnBasedAsciiParser*                           m_asciiParser;
    std::vector<time_t>                                  m_timeStepsTime_t;
};

