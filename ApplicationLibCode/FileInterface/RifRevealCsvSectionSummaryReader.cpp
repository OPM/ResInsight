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
    std::map<QString, std::pair<QString, double>> unitMapping = { { "Sm3", { "SM3", 1.0 } },
                                                                  { "Sm3/day", { "SM3/DAY", 1.0 } },
                                                                  { "Sm3/day/bar", { "SM3/DAY/BAR", 1.0 } },
                                                                  { "Sm3/Sm3", { "SM3/SM3", 1.0 } },
                                                                  { "rm3", { "RM3", 1.0 } },
                                                                  { "rm3/day", { "RM3/DAY", 1.0 } },
                                                                  { "rm3/day/bar", { "RM3/DAY/BAR", 1.0 } },
                                                                  { "Rm3/day/bar", { "RM3/DAY/BAR", 1.0 } },
                                                                  { "BARa", { "BARSA", 1.0 } },
                                                                  { "fraction", { "", 1.0 } },
                                                                  { "1000Sm3/d", { "SM3/DAY", 1000.0 } },
                                                                  { "MSm3", { "SM3", 1000000.0 } } };

    if ( !m_parser->parse( parseOptions, unitMapping ) )
    {
        RiaLogging::error( QString( "Failed to parse file" ) );
        return false;
    }

    buildTimeStepsAndMappings();

    return true;
}
