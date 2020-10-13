/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RicImportWellPaths.h"

#include "RiaApplication.h"
#include "RiaGuiApplication.h"
#include "RiaLogging.h"

#include "RimFileWellPath.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "Riu3DMainWindowTools.h"
#include "RiuFileDialogTools.h"

#include "cafPdmFieldScriptingCapability.h"

#include <QAction>
#include <QDir>

//==================================================================================================
///
///
//==================================================================================================
class RicImportWellPathsResult : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RicImportWellPathsResult()
    {
        CAF_PDM_InitObject( "well_path_result", "", "", "" );
        CAF_PDM_InitFieldNoDefault( &wellPathNames, "wellPathNames", "", "", "", "" );
    }

public:
    caf::PdmField<std::vector<QString>> wellPathNames;
};

CAF_PDM_SOURCE_INIT( RicImportWellPathsResult, "importWellPathsResult" );
RICF_SOURCE_INIT( RicImportWellPaths, "RicWellPathsImportFileFeature", "importWellPaths" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicImportWellPaths::RicImportWellPaths()
{
    CAF_PDM_InitScriptableFieldNoDefault( &m_wellPathFolder, "wellPathFolder", "", "", "", "" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_wellPathFiles, "wellPathFiles", "", "", "", "" );
}

caf::PdmScriptResponse RicImportWellPaths::execute()
{
    QStringList errorMessages, warningMessages;
    QStringList wellPathFiles;

    QDir wellPathDir;
    if ( m_wellPathFolder().isEmpty() )
    {
        wellPathDir = QDir( RiaApplication::instance()->startDir() );
    }
    else
    {
        wellPathDir = QDir( m_wellPathFolder );
    }

    if ( !m_wellPathFolder().isEmpty() )
    {
        if ( wellPathDir.exists() )
        {
            QStringList nameFilters;
            nameFilters << RicImportWellPaths::wellPathNameFilters();
            QStringList relativePaths = wellPathDir.entryList( nameFilters, QDir::Files | QDir::NoDotAndDotDot );
            for ( QString relativePath : relativePaths )
            {
                wellPathFiles.push_back( wellPathDir.absoluteFilePath( relativePath ) );
            }
        }
        else
        {
            errorMessages << ( wellPathDir.absolutePath() + " does not exist" );
        }
    }

    for ( QString wellPathFile : m_wellPathFiles() )
    {
        if ( QFileInfo::exists( wellPathFile ) )
        {
            wellPathFiles.push_back( wellPathFile );
        }
        else if ( QFileInfo::exists( wellPathDir.absoluteFilePath( wellPathFile ) ) )
        {
            wellPathFiles.push_back( wellPathDir.absoluteFilePath( wellPathFile ) );
        }
        else
        {
            errorMessages << ( wellPathFile + " does not exist" );
        }
    }

    caf::PdmScriptResponse response;
    if ( !wellPathFiles.empty() )
    {
        std::vector<RimWellPath*> importedWellPaths = importWellPaths( wellPathFiles, &warningMessages );
        if ( !importedWellPaths.empty() )
        {
            RicImportWellPathsResult* wellPathsResult = new RicImportWellPathsResult;
            for ( RimWellPath* wellPath : importedWellPaths )
            {
                wellPathsResult->wellPathNames.v().push_back( wellPath->name() );
            }

            response.setResult( wellPathsResult );
        }
    }
    else
    {
        warningMessages << "No well paths found";
    }

    for ( QString warningMessage : warningMessages )
    {
        response.updateStatus( caf::PdmScriptResponse::COMMAND_WARNING, warningMessage );
    }

    for ( QString errorMessage : errorMessages )
    {
        response.updateStatus( caf::PdmScriptResponse::COMMAND_ERROR, errorMessage );
    }

    return response;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPath*> RicImportWellPaths::importWellPaths( const QStringList& wellPathFilePaths,
                                                               QStringList*       errorMessages )
{
    RiaApplication* app = RiaApplication::instance();

    // Remember the path to next time
    app->setLastUsedDialogDirectory( "WELLPATH_DIR", QFileInfo( wellPathFilePaths.last() ).absolutePath() );

    std::vector<RimWellPath*> wellPaths = app->addWellPathsToModel( wellPathFilePaths, errorMessages );

    RimProject* project = app->project();

    if ( project )
    {
        project->scheduleCreateDisplayModelAndRedrawAllViews();
        RimOilField* oilField = project->activeOilField();

        if ( oilField && oilField->wellPathCollection->wellPaths().size() > 0 )
        {
            RimWellPath* wellPath = oilField->wellPathCollection->mostRecentlyUpdatedWellPath();
            if ( wellPath )
            {
                Riu3DMainWindowTools::selectAsCurrentItem( wellPath );
            }
        }
    }
    return wellPaths;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RicImportWellPaths::wellPathNameFilters()
{
    QStringList nameFilters;
    nameFilters << "*.json"
                << "*.asc"
                << " *.asci"
                << "*.ascii"
                << "*.dev";
    return nameFilters;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicImportWellPaths::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportWellPaths::onActionTriggered( bool isChecked )
{
    // Open dialog box to select well path files
    RiaApplication* app                = RiaApplication::instance();
    QString         lastUsedGridFolder = app->lastUsedDialogDirectory( "BINARY_GRID" );
    QString         defaultDir         = app->lastUsedDialogDirectoryWithFallback( "WELLPATH_DIR", lastUsedGridFolder );

    QString nameList = QString( "Well Paths (%1);;All Files (*.*)" ).arg( wellPathNameFilters().join( " " ) );

    QStringList wellPathFilePaths = RiuFileDialogTools::getOpenFileNames( Riu3DMainWindowTools::mainWindowWidget(),
                                                                          "Import Well Paths",
                                                                          defaultDir,
                                                                          nameList );

    if ( wellPathFilePaths.size() >= 1 )
    {
        m_wellPathFiles.v()             = std::vector<QString>( wellPathFilePaths.begin(), wellPathFilePaths.end() );
        caf::PdmScriptResponse response = execute();
        QStringList            messages = response.messages();

        if ( !messages.empty() )
        {
            QString displayMessage = QString( "Problem loading well path files:\n%2" ).arg( messages.join( "\n" ) );
            RiaLogging::errorInMessageBox( Riu3DMainWindowTools::mainWindowWidget(), "Well Path Loading", displayMessage );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportWellPaths::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Import &Well Paths from File" );
    actionToSetup->setIcon( QIcon( ":/Well.png" ) );
}
