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

#include "RimProject.h"
#include "RimWellLogTrack.h"
#include "RimWellMeasurement.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmUiTableViewEditor.h"
#include "cafPdmUiTreeOrdering.h"

CAF_PDM_SOURCE_INIT( RimWellMeasurementCollection, "WellMeasurements" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellMeasurementCollection::RimWellMeasurementCollection()
{
    CAF_PDM_InitObject( "Well Measurement", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_measurements, "Measurements", "Well Measurements", "", "", "" );
    m_measurements.uiCapability()->setUiEditorTypeName( caf::PdmUiTableViewEditor::uiEditorTypeName() );
    m_measurements.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::TOP );
    m_measurements.uiCapability()->setCustomContextMenuEnabled( true );
    this->setName( "Well Measurements" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellMeasurementCollection::~RimWellMeasurementCollection() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellMeasurementCollection::updateAllReferringTracks()
{
    std::vector<RimWellLogTrack*> wellLogTracks;

    this->objectsWithReferringPtrFieldsOfType( wellLogTracks );
    for ( RimWellLogTrack* track : wellLogTracks )
    {
        track->loadDataAndUpdate();
    }
    this->updateConnectedEditors();
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
void RimWellMeasurementCollection::insertMeasurement( RimWellMeasurement* insertBefore, RimWellMeasurement* measurement )
{
    size_t index = m_measurements.index( insertBefore );
    if ( index < m_measurements.size() )
        m_measurements.insert( index, measurement );
    else
        m_measurements.push_back( measurement );

    this->updateAllReferringTracks();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellMeasurementCollection::appendMeasurement( RimWellMeasurement* measurement )
{
    m_measurements.push_back( measurement );
    this->updateAllReferringTracks();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellMeasurementCollection::deleteMeasurement( RimWellMeasurement* measurementToDelete )
{
    m_measurements.removeChildObject( measurementToDelete );
    delete measurementToDelete;

    this->updateAllReferringTracks();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellMeasurementCollection::deleteAllMeasurements()
{
    m_measurements.deleteAllChildObjects();
    this->updateAllReferringTracks();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellMeasurementCollection::defineCustomContextMenu( const caf::PdmFieldHandle* fieldNeedingMenu,
                                                            QMenu*                     menu,
                                                            QWidget*                   fieldEditorWidget )
{
    caf::CmdFeatureMenuBuilder menuBuilder;

    // menuBuilder << "RicNewWellMeasurementFeature";
    // menuBuilder << "Separator";
    // menuBuilder << "RicDeleteWellMeasurementFeature";

    menuBuilder.appendToMenu( menu );
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
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellMeasurementCollection::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering,
                                                         QString                 uiConfigName /*= ""*/ )
{
    uiTreeOrdering.skipRemainingChildren( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellMeasurementCollection::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                     const QVariant&            oldValue,
                                                     const QVariant&            newValue )
{
    if ( changedField == &m_isChecked )
    {
        RimProject* proj;
        this->firstAncestorOrThisOfTypeAsserted( proj );
        proj->scheduleCreateDisplayModelAndRedrawAllViews();
        this->updateAllReferringTracks();
    }
}
