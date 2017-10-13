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

#include <map>
#include <memory>
#include <string>
#include <vector>

class QString;

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

    bool                                open(const QString& headerFileName,
                                             const QString& identifierName,
                                             RifEclipseSummaryAddress::SummaryVarCategory summaryCategory);

    virtual const std::vector<time_t>&  timeSteps(const RifEclipseSummaryAddress& resultAddress) const override;

    virtual bool                        values(const RifEclipseSummaryAddress& resultAddress,
                                               std::vector<double>* values) const override;

    std::string                         unitName(const RifEclipseSummaryAddress& resultAddress) const override;

private:
    RifEclipseSummaryAddress            address(const QString& quantity,
                                                const QString& identifierName, 
                                                RifEclipseSummaryAddress::SummaryVarCategory summaryCategory);

private:
    std::unique_ptr<RifColumnBasedAsciiParser>  m_asciiParser;
    std::vector<time_t>                         m_timeSteps;
};

