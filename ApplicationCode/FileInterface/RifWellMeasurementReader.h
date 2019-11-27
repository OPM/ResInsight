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

#pragma once

#include <vector>

#include <QDate>
#include <QString>

struct RifWellMeasurement
{
    QString wellName;
    double  MD;
    double  value;
    QDate   date;
    int     quality;
    QString kind;
    QString remark;
    QString filePath;
};

//==================================================================================================
///
//==================================================================================================
class RifWellMeasurementReader
{
public:
    static void readWellMeasurements( std::vector<RifWellMeasurement>& wellMeasurements, const QStringList& filePaths );

private:
    static void readWellMeasurements( std::vector<RifWellMeasurement>& wellMeasurements, const QString& filePath );
    static RifWellMeasurement parseWellMeasurement( const QString& line, int lineNumber, const QString& filePath );
    static QStringList        tokenize( const QString& line, const QString& separator );
    static void               verifyNonEmptyTokens( const QStringList& tokens,
                                                    const QStringList& nameOfNonEmptyTokens,
                                                    int                lineNumber,
                                                    const QString&     filePath );

    static QDate parseDate( const QString& token, const QString& propertyName, int lineNumber, const QString& filePath );
    static double parseDouble( const QString& token, const QString& propertyName, int lineNumber, const QString& filePath );
    static int parseInt( const QString& token, const QString& propertyName, int lineNumber, const QString& filePath );

    static bool isEmptyLine( const QString& line );
    static bool isCommentLine( const QString& line );
};
