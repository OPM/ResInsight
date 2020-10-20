/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RifElementPropertyTableReader.h"
#include "RifFileParseTools.h"

#include "RiaLogging.h"
#include "RiuMainWindow.h"

#include <QFile>
#include <QStringList>
#include <QTextStream>

#include <algorithm>
#include <cctype>
#include <string>

//--------------------------------------------------------------------------------------------------
/// Internal functions
//--------------------------------------------------------------------------------------------------
static QFile* openFile( const QString& fileName );
static void   closeFile( QFile* file );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifElementPropertyMetadata RifElementPropertyTableReader::readMetadata( const QString& fileName )
{
    RifElementPropertyMetadata metadata;
    QFile*                     file = nullptr;

    try
    {
        file = openFile( fileName );

        if ( file )
        {
            QTextStream stream( file );
            bool        metadataBlockFound = false;
            int         maxLinesToRead     = 50;
            int         lineNo             = 0;

            while ( lineNo < maxLinesToRead )
            {
                QString line = stream.readLine();
                lineNo++;

                if ( line.toUpper().startsWith( "*DISTRIBUTION TABLE" ) )
                {
                    metadataBlockFound = true;
                    continue;
                }
                else if ( line.toUpper().startsWith( "*DISTRIBUTION" ) )
                {
                    metadata.fileName = fileName;
                    break;
                }

                if ( !metadataBlockFound || line.startsWith( "*" ) ) continue;

                QStringList cols = RifFileParseTools::splitLineAndTrim( line, ",", true );

                for ( QString s : cols )
                {
                    metadata.dataColumns.push_back( s );
                }
            }

            closeFile( file );
        }
    }
    catch ( ... )
    {
        closeFile( file );
        throw;
    }

    return metadata;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifElementPropertyTableReader::readData( const RifElementPropertyMetadata* metadata, RifElementPropertyTable* table )
{
    CVF_ASSERT( metadata && table );

    int    expectedColumnCount = (int)metadata->dataColumns.size() + 1;
    QFile* file                = nullptr;

    try
    {
        file = openFile( metadata->fileName );

        if ( file && expectedColumnCount > 0 )
        {
            QTextStream stream( file );
            bool        dataBlockFound        = false;
            bool        completeDataLineFound = false;
            int         lineNo                = 0;
            QStringList collectedCols;

            // Init data vectors
            table->elementIds.clear();
            table->data = std::vector<std::vector<float>>( metadata->dataColumns.size() );

            while ( !stream.atEnd() )
            {
                QString line = stream.readLine();
                if ( !line.startsWith( "*" ) )
                {
                    if ( collectedCols.size() > 0 && collectedCols.size() != 8 )
                    {
                        if ( dataBlockFound )
                        {
                            throw FileParseException(
                                QString( "Number of columns mismatch at %1:%2" ).arg( metadata->fileName ).arg( lineNo ) );
                        }
                        collectedCols.clear();
                    }

                    collectedCols << RifFileParseTools::splitLineAndTrim( line, ",", true );
                }
                else
                {
                    collectedCols.clear();
                }
                lineNo++;

                if ( !completeDataLineFound )
                {
                    if ( !line.startsWith( "*" ) && !line.startsWith( "," ) && collectedCols.size() == expectedColumnCount )
                    {
                        completeDataLineFound = true;
                        dataBlockFound        = true;
                    }
                    else if ( collectedCols.size() > expectedColumnCount )
                    {
                        throw FileParseException(
                            QString( "Number of columns mismatch at %1:%2" ).arg( metadata->fileName ).arg( lineNo ) );
                    }
                    else
                    {
                        continue;
                    }
                }

                QStringList cols = collectedCols;

                for ( int c = 0; c < expectedColumnCount; c++ )
                {
                    bool parseOk;

                    if ( c == 0 )
                    {
                        // Remove elementId column prefix
                        QStringList parts = cols[0].split( "." );

                        int elementId = parts.last().toInt( &parseOk );
                        if ( !parseOk )
                        {
                            throw FileParseException(
                                QString( "Parse failed at %1:%2" ).arg( metadata->fileName ).arg( lineNo ) );
                        }
                        table->elementIds.push_back( elementId );
                    }
                    else
                    {
                        float value = cols[c].toFloat( &parseOk );
                        if ( !parseOk )
                        {
                            throw FileParseException(
                                QString( "Parse failed at %1:%2" ).arg( metadata->fileName ).arg( lineNo ) );
                        }
                        table->data[c - 1].push_back( value );
                    }
                }
                collectedCols.clear();
                completeDataLineFound = false;
            }

            table->hasData = true;
        }

        closeFile( file );
    }
    catch ( ... )
    {
        closeFile( file );
        throw;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QFile* openFile( const QString& fileName )
{
    QFile* file;
    file = new QFile( fileName );
    if ( !file->open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
        RiaLogging::error( QString( "Failed to open %1" ).arg( fileName ) );

        delete file;
        return nullptr;
    }
    return file;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void closeFile( QFile* file )
{
    if ( file )
    {
        file->close();
        delete file;
    }
}
