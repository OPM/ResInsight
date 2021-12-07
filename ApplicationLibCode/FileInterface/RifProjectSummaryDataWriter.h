/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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

#include <string>
#include <vector>

class RifEclipseSummaryAddress;

//==================================================================================================
//
//
//==================================================================================================
class RifProjectSummaryDataWriter
{
public:
    // Import start time, time values, and time step count from a source summary case
    void importFromSourceSummaryFile( const std::string& sourceFileName );

    // Import all data from project summary file. This file can than be deleted and content can be exported using
    // writeDataToFile()
    void importFromProjectSummaryFile( const std::string& projectSummaryFileName );

    // Set data for a list of keyword/unit/values. If a keyword exist, the data will be overwritten
    void setData( const std::vector<std::string>&        keywords,
                  const std::vector<std::string>&        units,
                  const std::vector<std::vector<float>>& values );

    void writeDataToFile( const std::string& fileName );

private:
    int indexForKeyword( const std::string& keyword ) const;

private:
    // Structure used to represent start time defined in the following order
    // [DAY, MONTH, YEAR, HOUR, MINUTE, SECOND, MILLI_SEC]
    std::vector<int> m_startTime;

    std::vector<std::string>        m_keywords;
    std::vector<std::string>        m_units;
    std::vector<std::vector<float>> m_values;

    size_t m_timeStepCount;
};
