/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020 Equinor ASA
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

#include "RifRoffReader.h"

#include <QFile>
#include <QTextStream>

const QString codeValuesString = QString( "array int codeValues" );
const QString codeNamesString  = QString( "array char codeNames" );
const QString headerString     = QString( "roff-asc" );

bool RifRoffReader::isCodeValuesDefinition( const QString& line )
{
    return line.startsWith( codeValuesString );
}

bool RifRoffReader::isCodeNamesDefinition( const QString& line )
{
    return line.startsWith( codeNamesString );
}

bool RifRoffReader::isCorrectHeader( const QString& line )
{
    return line.startsWith( headerString );
}

int RifRoffReader::extractNumberAfterString( const QString& line, const QString& prefix )
{
    QString copiedLine( line );

    bool ok;
    int  num = copiedLine.remove( prefix ).toInt( &ok );
    if ( !ok )
    {
        throw RifRoffReaderException( "Unexpected value: not an integer." );
    }
    return num;
}

int RifRoffReader::extractCodeValuesCount( const QString& line )
{
    return extractNumberAfterString( line, codeValuesString );
}

int RifRoffReader::extractCodeNamesCount( const QString& line )
{
    return extractNumberAfterString( line, codeNamesString );
}

void RifRoffReader::readCodeNames( const QString& filename, std::map<int, QString>& codeNames )
{
    QFile file( filename );
    if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
        throw RifRoffReaderException( "Unable to open roff file." );
    }

    bool isFirstLine   = true;
    int  numCodeValues = -1;
    int  numCodeNames  = -1;

    std::vector<int>     readCodeValues;
    std::vector<QString> readCodeNames;

    QTextStream in( &file );
    while ( !in.atEnd() )
    {
        QString line = in.readLine();

        // Check that the first line has the roff-asc header
        if ( isFirstLine )
        {
            if ( !isCorrectHeader( line ) )
            {
                throw RifRoffReaderException( "Unexpected file type: roff-asc header missing." );
            }
            isFirstLine = false;
        }
        else if ( isCodeValuesDefinition( line ) )
        {
            // Expected line:
            // array int codeValues 99
            numCodeValues = extractCodeValuesCount( line );
            for ( int i = 0; i < numCodeValues; i++ )
            {
                // The code values comes next, can be multiple per line.
                int codeValue;
                in >> codeValue;
                readCodeValues.push_back( codeValue );
            }
        }
        else if ( isCodeNamesDefinition( line ) )
        {
            // Expected line:
            // array char codeNames 99
            numCodeNames = extractCodeNamesCount( line );
            for ( int i = 0; i < numCodeNames; i++ )
            {
                // Read code names. Assumes one name per line.
                QString codeName = in.readLine();
                readCodeNames.push_back( codeName.trimmed().remove( "\"" ) );
            }
        }
    }

    if ( numCodeValues == -1 )
    {
        throw RifRoffReaderException( "Code values not found." );
    }

    if ( numCodeNames == -1 )
    {
        throw RifRoffReaderException( "Code names not found." );
    }

    if ( numCodeNames != numCodeValues )
    {
        throw RifRoffReaderException( "Inconsistent code names and values: must be equal length." );
    }

    for ( int i = 0; i < numCodeNames; i++ )
    {
        codeNames[readCodeValues[i]] = readCodeNames[i];
    }
}
