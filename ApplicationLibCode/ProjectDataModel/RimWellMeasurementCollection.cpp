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

#include "RigWellLogCurveData.h"

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
    CAF_PDM_InitObject( "Well Measurements", ":/WellMeasurement16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_measurements, "Measurements", "Well Measurements", "", "", "" );
    m_measurements.uiCapability()->setUiEditorTypeName( caf::PdmUiTableViewEditor::uiEditorTypeName() );
    m_measurements.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::TOP );
    m_measurements.uiCapability()->setUiTreeHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_importedFiles, "ImportedFiles", "Imported Files", "", "", "" );
    m_importedFiles.uiCapability()->setUiTreeHidden( true );
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
void RimWellMeasurementCollection::updateAllCurves()
{
    RimProject* proj;
    this->firstAncestorOrThisOfTypeAsserted( proj );
    RimMainPlotCollection* plotCollection = proj->mainPlotCollection();

    std::vector<RimWellMeasurementCurve*> measurementCurves;
    plotCollection->descendantsIncludingThisOfType( measurementCurves );

    for ( auto curve : measurementCurves )
    {
        curve->loadDataAndUpdate( true );
    }

    this->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellMeasurementCollection::deleteAllEmptyCurves()
{
    RimProject* proj;
    this->firstAncestorOrThisOfTypeAsserted( proj );
    RimMainPlotCollection* plotCollection = proj->mainPlotCollection();

    std::vector<RimWellMeasurementCurve*> measurementCurves;
    plotCollection->descendantsIncludingThisOfType( measurementCurves );

    for ( auto curve : measurementCurves )
    {
        if ( curve->curveData()->xValues().empty() )
        {
            RimWellLogTrack* track = nullptr;
            curve->firstAncestorOrThisOfTypeAsserted( track );

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
std::vector<RimWellMeasurement*> RimWellMeasurementCollection::measurements() const
{
    std::vector<RimWellMeasurement*> attrs;

    for ( auto attr : m_measurements )
    {
        attrs.push_back( attr.p() );
    }
    return attrs;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellMeasurementCollection::isEmpty() const
{
    return m_measurements.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellMeasurementCollection::insertMeasurement( RimWellMeasurement* insertBefore, RimWellMeasurement* measurement )
{
    size_t index = m_measurements.index( insertBefore );
    if ( index < m_measurements.size() )
        m_measurements.insert( index, measurement );
    else
        m_measurements.push_back( measurement );

    addFilePath( measurement->filePath() );
    this->updateAllCurves();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellMeasurementCollection::appendMeasurement( RimWellMeasurement* measurement )
{
    m_measurements.push_back( measurement );
    addFilePath( measurement->filePath() );
    this->updateAllCurves();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellMeasurementCollection::deleteMeasurement( RimWellMeasurement* measurementToDelete )
{
    m_measurements.removeChildObject( measurementToDelete );
    delete measurementToDelete;

    this->updateAllCurves();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellMeasurementCollection::deleteAllMeasurements()
{
    m_measurements.deleteAllChildObjects();
    this->updateAllCurves();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellMeasurementCollection::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                          QString                    uiConfigName,
                                                          caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_measurements )
    {
        auto tvAttribute = dynamic_cast<caf::PdmUiTableViewEditorAttribute*>( attribute );
        if ( tvAttribute )
        {
            tvAttribute->resizePolicy              = caf::PdmUiTableViewEditorAttribute::RESIZE_TO_FILL_CONTAINER;
            tvAttribute->alwaysEnforceResizePolicy = true;
            tvAttribute->minimumHeight             = 300;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellMeasurementCollection::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_measurements );
    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellMeasurementCollection::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering,
                                                         QString                 uiConfigName /*= ""*/ )
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
    m_importedFiles.removeChildObject( measurementFilePath );
    delete measurementFilePath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellMeasurementCollection::removeMeasurementsForFilePath( RimWellMeasurementFilePath* measurementFilePath )
{
    // Find all measurements for this file path
    std::vector<RimWellMeasurement*> measurementsToRemove;
    for ( auto attr : m_measurements )
    {
        if ( attr->filePath() == measurementFilePath->filePath() )
        {
            measurementsToRemove.push_back( attr );
        }
    }

    // Remove then remove them without invalidating the iterator
    for ( unsigned int i = 0; i < measurementsToRemove.size(); i++ )
    {
        m_measurements.removeChildObject( measurementsToRemove[i] );
        delete measurementsToRemove[i];
    }

    RimProject* proj;
    this->firstAncestorOrThisOfTypeAsserted( proj );
    proj->scheduleCreateDisplayModelAndRedrawAllViews();
    this->updateAllCurves();
}
