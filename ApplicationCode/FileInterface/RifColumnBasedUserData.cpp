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

#include "RiaDateStringParser.h"
#include "RiaLogging.h"
#include "RiaQDateTimeTools.h"

#include "RifColumnBasedUserDataParser.h"
#include "RifEclipseUserDataKeywordTools.h"
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
bool RifColumnBasedUserData::parse(const QString& data)
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

    buildTimeStepsFromTables();

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

        const ColumnInfo* ci = m_parser->columnInfo(tableColIndices.first, tableColIndices.second);
        if (!ci) return false;

        for (const auto& v : ci->values)
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

        const ColumnInfo* ci = m_parser->columnInfo(tableColIndices.first, tableColIndices.second);
        if (ci)
        {
            return ci->unitName;
        }
    }

    return "";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifColumnBasedUserData::buildTimeStepsFromTables()
{
    for (size_t tableIndex = 0; tableIndex < m_parser->tableData().size(); tableIndex++)
    {
        auto tableData = m_parser->tableData()[tableIndex];

        // Find time index
        size_t dateColumnIndex = tableData.columnInfos().size();
        size_t dayOrYearColumnIndex = tableData.columnInfos().size();

        for (size_t columIndex = 0; columIndex < tableData.columnInfos().size(); columIndex++)
        {
            const ColumnInfo& ci = tableData.columnInfos()[columIndex];
            if (dateColumnIndex == tableData.columnInfos().size() &&
                RifEclipseUserDataKeywordTools::isDate(ci.summaryAddress.quantityName()))
            {
                dateColumnIndex = columIndex;
            }
            else if (dayOrYearColumnIndex == tableData.columnInfos().size() &&
                     RifEclipseUserDataParserTools::hasTimeUnit(ci.unitName))
            {
                dayOrYearColumnIndex = columIndex;
            }
        }

        if (dateColumnIndex == tableData.columnInfos().size() &&
            dayOrYearColumnIndex == tableData.columnInfos().size())
        {
            RiaLogging::warning(QString("Failed to find time data for table %1 in file %2").arg(tableIndex));
            RiaLogging::warning(QString("No data for this table is imported"));
        }
        else
        {
            m_timeSteps.resize(m_timeSteps.size() + 1);
            std::vector<time_t>& timeSteps = m_timeSteps.back();

            if (dateColumnIndex != tableData.columnInfos().size())
            {
                const ColumnInfo& ci = tableData.columnInfos()[dateColumnIndex];

                QString dateFormat;
                for (auto s : ci.stringValues)
                {
                    QDateTime dt = RiaDateStringParser::parseDateString(s);

                    timeSteps.push_back(dt.toTime_t());
                }
            }
            else
            {
                QDateTime startDate = RiaQDateTimeTools::epoch();

                if (!tableData.startDate().empty())
                {
                    QDateTime candidate = RiaDateStringParser::parseDateString(tableData.startDate());
                    if (candidate.isValid())
                    {
                        startDate = candidate;
                    }
                }

                if (dayOrYearColumnIndex != tableData.columnInfos().size())
                {
                    const ColumnInfo& ci = tableData.columnInfos()[dayOrYearColumnIndex];

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

                }
            }

            for (size_t columIndex = 0; columIndex < tableData.columnInfos().size(); columIndex++)
            {
                const ColumnInfo& ci = tableData.columnInfos()[columIndex];
                if (!ci.isStringData)
                {
                    RifEclipseSummaryAddress sumAddress = ci.summaryAddress;

                    m_allResultAddresses.push_back(sumAddress);

                    m_mapFromAddressToTimeStepIndex[sumAddress] = m_timeSteps.size() - 1;
                    m_mapFromAddressToResultIndex[sumAddress] = std::make_pair(tableIndex, columIndex);
                }
            }
        }
    }
}
