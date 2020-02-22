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

#include "SummaryPlotCommands/RicPasteAsciiDataToSummaryPlotFeatureUi.h"

#include <map>
#include <memory>
#include <vector>

class QString;

class RifCsvUserDataParser;
class RifEclipseSummaryAddress;
class TableData;

//==================================================================================================
//
//
//==================================================================================================
class RifCsvUserData : public RifSummaryReaderInterface
{
public:
    RifCsvUserData();
    ~RifCsvUserData() override;

    bool parse( const QString& fileName, const AsciiDataParseOptions& parseOptions, QString* errorText = nullptr );

    const std::vector<time_t>& timeSteps( const RifEclipseSummaryAddress& resultAddress ) const override;

    bool values( const RifEclipseSummaryAddress& resultAddress, std::vector<double>* values ) const override;

    std::string unitName( const RifEclipseSummaryAddress& resultAddress ) const override;

    RiaEclipseUnitTools::UnitSystem unitSystem() const override;

private:
    void buildTimeStepsAndMappings();

private:
    std::unique_ptr<RifCsvUserDataParser> m_parser;

    std::map<RifEclipseSummaryAddress, size_t> m_mapFromAddressToResultIndex;
};
