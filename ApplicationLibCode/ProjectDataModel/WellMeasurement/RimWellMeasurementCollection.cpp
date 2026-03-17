/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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
#include "RimWellMeasurementCollection.h"

#include "Well/RigWellLogCurveData.h"

#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimWellLogTrack.h"
#include "RimWellMeasurement.h"
#include "RimWellMeasurementCurve.h"
#include "RimWellMeasurementFilePath.h"

#include "cafPdmUiTableViewEditor.h"
#include "cafPdmUiTreeOrdering.h"
#include "cafPdmUiTreeSelectionEditor.h"

CAF_PDM_SOURCE_INIT( RimWellMeasurementCollection, "WellMeasurements" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellMeasurementCollection::RimWellMeasurementCollection()
{
    CAF_PDM_InitObject( "Well Measurements", ":/WellMeasurement16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_items, "Measurements", "Measurements" );
    this->itemsField().uiCapability()->setUiEditorTypeName( caf::PdmUiTableViewEditor::uiEditorTypeName() );
    this->itemsField().uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::LabelPosition::TOP );

    CAF_PDM_InitFieldNoDefault( &m_importedFiles, "ImportedFiles", "Imported Files" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellMeasurementCollection::~RimWellMeasurementCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellMeasurement*> RimWellMeasurementCollection::measurements() const
{
    return this->items();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellMeasurementCollection::updateAllCurves()
{
    RimMainPlotCollection* plotCollection = RimMainPlotCollection::current();

    std::vector<RimWellMeasurementCurve*> measurementCurves = plotCollection->descendantsIncludingThisOfType<RimWellMeasurementCurve>();

    for ( auto curve : measurementCurves )
    {
        curve->loadDataAndUpdate( true );
    }

    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellMeasurementCollection::deleteAllEmptyCurves()
{
    RimMainPlotCollection* plotCollection = RimMainPlotCollection::current();

    std::vector<RimWellMeasurementCurve*> measurementCurves = plotCollection->descendantsIncludingThisOfType<RimWellMeasurementCurve>();

    for ( auto curve : measurementCurves )
    {
        if ( curve->curveData().propertyValues().empty() )
        {
            auto track = curve->firstAncestorOrThisOfTypeAsserted<RimWellLogTrack>();

            track->removeCurve( curve );
            delete curve;
            curve = nullptr;
            track->updateLayout();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellMeasurementCollection::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/ )
{
    if ( !m_importedFiles.empty() )
    {
        uiTreeOrdering.add( &m_importedFiles );
    }
    uiTreeOrdering.skipRemainingChildren( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<QString> RimWellMeasurementCollection::importedFiles() const
{
    std::set<QString> importedFiles;
    for ( auto importedFile : m_importedFiles )
    {
        importedFiles.insert( importedFile->filePath() );
    }

    return importedFiles;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellMeasurementCollection::addFilePath( const QString& filePath )
{
    std::set<QString> existingFilePaths = importedFiles();
    if ( existingFilePaths.find( filePath ) == existingFilePaths.end() )
    {
        RimWellMeasurementFilePath* measurementFilePath = new RimWellMeasurementFilePath;
        measurementFilePath->setFilePath( filePath );
        m_importedFiles.push_back( measurementFilePath );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellMeasurementCollection::removeFilePath( RimWellMeasurementFilePath* measurementFilePath )
{
    m_importedFiles.removeChild( measurementFilePath );
    delete measurementFilePath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellMeasurementCollection::removeMeasurementsForFilePath( RimWellMeasurementFilePath* measurementFilePath )
{
    // Find all measurements for this file path
    std::vector<RimWellMeasurement*> measurementsToRemove;
    for ( auto* measurement : this->items() )
    {
        if ( measurement->filePath() == measurementFilePath->filePath() )
        {
            measurementsToRemove.push_back( measurement );
        }
    }

    // Remove them without invalidating the iterator
    for ( auto* measurement : measurementsToRemove )
    {
        this->deleteItem( measurement );
    }

    RimProject::current()->scheduleCreateDisplayModelAndRedrawAllViews();
    updateAllCurves();
}
