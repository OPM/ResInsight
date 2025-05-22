/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "RicImportSurfacesFeature.h"

#include "RiaApplication.h"

#include "RimSurface.h"
#include "RimSurfaceCollection.h"
#include "RimTools.h"

#include "Riu3DMainWindowTools.h"
#include "RiuFileDialogTools.h"

#include "cafSelectionManagerTools.h"
#include "cafUtils.h"

#include <QAction>
#include <QFileInfo>

CAF_CMD_SOURCE_INIT( RicImportSurfacesFeature, "RicImportSurfacesFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportSurfacesFeature::onActionTriggered( bool isChecked )
{
    RiaApplication* app        = RiaApplication::instance();
    QString         defaultDir = app->lastUsedDialogDirectory( "BINARY_GRID" );

    QString generalExtensions = "*.ptl *.ts *.dat *.xyz";
    QString irapExtensions    = "*.irap *.gri";
    QString vtkExtensions     = "*.vtu *.pvd";
    QString allExtensions     = generalExtensions + " " + irapExtensions + " " + vtkExtensions;

    QString dialogFilter = QString( "Surface files (%1);;VTK files (%2);;IRAP GRI files (%3);;All Files (*.*)" )
                               .arg( allExtensions )
                               .arg( vtkExtensions )
                               .arg( irapExtensions );

    QStringList fileNames =
        RiuFileDialogTools::getOpenFileNames( Riu3DMainWindowTools::mainWindowWidget(), "Import Surfaces", defaultDir, dialogFilter );

    if ( fileNames.isEmpty() ) return;

    // Remember the path to next time
    app->setLastUsedDialogDirectory( "BINARY_GRID", QFileInfo( fileNames.last() ).absolutePath() );

    // Find the selected SurfaceCollection
    RimSurfaceCollection* surfColl = nullptr;
    {
        std::vector<RimSurfaceCollection*> colls = caf::selectedObjectsByTypeStrict<RimSurfaceCollection*>();
        if ( !colls.empty() )
        {
            surfColl = colls.front();
        }
    }

    if ( !surfColl )
    {
        surfColl = RimTools::surfaceCollection();
    }

    if ( !surfColl ) return;

    RimSurface* lastCreatedOrUpdated = surfColl->importSurfacesFromFiles( fileNames );

    if ( lastCreatedOrUpdated )
    {
        Riu3DMainWindowTools::selectAsCurrentItem( lastCreatedOrUpdated );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportSurfacesFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/ReservoirSurfaces16x16.png" ) );
    actionToSetup->setText( "Import Surfaces" );
}
