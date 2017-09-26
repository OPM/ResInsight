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

#include <string>
#include <vector>
#include <sstream>

struct ColumnInfo
{
    bool                                            isAVector = false;
    RifEclipseSummaryAddress                        summaryAddress;
    std::string                                     unitName;
    std::vector<double>                             values;
};

//==================================================================================================
/// 
//==================================================================================================
class RifRsmspecParserTools
{
public:
    static bool                                         isLineSkippable(const std::string& line);
    static std::vector<std::string>                     splitLine(const std::string& line);
    static bool                                         isAComment(const std::string& word);
    static std::vector<std::string>                     splitLineAndRemoveComments(const std::string& line);
    static bool                                         canBeAMnemonic(const std::string& word);
    static RifEclipseSummaryAddress::SummaryVarCategory identifyCategory(const std::string& word);
    static void                                         splitLineToDoubles(const std::string& line, std::vector<double>& values);
    static size_t                                       findFirstNonEmptyEntryIndex(std::vector<std::string>& list);
    static  RifEclipseSummaryAddress                    makeAndFillAddress(std::string scaleFactor, std::string quantityName, std::vector< std::string > headerColumn);
    static std::vector<ColumnInfo>                      columnInfoForTable(std::stringstream& data, std::string& line);
    static bool                                         isANumber(const std::string& line);
    static std::vector<std::string>                     headerReader(std::stringstream& streamData, std::string& line);
};
