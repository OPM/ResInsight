/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RicRunWellIntegrityAnalysisFeature.h"

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

#include "cafSelectionManagerTools.h"

#include <QAction>
#include <QMessageBox>

CAF_CMD_SOURCE_INIT( RicRunWellIntegrityAnalysisFeature, "RicRunWellIntegrityAnalysisFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRunWellIntegrityAnalysisFeature::onActionTriggered( bool isChecked )
{
    RimWellIASettings* modelSettings =
        dynamic_cast<RimWellIASettings*>( caf::SelectionManager::instance()->selectedItem() );

    if ( modelSettings == nullptr ) return;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRunWellIntegrityAnalysisFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/WellIntAnalysis.png" ) );
    actionToSetup->setText( "Run..." );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicRunWellIntegrityAnalysisFeature::isCommandEnabled()
{
    return true;
}
