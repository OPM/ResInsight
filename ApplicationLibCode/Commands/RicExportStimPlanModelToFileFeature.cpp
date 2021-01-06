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

#include "RicExportStimPlanModelToFileFeature.h"

#include "RiaApplication.h"

#include "RimStimPlanModel.h"

#include "RiuFileDialogTools.h"

#include "RifStimPlanModelExporter.h"

#include "cafSelectionManager.h"
#include "cafUtils.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicExportStimPlanModelToFileFeature, "RicExportStimPlanModelToFileFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicExportStimPlanModelToFileFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportStimPlanModelToFileFeature::onActionTriggered( bool isChecked )
{
    RimStimPlanModel* stimPlanModel = caf::SelectionManager::instance()->selectedItemOfType<RimStimPlanModel>();
    if ( !stimPlanModel ) return;
    if ( !stimPlanModel->stimPlanModelTemplate() ) return;

    RiaApplication* app        = RiaApplication::instance();
    QString         defaultDir = app->lastUsedDialogDirectory( "FRACTURE_MODEL_EXPORT" );

    QString directoryPath =
        RiuFileDialogTools::getExistingDirectory( nullptr, "Select Directory for StimPlan Model Export", defaultDir );

    if ( directoryPath.isEmpty() ) return;

    RifStimPlanModelExporter::writeToDirectory( stimPlanModel, stimPlanModel->useDetailedFluidLoss(), directoryPath );

    // Remember the path to next time
    app->setLastUsedDialogDirectory( "FRACTURE_MODEL_EXPORT", directoryPath );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportStimPlanModelToFileFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Export StimPlan Model to File" );
    actionToSetup->setIcon( QIcon( ":/Save.svg" ) );
}
