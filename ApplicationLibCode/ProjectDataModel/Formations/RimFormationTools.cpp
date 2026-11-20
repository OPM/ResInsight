/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 - Equinor ASA
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

#include "RimFormationTools.h"

#include "RiaLogging.h"

#include "RigFormationNames.h"

#include "Formations/RimFormationNames.h"
#include "Formations/RimFormationNamesCollection.h"
#include "RimCaseCollection.h"
#include "RimEclipseCase.h"
#include "RimReservoirGridEnsembleBase.h"

#include "cafUtils.h"

#include <QDir>
#include <QFileInfo>
#include <QStringList>

#include <algorithm>
#include <set>

//--------------------------------------------------------------------------------------------------
///  look for formation file two levels up from the egrid file
//--------------------------------------------------------------------------------------------------
QStringList RimFormationTools::formationFoldersFromCaseFileName( const QString caseFileName )
{
    QStringList folders;

    QFileInfo fi( caseFileName );

    auto formationFolder = QDir( fi.dir().path() + "/../../" );
    auto rmsFolder       = QDir( fi.dir().path() + "/../../rms/output/zone/" );

    folders.push_back( rmsFolder.absolutePath() ); // rms folder is 1st pri.
    folders.push_back( formationFolder.absolutePath() );

    return folders;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFormationNames* RimFormationTools::loadFormationNamesFromFolder( const QStringList& folderNames )
{
    // Priority: .lyr (may contain colors), then zone range yaml files, then FMU layer zone table
    const std::vector<QStringList> filterPriority = { { "*.lyr" },
                                                      { "zonation_ecl_map.yml", "zonation_ecl_map.yaml", "zone_layers.yml", "zone_layers.yaml" },
                                                      { "layer_zone_table.txt" } };

    QStringList fileList;

    for ( auto& folder : folderNames )
    {
        for ( const QStringList& filters : filterPriority )
        {
            fileList = caf::Utils::getFilesInDirectory( folder, filters, true /*absolute filename*/ );
            if ( !fileList.isEmpty() ) break;
        }
        if ( !fileList.isEmpty() ) break;
    }

    if ( fileList.isEmpty() ) return nullptr;

    RimFormationNamesCollection*    fomNameColl    = new RimFormationNamesCollection();
    std::vector<RimFormationNames*> formationNames = fomNameColl->importFiles( fileList );
    if ( formationNames.size() > 1 )
    {
        RiaLogging::warning( "Multiple formation name files found in ensemble folders." );
    }

    return formationNames.front();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<std::vector<int>, QString> RimFormationTools::formationIndicesFromNames( const std::vector<QString>& availableNames,
                                                                                       const std::vector<QString>& requestedNames )
{
    std::vector<int> indices;

    if ( requestedNames.empty() )
    {
        for ( size_t i = 0; i < availableNames.size(); i++ )
        {
            indices.push_back( static_cast<int>( i ) );
        }
        return indices;
    }

    std::set<int> selected;
    QStringList   unknownNames;
    for ( const QString& name : requestedNames )
    {
        bool found = false;
        for ( size_t i = 0; i < availableNames.size(); i++ )
        {
            if ( availableNames[i] == name )
            {
                selected.insert( static_cast<int>( i ) );
                found = true;
            }
        }
        if ( !found && !unknownNames.contains( name ) ) unknownNames.push_back( name );
    }

    if ( !unknownNames.isEmpty() )
    {
        QStringList available;
        for ( const QString& name : unionOfFormationNames( { availableNames } ) )
        {
            available.push_back( name );
        }

        return std::unexpected( QString( "Unknown formation name(s): %1. Available formations: %2" )
                                    .arg( unknownNames.join( ", " ) )
                                    .arg( available.isEmpty() ? QString( "none" ) : available.join( ", " ) ) );
    }

    indices.assign( selected.begin(), selected.end() );
    return indices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RimFormationTools::formationNamesFromIndices( const std::vector<QString>& availableNames, const std::vector<int>& indices )
{
    const std::set<int> selected( indices.begin(), indices.end() );

    std::vector<QString> names;
    for ( size_t i = 0; i < availableNames.size(); i++ )
    {
        if ( !selected.contains( static_cast<int>( i ) ) ) continue;
        if ( std::find( names.begin(), names.end(), availableNames[i] ) != names.end() ) continue;

        names.push_back( availableNames[i] );
    }

    return names;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RimFormationTools::unionOfFormationNames( const std::vector<std::vector<QString>>& formationNameLists )
{
    std::vector<QString> names;
    for ( const auto& list : formationNameLists )
    {
        for ( const QString& name : list )
        {
            if ( std::find( names.begin(), names.end(), name ) == names.end() ) names.push_back( name );
        }
    }

    return names;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RimFormationTools::formationNames( const RimEclipseCase* eclipseCase )
{
    if ( !eclipseCase ) return {};

    // Same resolution as RimEclipseCase::effectiveFormationNames(): the case's own formations, falling
    // back to the shared formations of the parent ensemble
    auto namesFrom = []( const RimFormationNames* formationNames ) -> std::vector<QString>
    {
        if ( !formationNames || !formationNames->formationNamesData() ) return {};
        return formationNames->formationNamesData()->formationNames();
    };

    if ( eclipseCase->activeFormationNames() && eclipseCase->activeFormationNames()->formationNamesData() )
    {
        return namesFrom( eclipseCase->activeFormationNames() );
    }

    if ( auto* caseCollection = const_cast<RimEclipseCase*>( eclipseCase )->parentCaseCollection() )
    {
        if ( auto* ensemble = caseCollection->parentGridEnsembleBase() )
        {
            return namesFrom( ensemble->activeFormationNames() );
        }
    }

    return {};
}
