/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Equinor ASA
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

#include "RicImportWellMeasurementsFeature.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "RimPerforationCollection.h"
#include "RimPerforationInterval.h"
#include "RimProject.h"
#include "RimWellMeasurement.h"
#include "RimWellMeasurementCollection.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "RifFileParseTools.h"
#include "RifWellMeasurementReader.h"

#include "Riu3DMainWindowTools.h"

#include "cafSelectionManager.h"

#include <QAction>
#include <QFileDialog>

CAF_CMD_SOURCE_INIT( RicImportWellMeasurementsFeature, "RicImportWellMeasurementsFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicImportWellMeasurementsFeature::isCommandEnabled()
{
    return ( RicImportWellMeasurementsFeature::selectedWellPathCollection() != nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportWellMeasurementsFeature::onActionTriggered( bool isChecked )
{
    RimWellPathCollection* wellPathCollection = RicImportWellMeasurementsFeature::selectedWellPathCollection();
    CVF_ASSERT( wellPathCollection );

    // Open dialog box to select well path files
    RiaApplication* app               = RiaApplication::instance();
    QString         defaultDir        = app->lastUsedDialogDirectory( "WELLPATH_DIR" );
    QStringList     wellPathFilePaths = QFileDialog::getOpenFileNames( Riu3DMainWindowTools::mainWindowWidget(),
                                                                   "Import Well Measurements",
                                                                   defaultDir,
                                                                   "Well Measurements (*.csv);;All Files (*.*)" );

    if ( wellPathFilePaths.size() < 1 ) return;

    // Remember the path to next time
    app->setLastUsedDialogDirectory( "WELLPATH_DIR", QFileInfo( wellPathFilePaths.last() ).absolutePath() );

    std::vector<RifWellMeasurement> wellMeasurements;
    try
    {
        RifWellMeasurementReader::readWellMeasurements( wellMeasurements, wellPathFilePaths );
    }
    catch ( FileParseException& exception )
    {
        RiaLogging::warning( QString( "Well measurement import failed: '%1'." ).arg( exception.message ) );
        return;
    }

    RimWellMeasurement* lastWellMeasurement = nullptr;
    for ( auto& measurement : wellMeasurements )
    {
        RimWellPath* wellPath = wellPathCollection->tryFindMatchingWellPath( measurement.wellName );
        if ( wellPath == nullptr )
        {
            RiaLogging::warning( QString( "Import Well Measurements : Imported file contains unknown well path '%1'." )
                                     .arg( measurement.wellName ) );
        }
        else
        {
            RimWellMeasurement* wellMeasurement = new RimWellMeasurement;
            wellMeasurement->setWellName( measurement.wellName );
            wellMeasurement->setMD( measurement.MD );
            wellMeasurement->setValue( measurement.value );
            wellMeasurement->setDate( measurement.date );
            wellMeasurement->setQuality( measurement.quality );
            wellMeasurement->setKind( measurement.kind );
            wellMeasurement->setRemark( measurement.remark );
            wellPath->measurementCollection()->appendMeasurement( wellMeasurement );
            lastWellMeasurement = wellMeasurement;
        }
    }
    wellPathCollection->uiCapability()->updateConnectedEditors();

    if ( app->project() )
    {
        app->project()->scheduleCreateDisplayModelAndRedrawAllViews();
    }

    if ( lastWellMeasurement )
    {
        Riu3DMainWindowTools::selectAsCurrentItem( lastWellMeasurement );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportWellMeasurementsFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Import Measurements" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathCollection* RicImportWellMeasurementsFeature::selectedWellPathCollection()
{
    std::vector<RimWellPathCollection*> objects;
    caf::SelectionManager::instance()->objectsByType( &objects );

    if ( objects.size() == 1 )
    {
        return objects[0];
    }

    return nullptr;
}
