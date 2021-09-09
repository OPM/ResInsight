/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-   Equinor ASA
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

#include "RicNewWellIntegrityAnalysisFeature.h"

#include "RiaApplication.h"
#include "RiaPreferencesGeoMech.h"

#include "RimGeoMechView.h"
#include "RimProject.h"
#include "RimWellIASettings.h"
#include "RimWellIASettingsCollection.h"
#include "RimWellPath.h"

#include "Riu3DMainWindowTools.h"
#include "Riu3dSelectionManager.h"
#include "RiuFileDialogTools.h"

#include <QAction>
#include <QMessageBox>

CAF_CMD_SOURCE_INIT( RicNewWellIntegrityAnalysisFeature, "RicNewWellIntegrityAnalysisFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewWellIntegrityAnalysisFeature::onActionTriggered( bool isChecked )
{
    RiuWellPathSelectionItem* wellPathItem = RiuWellPathSelectionItem::wellPathSelectionItem();
    if ( !wellPathItem ) return;

    RimWellPath* wellPath = wellPathItem->m_wellpath;
    if ( !wellPath ) return;

    RimWellIASettingsCollection* coll = wellPath->wellIASettingsCollection();
    if ( !coll ) return;

    RimGeoMechView* view    = dynamic_cast<RimGeoMechView*>( RiaApplication::instance()->activeGridView() );
    RimGeoMechCase* theCase = nullptr;
    if ( view )
    {
        theCase = view->geoMechCase();
    }

    if ( !RiaPreferencesGeoMech::current()->validateWIASettings() )
    {
        QMessageBox::critical( nullptr,
                               "Well Integrity Analysis",
                               "Well Integrity Analysis has not been properly set up.\nPlease go to ResInsight "
                               "preferences and set / check the GeoMechanical settings." );

        return;
    }

    // get base directory for our work, should be a new, empty folder somewhere
    QString defaultDir =
        RiaApplication::instance()->lastUsedDialogDirectoryWithFallbackToProjectFolder( "WELL_INTEGRITY_ANALYSIS" );
    QString baseDir = RiuFileDialogTools::getExistingDirectory( nullptr, tr( "Select Working Directory" ), defaultDir );
    if ( baseDir.isNull() || baseDir.isEmpty() ) return;
    RiaApplication::instance()->setLastUsedDialogDirectory( "WELL_INTEGRITY_ANALYSIS", baseDir );

    QString errMsg;

    RimWellIASettings* newWIA =
        coll->startWellIntegrationAnalysis( baseDir, wellPath, wellPathItem->m_measuredDepth, theCase, errMsg );

    if ( newWIA )
    {
        wellPath->updateConnectedEditors();
        Riu3DMainWindowTools::selectAsCurrentItem( newWIA );
        newWIA->updateVisualization();
    }
    else
    {
        QMessageBox::critical( nullptr, "Well Integrity Analysis", errMsg );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewWellIntegrityAnalysisFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/WellIntAnalysis.png" ) );
    actionToSetup->setText( "New Well Integration Analysis at this Depth" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewWellIntegrityAnalysisFeature::isCommandEnabled()
{
    return true;
}
