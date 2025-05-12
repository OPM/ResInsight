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

#include "RiaEnsembleImportTools.h"

#include "RiaGuiApplication.h"
#include "RiaLogging.h"
#include "RiaStdStringTools.h"
#include "Summary/RiaSummaryTools.h"

#include "RifSummaryCaseRestartSelector.h"

#include "RimSummaryCaseMainCollection.h"

#include <QDirIterator>
#include <QRegularExpression>

namespace RiaEnsembleImportTools
{

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCase*> createSummaryCasesFromFiles( const QStringList& fileNames, CreateConfig createConfig )
{
    RimSummaryCaseMainCollection* sumCaseColl = RiaSummaryTools::summaryCaseMainCollection();
    if ( !sumCaseColl ) return {};

    std::vector<RimSummaryCase*> newCases;

    std::vector<RifSummaryCaseFileResultInfo> importFileInfos;
    if ( createConfig.fileType == RiaDefines::FileType::SMSPEC )
    {
        RifSummaryCaseRestartSelector fileSelector;

        if ( !RiaGuiApplication::isRunning() || !createConfig.allowDialogs )
        {
            fileSelector.showDialog( false );
        }

        fileSelector.setEnsembleOrGroupMode( createConfig.ensembleOrGroup );
        fileSelector.determineFilesToImportFromSummaryFiles( fileNames );

        importFileInfos = fileSelector.summaryFileInfos();

        if ( fileSelector.foundErrors() )
        {
            QString errorMessage = fileSelector.createCombinedErrorMessage();
            RiaLogging::error( errorMessage );
        }
    }
    else
    {
        // No restart files for these file types: just copy to result info
        for ( const auto& f : fileNames )
        {
            importFileInfos.push_back( RifSummaryCaseFileResultInfo( f, false, createConfig.fileType ) );
        }
    }

    if ( !importFileInfos.empty() )
    {
        std::vector<RimSummaryCase*> sumCases = sumCaseColl->createSummaryCasesFromFileInfos( importFileInfos, true );
        newCases.insert( newCases.end(), sumCases.begin(), sumCases.end() );
    }

    return newCases;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<QString, QString> findPathPattern( const QStringList& filePaths, const QString& placeholderString )
{
    if ( filePaths.isEmpty() ) return {};

    QRegularExpression numberRegex( "\\d+" );

    auto getTableOfNumbers = []( const QStringList& filePaths, const QRegularExpression& expression ) -> std::vector<std::vector<int>>
    {
        std::vector<std::vector<int>> tableOfNumbers;

        for ( const auto& f : filePaths )
        {
            std::vector<int> valuesInString;
            auto             matchIterator = expression.globalMatch( f );
            while ( matchIterator.hasNext() )
            {
                auto    match  = matchIterator.next();
                QString number = match.captured( 0 );

                valuesInString.push_back( number.toInt() );
            }
            tableOfNumbers.push_back( valuesInString );
        }

        return tableOfNumbers;
    };

    auto tableOfNumbers = getTableOfNumbers( filePaths, numberRegex );
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
QStringList createPathsFromPattern( const QString& basePath, const QString& numberRange, const QString& placeholderString )
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
QStringList createPathsBySearchingFileSystem( const QString& pathPattern, const QString& placeholderString )
{
    auto basePath = pathPattern;

    // Find based path up to "/realization"
    auto realizationPos = basePath.indexOf( "/realization" );
    if ( realizationPos != -1 )
    {
        basePath = basePath.left( realizationPos );
    }
    // Replace placeholder string with a regex pattern to capture numbers
    QString regexPattern = pathPattern;
    regexPattern.replace( placeholderString, "(\\d+)" );

    auto getMatchingFiles = []( const QString& basePath, const QString& regexPattern ) -> QStringList
    {
        QStringList        filePaths;
        QRegularExpression regex( regexPattern );

        // Use QDirIterator to traverse the directory recursively
        QDirIterator it( basePath, QDir::Files, QDirIterator::Subdirectories );
        while ( it.hasNext() )
        {
            QString filePath = it.next();
            if ( regex.match( filePath ).hasMatch() )
            {
                filePaths << filePath;
            }
        }

        return filePaths;
    };

    auto matchingFiles = getMatchingFiles( basePath, regexPattern );

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

} // namespace RiaEnsembleImportTools
