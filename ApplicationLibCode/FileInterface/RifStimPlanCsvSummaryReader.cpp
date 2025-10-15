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

#include "RifStimPlanCsvSummaryReader.h"

#include "RiaDateStringParser.h"
#include "RiaLogging.h"
#include "RiaQDateTimeTools.h"

#include "RifCsvUserDataParser.h"
#include "RifEclipseUserDataKeywordTools.h"
#include "RifEclipseUserDataParserTools.h"

#include "RifAsciiDataParseOptions.h"

#include "cafUtils.h"

#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <memory>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifStimPlanCsvSummaryReader::RifStimPlanCsvSummaryReader()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifStimPlanCsvSummaryReader::~RifStimPlanCsvSummaryReader()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<bool, QString> RifStimPlanCsvSummaryReader::parse( const QString& fileName, const QDateTime& startDateTime, QString* errorText )
{
    m_allResultAddresses.clear();
    m_mapFromAddressToResultIndex.clear();

    QFile file( fileName );

    if ( !file.open( QFile::ReadOnly | QFile::Text ) ) return std::make_pair( false, "" );

    QTextStream in( &file );

    // Read case name from first line
    QString caseName = in.readLine().trimmed();

    // Split files on strange header line (starts with ",Date").
    QString fileContents = in.readAll();

    RifAsciiDataParseOptions parseOptions;
    parseOptions.useCustomDateTimeFormat = true;
    parseOptions.dateTimeFormat          = "m.zzz";
    parseOptions.cellSeparator           = ",";
    parseOptions.decimalSeparator        = ".";
    parseOptions.timeSeriesColumnName    = "Time";
    parseOptions.startDateTime           = startDateTime;

    m_parser = std::make_unique<RifCsvUserDataPastedTextParser>( fileContents );

    auto parseResult = m_parser->parse( parseOptions );
    if ( !parseResult )
    {
        QString errorMsg = QString( "Failed to parse file: " ) + parseResult.error();
        RiaLogging::error( errorMsg );
        if ( errorText ) *errorText = parseResult.error();
        return std::make_pair( false, "" );
    }

    buildTimeStepsAndMappings();

    return std::make_pair( true, caseName );
}
