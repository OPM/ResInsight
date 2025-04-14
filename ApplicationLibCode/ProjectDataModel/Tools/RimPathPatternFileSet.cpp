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

    CAF_PDM_InitField( &m_pathPattern, "PathPattern", QString(), "Path Pattern" );
    CAF_PDM_InitField( &m_rangeString, "RangeString", QString(), "Range" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPathPatternFileSet::setPathPattern( const QString& pathPattern )
{
    m_pathPattern = pathPattern;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimPathPatternFileSet::pathPattern() const
{
    return m_pathPattern();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPathPatternFileSet::setRangeString( const QString& rangeString )
{
    m_rangeString = rangeString;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimPathPatternFileSet::rangeString() const
{
    return m_rangeString();
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

    std::vector<std::vector<int>> tableOfNumbers;

    QRegularExpression numberRegex( "\\d+" );

    for ( auto f : filePaths )
    {
        std::vector<int>                valuesInString;
        QRegularExpressionMatchIterator matchIterator = numberRegex.globalMatch( f );
        while ( matchIterator.hasNext() )
        {
            QRegularExpressionMatch match    = matchIterator.next();
            QString                 number   = match.captured( 0 );
            int                     position = match.capturedStart( 0 );
            int                     length   = match.capturedLength( 0 );

            valuesInString.push_back( number.toInt() );
        }
        tableOfNumbers.push_back( valuesInString );
    }

    if ( tableOfNumbers.empty() )
    {
        return {};
    }

    auto             valuesFirstRow = tableOfNumbers[0];
    std::vector<int> valueCountEachIndex;
    valueCountEachIndex.resize( valuesFirstRow.size(), 1 );

    for ( int rowIndex = 1; rowIndex < tableOfNumbers.size(); ++rowIndex )
    {
        auto& values = tableOfNumbers[rowIndex];
        for ( int j = 0; j < values.size(); ++j )
        {
            if ( values[j] != valuesFirstRow[j] )
            {
                valueCountEachIndex[j]++;
            }
        }
    }

    // Analyze the first path to find number positions
    QString                firstPath = filePaths[0];
    QList<QPair<int, int>> numberPositions; // start pos, length

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

    for ( auto i = 0; i < numberPositions.size(); i++ )
    {
        const auto& varyPos = numberPositions[i];
        if ( i < valueCountEachIndex.size() && valueCountEachIndex[i] == tableOfNumbers.size() )
        {
            varyingNumberPositions.push_back( varyPos );
        }
    }

    if ( varyingNumberPositions.empty() )
    {
        return {};
    }

    // Extract the varying numbers
    std::vector<int> numbers;

    for ( auto i = 0; i < valueCountEachIndex.size(); i++ )
    {
        if ( numbers.empty() && valueCountEachIndex[i] == tableOfNumbers.size() )
        {
            for ( auto& values : tableOfNumbers )
            {
                numbers.push_back( values[i] );
            }
        }
    }

    if ( numbers.size() != filePaths.size() )
    {
        return {};
    }

    auto pattern = filePaths.front();

    // loop over varying number positions in reverse order
    // to avoid messing up the positions when replacing
    // the varying part with a placeholder
    for ( int i = varyingNumberPositions.size() - 1; i >= 0; --i )
    {
        const auto& varyPos = varyingNumberPositions[i];
        int         start   = varyPos.first;
        int         length  = varyPos.second;
        // Replace the varying part with a placeholder
        pattern.replace( start, length, placeHolderText );
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
