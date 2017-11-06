/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-  Statoil ASA
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

#include <QDateTime>
#include <QLocale>
#include <QString>

#include <sstream>
#include <string>
#include <vector>

//==================================================================================================
/// 
//==================================================================================================
class ColumnInfo
{
public:
    ColumnInfo()
        : scaleFactor(1.0),
        isStringData(false)
    {
    }

    ColumnInfo(const RifEclipseSummaryAddress& adr, const std::string& unit)
        : summaryAddress(adr),
        scaleFactor(1.0),
        unitName(unit),
        isStringData(false)
    {
    }

    size_t itemCount() const;

public:
    static ColumnInfo createColumnInfo(const std::string& quantity, const std::string& unit, const RifEclipseSummaryAddress& adr);

    RifEclipseSummaryAddress                        summaryAddress;
    std::string                                     unitName;
    double                                          scaleFactor;
    std::vector<double>                             values;
    bool                                            isStringData;
    std::vector<std::string>                        stringValues;
};

//==================================================================================================
/// 
//==================================================================================================
class TableData
{
public:
    TableData()
    {}

    TableData(const std::string& origin,
              const std::string& dateFormat,
              const std::string& startDate,
              const std::vector<ColumnInfo>& columnInfos)
        : m_origin(origin),
        m_dateFormat(dateFormat),
        m_startDate(startDate),
        m_columnInfos(columnInfos)
    {
    }

    std::string origin() const
    {
        return m_origin;
    }

    std::string startDate() const
    {
        return m_startDate;
    }

    std::string dateFormat() const
    {
        return m_dateFormat;
    }

    std::vector<ColumnInfo>& columnInfos()
    {
        return m_columnInfos;
    }

    const std::vector<ColumnInfo>& columnInfos() const
    {
        return m_columnInfos;
    }

private:
    std::string             m_origin;
    std::string             m_dateFormat;
    std::string             m_startDate;

    std::vector<ColumnInfo> m_columnInfos;
};

//==================================================================================================
/// 
//==================================================================================================
class RifEclipseUserDataParserTools
{
public:
    static bool                                         isLineSkippable(const std::string& line);
    static bool                                         isAComment(const std::string& word);
    static std::vector<std::string>                     splitLineAndRemoveComments(const std::string& line);
    static RifEclipseSummaryAddress::SummaryVarCategory identifyCategory(const std::string& word);
    static std::vector<double>                          splitLineToDoubles(const std::string& line);
    static size_t                                       findFirstNonEmptyEntryIndex(std::vector<std::string>& list);
    static bool                                         keywordParser(const std::string& line, std::string& origin, std::string& dateFormat, std::string& startDate);
    static bool                                         isANumber(const std::string& line);
    static std::vector<std::string>                     headerReader(std::stringstream& streamData, std::string& line);

    static bool                                         hasTimeUnit(const std::string& line);
    static bool                                         hasOnlyValidDoubleValues(const std::vector<std::string>& words, std::vector<double>* doubleValues = nullptr);

    static bool                                         isValidTableData(size_t columnCount, const std::string& line);

    static TableData                                    tableDataFromText(std::stringstream& data, std::vector<std::string>* errorText = nullptr);

    // Fixed width functions

    static bool                                         isFixedWidthHeader(const std::string& lines);
    static bool                                         hasCompleteDataForAllHeaderColumns(const std::string& lines);
    static std::vector<ColumnInfo>                      columnInfoForFixedColumnWidth(std::stringstream& streamData);
    static std::vector<std::string>                     findValidHeaderLines(std::stringstream& streamData);
    static std::vector<std::vector<std::string>>        splitIntoColumnHeaders(const std::vector<std::string>& headerLines);
    static std::vector<ColumnInfo>                      columnInfoFromColumnHeaders(const std::vector<std::vector<std::string>>& columnData);
    static std::vector<size_t>                          columnIndexForWords(const std::string& line);

    static std::vector<TableData>                       mergeEqualTimeSteps(const std::vector<TableData>& tables);

    static bool                                         isUnitText(const std::string& word);

private:
    static std::string                                  trimString(const std::string& s);
};
