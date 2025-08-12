/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023- Equinor ASA
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

#include "RifCsvSummaryReader.h"

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
RifCsvSummaryReader::RifCsvSummaryReader()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifCsvSummaryReader::~RifCsvSummaryReader()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<bool, std::vector<double>> RifCsvSummaryReader::values( const RifEclipseSummaryAddress& resultAddress ) const
{
    std::vector<double> values;

    auto search = m_mapFromAddressToResultIndex.find( resultAddress );
    if ( search != m_mapFromAddressToResultIndex.end() )
    {
        size_t columnIndex = search->second;

        const Column* ci = m_parser->columnInfo( columnIndex );
        if ( !ci ) return { false, {} };

        values.reserve( ci->values.size() );
        for ( double val : ci->values )
        {
            values.push_back( val );
        }
    }

    return { true, values };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<time_t> RifCsvSummaryReader::timeSteps( const RifEclipseSummaryAddress& resultAddress ) const
{
    // First, check whether date time values exist for the current address
    auto search = m_mapFromAddressToResultIndex.find( resultAddress );
    if ( search != m_mapFromAddressToResultIndex.end() )
    {
        size_t index = m_mapFromAddressToResultIndex.at( resultAddress );
        if ( !m_parser->tableData().columnInfos()[index].dateTimeValues.empty() )
        {
            return m_parser->tableData().columnInfos()[index].dateTimeValues;
        }
    }

    // Then check for a separate date time column
    int index = m_parser->tableData().dateTimeColumnIndex();
    if ( index >= 0 )
    {
        return m_parser->tableData().columnInfos()[index].dateTimeValues;
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RifCsvSummaryReader::unitName( const RifEclipseSummaryAddress& resultAddress ) const
{
    auto search = m_mapFromAddressToResultIndex.find( resultAddress );
    if ( search != m_mapFromAddressToResultIndex.end() )
    {
        size_t columnIndex = search->second;

        const Column* ci = m_parser->columnInfo( columnIndex );
        if ( ci )
        {
            return ci->unitName;
        }
    }

    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::EclipseUnitSystem RifCsvSummaryReader::unitSystem() const
{
    return RiaDefines::EclipseUnitSystem::UNITS_UNKNOWN;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifCsvSummaryReader::buildTimeStepsAndMappings()
{
    auto tableData = m_parser->tableData();

    for ( size_t columnIndex = 0; columnIndex < tableData.columnInfos().size(); columnIndex++ )
    {
        const Column& ci = tableData.columnInfos()[columnIndex];
        if ( ci.dataType == Column::NUMERIC )
        {
            RifEclipseSummaryAddress sumAddress = ci.summaryAddress;

            m_allResultAddresses.insert( sumAddress );
            if ( sumAddress.isErrorResult() ) m_allErrorAddresses.insert( sumAddress );

            m_mapFromAddressToResultIndex[sumAddress] = columnIndex;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RifCsvSummaryReader::keywordCount() const
{
    return m_mapFromAddressToResultIndex.size();
}
