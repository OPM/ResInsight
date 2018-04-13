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

#include <QString>

#include <string>
#include <vector>
#include <map>

class QStringList;


//==================================================================================================
//
//
//==================================================================================================
class RifRestartFileInfo
{
public:
    RifRestartFileInfo() : startDate(0), endDate(0) {}
    RifRestartFileInfo(const QString& _fileName, time_t _startDate, time_t _endDate) : 
        fileName(_fileName), startDate(_startDate), endDate(_endDate) {}
    bool valid() { return !fileName.isEmpty(); }

    QString  fileName;
    time_t   startDate;
    time_t   endDate;
};

//==================================================================================================
//
//
//==================================================================================================
class RifReaderEclipseSummary : public RifSummaryReaderInterface
{
public:
    RifReaderEclipseSummary();
    ~RifReaderEclipseSummary();

    bool                                open(const QString& headerFileName, bool includeRestartFiles);

    std::vector<RifRestartFileInfo>     getRestartFiles(const QString& headerFileName);
    RifRestartFileInfo                  getFileInfo(const QString& headerFileName);

    virtual const std::vector<time_t>&  timeSteps(const RifEclipseSummaryAddress& resultAddress) const override;

    virtual bool                        values(const RifEclipseSummaryAddress& resultAddress, std::vector<double>* values) const override;
    virtual std::string                 unitName(const RifEclipseSummaryAddress& resultAddress) const override;

private:
    int                                 timeStepCount() const;
    int                                 indexFromAddress(const RifEclipseSummaryAddress& resultAddress) const;
    void                                buildMetaData();
    RifRestartFileInfo                  getRestartFile(const QString& headerFileName);

private:
    // Taken from ecl_sum.h
    typedef struct ecl_sum_struct    ecl_sum_type;
    typedef struct ecl_smspec_struct ecl_smspec_type;

    ecl_sum_type*               m_ecl_sum;
    const ecl_smspec_type *     m_ecl_SmSpec;
    std::vector<time_t>         m_timeSteps;

    std::map<RifEclipseSummaryAddress, int> m_resultAddressToErtNodeIdx;
};

