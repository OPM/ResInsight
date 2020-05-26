/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "RifFaciesPropertiesReader.h"

#include "RifFileParseTools.h"

#include <QFileInfo>
#include <QTextStream>

//==================================================================================================
///
//==================================================================================================
void RifFaciesPropertiesReader::readFaciesProperties( std::vector<RifFaciesProperties>& faciesProperties,
                                                      const QStringList&                filePaths )
{
    for ( const QString& filePath : filePaths )
    {
        try
        {
            readFaciesProperties( faciesProperties, filePath );
        }
        catch ( FileParseException& )
        {
            // Delete all facies properties and rethrow exception
            faciesProperties.clear();
            throw;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifFaciesPropertiesReader::readFaciesProperties( std::vector<RifFaciesProperties>& faciesProperties,
                                                      const QString&                    filePath )
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
            RifFaciesProperties faciesProp = parseFaciesProperties( line, lineNumber, filePath );
            faciesProperties.push_back( faciesProp );
        }

        lineNumber++;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifFaciesProperties
    RifFaciesPropertiesReader::parseFaciesProperties( const QString& line, int lineNumber, const QString& filePath )
{
    QStringList tokens = tokenize( line, "," );

    if ( tokens.size() != 8 )
    {
        throw FileParseException( QString( "Incomplete data on line %1: %2" ).arg( lineNumber ).arg( filePath ) );
    }

    // Check for unexpected empty tokens
    QStringList nameOfNonEmptyTokens;
    nameOfNonEmptyTokens << "Field Name"
                         << "Formation Name"
                         << "Facies Name"
                         << "Porosity"
                         << "Young's Modulus"
                         << "Poisson's Ratio"
                         << "K-Ic"
                         << "Proppant Embedment";
    verifyNonEmptyTokens( tokens, nameOfNonEmptyTokens, lineNumber, filePath );

    RifFaciesProperties faciesProperties;
    faciesProperties.fieldName         = tokens[0];
    faciesProperties.formationName     = tokens[1];
    faciesProperties.faciesName        = tokens[2];
    faciesProperties.porosity          = parseDouble( tokens[3], "Porosity", lineNumber, filePath );
    faciesProperties.youngsModulus     = parseDouble( tokens[4], "Young's Modulus", lineNumber, filePath );
    faciesProperties.poissonsRatio     = parseDouble( tokens[5], "Poisson's Ratio", lineNumber, filePath );
    faciesProperties.K_Ic              = parseDouble( tokens[6], "K-Ic", lineNumber, filePath );
    faciesProperties.proppantEmbedment = parseDouble( tokens[7], "Proppant Embedment", lineNumber, filePath );

    return faciesProperties;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RifFaciesPropertiesReader::tokenize( const QString& line, const QString& separator )
{
    return RifFileParseTools::splitLineAndTrim( line, separator );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RifFaciesPropertiesReader::parseDouble( const QString& token,
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
bool RifFaciesPropertiesReader::isEmptyLine( const QString& line )
{
    return line.trimmed().isEmpty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifFaciesPropertiesReader::isCommentLine( const QString& line )
{
    return line.trimmed().startsWith( "#" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifFaciesPropertiesReader::verifyNonEmptyTokens( const QStringList& tokens,
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
