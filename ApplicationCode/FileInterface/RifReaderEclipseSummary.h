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
#include "RifSummaryReaderInterface.h"

#include <string>
#include <vector>
#include <map>

#include "cvfObject.h"


class QDateTime;


//==================================================================================================
//
//
//==================================================================================================
class RifReaderEclipseSummary : public RifSummaryReaderInterface
{
public:
    RifReaderEclipseSummary();
    ~RifReaderEclipseSummary();

    virtual bool                                         open(const std::string& headerFileName, const std::vector<std::string>& dataFileNames) override;

    virtual const std::vector<time_t>&                   timeSteps(size_t timeSeriesIndex = 0) const override;

    virtual bool                                         values(const RifEclipseSummaryAddress& resultAddress, std::vector<double>* values) override;
    std::string                                          unitName(const RifEclipseSummaryAddress& resultAddress) override;

private:
    virtual int                                          timeStepCount() const override;

    virtual void                                         buildMetaData() override;
private:
    // Taken from ecl_sum.h
    typedef struct ecl_sum_struct    ecl_sum_type;
    typedef struct ecl_smspec_struct ecl_smspec_type;

    ecl_sum_type*               m_ecl_sum;
    const ecl_smspec_type *     m_ecl_SmSpec;
    std::vector<time_t>         m_timeSteps;
};

