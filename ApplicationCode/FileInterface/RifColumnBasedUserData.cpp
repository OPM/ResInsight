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
bool RifColumnBasedUserData::parse(const QString& data, QString* errorText)
{
    m_allResultAddresses.clear();
    m_timeSteps.clear();
    m_mapFromAddressToTimeStepIndex.clear();
    m_mapFromAddressToResultIndex.clear();

    m_parser = std::unique_ptr<RifColumnBasedUserDataParser>(new RifColumnBasedUserDataParser(data, errorText));
    if (!m_parser)
    {
        RiaLogging::error(QString("Failed to parse file"));

        return false;
    }

    buildTimeStepsAndMappings();

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

        if (!ci->values.empty())
        {
            values->reserve(ci->values.size());

            for (const auto& v : ci->values)
            {
                values->push_back(v);
            }
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
void RifColumnBasedUserData::buildTimeStepsAndMappings()
{
    for (size_t tableIndex = 0; tableIndex < m_parser->tableData().size(); tableIndex++)
    {
        auto tableData = m_parser->tableData()[tableIndex];

        std::vector<time_t> timeStepsForTable = createTimeSteps(tableData);

        if (timeStepsForTable.empty())
        {
            RiaLogging::warning(QString("Failed to find time data for table %1 in file %2").arg(tableIndex));
            RiaLogging::warning(QString("No data for this table is imported"));

            return;
        }

        m_timeSteps.push_back(timeStepsForTable);

        for (size_t columIndex = 0; columIndex < tableData.columnInfos().size(); columIndex++)
        {
            const ColumnInfo& ci = tableData.columnInfos()[columIndex];
            if (ci.dataType == ColumnInfo::NUMERIC)
            {
                RifEclipseSummaryAddress sumAddress = ci.summaryAddress;

                m_allResultAddresses.push_back(sumAddress);

                m_mapFromAddressToTimeStepIndex[sumAddress] = m_timeSteps.size() - 1;
                m_mapFromAddressToResultIndex[sumAddress] = std::make_pair(tableIndex, columIndex);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<time_t> RifColumnBasedUserData::createTimeSteps(const TableData& tableData)
{
    std::vector<time_t> tsVector;

    size_t dateColumnIndex = tableData.columnInfos().size();
    size_t daysColumnIndex = tableData.columnInfos().size();
    size_t yearsColumnIndex = tableData.columnInfos().size();
    size_t yearXColumnIndex = tableData.columnInfos().size();

    // Find first column matching the text criteria
    
    for (size_t columIndex = 0; columIndex < tableData.columnInfos().size(); columIndex++)
    {
        const ColumnInfo& ci = tableData.columnInfos()[columIndex];
        
        if (dateColumnIndex == tableData.columnInfos().size() &&
            RifEclipseUserDataKeywordTools::isDate(ci.summaryAddress.quantityName()))
        {
            dateColumnIndex = columIndex;
        }

        if (daysColumnIndex == tableData.columnInfos().size() &&
            RifEclipseUserDataKeywordTools::isTime(ci.summaryAddress.quantityName()) &&
            RifEclipseUserDataKeywordTools::isDays(ci.unitName))
        {
            daysColumnIndex = columIndex;
        }

        if (yearsColumnIndex == tableData.columnInfos().size() &&
            RifEclipseUserDataKeywordTools::isYears(ci.summaryAddress.quantityName()) &&
            RifEclipseUserDataKeywordTools::isYears(ci.unitName))
        {
            yearsColumnIndex = columIndex;
        }

        if (yearXColumnIndex == tableData.columnInfos().size() &&
            RifEclipseUserDataKeywordTools::isYearX(ci.summaryAddress.quantityName()) &&
            RifEclipseUserDataKeywordTools::isYears(ci.unitName))
        {
            yearXColumnIndex = columIndex;
        }
    }

    // YEARX is interpreted as absolute decimal year (2014.32)
    if (tsVector.empty() && yearXColumnIndex != tableData.columnInfos().size())
    {
        const ColumnInfo& ci = tableData.columnInfos()[yearXColumnIndex];

        for (const auto& timeStepValue : ci.values)
        {
            QDateTime dateTime = RiaQDateTimeTools::fromYears(timeStepValue);
            tsVector.push_back(dateTime.toTime_t());
        }
    }

    // DAYS is interpreted as decimal days since simulation start (23.32)
    if (tsVector.empty() && daysColumnIndex != tableData.columnInfos().size())
    {
        const ColumnInfo& ci = tableData.columnInfos()[daysColumnIndex];

        QDateTime simulationStartDate = tableData.findFirstDate();

        for (const auto& timeStepValue : ci.values)
        {
            QDateTime dateTime = RiaQDateTimeTools::addDays(simulationStartDate, timeStepValue);
            tsVector.push_back(dateTime.toTime_t());
        }
    }

    // YEARS is interpreted as decimal years since simulation start (23.32)
    if (tsVector.empty() && yearsColumnIndex != tableData.columnInfos().size())
    {
        const ColumnInfo& ci = tableData.columnInfos()[yearsColumnIndex];

        QDateTime simulationStartDate = tableData.findFirstDate();

        for (const auto& timeStepValue : ci.values)
        {
            QDateTime dateTime = RiaQDateTimeTools::addYears(simulationStartDate, timeStepValue);
            tsVector.push_back(dateTime.toTime_t());
        }
    }

    // DATE is interpreted as date string (6-NOV-1997)
    if (tsVector.empty() && dateColumnIndex != tableData.columnInfos().size())
    {
        const ColumnInfo& ci = tableData.columnInfos()[dateColumnIndex];

        QString dateFormat;
        for (auto s : ci.textValues)
        {
            QDateTime dt = RiaDateStringParser::parseDateString(s);

            tsVector.push_back(dt.toTime_t());
        }
    }

    return tsVector;
}
