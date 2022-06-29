/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022 -     Equinor ASA
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

#include "RifThermalFractureReader.h"

#include "RiaTextStringTools.h"

#include "RigThermalFractureDefinition.h"

#include "RifFileParseTools.h"

#include <QDateTime>
#include <QFile>
#include <QTextStream>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<std::shared_ptr<RigThermalFractureDefinition>, QString>
    RifThermalFractureReader::readFractureCsvFile( const QString& filePath )
{
    QFile file( filePath );
    if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
        return std::make_pair( nullptr, QString( "Unable to open file: %1" ).arg( filePath ) );
    }

    std::shared_ptr<RigThermalFractureDefinition> def = std::make_shared<RigThermalFractureDefinition>();

    QString separator = ",";

    auto appendPropertyValues = [def]( int nodeIndex, int timeStepIndex, int valueOffset, const QStringList& values ) {
        for ( int i = valueOffset; i < values.size() - 1; i++ )
        {
            double value         = values[i].toDouble();
            int    propertyIndex = i - valueOffset;
            def->appendPropertyValue( propertyIndex, nodeIndex, timeStepIndex, value );
        }
    };

    QTextStream in( &file );
    int         lineNumber = 1;

    QStringList headerValues;
    // The two items in the csv is name and timestep
    const int valueOffset   = 2;
    int       nodeIndex     = 0;
    int       timeStepIndex = 0;
    bool      isFirstHeader = true;
    while ( !in.atEnd() )
    {
        QString line = in.readLine();
        if ( lineNumber == 1 )
        {
            // The first line is the name of the fracture
            def->setName( line );
        }
        else if ( isHeaderLine( line ) )
        {
            headerValues = RifFileParseTools::splitLineAndTrim( line, separator );
            if ( isFirstHeader )
            {
                // Create the result vector when encountering the first header
                for ( int i = valueOffset; i < headerValues.size() - 1; i++ )
                {
                    auto [name, unit] = parseNameAndUnit( headerValues[i] );
                    def->addProperty( name, unit );
                }

                isFirstHeader = false;
            }
            else
            {
                nodeIndex++;
                timeStepIndex = 0;
            }
        }
        else if ( isCenterNodeLine( line ) )
        {
            // The first node is the center node
            auto values = RifFileParseTools::splitLineAndTrim( line, separator );

            // Second is the timestamp
            QString   dateString = values[1];
            QString   dateFormat = "dd.MM.yyyy hh:mm:ss";
            QDateTime dateTime   = QDateTime::fromString( dateString, dateFormat );
            // Sometimes the datetime field is missing time
            if ( !dateTime.isValid() )
            {
                QString dateFormat = "dd.MM.yyyy";
                dateTime           = QDateTime::fromString( dateString, dateFormat );
            }

            def->addTimeStep( dateTime.toSecsSinceEpoch() );

            //
            appendPropertyValues( nodeIndex, timeStepIndex, valueOffset, values );
            timeStepIndex = 0;
        }
        else if ( isInternalNodeLine( line ) )
        {
            auto values = RifFileParseTools::splitLineAndTrim( line, separator );
            appendPropertyValues( nodeIndex, timeStepIndex, valueOffset, values );
            timeStepIndex = 0;
        }

        lineNumber++;
    }

    return std::make_pair( def, "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifThermalFractureReader::isHeaderLine( const QString& line )
{
    return line.contains( "XCoord" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifThermalFractureReader::isCenterNodeLine( const QString& line )
{
    return line.contains( "Centre Node" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifThermalFractureReader::isInternalNodeLine( const QString& line )
{
    return line.contains( "Internal Node" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<QString, QString> RifThermalFractureReader::parseNameAndUnit( const QString& value )
{
    QStringList values = value.split( " " );
    QString     name   = values[0];
    QString     unit   = values[1].replace( "(", "" ).replace( ")", "" );
    return std::make_pair( name, unit );
}
