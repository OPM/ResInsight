/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "RifWellMeasurementReader.h"

#include "RifFileParseTools.h"

#include <QDate>
#include <QFileInfo>
#include <QTextStream>

//==================================================================================================
///
//==================================================================================================
void RifWellMeasurementReader::readWellMeasurements( std::vector<RifWellMeasurement>& wellMeasurements,
                                                     const QStringList&               filePaths )
{
    for ( const QString& filePath : filePaths )
    {
        try
        {
            readWellMeasurements( wellMeasurements, filePath );
        }
        catch ( FileParseException& )
        {
            // Delete all well measurements and rethrow exception
            wellMeasurements.clear();
            throw;
        }
    }
}

//==================================================================================================
///
//==================================================================================================
void RifWellMeasurementReader::readWellMeasurements( std::vector<RifWellMeasurement>& wellMeasurements,
                                                     const QString&                   filePath )
{
    QFile file( filePath );
    if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
        throw FileParseException( QString( "Unable to open file: %1" ).arg( filePath ) );
    }

    QTextStream in( &file );
    int         lineNumber = 1;
    while ( !in.atEnd() )
    {
        QString line = in.readLine();
        if ( !isEmptyLine( line ) && !isCommentLine( line ) )
        {
            RifWellMeasurement wellMeasurement = parseWellMeasurement( line, lineNumber, filePath );
            wellMeasurements.push_back( wellMeasurement );
        }

        lineNumber++;
    }
}

//==================================================================================================
///
//==================================================================================================
RifWellMeasurement
    RifWellMeasurementReader::parseWellMeasurement( const QString& line, int lineNumber, const QString& filePath )
{
    QStringList tokens = tokenize( line, "," );

    if ( tokens.size() != 7 )
    {
        throw FileParseException( QString( "Incomplete data on line %1: %2" ).arg( lineNumber ).arg( filePath ) );
    }

    // Check for unexpected empty tokens
    QStringList nameOfNonEmptyTokens;
    nameOfNonEmptyTokens << "Well Name"
                         << "Measured Depth"
                         << "Date"
                         << "Value"
                         << "Kind";
    verifyNonEmptyTokens( tokens, nameOfNonEmptyTokens, lineNumber, filePath );

    RifWellMeasurement wellMeasurement;
    wellMeasurement.wellName = tokens[0];
    wellMeasurement.MD       = parseDouble( tokens[1], "Measured Depth", lineNumber, filePath );
    wellMeasurement.date     = parseDate( tokens[2], "Date", lineNumber, filePath );
    wellMeasurement.kind     = tokens[3].toUpper();
    wellMeasurement.value    = parseDouble( tokens[4], "Value", lineNumber, filePath );
    wellMeasurement.quality  = parseInt( tokens[5], "Quality", lineNumber, filePath );
    wellMeasurement.remark   = tokens[6];
    wellMeasurement.filePath = filePath;
    return wellMeasurement;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RifWellMeasurementReader::tokenize( const QString& line, const QString& separator )
{
    return RifFileParseTools::splitLineAndTrim( line, separator );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QDate RifWellMeasurementReader::parseDate( const QString& token,
                                           const QString& propertyName,
                                           int            lineNumber,
                                           const QString& filePath )
{
    QDate date = QDate::fromString( token, Qt::ISODate );
    if ( !date.isValid() )
    {
        throw FileParseException( QString( "Invalid date format (must be ISO 8601) for '%1' on line %2: %3" )
                                      .arg( propertyName )
                                      .arg( lineNumber )
                                      .arg( filePath ) );
    }

    return date;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RifWellMeasurementReader::parseDouble( const QString& token,
                                              const QString& propertyName,
                                              int            lineNumber,
                                              const QString& filePath )
{
    bool   isOk  = false;
    double value = token.toDouble( &isOk );
    if ( !isOk )
    {
        throw FileParseException(
            QString( "Invalid number for '%1' on line %2: %3" ).arg( propertyName ).arg( lineNumber ).arg( filePath ) );
    }

    return value;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RifWellMeasurementReader::parseInt( const QString& token, const QString& propertyName, int lineNumber, const QString& filePath )
{
    bool isOk  = false;
    int  value = token.toInt( &isOk );
    if ( !isOk )
    {
        throw FileParseException(
            QString( "Invalid number for '%1' on line %2: %3" ).arg( propertyName ).arg( lineNumber ).arg( filePath ) );
    }

    return value;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifWellMeasurementReader::isEmptyLine( const QString& line )
{
    return line.trimmed().isEmpty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifWellMeasurementReader::isCommentLine( const QString& line )
{
    return line.trimmed().startsWith( "#" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifWellMeasurementReader::verifyNonEmptyTokens( const QStringList& tokens,
                                                     const QStringList& nameOfNonEmptyTokens,
                                                     int                lineNumber,
                                                     const QString&     filePath )
{
    for ( int i = 0; i < nameOfNonEmptyTokens.size(); ++i )
    {
        if ( tokens[i].isEmpty() )
        {
            throw FileParseException(
                QString( "Unexpected empty '%1' on line %2: %3" ).arg( nameOfNonEmptyTokens[i] ).arg( lineNumber ).arg( filePath ) );
        }
    }
}
