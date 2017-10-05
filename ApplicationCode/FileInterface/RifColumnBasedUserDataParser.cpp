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

#include "RifColumnBasedUserDataParser.h"

#include "RifEclipseUserDataParserTools.h"

#include "RiaDateStringParser.h"
#include "RiaLogging.h"

#include "cvfAssert.h"

#include <QString>
#include <QStringList>
#include <QTextStream>


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifColumnBasedUserDataParser::RifColumnBasedUserDataParser(const QString& data)
{
    parseData(data);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector< std::vector<ColumnInfo> >& RifColumnBasedUserDataParser::tables() const
{
    return m_tables;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifColumnBasedUserDataParser::parseData(const QString& data)
{
    std::stringstream streamData;
    streamData.str(data.toStdString());

    do 
    {
        std::vector<ColumnInfo> table = RifEclipseUserDataParserTools::columnInfoForTable(streamData);
        size_t columnCount = table.size();
        if (columnCount == 0) break;
    
        std::string line;
        std::getline(streamData, line);

        size_t dateColumnIndex = table.size();
        for (size_t i = 0; i < columnCount; i++)
        {
            if (table[i].summaryAddress.quantityName() == "DATE" ||
                table[i].summaryAddress.quantityName() == "DATES")
            {
                dateColumnIndex = i;
            }
        }

        // If a DATE column is present, use the first date as the start date of the samples
        // This date is then used as basis for times defined by days or years given as double values
        QDateTime startDate;

        std::vector<double> values;
        QString qLine;
        do
        {
            qLine = QString::fromStdString(line);
            QStringList entries = qLine.split(" ", QString::SkipEmptyParts);

            if (entries.size() < static_cast<int>(columnCount)) break;

            for (size_t i = 0; i < columnCount; i++)
            {
                if (dateColumnIndex < columnCount)
                {
                    QDateTime observationDate = RiaDateStringParser::parseDateString(entries[static_cast<int>(dateColumnIndex)]);

                    if (observationDate.isValid() && !startDate.isValid())
                    {
                        startDate = observationDate;
                    }

                    table[i].observationDateTimes.push_back(observationDate);
                }

                double entry = entries.at(static_cast<int>(i)).toDouble();
                table[i].values.push_back(entry);
            }
        } while (std::getline(streamData, line));

        if (startDate.isValid())
        {
            for (auto& ci : table)
            {
                ci.startQDateTime = startDate;
            }
        }

        m_tables.push_back(table);

    } while (streamData.good());
}
