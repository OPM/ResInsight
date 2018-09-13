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

#include "cvfObject.h"

#include <string>
#include <vector>
#include <map>
#include <set>


class QDateTime;

//==================================================================================================
//
//
//==================================================================================================
class RifSummaryReaderInterface : public cvf::Object
{
public:
    bool                                            hasAddress(const RifEclipseSummaryAddress& resultAddress) const;
    const std::set<RifEclipseSummaryAddress>&       allResultAddresses() const;
    RifEclipseSummaryAddress                        errorAddress(const RifEclipseSummaryAddress& resultAddress) const;

    virtual const std::vector<time_t>&              timeSteps(const RifEclipseSummaryAddress& resultAddress) const = 0;
    
    virtual bool                                    values(const RifEclipseSummaryAddress& resultAddress,
                                                           std::vector<double>* values) const = 0;

    virtual std::string                             unitName(const RifEclipseSummaryAddress& resultAddress) const = 0;

    // TODO: Move this to a tools class with static members
    static std::vector<QDateTime>                   fromTimeT(const std::vector<time_t>& timeSteps);
    
    virtual void                                    markForCachePurge(const RifEclipseSummaryAddress& address) {}

protected:
    std::set<RifEclipseSummaryAddress>    m_allResultAddresses;     // Result and error addresses
    std::set<RifEclipseSummaryAddress>    m_allErrorAddresses;      // Error addresses
};
