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
#include "RimWellMeasurementInViewCollection.h"

#include "Rim3dView.h"
#include "RimGridView.h"
#include "RimProject.h"
#include "RimRegularLegendConfig.h"
#include "RimTools.h"
#include "RimWellLogTrack.h"
#include "RimWellMeasurement.h"
#include "RimWellMeasurementCollection.h"
#include "RimWellMeasurementFilter.h"
#include "RimWellMeasurementInView.h"
#include "RimWellPathCollection.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmUiTableViewEditor.h"
#include "cafPdmUiTreeOrdering.h"
#include "cafPdmUiTreeSelectionEditor.h"

#include <cmath>

CAF_PDM_SOURCE_INIT( RimWellMeasurementInViewCollection, "WellMeasurementsInView" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellMeasurementInViewCollection::RimWellMeasurementInViewCollection()
{
    CAF_PDM_InitObject( "Well Measurements", ":/WellMeasurement16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_measurementsInView, "MeasurementKinds", "Measurement Kinds", "", "", "" );
    m_measurementsInView.uiCapability()->setUiHidden( true );

    m_isChecked = false;

    this->setName( "Well Measurements" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellMeasurementInViewCollection::~RimWellMeasurementInViewCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellMeasurementInView*> RimWellMeasurementInViewCollection::measurements() const
{
    std::vector<RimWellMeasurementInView*> attrs;

    for ( auto attr : m_measurementsInView )
    {
        attrs.push_back( attr.p() );
    }
    return attrs;
}

//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellMeasurementInViewCollection::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering,
                                                               QString                 uiConfigName /*= ""*/ )
{
    uiTreeOrdering.add( &m_measurementsInView );
    uiTreeOrdering.skipRemainingChildren( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellMeasurementInViewCollection::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                           const QVariant&            oldValue,
                                                           const QVariant&            newValue )
{
    RimGridView* rimGridView = nullptr;
    this->firstAncestorOrThisOfTypeAsserted( rimGridView );
    rimGridView->scheduleCreateDisplayModelAndRedraw();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellMeasurementInViewCollection::syncWithChangesInWellMeasurementCollection()
{
    RimWellPathCollection* wellPathCollection = RimTools::wellPathCollection();
    if ( wellPathCollection )
    {
        const RimWellMeasurementCollection* measurementCollection = wellPathCollection->measurementCollection();

        // Make a set of the measurement kinds already present
        std::set<QString> currentMeasurementKinds;
        for ( RimWellMeasurementInView* wellMeasurement : measurements() )
        {
            currentMeasurementKinds.insert( wellMeasurement->measurementKind() );
        }

        // Make a set of the measurements we should have after the update
        std::set<QString> targetMeasurementKinds;
        for ( RimWellMeasurement* wellMeasurement : measurementCollection->measurements() )
        {
            targetMeasurementKinds.insert( wellMeasurement->kind() );
        }

        // The difference between the sets is the measurement kinds to be added
        std::set<QString> newMeasurementKinds;
        std::set_difference( targetMeasurementKinds.begin(),
                             targetMeasurementKinds.end(),
                             currentMeasurementKinds.begin(),
                             currentMeasurementKinds.end(),
                             std::inserter( newMeasurementKinds, newMeasurementKinds.end() ) );

        // Add the new measurement kinds
        for ( QString kind : newMeasurementKinds )
        {
            RimWellMeasurementInView* measurementInView = new RimWellMeasurementInView;
            measurementInView->setName( kind );
            measurementInView->setMeasurementKind( kind );
            measurementInView->setAllWellsSelected();
            measurementInView->setAllQualitiesSelected();

            m_measurementsInView.push_back( measurementInView );
        }

        updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
/// Get the "in-view" measurement corresponding to a give measurement.
//--------------------------------------------------------------------------------------------------
RimWellMeasurementInView*
    RimWellMeasurementInViewCollection::getWellMeasurementInView( const RimWellMeasurement* measurement ) const
{
    for ( RimWellMeasurementInView* wellMeasurementInView : measurements() )
    {
        if ( wellMeasurementInView->measurementKind() == measurement->kind() )
        {
            return wellMeasurementInView;
        }
    }

    // No match
    return nullptr;
}
