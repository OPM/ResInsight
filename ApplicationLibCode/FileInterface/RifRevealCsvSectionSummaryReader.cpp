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

#include "RifRevealCsvSectionSummaryReader.h"

#include "RiaDateStringParser.h"
#include "RiaLogging.h"
#include "RiaQDateTimeTools.h"

#include "RifCsvUserDataParser.h"
#include "RifEclipseUserDataKeywordTools.h"
#include "RifEclipseUserDataParserTools.h"

#include "SummaryPlotCommands/RicPasteAsciiDataToSummaryPlotFeatureUi.h"

#include "cafUtils.h"

#include <QDateTime>
#include <QFile>
#include <QTextStream>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifRevealCsvSectionSummaryReader::RifRevealCsvSectionSummaryReader()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifRevealCsvSectionSummaryReader::~RifRevealCsvSectionSummaryReader()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifRevealCsvSectionSummaryReader::parse( const QString& text, RifEclipseSummaryAddress::SummaryVarCategory defaultCategory, QString* errorText )
{
    m_allResultAddresses.clear();
    m_mapFromAddressToResultIndex.clear();

    AsciiDataParseOptions parseOptions;
    parseOptions.useCustomDateTimeFormat = true;
    parseOptions.dateTimeFormat          = "dd.MM.yyyy hh:mm:ss";
    parseOptions.fallbackDateTimeFormat  = "dd.MM.yyyy";
    parseOptions.cellSeparator           = ",";
    parseOptions.decimalSeparator        = ".";
    parseOptions.timeSeriesColumnName    = "Date";
    parseOptions.defaultCategory         = defaultCategory;

    m_parser = std::unique_ptr<RifCsvUserDataPastedTextParser>( new RifCsvUserDataPastedTextParser( text, errorText ) );
    if ( !m_parser->parse( parseOptions ) )
    {
        RiaLogging::error( QString( "Failed to parse file" ) );
        return false;
    }

    buildTimeStepsAndMappings();

    return true;
}
