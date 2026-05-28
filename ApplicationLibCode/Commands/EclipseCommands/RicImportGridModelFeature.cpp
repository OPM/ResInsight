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

#include "RicImportGridModelFeature.h"

#include "RiaApplication.h"
#include "RiaDefines.h"
#include "RiaGuiApplication.h"
#include "RiaLogging.h"

#include "RicImportEclipseCaseFeature.h"
#include "RicImportGeneralDataFeature.h"

#include "RimEclipseView.h"
#include "RimProject.h"

#include "RiuFileDialogTools.h"

#include <QAction>
#include <QFileInfo>

CAF_CMD_SOURCE_INIT( RicImportGridModelFeature, "RicImportGridModelFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportGridModelFeature::onActionTriggered( bool isChecked )
{
    RiaApplication* app = RiaApplication::instance();

    const QString binaryGridPattern = "*.GRID *.EGRID";
    const QString textGridPattern   = "*.GRDECL";
    const QString roffPattern       = "*.ROFF *.ROFFASC";
    const QString allGridsPattern   = QString( "%1 %2 %3" ).arg( binaryGridPattern, textGridPattern, roffPattern );

    const QString binaryGridFilter = QString( "Binary Grid Models (%1)" ).arg( binaryGridPattern );

    const QStringList filterGroups = { QString( "Grid Models (%1)" ).arg( allGridsPattern ),
                                       binaryGridFilter,
                                       QString( "Text Grid Models (%1)" ).arg( textGridPattern ),
                                       QString( "Roff Grid Models (%1)" ).arg( roffPattern ) };

    const QString defaultDirLabel = RiaDefines::defaultDirectoryLabel( RiaDefines::ImportFileType::ANY_ECLIPSE_FILE );
    const QString defaultDir      = app->lastUsedDialogDirectory( defaultDirLabel );

    // Default to the binary grid filter (GRID/EGRID) when the dialog opens.
    QString selectedFilter = binaryGridFilter;

    // Use nullptr as parent: this command can be invoked from both the 3D main window and the plot window.
    QStringList fileNames = RiuFileDialogTools::getOpenFileNames( RiaGuiApplication::widgetToUseAsParent(),
                                                                  "Import Grid Models",
                                                                  defaultDir,
                                                                  filterGroups.join( ";;" ),
                                                                  &selectedFilter );

    if ( fileNames.isEmpty() ) return;

    app->setLastUsedDialogDirectory( defaultDirLabel, QFileInfo( fileNames.last() ).absolutePath() );

    // Track existing views so newly imported PVD surfaces can be enabled only in new views.
    RimProject*                        project        = RimProject::current();
    const std::vector<RimEclipseView*> allViewsBefore = RicImportEclipseCaseFeature::allEclipseViews( project );

    constexpr bool doCreateDefaultPlot = true;
    constexpr bool createDefaultView   = true;
    const auto results = RicImportGeneralDataFeature::openEclipseFilesFromFileNames( fileNames, doCreateDefaultPlot, createDefaultView );

    if ( !results )
    {
        RiaLogging::error( std::format( "Failed to open grid model files: {}", fileNames.join( ", " ).toStdString() ) );
        return;
    }

    // openEclipseFilesFromFileNames already adds GRDECL/ROFF/summary files to recent files;
    // the binary Eclipse grid path does not, so add those here.
    for ( const QString& f : results.eclipseCaseFiles )
    {
        app->addToRecentFiles( f );
    }

    // Auto-import matching PVD surface files for any imported binary grid files.
    if ( !results.eclipseCaseFiles.isEmpty() )
    {
        RicImportEclipseCaseFeature::importPvdSurfacesForGridFiles( results.eclipseCaseFiles, allViewsBefore );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportGridModelFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/Case.svg" ) );
    actionToSetup->setText( "Import Grid Models" );
}
