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

#include "RiaLogging.h"
#include "RimWellFormationsFile.h"

#include "RiuMessageDialog.h"

#include "cafCmdFeatureMenuBuilder.h"

#include <QFileInfo>

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
/// Returns the existing entry for filePath if present, otherwise imports and returns a new one
//--------------------------------------------------------------------------------------------------
RimWellFormationsFile* RimWellFormationsCollection::findOrCreate( const QString& filePath )
{
    const QString absoluteFilePath = QFileInfo( filePath ).absoluteFilePath();

    for ( RimWellFormationsFile* file : m_wellFormationsFiles )
    {
        if ( QFileInfo( file->filePath() ).absoluteFilePath() == absoluteFilePath )
        {
            return file;
        }
    }

    auto newFile = new RimWellFormationsFile();
    newFile->setFilePath( filePath );
    if ( auto result = newFile->reload(); !result )
    {
        RiaLogging::error( result.error().toStdString() );
    }

    m_wellFormationsFiles.push_back( newFile );

    return newFile;
}

//--------------------------------------------------------------------------------------------------
/// Returns the first file containing zone picks for wellName (matched via RimWellFormationsFile's
/// normalized well name comparison), or nullptr if no file has a matching well.
//--------------------------------------------------------------------------------------------------
RimWellFormationsFile* RimWellFormationsCollection::findFileForWell( const QString& wellName ) const
{
    for ( RimWellFormationsFile* file : m_wellFormationsFiles )
    {
        if ( file->formationsForWell( wellName ).has_value() ) return file;
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
        auto newFile = new RimWellFormationsFile();
        newFile->setFilePath( filePath );
        if ( auto result = newFile->reload(); !result )
        {
            totalErrorMessage += "\nError in: " + filePath + "\n\t" + result.error();
        }

        m_wellFormationsFiles.push_back( newFile );
        importedFiles.push_back( newFile );
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
void RimWellFormationsCollection::appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const
{
    menuBuilder << "RicImportWellFormationsFeature";
}
