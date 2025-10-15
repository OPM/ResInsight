/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-  Statoil ASA
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

#include "RifCsvUserDataParser.h"

#include "RifAsciiDataParseOptions.h"
#include "RifEclipseSummaryAddress.h"
#include "RifEclipseUserDataParserTools.h"
#include "RifFileParseTools.h"

#include "RiaQDateTimeTools.h"
#include "RiaStdStringTools.h"
#include "RiaTextStringTools.h"

#include "caf.h"

#include <QFile>
#include <QString>
#include <QTextStream>

#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>

//--------------------------------------------------------------------------------------------------
/// Internal constants
//--------------------------------------------------------------------------------------------------
#define DOUBLE_INF std::numeric_limits<double>::infinity()

#define ISO_DATE_FORMAT "yyyy-MM-dd"
#define TIME_FORMAT "hh:mm:ss"

using Sample     = std::pair<time_t, double>;
using SampleList = std::vector<Sample>;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
enum class CsvLineBasedColumnType
{
    DATE,
    VECTOR,
    VALUE,
    ERROR_VALUE,
    COMMENTS
};
const std::vector<QString> CSV_LINE_BASED_COL_NAMES = { "DATE", "VECTOR", "VALUE", "ERROR", "COMMENTS" };

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifCsvUserDataParser::RifCsvUserDataParser()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifCsvUserDataParser::~RifCsvUserDataParser()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<void, QString> RifCsvUserDataParser::parse( const RifAsciiDataParseOptions&                      parseOptions,
                                                          const std::map<QString, QString>&                    nameMapping,
                                                          const std::map<QString, std::pair<QString, double>>& unitMapping )
{
    if ( determineCsvLayout() == LineBased )
        return parseLineBasedData( parseOptions );
    else
        return parseColumnBasedData( parseOptions, nameMapping, unitMapping );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const TableData& RifCsvUserDataParser::tableData() const
{
    return m_tableData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const Column* RifCsvUserDataParser::columnInfo( size_t columnIndex ) const
{
    if ( columnIndex >= m_tableData.columnInfos().size() ) return nullptr;

    return &( m_tableData.columnInfos()[columnIndex] );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const Column* RifCsvUserDataParser::dateTimeColumn() const
{
    for ( const Column& col : m_tableData.columnInfos() )
    {
        if ( col.dataType == Column::DATETIME )
        {
            return &col;
        }
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<int> RifCsvUserDataParser::parseLineBasedHeader( QStringList headerCols )
{
    std::vector<int> colIndexes;

    for ( int i = 0; i < (int)CSV_LINE_BASED_COL_NAMES.size(); i++ )
    {
        for ( int j = 0; j < (int)headerCols.size(); j++ )
        {
            if ( headerCols[j] == CSV_LINE_BASED_COL_NAMES[i] )
            {
                colIndexes.push_back( j );
                break;
            }
        }

        if ( i < 3 && (int)colIndexes.size() < i + 1 ) return {};
    }
    return colIndexes;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifCsvUserDataParser::parseColumnInfo( const RifAsciiDataParseOptions& parseOptions )
{
    auto streamResult = openDataStream();
    if ( !streamResult ) return false;

    auto dataStream = std::move( streamResult.value() );

    std::vector<Column> columnInfoList;
    auto                result = parseColumnInfo( *dataStream, parseOptions, columnInfoList );

    if ( result )
    {
        m_tableData = TableData( "", "", columnInfoList );
    }
    return result.has_value();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifCsvUserDataParser::previewText( int lineCount, const RifAsciiDataParseOptions& parseOptions )
{
    auto streamResult = openDataStream();
    if ( !streamResult ) return "";

    auto stream = std::move( streamResult.value() );

    QString     preview;
    QTextStream outStream( &preview );
    int         iLine           = 0;
    bool        header          = true;
    int         timeColumnIndex = -1;

    outStream << "<Table>";
    outStream << "<Style> th, td {padding-right: 15px;} </Style>";
    while ( iLine < lineCount && !stream->atEnd() )
    {
        QString line = stream->readLine();

        if ( line.isEmpty() ) continue;

        outStream << "<tr>";
        int         iCol = 0;
        QStringList cols = RifFileParseTools::splitLineAndTrim( line, parseOptions.cellSeparator );
        for ( const QString& cellData : cols )
        {
            if ( cellData == parseOptions.timeSeriesColumnName && header )
            {
                timeColumnIndex = iCol;
            }

            outStream << ( header ? "<th" : "<td" );

            if ( iCol == timeColumnIndex )
            {
                outStream << " style=\"background-color: #FFFFD0;\"";
            }
            outStream << ">";
            outStream << cellData;
            if ( iCol < cols.size() - 1 && ( parseOptions.cellSeparator == ";" || parseOptions.cellSeparator == "," ) )
            {
                outStream << parseOptions.cellSeparator;
            }
            outStream << ( header ? "</th>" : "</td>" );

            iCol++;
        }
        outStream << "</tr>";

        header = false;
        iLine++;
    }

    outStream << "</Table>";

    return preview;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RifCsvUserDataParser::timeColumnPreviewData( int lineCount, const RifAsciiDataParseOptions& parseOptions )
{
    QStringList timeStrings;

    auto streamResult = openDataStream();
    if ( !streamResult ) return timeStrings;

    auto stream = std::move( streamResult.value() );

    int timeColumnIndex = -1;
    int iLine           = 0;

    while ( iLine < lineCount && !stream->atEnd() )
    {
        QString line = stream->readLine();

        if ( line.isEmpty() ) continue;

        int         iCol = 0;
        QStringList cols = RifFileParseTools::splitLineAndTrim( line, parseOptions.cellSeparator );
        for ( const QString& cellData : cols )
        {
            if ( cellData == parseOptions.timeSeriesColumnName && iLine == 0 )
            {
                timeColumnIndex = iCol;
            }

            if ( iLine > 0 && timeColumnIndex != -1 && timeColumnIndex == iCol )
            {
                timeStrings.push_back( cellData );
            }

            iCol++;
        }

        iLine++;
    }

    return timeStrings;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifCsvUserDataParser::CsvLayout RifCsvUserDataParser::determineCsvLayout()
{
    auto streamResult = openDataStream();
    if ( !streamResult ) return LineBased;

    auto dataStream = std::move( streamResult.value() );

    QStringList headers;
    while ( !dataStream->atEnd() )
    {
        QString firstLine = dataStream->readLine();
        if ( firstLine.isEmpty() ) continue;

        headers = RifFileParseTools::splitLineAndTrim( firstLine, ";" );

        if ( headers.size() < 3 || headers.size() > 5 ) continue;
        break;
    }

    if ( headers.contains( CSV_LINE_BASED_COL_NAMES[(size_t)CsvLineBasedColumnType::DATE] ) &&
         headers.contains( CSV_LINE_BASED_COL_NAMES[(size_t)CsvLineBasedColumnType::VECTOR] ) &&
         headers.contains( CSV_LINE_BASED_COL_NAMES[(size_t)CsvLineBasedColumnType::VALUE] ) )
        return LineBased;
    return ColumnBased;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<void, QString> RifCsvUserDataParser::parseColumnInfo( QTextStream&                                         dataStream,
                                                                    const RifAsciiDataParseOptions&                      parseOptions,
                                                                    std::vector<Column>&                                 columnInfoList,
                                                                    const std::map<QString, QString>&                    nameMapping,
                                                                    const std::map<QString, std::pair<QString, double>>& unitMapping )
{
    bool headerFound = false;

    columnInfoList.clear();
    while ( !headerFound && dataStream.status() == QTextStream::Status::Ok )
    {
        QString line = dataStream.readLine();
        if ( line.trimmed().isEmpty() )
        {
            if ( !headerFound && dataStream.atEnd() )
            {
                // Handle empty stream
                return std::unexpected( QString( "Empty data stream" ) );
            }
            else
            {
                // Empty lines are skipped.
                continue;
            }
        }

        QStringList columnHeaders = RifFileParseTools::splitLineAndTrim( line, parseOptions.cellSeparator );

        // Optional support for unit text (second header line) and names (third header line)
        QStringList unitTexts;
        QStringList names;

        auto    startOfLineWithDataValues = dataStream.pos();
        bool    hasDataValues             = false;
        QString nameFromData;
        while ( !hasDataValues && !dataStream.atEnd() )
        {
            QString candidateLine = dataStream.readLine();

            QStringList candidateColumnHeaders = RifFileParseTools::splitLineAndTrim( candidateLine, parseOptions.cellSeparator );
            for ( const auto& text : candidateColumnHeaders )
            {
                if ( RiaTextStringTools::isNumber( text, parseOptions.locale.decimalPoint() ) )
                {
                    hasDataValues = true;
                }
                else if ( nameFromData.isEmpty() )
                {
                    // Keep the first non-number data field as a possible name.
                    nameFromData = text;
                }
            }

            if ( !hasDataValues && candidateColumnHeaders.size() == columnHeaders.size() )
            {
                if ( unitTexts.empty() )
                {
                    unitTexts = candidateColumnHeaders;
                }
                else if ( names.empty() )
                {
                    names = candidateColumnHeaders;
                }

                startOfLineWithDataValues = dataStream.pos();
            }
        }

        if ( !hasDataValues ) break;

        dataStream.seek( startOfLineWithDataValues );

        int colCount = columnHeaders.size();

        for ( int iCol = 0; iCol < colCount; iCol++ )
        {
            QString colName = RiaTextStringTools::trimAndRemoveDoubleSpaces( columnHeaders[iCol] );

            QString unit;

            // Find unit from column header text
            // "VECTOR_NAME (unit)"
            // "VECTOR_NAME [unit]"
            {
                // "VECTORNAME (unit)" ==> "(unit)"
                QRegularExpression      exp( R"(\[([^\]]+)\])" );
                QRegularExpressionMatch match = exp.match( colName );
                if ( match.hasMatch() )
                {
                    QString fullCapture = match.captured( 0 );
                    QString unitCapture = match.captured( 1 );

                    unit    = unitCapture;
                    colName = RiaTextStringTools::trimAndRemoveDoubleSpaces( colName.remove( fullCapture ) );
                }
            }

            {
                // "VECTOR_NAME [unit]" ==> "[unit]"
                QRegularExpression      exp( R"(\(([^)]+)\))" );
                QRegularExpressionMatch match = exp.match( colName );
                if ( match.hasMatch() )
                {
                    QString fullCapture = match.captured( 0 );
                    QString unitCapture = match.captured( 1 );

                    unit    = unitCapture;
                    colName = RiaTextStringTools::trimAndRemoveDoubleSpaces( colName.remove( fullCapture ) );
                }
            }

            if ( auto it = nameMapping.find( colName ); it != nameMapping.end() )
            {
                colName = it->second;
            }

            if ( iCol < names.size() )
            {
                QString name = RiaTextStringTools::trimAndRemoveDoubleSpaces( names[iCol] );
                if ( !name.isEmpty() )
                {
                    // Create summary address in the form "WBHP:A-1", <vector name>:<well name>
                    colName += ":" + name;
                }
            }

            if ( iCol < unitTexts.size() ) unit = unitTexts[iCol];

            RifEclipseSummaryAddress addr = RifEclipseSummaryAddress::fromEclipseTextAddressParseErrorTokens( colName.toStdString() );

            // Create address of a give category if provided
            if ( parseOptions.defaultCategory == RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_WELL )
                addr = RifEclipseSummaryAddress::wellAddress( colName.toStdString(), nameFromData.toStdString() );
            else if ( parseOptions.defaultCategory == RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_FIELD )
                addr = RifEclipseSummaryAddress::fieldAddress( colName.toStdString() );

            double scaleFactor = 1.0;
            if ( !unit.isEmpty() )
            {
                if ( auto it = unitMapping.find( unit ); it != unitMapping.end() )
                {
                    std::tie( unit, scaleFactor ) = it->second;
                }
            }

            Column col      = Column::createColumnInfoFromCsvData( addr, unit.toStdString() );
            col.scaleFactor = scaleFactor;
            columnInfoList.push_back( col );
        }

        headerFound = true;
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<void, QString> RifCsvUserDataParser::parseColumnBasedData( const RifAsciiDataParseOptions&                      parseOptions,
                                                                         const std::map<QString, QString>&                    nameMapping,
                                                                         const std::map<QString, std::pair<QString, double>>& unitMapping )
{
    enum
    {
        FIRST_DATA_ROW,
        DATA_ROW
    } parseState = FIRST_DATA_ROW;
    int colCount;

    auto streamResult = openDataStream();
    if ( !streamResult )
    {
        return std::unexpected( streamResult.error() );
    }

    auto dataStream = std::move( streamResult.value() );

    // Parse header
    std::vector<Column> columnInfoList;
    auto                headerResult = parseColumnInfo( *dataStream, parseOptions, columnInfoList, nameMapping, unitMapping );
    if ( !headerResult )
    {
        return std::unexpected( QString( "CSV import: Failed to parse header columns - " ) + headerResult.error() );
    }

    colCount = (int)columnInfoList.size();
    while ( !dataStream->atEnd() )
    {
        QString line = dataStream->readLine();
        if ( line.trimmed().isEmpty() ) continue;

        QStringList lineColumns = RifFileParseTools::splitLineAndTrim( line, parseOptions.cellSeparator );

        if ( lineColumns.size() != colCount )
        {
            return std::unexpected( QString( "CSV import: Varying number of columns" ) );
        }
        else if ( parseState == FIRST_DATA_ROW )
        {
            for ( int iCol = 0; iCol < colCount; iCol++ )
            {
                auto    colData = lineColumns[iCol];
                Column& col     = columnInfoList[iCol];

                // Determine column data type
                if ( col.dataType == Column::NONE )
                {
                    if ( QString::fromStdString( col.summaryAddress.vectorName() ) == parseOptions.timeSeriesColumnName )
                    {
                        col.dataType = Column::DATETIME;
                    }
                    else
                    {
                        if ( parseOptions.assumeNumericDataColumns ||
                             RiaTextStringTools::isNumber( colData, parseOptions.locale.decimalPoint() ) )
                        {
                            col.dataType = Column::NUMERIC;
                        }
                        else
                        {
                            col.dataType = Column::TEXT;
                        }
                    }
                }
            }

            parseState = DATA_ROW;
        }

        if ( parseState == DATA_ROW )
        {
            for ( int iCol = 0; iCol < colCount; iCol++ )
            {
                QString& colData = lineColumns[iCol];
                Column&  col     = columnInfoList[iCol];

                if ( col.dataType == Column::NUMERIC )
                {
                    bool   parseOk = true;
                    double value   = parseOptions.locale.toDouble( colData, &parseOk );

                    if ( !parseOk )
                    {
                        // Find the error reason, wrong decimal sign or something else
                        if ( RiaStdStringTools::isNumber( colData.toStdString(), '.' ) ||
                             RiaStdStringTools::isNumber( colData.toStdString(), ',' ) )
                        {
                            QString errorMsg = QString( "CSV import: Failed to parse numeric value in column %1" ).arg( iCol + 1 );
                            return std::unexpected( errorMsg );
                        }

                        // Add nullptr value
                        value = HUGE_VAL;
                    }
                    col.values.push_back( value * col.scaleFactor );
                }
                else if ( col.dataType == Column::TEXT )
                {
                    col.textValues.push_back( colData.toStdString() );
                }
                else if ( col.dataType == Column::DATETIME )
                {
                    QDateTime dt = parseDateTime( colData, parseOptions );
                    if ( !dt.isValid() )
                    {
                        QString errorMsg = "CSV import: Failed to parse date time value";
                        return std::unexpected( errorMsg );
                    }
                    col.dateTimeValues.push_back( dt.toSecsSinceEpoch() );
                }
            }
        }
    }

    TableData td( "", "", columnInfoList );
    m_tableData = td;
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<void, QString> RifCsvUserDataParser::parseLineBasedData( const RifAsciiDataParseOptions& parseOptions )
{
    auto streamResult = openDataStream();
    if ( !streamResult )
    {
        return std::unexpected( streamResult.error() );
    }

    auto dataStream = std::move( streamResult.value() );

    std::map<RifEclipseSummaryAddress, std::vector<std::pair<time_t, double>>> addressesAndData;
    std::vector<int>                                                           colIndexes;

    // Parse header
    int  lineCount        = 0;
    bool headerFound      = false;
    bool expectErrorValue = false;

    while ( !dataStream->atEnd() )
    {
        lineCount++;

        QString line = dataStream->readLine();
        if ( line.trimmed().isEmpty() ) continue;

        QStringList dataItems = RifFileParseTools::splitLineAndTrim( line, ";" );
        if ( dataItems.size() < 3 || dataItems.size() > 5 ) continue;

        if ( !headerFound )
        {
            colIndexes = parseLineBasedHeader( dataItems );
            if ( !colIndexes.empty() )
            {
                headerFound      = true;
                expectErrorValue = colIndexes.size() > (size_t)CsvLineBasedColumnType::ERROR_VALUE &&
                                   colIndexes[(size_t)CsvLineBasedColumnType::ERROR_VALUE] >= 0;
            }
            continue;
        }

        if ( dataItems.size() != (int)colIndexes.size() ) continue;

        {
            auto textAddr = dataItems[colIndexes[(size_t)CsvLineBasedColumnType::VECTOR]];
            auto addr     = RifEclipseSummaryAddress::fromEclipseTextAddressParseErrorTokens( textAddr.toStdString() );
            auto errAddr  = addr;
            errAddr.setAsErrorResult();

            if ( !addr.isValid() ) continue;

            // VECTOR
            {
                if ( addressesAndData.find( addr ) == addressesAndData.end() )
                {
                    addressesAndData.insert( std::make_pair( addr, std::vector<Sample>() ) );
                }

                // Create error address if error value is expected
                if ( expectErrorValue )
                {
                    if ( addressesAndData.find( errAddr ) == addressesAndData.end() )
                    {
                        addressesAndData.insert( std::make_pair( errAddr, std::vector<Sample>() ) );
                    }
                }
            }

            // DATE
            QDateTime dateTime;
            {
                auto dateText = dataItems[colIndexes[(size_t)CsvLineBasedColumnType::DATE]];

                const auto formats = { parseOptions.dateFormat, QString( ISO_DATE_FORMAT ), QString( ISO_DATE_FORMAT ) + " " + TIME_FORMAT };
                for ( const auto& format : formats )
                {
                    dateTime = tryParseDateTime( dateText, format );
                    if ( dateTime.isValid() ) break;
                }

                if ( !dateTime.isValid() )
                {
                    QString errorMsg = QString( "CSV import: Failed to parse date time value in line %1" ).arg( QString::number( lineCount ) );
                    return std::unexpected( errorMsg );
                }
            }

            // VALUE
            {
                bool   parseOk = true;
                double value   = QLocale::c().toDouble( dataItems[colIndexes[(size_t)CsvLineBasedColumnType::VALUE]], &parseOk );

                if ( !parseOk )
                {
                    QString errorMsg = QString( "CSV import: Failed to parse numeric value in line %1" ).arg( QString::number( lineCount ) );
                    return std::unexpected( errorMsg );
                }

                auto& samples = addressesAndData[addr];
                samples.push_back( std::make_pair( dateTime.toSecsSinceEpoch(), value ) );
            }

            // ERROR VALUE
            if ( expectErrorValue )
            {
                bool   parseOk = true;
                double value   = QLocale::c().toDouble( dataItems[colIndexes[(size_t)CsvLineBasedColumnType::ERROR_VALUE]], &parseOk );

                if ( !parseOk ) value = DOUBLE_INF;

                auto& samples = addressesAndData[errAddr];
                samples.push_back( std::make_pair( dateTime.toSecsSinceEpoch(), value ) );
            }
        }
    }

    {
        std::vector<Column> columnInfoList;
        for ( const auto& item : addressesAndData )
        {
            auto samples = item.second;

            // Sort samples by time
            std::sort( samples.begin(), samples.end(), []( const Sample& s1, const Sample& s2 ) { return s1.first < s2.first; } );

            // Copy
            Column c   = Column::createColumnInfoFromCsvData( item.first, "" );
            c.dataType = Column::NUMERIC;

            for ( const auto& sample : samples )
            {
                c.dateTimeValues.push_back( sample.first );
                c.values.push_back( sample.second );
            }
            columnInfoList.push_back( c );
        }

        TableData td( "", "", columnInfoList );
        m_tableData = td;
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QDateTime RifCsvUserDataParser::tryParseDateTime( const QString& colData, const QString& format )
{
    return RiaQDateTimeTools::fromString( colData, format );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QDateTime RifCsvUserDataParser::parseDateTime( const QString& colData, const RifAsciiDataParseOptions& parseOptions )
{
    QDateTime dt = tryParseDateTime( colData, parseOptions.dateTimeFormat );

    // Try to match date format only
    if ( !dt.isValid() && parseOptions.dateFormat != parseOptions.dateTimeFormat )
    {
        dt = tryParseDateTime( colData, parseOptions.dateFormat );
    }

    if ( !dt.isValid() && !parseOptions.fallbackDateTimeFormat.isEmpty() )
    {
        dt = tryParseDateTime( colData, parseOptions.fallbackDateTimeFormat );
    }

    if ( !dt.isValid() && parseOptions.startDateTime.isValid() )
    {
        double minutes = colData.toDouble();
        dt             = parseOptions.startDateTime.addSecs( minutes * 60 );
    }

    return dt;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifCsvUserDataParser::tryDetermineCellSeparator()
{
    auto streamResult = openDataStream();
    if ( !streamResult )
    {
        return "";
    }

    auto dataStream = std::move( streamResult.value() );

    std::vector<QString> lines;
    int                  iLine = 0;

    while ( iLine < 10 && !dataStream->atEnd() )
    {
        QString line = dataStream->readLine();
        if ( line.isEmpty() ) continue;

        lines.push_back( line );
        iLine++;
    }

    // Try different cell separators
    int totColumnCountTab       = 0;
    int totColumnCountSemicolon = 0;
    int totColumnCountComma     = 0;

    for ( const QString& line : lines )
    {
        totColumnCountTab += RifFileParseTools::splitLineAndTrim( line, "\t" ).size();
        totColumnCountSemicolon += RifFileParseTools::splitLineAndTrim( line, ";" ).size();
        totColumnCountComma += RifFileParseTools::splitLineAndTrim( line, "," ).size();
    }

    double avgColumnCountTab       = (double)totColumnCountTab / lines.size();
    double avgColumnCountSemicolon = (double)totColumnCountSemicolon / lines.size();
    double avgColumnCountComma     = (double)totColumnCountComma / lines.size();

    // Select the one having highest average
    double maxAvg = std::max( std::max( avgColumnCountTab, avgColumnCountSemicolon ), avgColumnCountComma );

    if ( maxAvg == avgColumnCountTab ) return "\t";
    if ( maxAvg == avgColumnCountSemicolon ) return ";";
    if ( maxAvg == avgColumnCountComma ) return ",";
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifCsvUserDataParser::tryDetermineDecimalSeparator( const QString& cellSeparator )
{
    auto streamResult = openDataStream();
    if ( !streamResult )
    {
        return ".";
    }

    auto dataStream = std::move( streamResult.value() );

    int iLine = 0;

    int successfulParsesDot   = 0;
    int successfulParsesComma = 0;

    while ( iLine < 10 && !dataStream->atEnd() )
    {
        QString line = dataStream->readLine();
        if ( line.isEmpty() ) continue;

        auto parseWithLocalDecimalSeparator = []( const QString& cellData, const QString& decimalSeparator )
        {
            QLocale locale = localeFromDecimalSeparator( decimalSeparator );

            bool parseOk;
            locale.toDouble( cellData, &parseOk );
            return parseOk;
        };

        for ( const QString& cellData : RifFileParseTools::splitLineAndTrim( line, cellSeparator ) )
        {
            if ( parseWithLocalDecimalSeparator( cellData, "." ) ) successfulParsesDot++;
            if ( parseWithLocalDecimalSeparator( cellData, "," ) ) successfulParsesComma++;
        }

        iLine++;
    }

    if ( successfulParsesComma > successfulParsesDot )
        return ",";
    else
        return ".";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QLocale RifCsvUserDataParser::localeFromDecimalSeparator( const QString& decimalSeparator )
{
    if ( decimalSeparator == "," )
    {
        return caf::norwegianLocale();
    }
    return QLocale::c();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifCsvUserDataFileParser::RifCsvUserDataFileParser( const QString& fileName )
    : RifCsvUserDataParser()
    , m_fileName( fileName )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<std::unique_ptr<QTextStream>, QString> RifCsvUserDataFileParser::openDataStream()
{
    m_file = std::make_unique<QFile>( m_fileName );
    if ( !m_file->open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
        m_file.reset();
        return std::unexpected( QString( "Failed to open %1" ).arg( m_fileName ) );
    }

    return std::make_unique<QTextStream>( m_file.get() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifCsvUserDataPastedTextParser::RifCsvUserDataPastedTextParser( const QString& text )
    : RifCsvUserDataParser()
    , m_text( text )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<std::unique_ptr<QTextStream>, QString> RifCsvUserDataPastedTextParser::openDataStream()
{
    return std::make_unique<QTextStream>( &m_text );
}
