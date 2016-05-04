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


class QDateTime;

// Taken from ecl_sum.h
typedef struct ecl_sum_struct ecl_sum_type;

// Taken from stringlist.h
typedef struct stringlist_struct stringlist_type;

//==================================================================================================
//
//
//==================================================================================================
class RifReaderEclipseSummary
{
public:
    RifReaderEclipseSummary();
    ~RifReaderEclipseSummary();

    bool    open(const std::string& headerFileName, const std::vector<std::string>& dataFileNames);
    void    close();

    std::vector<std::string> variableNames() const;
    std::vector<RifEclipseSummaryAddress> allResultAddresses() const;

    std::vector<time_t> timeSteps() const;

    bool    values(const std::string& variableName, std::vector<double>* values);
    bool    values(const RifEclipseSummaryAddress& resultAddress, std::vector<double>* values);

    // TODO: Move this to a tools class with static members
    static std::vector<QDateTime> fromTimeT(const std::vector<time_t>& timeSteps);
    
private:
    int     variableIndexFromVariableName(const std::string& variableName) const;

    int     timeStepCount() const;

    static void    populateVectorFromStringList(stringlist_type* stringList, std::vector<std::string>* strings);

private:
    ecl_sum_type* ecl_sum;
};
