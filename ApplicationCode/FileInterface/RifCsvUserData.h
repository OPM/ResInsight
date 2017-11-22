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

#include "../Commands/SummaryPlotCommands/RicPasteAsciiDataToSummaryPlotFeatureUi.h"

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
    ~RifCsvUserData();

    bool                                parse(const QString& data, const AsciiDataParseOptions& parseOptions, QString* errorText = nullptr);

    virtual const std::vector<time_t>&  timeSteps(const RifEclipseSummaryAddress& resultAddress) const override;

    virtual bool                        values(const RifEclipseSummaryAddress& resultAddress,
                                               std::vector<double>* values) const override;

    std::string                         unitName(const RifEclipseSummaryAddress& resultAddress) const override;

private:
    void                                buildTimeStepsAndMappings();
    static std::vector<time_t>          createTimeSteps(const TableData& table);

private:
    std::unique_ptr<RifCsvUserDataParser>            m_parser;
    std::vector<time_t>                              m_timeSteps;

    std::map<RifEclipseSummaryAddress, size_t >      m_mapFromAddressToTimeStepIndex;
    std::map<RifEclipseSummaryAddress, size_t >      m_mapFromAddressToResultIndex;
};
