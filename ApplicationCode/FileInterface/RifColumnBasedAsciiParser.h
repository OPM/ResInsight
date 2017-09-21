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

#include <QDateTime>
#include <QLocale>
#include <QString>

#include <vector>


//==================================================================================================
/// 
//==================================================================================================
class RifColumnBasedAsciiParser
{
public:
    RifColumnBasedAsciiParser(QString& data, const QString dateFormat, QLocale decimalLocale, QString cellSeparator);

    const std::vector<QString>&                     headers() const;
    const std::vector<QDateTime>&                   timeSteps() const;
    const std::vector<double>                       columnValues(size_t columnIndex) const;
    const std::vector<std::vector<double>>&         values() const;
    const size_t                                    columnCount() const;

private:
    void parseData(QString& data, QString dateFormat, QLocale decimalLocale, QString cellSeparator);

private:
    
    struct AsciiData
    {
        std::vector<QString> m_headers;
        std::vector<QDateTime> m_timeSteps;
        std::vector< std::vector<double> > m_values;
    };

    AsciiData m_data;
};
