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

#include "RifReaderRmspecColumnBasedData.h"

#include "RiaLogging.h"

#include "RifColumnBasedRsmspecParser.h"
#include "RifRsmspecParserTools.h"

#include "cafUtils.h"

#include <QDateTime>
#include <QFile>
#include <QTextStream>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifReaderRmspecColumnBasedData::RifReaderRmspecColumnBasedData()
{
    
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifReaderRmspecColumnBasedData::~RifReaderRmspecColumnBasedData()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifReaderRmspecColumnBasedData::open(const QString& headerFileName)
{
    if (!caf::Utils::fileExists(headerFileName)) return false;

    QFile file(headerFileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        RiaLogging::error(QString("Failed to open %1").arg(headerFileName));
        
        return false;
    }

    QTextStream in(&file);
    QString fileContents = in.readAll();

    m_parser = std::unique_ptr<RifColumnBasedRsmspecParser>(new RifColumnBasedRsmspecParser(fileContents));
    if (!m_parser)
    {
        RiaLogging::error(QString("Failed to parse file %1").arg(headerFileName));

        return false;
    }

    m_allResultAddresses.clear();

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
            RiaLogging::warning(QString("Failed to find time data for table %1 in file %2").arg(tableIndex).arg(headerFileName));
            RiaLogging::warning(QString("No data for this table is imported"));
        }
        else
        {
            const ColumnInfo& ci = m_parser->tables()[tableIndex][timeColumnIndex];

            m_timeSteps.resize(m_timeSteps.size() + 1);

            std::vector<time_t>& timeSteps = m_timeSteps.back();
            {
                for (auto v : ci.values)
                {
                    timeSteps.push_back(v);
                }
            }
                
            for (size_t columIndex = 0; columIndex < m_parser->tables()[tableIndex].size(); columIndex++)
            {
                const ColumnInfo& ci = m_parser->tables()[tableIndex][columIndex];
                if (ci.isAVector)
                {
                    RifEclipseSummaryAddress sumAddress = address(ci);
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
bool RifReaderRmspecColumnBasedData::values(const RifEclipseSummaryAddress& resultAddress, std::vector<double>* values) const
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
const std::vector<time_t>& RifReaderRmspecColumnBasedData::timeSteps(const RifEclipseSummaryAddress& resultAddress) const
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
RifEclipseSummaryAddress RifReaderRmspecColumnBasedData::address(const ColumnInfo& columnInfo)
{
    return RifEclipseSummaryAddress(columnInfo.category,
                                    columnInfo.quantityName,
                                    columnInfo.regionNumber,
                                    columnInfo.regionNumber2,
                                    columnInfo.wellGroupName,
                                    columnInfo.wellName,
                                    columnInfo.wellSegmentNumber,
                                    columnInfo.lgrName,
                                    columnInfo.cellI,
                                    columnInfo.cellJ,
                                    columnInfo.cellK);

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::string RifReaderRmspecColumnBasedData::unitName(const RifEclipseSummaryAddress& resultAddress) const
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
