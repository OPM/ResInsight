/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
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
class RifReaderEclipseSummary : public cvf::Object
{
public:
    RifReaderEclipseSummary();
    ~RifReaderEclipseSummary();

    bool                                         open(const std::string& headerFileName, const std::vector<std::string>& dataFileNames);
    void                                         close();

    bool                                         hasAddress(const RifEclipseSummaryAddress& resultAddress);
    const std::vector<RifEclipseSummaryAddress>& allResultAddresses();
    const std::vector<time_t>&                   timeSteps() const;

    bool                                         values(const RifEclipseSummaryAddress& resultAddress, std::vector<double>* values);
    std::string                                  unitName(const RifEclipseSummaryAddress& resultAddress);

    // TODO: Move this to a tools class with static members
    static std::vector<QDateTime>                fromTimeT(const std::vector<time_t>& timeSteps);
    
private:

    int                                          timeStepCount() const;
    int                                          indexFromAddress(const RifEclipseSummaryAddress& resultAddress);

    void                                         buildMetaData();
private:
    // Taken from ecl_sum.h
    typedef struct ecl_sum_struct    ecl_sum_type;
    typedef struct ecl_smspec_struct ecl_smspec_type;

    ecl_sum_type*               m_ecl_sum;
    const ecl_smspec_type *     m_ecl_SmSpec;
    std::vector<time_t>         m_timeSteps;

    std::vector<RifEclipseSummaryAddress> m_allResultAddresses;
    std::map<RifEclipseSummaryAddress, int> m_resultAddressToErtNodeIdx;

};

