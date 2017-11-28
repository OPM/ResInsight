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
class Column
{
public:
    enum DataType
    {
        NONE,
        NUMERIC,
        TEXT,
        DATETIME
    };

    Column()
        : scaleFactor(1.0),
        dataType(NONE)
    {
    }

    Column(const RifEclipseSummaryAddress& adr, const std::string& unit)
        : summaryAddress(adr),
        scaleFactor(1.0),
        unitName(unit),
        dataType(NONE)
    {
    }

    std::string                     columnName() const;
    size_t                          itemCount() const;

public:
    static Column createColumnInfoFromRsmData(const std::string& quantity, const std::string& unit, const RifEclipseSummaryAddress& adr);
    static Column createColumnInfoFromCsvData(const RifEclipseSummaryAddress& addr, const std::string& unit);

    RifEclipseSummaryAddress                        summaryAddress;
    std::string                                     unitName;
    double                                          scaleFactor;
    DataType                                        dataType;

    // Data containers
    std::vector<double>                             values;
    std::vector<std::string >                       textValues;
    std::vector<QDateTime>                          dateTimeValues;
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
              const std::string& startDate,
              const std::vector<Column>& columnInfos)
        : m_origin(origin),
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

    std::vector<Column>& columnInfos()
    {
        return m_columnInfos;
    }

    const std::vector<Column>& columnInfos() const
    {
        return m_columnInfos;
    }

    QDateTime findFirstDate() const;

private:
    std::string             m_origin;
    std::string             m_startDate;

    std::vector<Column> m_columnInfos;
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
    static std::vector<Column>                      columnInfoForFixedColumnWidth(std::stringstream& streamData);
    static std::vector<std::string>                     findValidHeaderLines(std::stringstream& streamData);
    static std::vector<std::vector<std::string>>        splitIntoColumnHeaders(const std::vector<std::string>& headerLines);
    static std::vector<Column>                      columnInfoFromColumnHeaders(const std::vector<std::vector<std::string>>& columnData);
    static std::vector<size_t>                          columnIndexForWords(const std::string& line);

    static std::vector<TableData>                       mergeEqualTimeSteps(const std::vector<TableData>& tables);

    static bool                                         isUnitText(const std::string& word);
    static bool                                         isScalingText(const std::string& word);
    
private:
    static std::string                                  trimString(const std::string& s);
};
