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

#include "RicReloadWellMeasurementsFeature.h"

#include "RiaLogging.h"

#include "RimWellMeasurementFilePath.h"

#include "RicWellMeasurementImportTools.h"

#include "cafPdmObject.h"
#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicReloadWellMeasurementsFeature, "RicReloadWellMeasurementsFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicReloadWellMeasurementsFeature::isCommandEnabled()
{
    std::vector<RimWellMeasurementFilePath*> filePaths;
    caf::SelectionManager::instance()->objectsByType( &filePaths );
    return !filePaths.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicReloadWellMeasurementsFeature::onActionTriggered( bool isChecked )
{
    std::vector<RimWellMeasurementFilePath*> filePaths;
    caf::SelectionManager::instance()->objectsByType( &filePaths );
    if ( filePaths.empty() ) return;

    RicWellMeasurementImportTools::removeWellMeasurementsFromFiles( filePaths );
    RicWellMeasurementImportTools::importWellMeasurementsFromFiles( filePaths );
    RicWellMeasurementImportTools::deleteAllEmptyMeasurementCurves();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicReloadWellMeasurementsFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Reload" );
    actionToSetup->setIcon( QIcon( ":/Refresh.svg" ) );
}
