/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021     Equinor ASA
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

#include "RiaEnsembleNameTools.h"

#include "RiaFilePathTools.h"
#include "RiaTextStringTools.h"
#include "Summary/RiaSummaryDefines.h"
#include "Tools/RiaStdStringTools.h"

#include "RimCaseDisplayNameTools.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryEnsemble.h"

#include "cafAppEnum.h"

#include <QFileInfo>
#include <QRegularExpression>

#include <regex>

namespace internal
{

// Find the last common folder in the common prefix of paths
std::string findLastCommonFolder( const std::vector<std::string>& paths )
{
    if ( paths.empty() )
    {
        return "";
    }

    auto commonPrefix = RiaStdStringTools::findCommonPrefix( paths );
    if ( commonPrefix.empty() )
    {
        return "";
    }

    // Handle the case where common prefix doesn't end at a folder boundary
    // Find the last slash in the common prefix
    size_t lastSlashPos = commonPrefix.find_last_of( '/' );
    if ( lastSlashPos == std::string::npos )
    {
        // No folder structure in the common prefix
        return "";
    }

    std::string              commonPath = commonPrefix.substr( 0, lastSlashPos );
    std::vector<std::string> folders    = RiaStdStringTools::splitString( commonPath, '/' );
    return folders.empty() ? "" : folders.back();
}

} // namespace internal

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<std::pair<std::string, std::string>, std::vector<std::string>>
    RiaEnsembleNameTools::groupFilePathsFmu( const std::vector<std::string>& filepaths )
{
    std::map<std::pair<std::string, std::string>, std::vector<std::string>> groupedPaths;

    // Example path
    // "f:/Models/scratch/project_a/realization-0/iter-0/eclipse/model/PROJECT-0.SMSPEC"
    //
    // Regex pattern to extract case name and iteration folder
    // Group 1: Case folder name (top level folder)
    // Group 2: Iteration folder name
    std::regex pattern( R"(.*[\\/]+([^\\/]+)[\\/]+realization-\d+[\\/]+([^\\/]+).*)", std::regex::icase );

    for ( const std::string& filepath : filepaths )
    {
        std::smatch matches;
        if ( std::regex_match( filepath, matches, pattern ) )
        {
            auto key = std::make_pair( matches[1].str(), matches[2].str() );
            groupedPaths[key].push_back( filepath );
        }
    }

    return groupedPaths;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<std::pair<std::string, std::string>, std::vector<std::string>>
    RiaEnsembleNameTools::groupFilePathsEverest( const std::vector<std::string>& filepaths )
{
    std::map<std::pair<std::string, std::string>, std::vector<std::string>> groupedPaths;

    std::vector<std::string> normalizedPaths;
    for ( const auto& filepath : filepaths )
    {
        normalizedPaths.push_back( RiaFilePathTools::normalizePath( filepath ) );
    }

    auto commonPrefix = RiaStdStringTools::findCommonPrefix( normalizedPaths );

    // strip partial folder name in prefix
    auto lastSlashPos = commonPrefix.find_last_of( '/' );
    if ( lastSlashPos != std::string::npos )
    {
        commonPrefix = commonPrefix.substr( 0, lastSlashPos );
    }

    auto commonFolder = internal::findLastCommonFolder( normalizedPaths );

    if ( RiaStdStringTools::toUpper( commonFolder ).contains( "BATCH" ) )
    {
        // If the common folder is "BATCH", we assume all files are in the same ensemble and group them under the common folder name
        groupedPaths[{ commonFolder, commonFolder }] = filepaths;
        return groupedPaths;
    }

    for ( const auto& filepath : normalizedPaths )
    {
        auto rest  = filepath.substr( commonPrefix.size() );
        auto parts = RiaStdStringTools::splitString( rest, '/' );

        std::string firstFolder = parts.empty() ? "" : parts.front();

        groupedPaths[{ commonFolder, firstFolder }].push_back( filepath );
    }

    return groupedPaths;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<std::pair<std::string, std::string>, std::vector<std::string>>
    RiaEnsembleNameTools::groupFilePathsOpm( const std::vector<std::string>& filepaths )
{
    std::map<std::pair<std::string, std::string>, std::vector<std::string>> groupedPaths;

    // Example path
    // "f:/Models/scratch/project_a/run-0/PROJECT-0.SMSPEC"
    //
    // Regex pattern to extract case folder and name
    // Group 1: Case folder name (top level folder)
    // Group 2: Case name
    std::regex pattern( R"(.*[\\/]+([^\\/]+)[\\/]+run-\d+[\\/]+([^\\/]+).*-\d.SMSPEC)", std::regex::icase );

    for ( const std::string& filepath : filepaths )
    {
        std::smatch matches;
        if ( std::regex_match( filepath, matches, pattern ) )
        {
            auto key = std::make_pair( matches[1].str(), matches[2].str() );
            groupedPaths[key].push_back( filepath );
        }
    }

    return groupedPaths;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaEnsembleNameTools::findSuitableEnsembleName( const QStringList& fileNames, RiaDefines::EnsembleGroupingMode folderLevel )
{
    if ( folderLevel == RiaDefines::EnsembleGroupingMode::EVEREST_FOLDER_STRUCTURE )
    {
        QString commonRoot         = RiaTextStringTools::commonRoot( fileNames );
        commonRoot                 = commonRoot.left( commonRoot.lastIndexOf( '/' ) );
        QStringList rootComponents = RiaFilePathTools::splitPathIntoComponents( commonRoot );

        if ( !rootComponents.empty() )
        {
            return rootComponents.back();
        }
    }

    std::vector<QStringList> componentsForAllFilePaths;

    for ( const auto& filePath : fileNames )
    {
        QStringList components = RiaFilePathTools::splitPathIntoComponents( filePath );
        componentsForAllFilePaths.push_back( components );
    }

    QStringList iterations = findUniqueEnsembleNames( fileNames, componentsForAllFilePaths, folderLevel );
    if ( iterations.size() == 1u )
    {
        return iterations.front();
    }

    if ( !iterations.empty() )
    {
        return QString( "Multiple iterations: %1" ).arg( iterations.join( ", " ) );
    }

    QString root = RiaFilePathTools::commonRootOfFileNames( fileNames );

    QRegularExpression trimRe( "[^a-zA-Z0-9]+$" );
    QString            trimmedRoot = root.replace( trimRe, "" );
    if ( trimmedRoot.length() >= 4 )
    {
        return trimmedRoot;
    }

    return "Ensemble";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QStringList> RiaEnsembleNameTools::groupFilesByEnsemble( const QStringList& fileNames, RiaDefines::EnsembleGroupingMode groupingMode )
{
    std::vector<QStringList> componentsForAllFilePaths;

    for ( const auto& filePath : fileNames )
    {
        QStringList components = RiaFilePathTools::splitPathIntoComponents( filePath );
        componentsForAllFilePaths.push_back( components );
    }

    QStringList iterations = findUniqueEnsembleNames( fileNames, componentsForAllFilePaths, groupingMode );
    if ( iterations.size() <= 1 )
    {
        // All the files are in the same ensemble
        return { fileNames };
    }

    std::vector<QStringList> groupedByIteration;
    for ( const auto& iteration : iterations )
    {
        QStringList fileNamesFromIteration;

        for ( int i = 0; i < fileNames.size(); i++ )
        {
            auto components = componentsForAllFilePaths[i];
            bool foundMatch = false;
            for ( const auto& component : components )
            {
                if ( component == iteration ) foundMatch = true;
            }

            if ( foundMatch )
            {
                fileNamesFromIteration << fileNames[i];
            }
        }
        groupedByIteration.push_back( fileNamesFromIteration );
    }

    return groupedByIteration;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<QString, QStringList> RiaEnsembleNameTools::groupFilesByCustomEnsemble( const QStringList& fileNames, RiaDefines::FileType fileType )
{
    std::vector<QStringList> componentsForAllFilePaths;
    for ( const auto& filePath : fileNames )
    {
        QStringList components = RiaFilePathTools::splitPathIntoComponents( filePath );
        componentsForAllFilePaths.push_back( components );
    }

    auto mapping = findUniqueCustomEnsembleNames( fileType, fileNames, componentsForAllFilePaths );

    std::set<QString> iterations;
    for ( const auto& [name, iterFileNamePair] : mapping )
    {
        iterations.insert( iterFileNamePair.first );
    }

    std::map<QString, QStringList> groupedByIteration;

    for ( const QString& groupIteration : iterations )
    {
        QStringList fileNamesFromIteration;

        for ( const auto& [name, iterFileNamePair] : mapping )
        {
            QString iteration = iterFileNamePair.first;
            QString fileName  = iterFileNamePair.second;

            if ( groupIteration == iteration )
            {
                fileNamesFromIteration << fileName;
            }
        }
        groupedByIteration[groupIteration] = fileNamesFromIteration;
    }

    return groupedByIteration;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<QString, QStringList> RiaEnsembleNameTools::groupFilesByEnsembleName( const QStringList&               fileNames,
                                                                               RiaDefines::EnsembleGroupingMode groupingMode )
{
    std::map<QString, QStringList> ensemblePaths;

    if ( groupingMode == RiaDefines::EnsembleGroupingMode::FMU_FOLDER_STRUCTURE )
    {
        std::vector<std::string> allPaths;
        for ( const auto& fileName : fileNames )
        {
            allPaths.push_back( fileName.toStdString() );
        }

        auto groupedPaths = RiaEnsembleNameTools::groupFilePathsFmu( allPaths );

        auto shouldIncludeTopLevel = []( const auto& paths ) -> bool
        {
            std::set<std::string> topLevelFolderNames;
            for ( auto group : paths )
            {
                topLevelFolderNames.insert( group.first.first );
            }

            return topLevelFolderNames.size() > 1;
        };

        bool includeTopLevel = shouldIncludeTopLevel( groupedPaths );

        for ( auto group : groupedPaths )
        {
            std::string ensembleName;
            if ( includeTopLevel )
            {
                ensembleName += group.first.first;
                ensembleName += ", ";
            }
            ensembleName += group.first.second;

            QStringList groupPaths;
            for ( const auto& path : group.second )
            {
                groupPaths.push_back( QString::fromStdString( path ) );
            }

            ensemblePaths[QString::fromStdString( ensembleName )] = groupPaths;
        }
    }
    else if ( groupingMode == RiaDefines::EnsembleGroupingMode::EVEREST_FOLDER_STRUCTURE )
    {
        std::vector<std::string> allPaths;
        for ( const auto& fileName : fileNames )
        {
            allPaths.push_back( fileName.toStdString() );
        }
        auto groupedPaths = RiaEnsembleNameTools::groupFilePathsEverest( allPaths );
        for ( auto group : groupedPaths )
        {
            std::string ensembleName = group.first.first + ", " + group.first.second;
            QStringList groupPaths;
            for ( const auto& path : group.second )
            {
                groupPaths.push_back( QString::fromStdString( path ) );
            }
            ensemblePaths[QString::fromStdString( ensembleName )] = groupPaths;
        }
    }
    else if ( groupingMode == RiaDefines::EnsembleGroupingMode::RESINSIGHT_OPMFLOW_STRUCTURE )
    {
        std::vector<std::string> allPaths;
        for ( const auto& fileName : fileNames )
        {
            allPaths.push_back( fileName.toStdString() );
        }
        auto groupedPaths = RiaEnsembleNameTools::groupFilePathsOpm( allPaths );
        for ( auto group : groupedPaths )
        {
            std::string ensembleName = group.first.first + ", " + group.first.second;
            QStringList groupPaths;
            for ( const auto& path : group.second )
            {
                groupPaths.push_back( QString::fromStdString( path ) );
            }
            ensemblePaths[QString::fromStdString( ensembleName )] = groupPaths;
        }
    }

    return ensemblePaths;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<QString, std::pair<QString, QString>>
    RiaEnsembleNameTools::findUniqueCustomEnsembleNames( RiaDefines::FileType            fileType,
                                                         const QStringList&              fileNames,
                                                         const std::vector<QStringList>& fileNameComponents )
{
    CAF_ASSERT( fileNames.size() == static_cast<int>( fileNameComponents.size() ) );

    std::map<QString, std::pair<QString, QString>> mapping;

    int i = 0;
    for ( const auto& fileComponents : fileNameComponents )
    {
        int numComponents = fileComponents.size();

        if ( fileType == RiaDefines::FileType::STIMPLAN_SUMMARY )
        {
            if ( numComponents >= 8 && fileComponents[numComponents - 6] == "stimplan" )
            {
                // Expected path: realization/iteration/stimplan/output/well/job/fracture/data.csv
                // Example: realization-1/iteration-1/stimplan/output/H-B33/job-01/mainfrac/data.csv
                QString fileName    = fileNames[i];
                QString fracture    = fileComponents[numComponents - 2];
                QString job         = fileComponents[numComponents - 3];
                QString well        = fileComponents[numComponents - 4];
                QString iteration   = fileComponents[numComponents - 7];
                QString realization = fileComponents[numComponents - 8];

                QString key         = QString( "%1, %2, %3, %4, %5" ).arg( iteration, realization, well, fracture, job );
                QString displayName = QString( "%1, %2, %3, %4" ).arg( iteration, well, fracture, job );
                mapping[key]        = std::make_pair( displayName, fileName );
            }
        }
        else if ( fileType == RiaDefines::FileType::REVEAL_SUMMARY )
        {
            if ( numComponents >= 6 && fileComponents[numComponents - 4] == "reveal" )
            {
                // Expected path: realization/iteration/reveal/output/well/data.csv
                // Example: realization-1/iteration-1/reveal/output/H-B33/data.csv

                QString fileName    = fileNames[i];
                QString well        = fileComponents[numComponents - 2];
                QString iteration   = fileComponents[numComponents - 5];
                QString realization = fileComponents[numComponents - 6];
                QString key         = QString( "%1, %2, %3" ).arg( iteration, realization, well );
                QString displayName = QString( "%1, %2" ).arg( iteration, well );
                mapping[key]        = std::make_pair( displayName, fileName );
            }
        }
        i++;
    }

    return mapping;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaEnsembleNameTools::uniqueShortNameForEnsembleCase( RimSummaryCase* summaryCase )
{
    CAF_ASSERT( summaryCase && summaryCase->ensemble() );

    auto ensembleCaseName = summaryCase->nativeCaseName();
    auto ensemble         = summaryCase->ensemble();
    auto summaryCases     = ensemble->allSummaryCases();

    QStringList summaryFilePaths;
    summaryFilePaths.push_back( summaryCase->summaryHeaderFilename() );

    // Use a small number of names to find a short name for the ensemble, as RiaEnsembleNameTools::uniqueShortName is slow
    if ( summaryCases.size() > 1 )
    {
        const int maxNameCount = 4;
        for ( int i = 0; i < std::min( maxNameCount, static_cast<int>( summaryCases.size() ) ); ++i )
        {
            auto otherSummaryCase = summaryCases[i];
            if ( otherSummaryCase != summaryCase )
            {
                summaryFilePaths.push_back( otherSummaryCase->summaryHeaderFilename() );
            }
        }

        summaryFilePaths.push_back( summaryCases.back()->summaryHeaderFilename() );
    }

    return RiaEnsembleNameTools::uniqueShortName( summaryCase->summaryHeaderFilename(), summaryFilePaths, ensembleCaseName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaEnsembleNameTools::uniqueShortNameForSummaryCase( RimSummaryCase* summaryCase )
{
    std::set<QString> allAutoShortNames;

    std::vector<RimSummaryCase*> allCases = RimProject::current()->descendantsOfType<RimSummaryCase>();

    for ( auto sumCase : allCases )
    {
        if ( sumCase && sumCase != summaryCase )
        {
            allAutoShortNames.insert( sumCase->displayCaseName() );
        }
    }

    return RimCaseDisplayNameTools::uniqueShortName( summaryCase->nativeCaseName(),
                                                     allAutoShortNames,
                                                     RimCaseDisplayNameTools::CASE_SHORT_NAME_LENGTH );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RiaEnsembleNameTools::findUniqueEnsembleNames( const QStringList&               fileNames,
                                                           const std::vector<QStringList>&  fileNameComponents,
                                                           RiaDefines::EnsembleGroupingMode groupingMode )
{
    QStringList iterations;
    if ( groupingMode == RiaDefines::EnsembleGroupingMode::EVEREST_FOLDER_STRUCTURE )
    {
        QString     commonRoot           = RiaTextStringTools::commonRoot( fileNames );
        QStringList rootComponents       = RiaFilePathTools::splitPathIntoComponents( commonRoot );
        auto        commonComponentCount = rootComponents.size();

        // Example:
        // /myPath/batch_01/realization-1/....
        // /myPath/batch_01/realization-2/....
        // /myPath/batch_01/realization-N/....
        // /myPath/batch_02/realization-1/....
        // /myPath/batch_02/realization-2/....
        // /myPath/batch_02/realization-N/....

        // commonRoot will return /myPath/batch_
        // ensembleNameSet will contain [batch_01, batch_02]

        std::set<QString> ensembleNameSet;
        for ( const auto& componentsForFile : fileNameComponents )
        {
            if ( commonComponentCount - 1 < componentsForFile.size() )
            {
                ensembleNameSet.insert( componentsForFile[commonComponentCount - 1] );
            }
        }

        for ( const auto& ensembleName : ensembleNameSet )
        {
            iterations.push_back( ensembleName );
        }
    }
    else if ( groupingMode == RiaDefines::EnsembleGroupingMode::FMU_FOLDER_STRUCTURE )
    {
        // Find list of all folders inside a folder matching realization-*
        QRegularExpression realizationRe( "realization\\-\\d+" );

        for ( const auto& fileComponents : fileNameComponents )
        {
            QString lastComponent = "";
            for ( auto it = fileComponents.rbegin(); it != fileComponents.rend(); ++it )
            {
                if ( realizationRe.match( *it ).hasMatch() )
                {
                    iterations.push_back( lastComponent );
                }
                lastComponent = *it;
            }
        }
    }

    iterations.removeDuplicates();
    return iterations;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaEnsembleNameTools::findCommonBaseName( const QStringList& fileNames )
{
    QStringList baseNames;
    for ( const auto& f : fileNames )
    {
        QFileInfo fi( f );
        baseNames.push_back( fi.baseName() );
    }

    if ( baseNames.isEmpty() ) return "Empty";

    auto firstName = baseNames.front();
    for ( int i = 0; i < firstName.size(); i++ )
    {
        auto candidate      = firstName.left( firstName.size() - i );
        bool identicalNames = true;

        for ( const auto& baseName : baseNames )
        {
            auto str = baseName.left( firstName.size() - i );
            if ( candidate != str ) identicalNames = false;
        }

        if ( identicalNames )
        {
            return candidate;
        }
    }

    return "Mixed Items";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaEnsembleNameTools::uniqueShortName( const QString& sourceFileName, const QStringList& allFileNames, const QString& ensembleCaseName )
{
    // Handle ensemble with only one realizations separately.
    if ( allFileNames.size() == 1 ) return ensembleCaseName;

    std::map<QString, QStringList> keyFileComponentsForAllFiles = RiaFilePathTools::keyPathComponentsForEachFilePath( allFileNames );
    return uniqueShortNameFromComponents( sourceFileName, keyFileComponentsForAllFiles, ensembleCaseName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaEnsembleNameTools::uniqueShortNameFromComponents( const QString&                        sourceFileName,
                                                             const std::map<QString, QStringList>& keyFileComponentsForAllFiles,
                                                             const QString&                        ensembleCaseName )
{
    QRegularExpression trimRe( "^[^a-zA-Z0-9]+" );

    auto        modifyableMap( keyFileComponentsForAllFiles );
    QStringList keyFileComponents = modifyableMap[sourceFileName];
    if ( keyFileComponents.empty() )
    {
        return "Unnamed";
    }

    if ( !ensembleCaseName.isEmpty() )
    {
        for ( auto& component : keyFileComponents )
        {
            component = component.replace( ensembleCaseName, "" );
            component = component.replace( trimRe, "" );
        }
    }

    QStringList        shortNameComponents;
    QRegularExpression numberRe( "[0-9]+" );
    for ( auto keyComponent : keyFileComponents )
    {
        QStringList subComponents;
        QString     numberGroup = numberRe.match( keyComponent ).captured();
        if ( !numberGroup.isEmpty() )
        {
            keyComponent = keyComponent.replace( numberGroup, "" );
            QString stem = keyComponent.left( RimCaseDisplayNameTools::CASE_SHORT_NAME_LENGTH );
            stem         = stem.remove( "-" );
            if ( !stem.isEmpty() ) subComponents.push_back( stem );
            subComponents.push_back( numberGroup );
        }
        else
        {
            subComponents.push_back( keyComponent.left( RimCaseDisplayNameTools::CASE_SHORT_NAME_LENGTH ) );
        }

        shortNameComponents.push_back( subComponents.join( "-" ) );
    }
    return shortNameComponents.join( "," );
}
