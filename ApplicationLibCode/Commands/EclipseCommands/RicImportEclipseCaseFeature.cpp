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

#include "RicImportEclipseCaseFeature.h"

#include "RiaApplication.h"
#include "RiaPreferencesGrid.h"

#include "RifReaderSettings.h"

#include "RimEclipseCase.h"
#include "RimEclipseCaseCollection.h"
#include "RimEclipseView.h"
#include "RimProject.h"
#include "RimSurface.h"
#include "RimSurfaceCollection.h"
#include "RimSurfaceInView.h"
#include "RimSurfaceInViewCollection.h"
#include "RimTools.h"

#include "Riu3DMainWindowTools.h"
#include "RiuFileDialogTools.h"

#include <QAction>
#include <QDir>
#include <QFileInfo>

CAF_CMD_SOURCE_INIT( RicImportEclipseCaseFeature, "RicImportEclipseCaseFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportEclipseCaseFeature::onActionTriggered( bool isChecked )
{
    RiaApplication* app = RiaApplication::instance();

    QString     defaultDir = app->lastUsedDialogDirectory( "BINARY_GRID" );
    QStringList fileNames  = RiuFileDialogTools::getOpenFileNames( Riu3DMainWindowTools::mainWindowWidget(),
                                                                  "Import Eclipse File",
                                                                  defaultDir,
                                                                  "Eclipse Grid Files (*.GRID *.EGRID)" );

    if ( fileNames.isEmpty() ) return;

    defaultDir = QFileInfo( fileNames.last() ).absolutePath();
    app->setLastUsedDialogDirectory( "BINARY_GRID", defaultDir );

    bool              createDefaultView = true;
    std::vector<int>  caseIds;
    RifReaderSettings readerSettings = RiaPreferencesGrid::current()->readerSettings();

    // Track existing views before import
    RimProject*                  project        = RimProject::current();
    std::vector<RimEclipseView*> allViewsBefore = allEclipseViews( project );

    openEclipseCaseFromFileNames( fileNames, createDefaultView, caseIds, readerSettings );

    for ( const auto& f : fileNames )
    {
        RiaApplication::instance()->addToRecentFiles( f );
    }

    // Auto-import matching PVD surface files
    if ( RimSurfaceCollection* surfColl = RimTools::surfaceCollection() )
    {
        QStringList pvdFilesToImport = findPvdFilesToImport( fileNames );

        if ( !pvdFilesToImport.isEmpty() )
        {
            // Get count of existing surfaces before import
            auto allSurfacesBefore = surfColl->descendantsIncludingThisOfType<RimSurface>();

            surfColl->importSurfacesFromFiles( pvdFilesToImport );

            // Get newly imported surfaces
            auto                     allSurfacesAfter = surfColl->descendantsIncludingThisOfType<RimSurface>();
            std::vector<RimSurface*> newlyImportedSurfaces;
            for ( auto* surf : allSurfacesAfter )
            {
                if ( std::find( allSurfacesBefore.begin(), allSurfacesBefore.end(), surf ) == allSurfacesBefore.end() )
                {
                    newlyImportedSurfaces.push_back( surf );
                }
            }

            // Enable newly imported surfaces in new views, disable them in old views
            if ( project && !newlyImportedSurfaces.empty() )
            {
                // Process all views (both old and new)
                for ( auto* eclipseCase : project->eclipseCases() )
                {
                    for ( RimEclipseView* view : eclipseCase->reservoirViews() )
                    {
                        if ( RimSurfaceInViewCollection* surfInViewColl = view->surfaceInViewCollection() )
                        {
                            // Check if this is a new view or an existing view
                            bool isNewView = std::find( allViewsBefore.begin(), allViewsBefore.end(), view ) == allViewsBefore.end();

                            // Get all surface-in-view objects
                            auto allSurfacesInView = surfInViewColl->descendantsIncludingThisOfType<RimSurfaceInView>();

                            for ( RimSurfaceInView* surfInView : allSurfacesInView )
                            {
                                if ( RimSurface* surf = surfInView->surface() )
                                {
                                    bool isNewlyImported = std::find( newlyImportedSurfaces.begin(), newlyImportedSurfaces.end(), surf ) !=
                                                           newlyImportedSurfaces.end();

                                    if ( isNewlyImported )
                                    {
                                        // Enable newly imported surfaces only in new views
                                        // Disable them in old views
                                        surfInView->setActive( isNewView );
                                    }
                                    else if ( isNewView )
                                    {
                                        // Disable old surfaces in new views
                                        surfInView->setActive( false );
                                    }
                                    // Leave old surfaces in old views unchanged
                                }
                            }

                            surfInViewColl->updateConnectedEditors();
                        }
                    }
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimEclipseView*> RicImportEclipseCaseFeature::allEclipseViews( RimProject* project )
{
    std::vector<RimEclipseView*> allViewsBefore;
    if ( project )
    {
        auto allCases = project->eclipseCases();
        for ( auto* eclipseCase : allCases )
        {
            auto views = eclipseCase->reservoirViews();
            allViewsBefore.insert( allViewsBefore.end(), views.begin(), views.end() );
        }
    }
    return allViewsBefore;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RicImportEclipseCaseFeature::findPvdFilesToImport( const QStringList& fileNames )
{
    QStringList pvdFilesToImport;
    for ( const auto& gridFile : fileNames )
    {
        QFileInfo gridFileInfo( gridFile );
        QString   baseName = gridFileInfo.completeBaseName(); // e.g., "CASE" from "CASE.EGRID"
        QString   dirPath  = gridFileInfo.absolutePath();
        QDir      dir( dirPath );

        // Search for PVD files with matching base name
        QStringList pvdFilters;
        pvdFilters << baseName + "*.pvd";

        QStringList matchingPvdFiles = dir.entryList( pvdFilters, QDir::Files );
        for ( const QString& pvdFile : matchingPvdFiles )
        {
            QFileInfo pvdFileInfo( pvdFile );
            QString   pvdBaseName = pvdFileInfo.completeBaseName();

            // Skip files with exactly the same base name (e.g., CASE.EGRID should not import CASE.pvd)
            // Only import files with additional text (e.g., CASE_surface.pvd, CASE-horizon.pvd)
            if ( pvdBaseName != baseName )
            {
                QString fullPath = dir.absoluteFilePath( pvdFile );
                pvdFilesToImport << fullPath;
            }
        }
    }
    return pvdFilesToImport;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportEclipseCaseFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/Case.svg" ) );
    actionToSetup->setText( "Import Eclipse Case" );
}
