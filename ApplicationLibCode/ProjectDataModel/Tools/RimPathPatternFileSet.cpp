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

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
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
QStringList RimPathPatternFileSet::createPaths( const QString& placeholderString ) const
{
    if ( rangeString().isEmpty() || rangeString().trimmed() == "*" )
    {
        return createPathsBySearchingFileSystem( pathPattern(), placeholderString );
    }

    auto pathsCandidates = RimPathPatternFileSet::createPathsFromPattern( pathPattern(), rangeString(), placeholderString );

    QStringList paths;
    for ( const auto& path : pathsCandidates )
    {
        if ( QFileInfo::exists( path ) )
        {
            paths.push_back( path );
        }
    }

    return paths;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPathPatternFileSet::findAndSetPathPatternAndRangeString( const QStringList& filePaths, const QString& placeholderString )
{
    const auto& [pattern, rangeString] = RimPathPatternFileSet::findPathPattern( filePaths, placeholderString );

    setPathPattern( pattern );
    setRangeString( rangeString );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<QString, QString> RimPathPatternFileSet::findPathPattern( const QStringList& filePaths, const QString& placeholderString )
{
    if ( filePaths.isEmpty() ) return {};

    std::vector<std::vector<int>> tableOfNumbers;

    QRegularExpression numberRegex( "\\d+" );

    for ( const auto& f : filePaths )
    {
        std::vector<int> valuesInString;
        auto             matchIterator = numberRegex.globalMatch( f );
        while ( matchIterator.hasNext() )
        {
            auto    match  = matchIterator.next();
            QString number = match.captured( 0 );

            valuesInString.push_back( number.toInt() );
        }
        tableOfNumbers.push_back( valuesInString );
    }

    if ( tableOfNumbers.empty() ) return {};

    const auto valuesFirstRow = tableOfNumbers[0];
    if ( valuesFirstRow.empty() ) return {};

    std::vector<size_t> valueCountEachIndex;
    valueCountEachIndex.resize( valuesFirstRow.size(), 1 );

    for ( size_t rowIndex = 1; rowIndex < tableOfNumbers.size(); ++rowIndex )
    {
        const auto& values = tableOfNumbers[rowIndex];
        for ( size_t j = 0; j < std::min( valuesFirstRow.size(), values.size() ); ++j )
        {
            if ( values[j] != valuesFirstRow[j] )
            {
                valueCountEachIndex[j]++;
            }
        }
    }

    QString                firstPath = filePaths[0];
    QList<QPair<int, int>> numberPositions; // start pos, length

    auto i = numberRegex.globalMatch( firstPath );
    while ( i.hasNext() )
    {
        auto match = i.next();
        numberPositions.append( qMakePair( match.capturedStart(), match.capturedLength() ) );
    }

    if ( numberPositions.isEmpty() ) return {};

    // For each number position, check if there are unique values for all rows
    QList<QPair<int, int>> varyingNumberPositions;

    for ( size_t i = 0; i < static_cast<size_t>( numberPositions.size() ); i++ )
    {
        const auto& varyPos = numberPositions[i];
        if ( i < valueCountEachIndex.size() && valueCountEachIndex[i] == tableOfNumbers.size() )
        {
            varyingNumberPositions.push_back( varyPos );
        }
    }

    if ( varyingNumberPositions.empty() ) return {};

    // Extract the varying numbers
    std::vector<int> numbers;

    // Find the first varying position
    int varyingPosition = -1;
    for ( size_t i = 0; i < valueCountEachIndex.size(); i++ )
    {
        if ( valueCountEachIndex[i] == tableOfNumbers.size() )
        {
            varyingPosition = static_cast<int>( i );
            break;
        }
    }

    // Extract numbers from the varying position
    if ( varyingPosition >= 0 )
    {
        for ( const auto& values : tableOfNumbers )
        {
            if ( varyingPosition < static_cast<int>( values.size() ) )
            {
                numbers.push_back( values[varyingPosition] );
            }
        }
    }

    if ( numbers.size() != static_cast<size_t>( filePaths.size() ) ) return {};

    auto pattern = filePaths.front();

    // Loop over varying number positions in reverse order to avoid messing up the positions when replacing the varying part with a
    // placeholder
    for ( int i = varyingNumberPositions.size() - 1; i >= 0; --i )
    {
        const auto& [start, length] = varyingNumberPositions[i];

        pattern.replace( start, length, placeholderString );
    }

    auto rangeString = QString::fromStdString( RiaStdStringTools::formatRangeSelection( numbers ) );

    return { pattern, rangeString };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RimPathPatternFileSet::createPathsFromPattern( const QString& basePath, const QString& numberRange, const QString& placeholderString )
{
    QStringList paths;

    auto numbers = RiaStdStringTools::valuesFromRangeSelection( numberRange.toStdString() );
    for ( const auto& number : numbers )
    {
        QString path = basePath;
        path.replace( placeholderString, QString::number( number ) );
        paths.push_back( path );
    }

    return paths;
}

//--------------------------------------------------------------------------------------------------
/// Find all files matching the path pattern in the file system
//
// Example:
// pathPattern = "/myfolder/drogon-varying-grid-geometry/realization-*/iter-0/eclipse/model/DROGON-*.EGRID"
//
// basePath will be      "/myfolder/drogon-varying-grid-geometry/"
//
// The '*' in the pathPattern is replaced with a regex pattern to capture numbers
// regexPattern will be  "/myfolder/drogon-varying-grid-geometry/realization-(\\d+)/iter-0/eclipse/model/DROGON-(\\d+).EGRID"
//
// The basePath will be traversed recursively to find all files matching the regex pattern
//
//--------------------------------------------------------------------------------------------------
QStringList RimPathPatternFileSet::createPathsBySearchingFileSystem( const QString& pathPattern, const QString& placeholderString )
{
    auto basePath = pathPattern;

    // Find based path up to "/realization"
    auto realizationPos = basePath.indexOf( "/realization" );
    if ( realizationPos != -1 )
    {
        basePath = basePath.left( realizationPos );
    }

    QStringList matchingFiles;

    // Replace placeholder string with a regex pattern to capture numbers
    QString regexPattern = pathPattern;
    regexPattern.replace( placeholderString, "(\\d+)" );

    QRegularExpression regex( regexPattern );

    // Use QDirIterator to traverse the directory recursively
    QDirIterator it( basePath, QDir::Files, QDirIterator::Subdirectories );
    while ( it.hasNext() )
    {
        QString filePath = it.next();

        QRegularExpressionMatch match = regex.match( filePath );
        if ( match.hasMatch() )
        {
            QString index1 = match.captured( 1 );
            QString index2 = match.captured( 2 );

            if ( index1 == index2 )
            {
                matchingFiles << filePath;
            }
        }
    }

    // Sort files by realization number
    std::sort( matchingFiles.begin(),
               matchingFiles.end(),
               []( const QString& a, const QString& b )
               {
                   QRegularExpression regex( "realization-(\\d+)" );

                   auto matchA = regex.match( a );
                   auto matchB = regex.match( b );

                   if ( matchA.hasMatch() && matchB.hasMatch() )
                   {
                       int numA = matchA.captured( 1 ).toInt();
                       int numB = matchB.captured( 1 ).toInt();
                       return numA < numB;
                   }

                   // Fallback to alphabetical if regex doesn't match
                   return a < b;
               } );

    return matchingFiles;
}
