/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 Equinor ASA
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

#include "RicWellPathExportCompletionsFileTools.h"

#include "RiaFilePathTools.h"
#include "RiaLogging.h"

#include "RimProject.h"
#include "RimWellPath.h"
#include "RimWellPathCompletions.h"

#include "cafUtils.h"

#include <QDir>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicWellPathExportCompletionsFileTools::OpenFileException::OpenFileException( const QString& message )
    : message( message )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::shared_ptr<QFile> RicWellPathExportCompletionsFileTools::openFileForExport( const QString& fullFileName )
{
    std::pair<QString, QString> folderAndFileName = RiaFilePathTools::toFolderAndFileName( fullFileName );
    return openFileForExport( folderAndFileName.first, folderAndFileName.second );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::shared_ptr<QFile> RicWellPathExportCompletionsFileTools::openFileForExport( const QString& folderName,
                                                                                 const QString& fileName,
                                                                                 const QString& suffix )
{
    QString validFileName = caf::Utils::makeValidFileBasename( fileName );

    QDir exportFolder = QDir( folderName );
    if ( !exportFolder.exists() )
    {
        bool createdPath = exportFolder.mkpath( "." );
        if ( createdPath )
            RiaLogging::info( "Created export folder " + folderName );
        else
        {
            auto errorMessage = QString( "Selected output folder does not exist, and could not be created." );
            RiaLogging::error( errorMessage );
            throw OpenFileException( errorMessage );
        }
    }

    QString filePath = exportFolder.filePath( validFileName );
    if ( !suffix.isEmpty() ) filePath += "." + suffix;
    std::shared_ptr<QFile> exportFile( new QFile( filePath ) );
    if ( !exportFile->open( QIODevice::WriteOnly | QIODevice::Text ) )
    {
        auto errorMessage = QString( "Export Completions Data: Could not open the file: %1" ).arg( filePath );
        RiaLogging::error( errorMessage );
        throw OpenFileException( errorMessage );
    }
    return exportFile;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::shared_ptr<QFile> RicWellPathExportCompletionsFileTools::openFileForExport( const QString& folderName,
                                                                                 const QString& fileName )
{
    return openFileForExport( folderName, fileName, "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RimWellPath* RicWellPathExportCompletionsFileTools::findWellPathFromExportName( const QString& wellNameForExport )
{
    auto allWellPaths = RimProject::current()->allWellPaths();

    for ( const auto wellPath : allWellPaths )
    {
        if ( wellPath->completionSettings()->wellNameForExport() == wellNameForExport ) return wellPath;
    }
    return nullptr;
}
