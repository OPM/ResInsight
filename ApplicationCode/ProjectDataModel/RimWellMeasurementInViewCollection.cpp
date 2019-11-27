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
#include "RimWellPathCollection.h"

#include "RiuViewer.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmUiTableViewEditor.h"
#include "cafPdmUiTreeOrdering.h"
#include "cafPdmUiTreeSelectionEditor.h"

#include <cmath>

#include <QStringList>

CAF_PDM_SOURCE_INIT( RimWellMeasurementInViewCollection, "WellMeasurementsInView" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellMeasurementInViewCollection::RimWellMeasurementInViewCollection()
{
    CAF_PDM_InitObject( "Well Measurement", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_legendConfig, "LegendDefinition", "Color Legend", "", "", "" );
    m_legendConfig = new RimRegularLegendConfig();
    m_legendConfig.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_measurementKinds, "MeasurementKinds", "Measurent Kinds", "", "", "" );
    m_measurementKinds.uiCapability()->setAutoAddingOptionFromValue( false );
    m_measurementKinds.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );
    m_measurementKinds.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    m_measurementKinds.xmlCapability()->disableIO();

    this->setName( "Well Measurements" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellMeasurementInViewCollection::~RimWellMeasurementInViewCollection()
{
    delete m_legendConfig;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellMeasurementInViewCollection::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering,
                                                               QString                 uiConfigName /*= ""*/ )
{
    uiTreeOrdering.add( &m_legendConfig );
    uiTreeOrdering.skipRemainingChildren( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellMeasurementInViewCollection::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                           const QVariant&            oldValue,
                                                           const QVariant&            newValue )
{
    updateLegendData();
    RimGridView* rimGridView = nullptr;
    this->firstAncestorOrThisOfTypeAsserted( rimGridView );
    rimGridView->scheduleCreateDisplayModelAndRedraw();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimRegularLegendConfig* RimWellMeasurementInViewCollection::legendConfig()
{
    return m_legendConfig;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RimRegularLegendConfig* RimWellMeasurementInViewCollection::legendConfig() const
{
    return m_legendConfig;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellMeasurementInViewCollection::updateLegendData()
{
    RimWellPathCollection* wellPathCollection = RimTools::wellPathCollection();
    if ( !wellPathCollection ) return false;

    RimWellMeasurementCollection* wellMeasurementCollection = wellPathCollection->measurementCollection();
    if ( !wellMeasurementCollection ) return false;

    std::vector<QString> selectedMeasurementKinds = measurementKinds();
    RimWellMeasurement::excludeMeasurementKindsWithoutValue( selectedMeasurementKinds );

    // Only show legend when there is at least one range-based measurement
    if ( !selectedMeasurementKinds.empty() )
    {
        std::vector<RimWellMeasurement*> wellMeasurements =
            RimWellMeasurementFilter::filterMeasurements( wellMeasurementCollection->measurements(),
                                                          selectedMeasurementKinds );

        double minValue         = HUGE_VAL;
        double maxValue         = -HUGE_VAL;
        double posClosestToZero = HUGE_VAL;
        double negClosestToZero = -HUGE_VAL;

        for ( auto& measurement : wellMeasurements )
        {
            minValue = std::min( measurement->value(), minValue );
            maxValue = std::max( measurement->value(), maxValue );
        }

        if ( minValue != HUGE_VAL )
        {
            QString title = createTitle( selectedMeasurementKinds );
            m_legendConfig->setTitle( QString( "Well Measurement: \n" ) + title );
            m_legendConfig->setAutomaticRanges( minValue, maxValue, minValue, maxValue );
            m_legendConfig->setClosestToZeroValues( posClosestToZero, negClosestToZero, posClosestToZero, negClosestToZero );
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellMeasurementInViewCollection::updateLegendRangesTextAndVisibility( RiuViewer* nativeOrOverrideViewer,
                                                                              bool       isUsingOverrideViewer )
{
    bool addToViewer = updateLegendData();
    if ( addToViewer )
    {
        nativeOrOverrideViewer->addColorLegendToBottomLeftCorner( m_legendConfig->titledOverlayFrame(),
                                                                  isUsingOverrideViewer );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RimWellMeasurementInViewCollection::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                               bool*                      useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;
    if ( fieldNeedingOptions == &m_measurementKinds )
    {
        RimWellPathCollection* wellPathCollection = RimTools::wellPathCollection();
        if ( wellPathCollection )
        {
            std::vector<RimWellMeasurement*> measurements = wellPathCollection->measurementCollection()->measurements();

            std::set<QString> measurementKindsInData;
            for ( auto measurement : measurements )
            {
                measurementKindsInData.insert( measurement->kind() );
            }

            for ( auto measurementKind : measurementKindsInData )
            {
                options.push_back( caf::PdmOptionItemInfo( measurementKind, measurementKind ) );
            }
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RimWellMeasurementInViewCollection::measurementKinds() const
{
    return m_measurementKinds;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellMeasurementInViewCollection::createTitle( const std::vector<QString>& kinds )
{
    QStringList kindsList;
    kindsList.reserve( kinds.size() );
    std::copy( kinds.begin(), kinds.end(), std::back_inserter( kindsList ) );
    return kindsList.join( ", " );
}
