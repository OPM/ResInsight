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

#include "RimCaseDisplayNameTools.h"

#include "cafAppEnum.h"

#include <QFileInfo>
#include <QRegularExpression>

template <>
void caf::AppEnum<RiaEnsembleNameTools::EnsembleGroupingMode>::setUp()
{
    addItem( RiaEnsembleNameTools::EnsembleGroupingMode::FIRST_FOLDER, "FIRST_FOLDER", "First Folder" );
    addItem( RiaEnsembleNameTools::EnsembleGroupingMode::SECOND_FOLDER, "SECOND_FOLDER", "Second Folder" );
    addItem( RiaEnsembleNameTools::EnsembleGroupingMode::NONE, "None", "None" );
    setDefault( RiaEnsembleNameTools::EnsembleGroupingMode::SECOND_FOLDER );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaEnsembleNameTools::findSuitableEnsembleName( const QStringList& fileNames, EnsembleGroupingMode folderLevel )
{
    if ( folderLevel == EnsembleGroupingMode::FIRST_FOLDER )
    {
        QString     commonRoot     = RiaTextStringTools::commonRoot( fileNames );
        QStringList rootComponents = RiaFilePathTools::splitPathIntoComponents( commonRoot );

        if ( rootComponents.size() > 2 )
        {
            return rootComponents[rootComponents.size() - 2];
        }

        return "Ensemble";
    }

    QStringList iterations = findUniqueIterations( fileNames, folderLevel );
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
std::vector<QStringList> RiaEnsembleNameTools::groupFilesByEnsemble( const QStringList&   fileNames,
                                                                     EnsembleGroupingMode groupingMode )
{
    QStringList iterations = findUniqueIterations( fileNames, groupingMode );
    if ( iterations.size() <= 1 )
    {
        // All the files are in the same ensemble
        return { fileNames };
    }

    std::vector<QStringList> groupedByIteration;
    for ( auto iteration : iterations )
    {
        QStringList fileNamesFromIteration;
        for ( auto filePath : fileNames )
        {
            if ( filePath.contains( iteration ) )
            {
                fileNamesFromIteration << filePath;
            }
        }
        groupedByIteration.push_back( fileNamesFromIteration );
    }

    return groupedByIteration;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RiaEnsembleNameTools::findUniqueIterations( const QStringList& fileNames, EnsembleGroupingMode groupingMode )
{
    std::vector<QStringList> componentsForAllFilePaths;

    for ( const auto& filePath : fileNames )
    {
        QStringList components = RiaFilePathTools::splitPathIntoComponents( filePath );
        componentsForAllFilePaths.push_back( components );
    }

    QStringList iterations;
    if ( groupingMode == EnsembleGroupingMode::FIRST_FOLDER )
    {
        QString     commonRoot           = RiaTextStringTools::commonRoot( fileNames );
        QStringList rootComponents       = RiaFilePathTools::splitPathIntoComponents( commonRoot );
        auto        commonComponentCount = rootComponents.size();

        std::set<QString> ensembleSet;
        for ( const auto& componentsForFile : componentsForAllFilePaths )
        {
            if ( commonComponentCount - 1 < componentsForFile.size() )
            {
                ensembleSet.insert( componentsForFile[commonComponentCount - 1] );
            }
        }

        for ( const auto& ensembleName : ensembleSet )
        {
            iterations.push_back( ensembleName );
        }
    }
    else if ( groupingMode == EnsembleGroupingMode::SECOND_FOLDER )
    {
        // Find list of all folders inside a folder matching realization-*
        QRegularExpression realizationRe( "realization\\-\\d+" );

        for ( const auto& fileComponents : componentsForAllFilePaths )
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
QString RiaEnsembleNameTools::uniqueShortName( const QString&     sourceFileName,
                                               const QStringList& allFileNames,
                                               const QString&     ensembleCaseName )
{
    std::map<QString, QStringList> keyFileComponentsForAllFiles =
        RiaFilePathTools::keyPathComponentsForEachFilePath( allFileNames );

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
    if ( keyFileComponents.empty() ) return "Unnamed";

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
