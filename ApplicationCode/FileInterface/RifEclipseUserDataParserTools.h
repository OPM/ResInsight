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

struct ColumnInfo
{
    bool                                            isAVector = false;
    RifEclipseSummaryAddress                        summaryAddress;
    std::string                                     unitName;
    double                                          scaleFactor;
    std::vector<double>                             values;
    std::string                                     origin;
    std::string                                     dateFormatString;
    std::string                                     startDateString;
    QDateTime                                       startQDateTime;
    std::vector<QDateTime>                          observationDateTimes;
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
    static void                                         splitLineToDoubles(const std::string& line, std::vector<double>& values);
    static size_t                                       findFirstNonEmptyEntryIndex(std::vector<std::string>& list);
    static  RifEclipseSummaryAddress                    makeAndFillAddress(std::string quantityName, std::vector< std::string > headerColumn);
    static bool                                         keywordParser(const std::string& line, std::string& origin, std::string& dateFormat, std::string& startDate);
    static std::vector<ColumnInfo>                      columnInfoForTable(std::stringstream& data);
    static bool                                         isANumber(const std::string& line);
    static std::vector<std::string>                     headerReader(std::stringstream& streamData, std::string& line);

    static bool                                         hasTimeUnit(const std::string& line);
    static bool                                         hasOnlyValidDoubleValues(const std::vector<std::string>& words, std::vector<double>* doubleValues = nullptr);
};
