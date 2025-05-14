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

#include "RifReaderObservedData.h"

#include "caf.h"

#include "RifCsvUserDataParser.h"
#include "RifEclipseSummaryAddress.h"

#include "RifAsciiDataParseOptions.h"

#include <QDateTime>
#include <QTextStream>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReaderObservedData::RifReaderObservedData()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReaderObservedData::~RifReaderObservedData()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifReaderObservedData::open( const QString&                                   headerFileName,
                                  const QString&                                   identifierName,
                                  RifEclipseSummaryAddressDefines::SummaryCategory summaryCategory )
{
    RifAsciiDataParseOptions parseOptions;
    parseOptions.dateFormat    = "yyyy-MM-dd";
    parseOptions.cellSeparator = "\t";
    parseOptions.locale        = caf::norwegianLocale();

    QString     data;
    QTextStream out( &data );
    out << "Date" << "\t" << "Oil" << "\t" << "PW" << "\n";
    out << "1993-02-23" << "\t" << "10" << "\t" << "1" << "\n";
    out << "1993-06-15" << "\t" << "20" << "\t" << "2" << "\n";
    out << "1994-02-26" << "\t" << "30" << "\t" << "3" << "\n";
    out << "1994-05-23" << "\t" << "40" << "\t" << "4" << "\n";

    m_asciiParser = std::unique_ptr<RifCsvUserDataParser>( new RifCsvUserDataPastedTextParser( data ) );

    m_timeSteps.clear();
    if ( m_asciiParser->parse( parseOptions ) )
    {
        if ( m_asciiParser && m_asciiParser->dateTimeColumn() )
        {
            for ( time_t timeStep : m_asciiParser->dateTimeColumn()->dateTimeValues )
            {
                m_timeSteps.push_back( timeStep );
            }

            m_allResultAddresses.clear();
            for ( auto s : m_asciiParser->tableData().columnInfos() )
            {
                m_allResultAddresses.insert( s.summaryAddress );
            }
        }

        return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<bool, std::vector<double>> RifReaderObservedData::values( const RifEclipseSummaryAddress& resultAddress ) const
{
    size_t columnIndex = m_allResultAddresses.size();

    int i = 0;
    for ( auto& address : m_allResultAddresses )
    {
        if ( address == resultAddress )
        {
            columnIndex = i;
        }
        i++;
    }

    std::vector<double> values;
    if ( columnIndex != m_allResultAddresses.size() )
    {
        const Column* col = m_asciiParser->columnInfo( columnIndex );
        if ( col && col->dataType == Column::NUMERIC )
        {
            for ( auto& v : col->values )
            {
                values.push_back( v );
            }
        }
    }

    return { true, values };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<time_t> RifReaderObservedData::timeSteps( const RifEclipseSummaryAddress& resultAddress ) const
{
    return m_timeSteps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifReaderObservedData::address( const QString&                                   vectorName,
                                                         const QString&                                   identifierName,
                                                         RifEclipseSummaryAddressDefines::SummaryCategory summaryCategory )
{
    std::string stdVectorName = vectorName.toStdString();
    int         regionNumber( -1 );
    int         regionNumber2( -1 );
    std::string groupName;
    std::string networkName;
    std::string wellName;
    int         wellSegmentNumber( -1 );
    std::string lgrName;
    int         cellI( -1 );
    int         cellJ( -1 );
    int         cellK( -1 );
    int         aquiferNumber( -1 );
    int         wellCompletionNumber( -1 );
    bool        isErrorResult( false );
    int         id( -1 );

    switch ( summaryCategory )
    {
        case RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_GROUP:
            groupName = identifierName.toStdString();
            break;
        case RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_WELL:
            wellName = identifierName.toStdString();
            break;
        case RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_WELL_LGR:
            lgrName = identifierName.toStdString();
            break;
        default:
            break;
    }

    return RifEclipseSummaryAddress( summaryCategory,
                                     RifEclipseSummaryAddressDefines::StatisticsType::NONE,
                                     stdVectorName,
                                     regionNumber,
                                     regionNumber2,
                                     groupName,
                                     networkName,
                                     wellName,
                                     wellSegmentNumber,
                                     lgrName,
                                     cellI,
                                     cellJ,
                                     cellK,
                                     aquiferNumber,
                                     wellCompletionNumber,
                                     isErrorResult,
                                     id );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RifReaderObservedData::unitName( const RifEclipseSummaryAddress& resultAddress ) const
{
    std::string str = "";
    return str;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::EclipseUnitSystem RifReaderObservedData::unitSystem() const
{
    return RiaDefines::EclipseUnitSystem::UNITS_UNKNOWN;
}
