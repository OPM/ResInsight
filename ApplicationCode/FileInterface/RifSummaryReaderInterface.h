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

#include "RifEclipseSummaryAddress.h"

#include <string>
#include <vector>
#include <map>

#include "cvfObject.h"


class QDateTime;


//==================================================================================================
//
//
//==================================================================================================
class RifSummaryReaderInterface : public cvf::Object
{
public:
    virtual bool                                         open(const std::string& headerFileName, const std::vector<std::string>& dataFileNames) = 0;

    bool                                                 hasAddress(const RifEclipseSummaryAddress& resultAddress);
    const std::vector<RifEclipseSummaryAddress>&         allResultAddresses();
    virtual const std::vector<time_t>&                   timeSteps() const = 0;

    virtual bool                                         values(const RifEclipseSummaryAddress& resultAddress, std::vector<double>* values) = 0;
    virtual std::string                                  unitName(const RifEclipseSummaryAddress& resultAddress) = 0;

    // TODO: Move this to a tools class with static members
    static std::vector<QDateTime>                        fromTimeT(const std::vector<time_t>& timeSteps);
    
protected:

    virtual int                                          timeStepCount() const = 0;
    int                                                  indexFromAddress(const RifEclipseSummaryAddress& resultAddress);

    virtual void                                         buildMetaData() = 0;

protected:
    std::vector<RifEclipseSummaryAddress> m_allResultAddresses;
    std::map<RifEclipseSummaryAddress, int> m_resultAddressToErtNodeIdx;
};

