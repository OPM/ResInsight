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

#include "RiaDefines.h"

#include "RifEclipseSummaryAddress.h"
#include "RifSummaryReaderInterface.h"

#include "ert/ecl/ecl_sum.hpp"

#include <QString>
#include <QStringList>

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

class RiaThreadSafeLogger;

//==================================================================================================
//
//
//==================================================================================================
class RifEclEclipseSummary : public RifSummaryReaderInterface
{
public:
    RifEclEclipseSummary();
    ~RifEclEclipseSummary() override;

    bool open( const QString& headerFileName, RiaThreadSafeLogger* threadSafeLogger );

    std::vector<time_t> timeSteps( const RifEclipseSummaryAddress& resultAddress ) const override;

    std::pair<bool, std::vector<double>> values( const RifEclipseSummaryAddress& resultAddress ) const override;
    std::string                          unitName( const RifEclipseSummaryAddress& resultAddress ) const override;
    RiaDefines::EclipseUnitSystem        unitSystem() const override;

    static std::string normalizeCompletionAddress( const std::string& address );

private:
    int  indexFromAddress( const RifEclipseSummaryAddress& resultAddress ) const;
    void createAndSetAddresses() override;

private:
    ecl_sum_type*          m_ecl_sum;
    const ecl_smspec_type* m_ecl_SmSpec;
    std::vector<time_t>    m_timeSteps;

    RiaDefines::EclipseUnitSystem m_unitSystem;

    std::map<RifEclipseSummaryAddress, int> m_resultAddressToErtNodeIdx;
};
