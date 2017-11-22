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

#include "RifCsvUserData.h"

#include "RiaDateStringParser.h"
#include "RiaLogging.h"
#include "RiaQDateTimeTools.h"

#include "RifCsvUserDataParser.h"
#include "RifEclipseUserDataKeywordTools.h"
#include "RifEclipseUserDataParserTools.h"

#include "cafUtils.h"

#include <QDateTime>
#include <QFile>
#include <QTextStream>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifCsvUserData::RifCsvUserData()
{
    
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifCsvUserData::~RifCsvUserData()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifCsvUserData::parse(const QString& data, const AsciiDataParseOptions& parseOptions, QString* errorText)
{
    m_allResultAddresses.clear();
    m_timeSteps.clear();
    m_mapFromAddressToTimeStepIndex.clear();
    m_mapFromAddressToResultIndex.clear();

    m_parser = std::unique_ptr<RifCsvUserDataParser>(new RifCsvUserDataParser(errorText));
    m_parser->parse(data, parseOptions);
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
bool RifCsvUserData::values(const RifEclipseSummaryAddress& resultAddress, std::vector<double>* values) const
{
    auto search = m_mapFromAddressToResultIndex.find(resultAddress);
    if (search != m_mapFromAddressToResultIndex.end())
    {
        size_t columnIndex = search->second;

        const ColumnInfo* ci = m_parser->columnInfo(columnIndex);
        if (!ci) return false;

        values->clear();
        values->reserve(ci->values.size());
        for (double val : ci->values)
        {
            values->push_back(val);
        }
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<time_t>& RifCsvUserData::timeSteps(const RifEclipseSummaryAddress& resultAddress) const
{
    auto search = m_mapFromAddressToTimeStepIndex.find(resultAddress);
    if (search != m_mapFromAddressToTimeStepIndex.end())
    {
        return m_timeSteps;
    }

    static std::vector<time_t> emptyVector;
    
    return emptyVector;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::string RifCsvUserData::unitName(const RifEclipseSummaryAddress& resultAddress) const
{
    auto search = m_mapFromAddressToResultIndex.find(resultAddress);
    if (search != m_mapFromAddressToResultIndex.end())
    {
        size_t columnIndex = search->second;

        const ColumnInfo* ci = m_parser->columnInfo(columnIndex);
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
void RifCsvUserData::buildTimeStepsAndMappings()
{
    auto tableData = m_parser->tableData();

    std::vector<time_t> timeStepsForTable = createTimeSteps(tableData);

    if (timeStepsForTable.empty())
    {
        RiaLogging::warning(QString("Failed to find time data for table in file"));
        RiaLogging::warning(QString("No data for this table is imported"));

        return;
    }

    m_timeSteps = timeStepsForTable;


    for (size_t columnIndex = 0; columnIndex < tableData.columnInfos().size(); columnIndex++)
    {
        const ColumnInfo& ci = tableData.columnInfos()[columnIndex];
        if (ci.dataType == ColumnInfo::NUMERIC)
        {
            RifEclipseSummaryAddress sumAddress = ci.summaryAddress;

            m_allResultAddresses.push_back(sumAddress);

            m_mapFromAddressToTimeStepIndex[sumAddress] = m_timeSteps.size() - 1;
            m_mapFromAddressToResultIndex[sumAddress] = columnIndex;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<time_t> RifCsvUserData::createTimeSteps(const TableData& tableData)
{
    std::vector<time_t> tsVector;

    const ColumnInfo& col = tableData.columnInfos()[0];

    tsVector.reserve(col.dateTimeValues.size());
    for (const QDateTime& qdt : col.dateTimeValues)
    {
        tsVector.push_back(qdt.toTime_t());
    }

    return tsVector;
}
