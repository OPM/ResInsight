/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "RicImportElementPropertyFeature.h"

#include "RiaApplication.h"

#include "RigFemPartResultsCollection.h"
#include "RigGeoMechCaseData.h"

#include "RimGeoMechCase.h"
#include "RimGeoMechView.h"

#include "RiuFileDialogTools.h"

#include "cafSelectionManagerTools.h"

#include <QAction>
#include <QFileInfo>

CAF_CMD_SOURCE_INIT( RicImportElementPropertyFeature, "RicImportElementPropertyFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportElementPropertyFeature::onActionTriggered( bool isChecked )
{
    std::vector<caf::PdmUiItem*> uiItems;
    caf::SelectionManager::instance()->selectedItems( uiItems );

    RimGeoMechCase* geomCase = nullptr;

    if ( !uiItems.empty() )
    {
        geomCase = dynamic_cast<RimGeoMechCase*>( uiItems[0] );
    }

    if ( geomCase == nullptr )
    {
        Rim3dView* activeView = RiaApplication::instance()->activeReservoirView();
        if ( !activeView ) return;

        RimGeoMechView* activeGmv = dynamic_cast<RimGeoMechView*>( activeView );
        if ( !activeGmv ) return;

        geomCase = activeGmv->geoMechCase();
    }

    importElementProperties( geomCase );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportElementPropertyFeature::importElementProperties( RimGeoMechCase* pCase )
{
    if ( pCase == nullptr ) return;

    RiaApplication* app = RiaApplication::instance();

    QString     defaultDir = app->lastUsedDialogDirectory( "ELM_PROPS" );
    QStringList fileNames =
        RiuFileDialogTools::getOpenFileNames( nullptr, "Import Element Property Table", defaultDir, "Property Table (*.inp)" );

    if ( !fileNames.empty() )
    {
        defaultDir = QFileInfo( fileNames.last() ).absolutePath();
    }

    std::vector<caf::FilePath> filePaths;
    for ( QString filename : fileNames )
    {
        filePaths.push_back( caf::FilePath( filename ) );
    }

    app->setLastUsedDialogDirectory( "ELM_PROPS", defaultDir );

    pCase->addElementPropertyFiles( filePaths );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportElementPropertyFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/GeoMechCasePropTable24x24.png" ) );
    actionToSetup->setText( "Import &Element Property Table" );
}
