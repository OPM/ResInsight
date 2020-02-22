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

#include "RicWellMeasurementImportTools.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "Rim3dView.h"
#include "RimGridView.h"
#include "RimProject.h"
#include "RimTools.h"
#include "RimWellMeasurement.h"
#include "RimWellMeasurementCollection.h"
#include "RimWellMeasurementFilePath.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "RifFileParseTools.h"
#include "RifWellMeasurementReader.h"

#include "Riu3DMainWindowTools.h"
#include "Riu3dSelectionManager.h"

#include "cafSelectionManager.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellMeasurementImportTools::importWellMeasurementsFromFiles( const std::vector<RimWellMeasurementFilePath*>& filePaths )
{
    if ( filePaths.empty() ) return;

    QStringList files;
    for ( RimWellMeasurementFilePath* filePath : filePaths )
    {
        files.append( filePath->filePath() );
    }

    // This assumes that the filepaths have the same well path collection
    RimWellPathCollection* wellPathCollection = nullptr;
    filePaths[0]->firstAncestorOrThisOfType( wellPathCollection );
    if ( wellPathCollection )
    {
        importWellMeasurementsFromFiles( files, wellPathCollection );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellMeasurementImportTools::importWellMeasurementsFromFiles( const QStringList&     filePaths,
                                                                     RimWellPathCollection* wellPathCollection )
{
    std::vector<RifWellMeasurement> wellMeasurements;
    try
    {
        RifWellMeasurementReader::readWellMeasurements( wellMeasurements, filePaths );
    }
    catch ( FileParseException& exception )
    {
        RiaLogging::warning( QString( "Well measurement import failed: '%1'." ).arg( exception.message ) );
        return;
    }

    RimWellMeasurement* lastWellMeasurement = nullptr;
    for ( auto& measurement : wellMeasurements )
    {
        RimWellMeasurement* wellMeasurement = new RimWellMeasurement;
        wellMeasurement->setWellName( measurement.wellName );
        wellMeasurement->setMD( measurement.MD );
        wellMeasurement->setValue( measurement.value );
        wellMeasurement->setDate( measurement.date );
        wellMeasurement->setQuality( measurement.quality );
        wellMeasurement->setKind( measurement.kind );
        wellMeasurement->setRemark( measurement.remark );
        wellMeasurement->setFilePath( measurement.filePath );

        // Ignore values for kinds which is known to not have values
        if ( !RimWellMeasurement::kindHasValue( measurement.kind ) )
        {
            wellMeasurement->setValue( 0.0 );
        }

        wellPathCollection->measurementCollection()->appendMeasurement( wellMeasurement );
        lastWellMeasurement = wellMeasurement;
    }
    wellPathCollection->uiCapability()->updateConnectedEditors();

    RiaApplication* app = RiaApplication::instance();
    if ( app->project() )
    {
        std::vector<Rim3dView*> views;
        app->project()->allViews( views );
        for ( auto& view : views )
        {
            RimGridView* gridView = dynamic_cast<RimGridView*>( view );
            if ( gridView )
            {
                gridView->updateWellMeasurements();
            }
        }

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
void RicWellMeasurementImportTools::removeWellMeasurementsFromFiles( const std::vector<RimWellMeasurementFilePath*>& filePaths )
{
    for ( RimWellMeasurementFilePath* filePath : filePaths )
    {
        RimWellMeasurementCollection* wellMeasurementCollection = nullptr;
        filePath->firstAncestorOrThisOfType( wellMeasurementCollection );
        if ( wellMeasurementCollection )
        {
            wellMeasurementCollection->removeMeasurementsForFilePath( filePath );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellMeasurementImportTools::deleteAllEmptyMeasurementCurves()
{
    RiaApplication* app = RiaApplication::instance();

    std::vector<RimWellMeasurementCollection*> measurementCollections;
    app->project()->descendantsIncludingThisOfType( measurementCollections );
    for ( auto measurementCollection : measurementCollections )
    {
        measurementCollection->deleteAllEmptyCurves();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathCollection* RicWellMeasurementImportTools::selectedWellPathCollection()
{
    std::vector<RimWellPathCollection*> objects;
    caf::SelectionManager::instance()->objectsByType( &objects );

    if ( objects.size() == 1 )
    {
        return objects[0];
    }

    auto measurementColl = caf::SelectionManager::instance()->selectedItemAncestorOfType<RimWellMeasurementCollection>();
    if ( measurementColl )
    {
        return caf::SelectionManager::instance()->selectedItemAncestorOfType<RimWellPathCollection>();
    }

    return RimTools::wellPathCollection();
}
