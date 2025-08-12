/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023- Equinor ASA
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
#include <vector>

class QString;

class RifCsvUserDataParser;
class RifEclipseSummaryAddress;

//==================================================================================================
//
//
//==================================================================================================
class RifCsvSummaryReader : public RifSummaryReaderInterface
{
public:
    RifCsvSummaryReader();
    ~RifCsvSummaryReader() override;

    std::vector<time_t> timeSteps( const RifEclipseSummaryAddress& resultAddress ) const override;

    std::pair<bool, std::vector<double>> values( const RifEclipseSummaryAddress& resultAddress ) const override;

    std::string unitName( const RifEclipseSummaryAddress& resultAddress ) const override;

    RiaDefines::EclipseUnitSystem unitSystem() const override;

protected:
    void   buildTimeStepsAndMappings();
    size_t keywordCount() const override;

protected:
    std::unique_ptr<RifCsvUserDataParser> m_parser;

    std::map<RifEclipseSummaryAddress, size_t> m_mapFromAddressToResultIndex;
};
