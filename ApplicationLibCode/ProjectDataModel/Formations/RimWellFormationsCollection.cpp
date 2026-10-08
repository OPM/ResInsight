/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RimWellFormationsCollection.h"

#include "RimWellFormationsFile.h"

#include "RiuMessageDialog.h"

#include "cafCmdFeatureMenuBuilder.h"

#include <QFileInfo>

#include <memory>

CAF_PDM_SOURCE_INIT( RimWellFormationsCollection, "WellFormationsCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellFormationsCollection::RimWellFormationsCollection()
{
    CAF_PDM_InitObject( "Well Picks (Formations)", ":/FormationCollection16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_wellFormationsFiles, "WellFormationsFiles", "Well Formations" );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
/// Returns the existing entry for filePath if present, otherwise imports and returns a new one. A
/// file that cannot be read is not added.
//--------------------------------------------------------------------------------------------------
std::expected<RimWellFormationsFile*, QString> RimWellFormationsCollection::findOrCreate( const QString& filePath )
{
    const QString absoluteFilePath = QFileInfo( filePath ).absoluteFilePath();

    for ( RimWellFormationsFile* file : m_wellFormationsFiles )
    {
        if ( QFileInfo( file->filePath() ).absoluteFilePath() == absoluteFilePath )
        {
            // An earlier read may have failed, e.g. if the file was missing or malformed at the time
            if ( file->wellNames().isEmpty() )
            {
                if ( auto result = file->reload(); !result ) return std::unexpected( result.error() );
            }
            return file;
        }
    }

    auto newFile = std::make_unique<RimWellFormationsFile>();
    newFile->setFilePath( filePath );
    if ( auto result = newFile->reload(); !result )
    {
        return std::unexpected( result.error() );
    }

    m_wellFormationsFiles.push_back( newFile.get() );

    return newFile.release();
}

//--------------------------------------------------------------------------------------------------
/// Returns the first file containing zone picks for wellName (matched via RimWellFormationsFile's
/// normalized well name comparison), or nullptr if no file has a matching well.
//--------------------------------------------------------------------------------------------------
RimWellFormationsFile* RimWellFormationsCollection::findFileForWell( const QString& wellName ) const
{
    for ( RimWellFormationsFile* file : m_wellFormationsFiles )
    {
        if ( file->formationsForWell( wellName ) ) return file;
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellFormationsFile*> RimWellFormationsCollection::importFiles( const QStringList& filePaths )
{
    std::vector<RimWellFormationsFile*> importedFiles;
    QString                             totalErrorMessage;

    for ( const QString& filePath : filePaths )
    {
        auto file = findOrCreate( filePath );
        if ( !file )
        {
            totalErrorMessage += "\nError in: " + filePath + "\n\t" + file.error();
            continue;
        }

        importedFiles.push_back( *file );
    }

    if ( !totalErrorMessage.isEmpty() )
    {
        RiuMessageDialog::showError( nullptr, "Import Well Formations", totalErrorMessage );
    }

    return importedFiles;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellFormationsCollection::onChildDeleted( caf::PdmChildArrayFieldHandle*      childArray,
                                                  std::vector<caf::PdmObjectHandle*>& referringObjects )
{
    RimWellFormationsFile::updateReferringObjects( referringObjects );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellFormationsCollection::appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const
{
    menuBuilder << "RicImportWellFormationsFeature";
}
