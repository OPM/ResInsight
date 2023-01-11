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

#include "RiaDefines.h"

#include "RigPressureDepthData.h"

#include "RifFileParseTools.h"

#include "cafAssert.h"

#include <QFile>
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

    QString separator = " ";

    QTextStream in( &file );
    while ( !in.atEnd() )
    {
        QString line = in.readLine();
        if ( isHeaderLine( line ) )
        {
            QStringList          headerValues = RifFileParseTools::splitLineAndTrim( line, separator );
            RigPressureDepthData data;
            data.setWellName( headerValues[1] );
            items.push_back( data );
        }
        else if ( isDateLine( line ) )
        {
            // TODO: parse date
        }
        else if ( isPropertiesLine( line ) || isUnitsLine( line ) || isCommentLine( line ) )
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
bool RifPressureDepthTextFileReader::isPropertiesLine( const QString& line )
{
    // TODO: this might be to strict..
    return line.startsWith( "PRESSURE DEPTH" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifPressureDepthTextFileReader::isUnitsLine( const QString& line )
{
    // TODO: this might be to strict..
    return line.startsWith( "BARSA METRES" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<std::pair<double, double>> RifPressureDepthTextFileReader::parseDataLine( const QString& line )
{
    // Expect two data values separated by one space
    QStringList values = RifFileParseTools::splitLineAndTrim( line, " " );
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
