/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023 -     Equinor ASA
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

#include "RifPressureDepthTextFileReader.h"

#include "RiaDateStringParser.h"

#include "RigPressureDepthData.h"

#include "RifFileParseTools.h"

#include <QFile>
#include <QRegularExpression>
#include <QTextStream>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<std::vector<RigPressureDepthData>, QString> RifPressureDepthTextFileReader::readFile( const QString& filePath )
{
    std::vector<RigPressureDepthData> items;

    QFile file( filePath );
    if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
        return std::make_pair( items, QString( "Unable to open file: %1" ).arg( filePath ) );
    }

    return parse( file.readAll() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<std::vector<RigPressureDepthData>, QString> RifPressureDepthTextFileReader::parse( const QString& content )
{
    std::vector<RigPressureDepthData> items;

    QString separator = " ";

    QString     streamContent( content );
    QTextStream in( &streamContent );
    while ( !in.atEnd() )
    {
        QString line = in.readLine();
        if ( isHeaderLine( line ) )
        {
            bool                 skipEmptyParts = true;
            QStringList          headerValues   = RifFileParseTools::splitLineAndTrim( line, separator, skipEmptyParts );
            RigPressureDepthData data;
            data.setWellName( headerValues[1].replace( "'", "" ) );
            items.push_back( data );
        }
        else if ( isDateLine( line ) )
        {
            if ( std::optional<QDateTime> date = parseDateLine( line ) )
            {
                items.back().setTimeStep( date.value() );
            }
        }
        else if ( containsLetters( line ) || isCommentLine( line ) )
        {
            // Ignored.
        }
        else if ( std::optional<std::pair<double, double>> p = parseDataLine( line ) )
        {
            auto [pressure, depth] = p.value();
            items.back().addPressureAtDepth( pressure, depth );
        }
    }

    return std::make_pair( items, "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifPressureDepthTextFileReader::isHeaderLine( const QString& line )
{
    return line.startsWith( "WELLNAME" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifPressureDepthTextFileReader::isCommentLine( const QString& line )
{
    return line.startsWith( "--" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifPressureDepthTextFileReader::isDateLine( const QString& line )
{
    return line.startsWith( "DATE" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifPressureDepthTextFileReader::containsLetters( const QString& line )
{
    QRegularExpression regex( "[a-zA-Z]" );
    return regex.match( line ).hasMatch();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<std::pair<double, double>> RifPressureDepthTextFileReader::parseDataLine( const QString& line )
{
    // Expect two data values separated by one space
    bool        skipEmptyParts = true;
    QStringList values         = RifFileParseTools::splitLineAndTrim( line, " ", skipEmptyParts );
    if ( values.size() != 2 ) return {};

    // First value is pressure
    bool   isOk     = false;
    double pressure = values[0].toDouble( &isOk );
    if ( !isOk ) return {};

    // Second value is depth
    double depth = values[1].toDouble( &isOk );
    if ( !isOk ) return {};

    return std::make_pair( pressure, depth );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<QDateTime> RifPressureDepthTextFileReader::parseDateLine( const QString& line )
{
    // Expect two data values separated by one space
    bool        skipEmptyParts = true;
    QStringList values         = RifFileParseTools::splitLineAndTrim( line, " ", skipEmptyParts );
    if ( values.size() != 2 ) return {};

    // Second value is depth
    QDateTime dateTime = RiaDateStringParser::parseDateString( values[1], RiaDateStringParser::OrderPreference::DAY_FIRST );
    if ( !dateTime.isValid() ) return {};

    return dateTime;
}
