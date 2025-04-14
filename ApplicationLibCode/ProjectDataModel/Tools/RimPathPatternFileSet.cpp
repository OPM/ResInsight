/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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

#include "RimPathPatternFileSet.h"

#include "RiaStdStringTools.h"
#include <QRegularExpression>

CAF_PDM_SOURCE_INIT( RimPathPatternFileSet, "PathPatternFileSet" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPathPatternFileSet::RimPathPatternFileSet()
{
    CAF_PDM_InitObject( "Automation Settings", ":/gear.svg" );

    CAF_PDM_InitField( &m_templatePath, "TemplatePath", QString(), "Template Path" );

    CAF_PDM_InitField( &m_variableDefinition, "VariableDefinition", QString(), "Variable Definition" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<QString, QString> RimPathPatternFileSet::findPathPattern( const QStringList& filePaths, const QString& placeHolderText )
{
    if ( filePaths.isEmpty() )
    {
        return {};
    }

    // Analyze the first path to find number positions
    QString                firstPath = filePaths[0];
    QList<QPair<int, int>> numberPositions; // start pos, length

    QRegularExpression              numberRegex( "\\d+" );
    int                             pos = 0;
    QRegularExpressionMatchIterator i   = numberRegex.globalMatch( firstPath );
    while ( i.hasNext() )
    {
        QRegularExpressionMatch match = i.next();
        numberPositions.append( qMakePair( match.capturedStart(), match.capturedLength() ) );
    }

    if ( numberPositions.isEmpty() )
    {
        return {};
    }

    // For each number position, check if all paths have the same number at that position
    QList<QPair<int, int>> varyingNumberPositions;

    for ( const auto& posInfo : numberPositions )
    {
        int startPos = posInfo.first;
        int length   = posInfo.second;

        bool    allSame     = true;
        QString firstNumber = firstPath.mid( startPos, length );

        for ( int i = 1; i < filePaths.size(); i++ )
        {
            if ( startPos + length > filePaths[i].length() || filePaths[i].mid( startPos, length ) != firstNumber )
            {
                allSame = false;
                break;
            }
        }

        if ( !allSame )
        {
            varyingNumberPositions.append( posInfo );
        }
    }

    if ( varyingNumberPositions.empty() )
    {
        return {};
    }

    // Extract the varying numbers
    std::vector<int> numbers;
    const auto&      varyPos  = varyingNumberPositions[0];
    int              startPos = varyPos.first;

    for ( const QString& path : filePaths )
    {
        QRegularExpression      numRegex( "\\d+" );
        QRegularExpressionMatch match = numRegex.match( path, startPos );
        if ( match.hasMatch() && match.capturedStart() == startPos )
        {
            bool ok;
            int  num = match.captured( 0 ).toInt( &ok );
            if ( ok )
            {
                numbers.push_back( num );
            }
        }
    }

    if ( numbers.size() != filePaths.size() )
    {
        return {};
    }

    // Create the pattern by replacing the varying part with placeholder
    int  endPos             = varyPos.first + varyPos.second;
    auto basePath           = firstPath.left( varyPos.first );
    auto patternPlaceholder = basePath + placeHolderText;
    auto pattern            = patternPlaceholder + firstPath.mid( endPos );

    // Try to detect repeated values
    // If the last part of the pattern contains the same number, replace it too
    QString            remaining = firstPath.mid( endPos );
    QRegularExpression finalNumberRegex( QString( "\\b%1\\b" ).arg( numbers.front() ) );
    auto               match = finalNumberRegex.match( remaining );
    if ( match.hasMatch() )
    {
        int matchPos = match.capturedStart();
        int matchLen = match.capturedLength();

        pattern = patternPlaceholder + remaining.left( matchPos ) + placeHolderText + remaining.mid( matchPos + matchLen );
    }

    auto rangeString = QString::fromStdString( RiaStdStringTools::formatRangeSelection( numbers ) );

    return { pattern, rangeString };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RimPathPatternFileSet::createPathsFromPattern( const std::pair<QString, QString>& pathPattern, const QString& placeHolderText )
{
    QStringList paths;

    QString basePath    = pathPattern.first;
    QString numberRange = pathPattern.second;

    auto numbers = RiaStdStringTools::valuesFromRangeSelection( numberRange.toStdString() );
    for ( const auto& number : numbers )
    {
        QString path = basePath;
        path.replace( placeHolderText, QString::number( number ) );
        paths.push_back( path );
    }

    return paths;
}
