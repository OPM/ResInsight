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

#include "RicNewGenericDataViewFeature.h"

#include "RiaApplication.h"
#include "RiaGuiApplication.h"
#include "RiaLogging.h"
#include "RiuMessageDialog.h"

#include "RimFileWellPath.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "RimcDataContainerString.h"
#include "RimcWellPathCollection.h"

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
        CAF_PDM_InitObject( "well_path_result" );
        CAF_PDM_InitFieldNoDefault( &wellPathNames, "wellPathNames", "" );
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
    CAF_PDM_InitScriptableFieldNoDefault( &m_wellPathFolder, "wellPathFolder", "" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_wellPathFiles, "wellPathFiles", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmScriptResponse RicImportWellPaths::execute()
{
    RimProject* project = RimProject::current();
    if ( !project || !project->activeOilField() || !project->activeOilField()->wellPathCollection() )
    {
        return caf::PdmScriptResponse( caf::PdmScriptResponse::COMMAND_ERROR, "importWellPaths: No well path collection available" );
    }

    RimWellPathCollection_importWellPaths method( project->activeOilField()->wellPathCollection() );
    method.setWellPathFiles( m_wellPathFiles() );
    method.setWellPathFolder( m_wellPathFolder() );

    auto result = method.execute();

    caf::PdmScriptResponse response;
    for ( const QString& warningMessage : method.warnings() )
    {
        response.updateStatus( caf::PdmScriptResponse::COMMAND_WARNING, warningMessage );
    }

    if ( !result )
    {
        // Legacy behavior: "no files found" is a warning, missing files are errors
        if ( result.error() == "No well path files found" )
        {
            response.updateStatus( caf::PdmScriptResponse::COMMAND_WARNING, "No well paths found" );
        }
        else
        {
            response.updateStatus( caf::PdmScriptResponse::COMMAND_ERROR, result.error() );
        }
        return response;
    }

    auto* names = dynamic_cast<RimcDataContainerString*>( result.value() );
    if ( names && !names->m_stringValues().empty() )
    {
        auto* wellPathsResult          = new RicImportWellPathsResult;
        wellPathsResult->wellPathNames = names->m_stringValues();
        response.setResult( wellPathsResult );
    }
    delete names;

    return response;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPath*> RicImportWellPaths::importWellPaths( const QStringList& wellPathFilePaths, QStringList* errorMessages )
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

        if ( oilField && !oilField->wellPathCollection->allWellPaths().empty() )
        {
            RicNewGenericDataViewFeature::createInitialViewIfNeeded();

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
    nameFilters << "*.json" << "*.asc" << " *.asci" << "*.ascii" << "*.dev" << "*.rmswell" << "*.w";
    return nameFilters;
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

    QStringList wellPathFilePaths =
        RiuFileDialogTools::getOpenFileNames( Riu3DMainWindowTools::mainWindowWidget(), "Import Well Paths", defaultDir, nameList );

    if ( !wellPathFilePaths.empty() )
    {
        m_wellPathFiles.v()             = std::vector<QString>( wellPathFilePaths.begin(), wellPathFilePaths.end() );
        caf::PdmScriptResponse response = execute();
        QStringList            messages = response.messages();

        if ( !messages.empty() )
        {
            QString displayMessage = QString( "Problem loading well path files:\n%2" ).arg( messages.join( "\n" ) );
            RiuMessageDialog::showError( Riu3DMainWindowTools::mainWindowWidget(), "Well Path Loading", displayMessage );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportWellPaths::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Import &Well Paths from File" );
    actionToSetup->setIcon( QIcon( ":/Well.svg" ) );
}
