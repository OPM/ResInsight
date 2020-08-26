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

#include "RifElasticPropertiesReader.h"

#include "RifFileParseTools.h"

#include <QFileInfo>
#include <QTextStream>

//==================================================================================================
///
//==================================================================================================
void RifElasticPropertiesReader::readElasticProperties( std::vector<RifElasticProperties>& elasticProperties,
                                                        const QStringList&                 filePaths )
{
    for ( const QString& filePath : filePaths )
    {
        try
        {
            readElasticProperties( elasticProperties, filePath );
        }
        catch ( FileParseException& )
        {
            // Delete all facies properties and rethrow exception
            elasticProperties.clear();
            throw;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifElasticPropertiesReader::readElasticProperties( std::vector<RifElasticProperties>& elasticProperties,
                                                        const QString&                     filePath )
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
            RifElasticProperties faciesProp = parseElasticProperties( line, lineNumber, filePath );
            elasticProperties.push_back( faciesProp );
        }

        lineNumber++;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifElasticProperties
    RifElasticPropertiesReader::parseElasticProperties( const QString& line, int lineNumber, const QString& filePath )
{
    QStringList tokens = tokenize( line, "," );

    if ( tokens.size() != 13 )
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
                         << "Proppant Embedment"
                         << "Biot Coefficient"
                         << "k0"
                         << "Fluid Loss Coefficient"
                         << "Spurt Loss"
                         << "Immobile Fluid Saturation";
    verifyNonEmptyTokens( tokens, nameOfNonEmptyTokens, lineNumber, filePath );

    RifElasticProperties elasticProperties;
    elasticProperties.fieldName            = tokens[0];
    elasticProperties.formationName        = tokens[1];
    elasticProperties.faciesName           = tokens[2];
    elasticProperties.porosity             = parseDouble( tokens[3], "Porosity", lineNumber, filePath );
    elasticProperties.youngsModulus        = parseDouble( tokens[4], "Young's Modulus", lineNumber, filePath );
    elasticProperties.poissonsRatio        = parseDouble( tokens[5], "Poisson's Ratio", lineNumber, filePath );
    elasticProperties.K_Ic                 = parseDouble( tokens[6], "K-Ic", lineNumber, filePath );
    elasticProperties.proppantEmbedment    = parseDouble( tokens[7], "Proppant Embedment", lineNumber, filePath );
    elasticProperties.biotCoefficient      = parseDouble( tokens[8], "Biot Coefficient", lineNumber, filePath );
    elasticProperties.k0                   = parseDouble( tokens[9], "k0", lineNumber, filePath );
    elasticProperties.fluidLossCoefficient = parseDouble( tokens[10], "Fluid Loss Coefficient", lineNumber, filePath );
    elasticProperties.spurtLoss            = parseDouble( tokens[11], "Spurt Loss", lineNumber, filePath );
    elasticProperties.immobileFluidSaturation =
        parseDouble( tokens[12], "Immobile Fluid Saturation", lineNumber, filePath );

    return elasticProperties;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RifElasticPropertiesReader::tokenize( const QString& line, const QString& separator )
{
    return RifFileParseTools::splitLineAndTrim( line, separator );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RifElasticPropertiesReader::parseDouble( const QString& token,
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
bool RifElasticPropertiesReader::isEmptyLine( const QString& line )
{
    return line.trimmed().isEmpty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifElasticPropertiesReader::isCommentLine( const QString& line )
{
    return line.trimmed().startsWith( "#" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifElasticPropertiesReader::verifyNonEmptyTokens( const QStringList& tokens,
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
