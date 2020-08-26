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

#pragma once

#include <vector>

#include <QString>

struct RifElasticProperties
{
    QString fieldName;
    QString formationName;
    QString faciesName;
    double  porosity;
    double  youngsModulus;
    double  poissonsRatio;
    double  K_Ic;
    double  proppantEmbedment;
    double  biotCoefficient;
    double  k0;
    double  fluidLossCoefficient;
    double  spurtLoss;
    double  immobileFluidSaturation;
};

//==================================================================================================
///
//==================================================================================================
class RifElasticPropertiesReader
{
public:
    static void readElasticProperties( std::vector<RifElasticProperties>& elasticProperties, const QStringList& filePaths );

private:
    static void readElasticProperties( std::vector<RifElasticProperties>& elasticProperties, const QString& filePath );
    static RifElasticProperties parseElasticProperties( const QString& line, int lineNumber, const QString& filePath );
    static QStringList          tokenize( const QString& line, const QString& separator );
    static void                 verifyNonEmptyTokens( const QStringList& tokens,
                                                      const QStringList& nameOfNonEmptyTokens,
                                                      int                lineNumber,
                                                      const QString&     filePath );

    static double parseDouble( const QString& token, const QString& propertyName, int lineNumber, const QString& filePath );

    static bool isEmptyLine( const QString& line );
    static bool isCommentLine( const QString& line );
};
