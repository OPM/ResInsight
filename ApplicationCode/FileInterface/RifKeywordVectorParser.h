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

struct KeywordBasedVector
{
    std::vector<std::string> header;
    std::vector<double> values;
};

//==================================================================================================
/// 
//==================================================================================================
class RifKeywordVectorParser
{
public:
    RifKeywordVectorParser(const QString& data);
    
    const std::vector<KeywordBasedVector>& keywordBasedVectors() const;
    static bool canBeParsed(const QString& data);

private:
    void parseData(const QString& data);

private:
    std::vector<KeywordBasedVector> m_keywordBasedVectors;
};
