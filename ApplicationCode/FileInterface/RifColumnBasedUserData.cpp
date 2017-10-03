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

#include "RifColumnBasedUserData.h"

#include "RiaQDateTimeTools.h"
#include "RiaLogging.h"

#include "RifColumnBasedUserDataParser.h"
#include "RifEclipseUserDataParserTools.h"

#include "cafUtils.h"

#include <QDateTime>
#include <QFile>
#include <QTextStream>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifColumnBasedUserData::RifColumnBasedUserData()
{
    
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifColumnBasedUserData::~RifColumnBasedUserData()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifColumnBasedUserData::parse(const QString& data, const QString& customWellName, const QString& customWellGroupName)
{
    m_allResultAddresses.clear();
    m_timeSteps.clear();
    m_mapFromAddressToTimeStepIndex.clear();
    m_mapFromAddressToResultIndex.clear();

    m_parser = std::unique_ptr<RifColumnBasedUserDataParser>(new RifColumnBasedUserDataParser(data));
    if (!m_parser)
    {
        RiaLogging::error(QString("Failed to parse file"));

        return false;
    }

    for (size_t tableIndex = 0; tableIndex < m_parser->tables().size(); tableIndex++)
    {
        size_t timeColumnIndex = m_parser->tables()[tableIndex].size();

        // Find time index
        for (size_t columIndex = 0; columIndex < m_parser->tables()[tableIndex].size(); columIndex++)
        {
            const ColumnInfo& ci = m_parser->tables()[tableIndex][columIndex];
            if (!ci.isAVector)
            {
                timeColumnIndex = columIndex;
                break;
            }
        }

        if (timeColumnIndex == m_parser->tables()[tableIndex].size())
        {
            RiaLogging::warning(QString("Failed to find time data for table %1 in file %2").arg(tableIndex));
            RiaLogging::warning(QString("No data for this table is imported"));
        }
        else
        {
            const ColumnInfo& ci = m_parser->tables()[tableIndex][timeColumnIndex];
            QDateTime startDate;
            QString startDateString = QString::fromStdString(ci.startDate);
            if (!startDateString.isEmpty())
            {
                QString dateFormatString = "ddMMyyyy";

                startDate = RiaQDateTimeTools::fromString(startDateString, dateFormatString);
            }
            else
            {
                startDate = RiaQDateTimeTools::epoch();
            }

            m_timeSteps.resize(m_timeSteps.size() + 1);

            std::vector<time_t>& timeSteps = m_timeSteps.back();

            QString unit = QString::fromStdString(ci.unitName).trimmed().toUpper();

            if (unit == "DAY" || unit == "DAYS")
            {
                for (const auto& timeStepValue : ci.values)
                {
                    QDateTime dateTime = RiaQDateTimeTools::addDays(startDate, timeStepValue);
                    timeSteps.push_back(dateTime.toTime_t());
                }
            }
            else if (unit == "YEAR" || unit == "YEARS")
            {
                for (const auto& timeStepValue : ci.values)
                {
                    QDateTime dateTime = RiaQDateTimeTools::addYears(startDate, timeStepValue);
                    timeSteps.push_back(dateTime.toTime_t());
                }
            }
            
            for (size_t columIndex = 0; columIndex < m_parser->tables()[tableIndex].size(); columIndex++)
            {
                const ColumnInfo& ci = m_parser->tables()[tableIndex][columIndex];
                if (ci.isAVector)
                {
                    RifEclipseSummaryAddress sumAddress = ci.summaryAddress;

                    if (customWellName.size() > 0)
                    {
                        sumAddress.setWellName(customWellName.toStdString());
                    }

                    if (customWellGroupName.size() > 0)
                    {
                        sumAddress.setWellGroupName(customWellGroupName.toStdString());
                    }

                    m_allResultAddresses.push_back(sumAddress);

                    m_mapFromAddressToTimeStepIndex[sumAddress] = m_timeSteps.size() - 1;
                    m_mapFromAddressToResultIndex[sumAddress] = std::make_pair(tableIndex, columIndex);
                }
            }
        }
    }

    return true;

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifColumnBasedUserData::values(const RifEclipseSummaryAddress& resultAddress, std::vector<double>* values) const
{
    auto search = m_mapFromAddressToResultIndex.find(resultAddress);
    if (search != m_mapFromAddressToResultIndex.end())
    {
        std::pair<size_t, size_t> tableColIndices = search->second;

        const ColumnInfo& ci = m_parser->tables()[tableColIndices.first][tableColIndices.second];
        for (const auto& v : ci.values)
        {
            values->push_back(v);
        }
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<time_t>& RifColumnBasedUserData::timeSteps(const RifEclipseSummaryAddress& resultAddress) const
{
    auto search = m_mapFromAddressToTimeStepIndex.find(resultAddress);
    if (search != m_mapFromAddressToTimeStepIndex.end())
    {
        return m_timeSteps[search->second];
    }

    static std::vector<time_t> emptyVector;
    
    return emptyVector;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::string RifColumnBasedUserData::unitName(const RifEclipseSummaryAddress& resultAddress) const
{
    auto search = m_mapFromAddressToResultIndex.find(resultAddress);
    if (search != m_mapFromAddressToResultIndex.end())
    {
        std::pair<size_t, size_t> tableColIndices = search->second;

        const ColumnInfo& ci = m_parser->tables()[tableColIndices.first][tableColIndices.second];

        return ci.unitName;
    }

    return "";
}
