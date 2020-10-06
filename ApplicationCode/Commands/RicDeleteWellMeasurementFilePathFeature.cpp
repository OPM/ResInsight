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

#include "RicDeleteWellMeasurementFilePathFeature.h"

#include "RimWellMeasurement.h"
#include "RimWellMeasurementCollection.h"
#include "RimWellMeasurementFilePath.h"
#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicDeleteWellMeasurementFilePathFeature, "RicDeleteWellMeasurementFilePathFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicDeleteWellMeasurementFilePathFeature::isCommandEnabled()
{
    std::vector<RimWellMeasurementFilePath*> objects;
    caf::SelectionManager::instance()->objectsByType( &objects );
    return !objects.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeleteWellMeasurementFilePathFeature::onActionTriggered( bool isChecked )
{
    std::vector<RimWellMeasurementFilePath*> selectedFilePaths;
    caf::SelectionManager::instance()->objectsByType( &selectedFilePaths );
    if ( selectedFilePaths.empty() ) return;

    RimWellMeasurementFilePath* filePath = selectedFilePaths[0];

    RimWellMeasurementCollection* wellMeasurementCollection = nullptr;
    filePath->firstAncestorOrThisOfType( wellMeasurementCollection );
    if ( !wellMeasurementCollection ) return;

    wellMeasurementCollection->removeMeasurementsForFilePath( filePath );
    wellMeasurementCollection->removeFilePath( filePath );
    wellMeasurementCollection->deleteAllEmptyCurves();
    wellMeasurementCollection->uiCapability()->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeleteWellMeasurementFilePathFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Delete Well Measurement File" );
    actionToSetup->setIcon( QIcon( ":/Erase.svg" ) );
}
