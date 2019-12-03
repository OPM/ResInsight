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
#include "RimWellMeasurementInView.h"

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

CAF_PDM_SOURCE_INIT( RimWellMeasurementInView, "WellMeasurementInView" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellMeasurementInView::RimWellMeasurementInView()
{
    CAF_PDM_InitObject( "Well Measurement", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_measurementKind, "MeasurementKind", "Measurement Kind", "", "", "" );
    m_measurementKind.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_legendConfig, "LegendDefinition", "Color Legend", "", "", "" );
    m_legendConfig = new RimRegularLegendConfig();
    m_legendConfig.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_wells, "Wells", "Wells", "", "", "" );
    m_wells.uiCapability()->setAutoAddingOptionFromValue( false );
    m_wells.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );
    m_wells.xmlCapability()->disableIO();

    this->setName( "Well Measurement" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellMeasurementInView::~RimWellMeasurementInView()
{
    delete m_legendConfig;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellMeasurementInView::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering,
                                                     QString                 uiConfigName /*= ""*/ )
{
    uiTreeOrdering.add( &m_legendConfig );
    uiTreeOrdering.skipRemainingChildren( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellMeasurementInView::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
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
RimRegularLegendConfig* RimWellMeasurementInView::legendConfig()
{
    return m_legendConfig;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellMeasurementInView::updateLegendData()
{
    RimWellPathCollection* wellPathCollection = RimTools::wellPathCollection();
    if ( !wellPathCollection ) return false;

    RimWellMeasurementCollection* wellMeasurementCollection = wellPathCollection->measurementCollection();
    if ( !wellMeasurementCollection ) return false;

    if ( hasCategoryResult() )
    {
        cvf::Color3ub color = cvf::Color3ub( RimWellMeasurement::mapToColor( measurementKind() ) );
        std::vector<std::tuple<QString, int, cvf::Color3ub>> categories;
        categories.push_back( std::make_tuple( measurementKind(), 0, color ) );
        m_legendConfig->setCategoryItems( categories );
        m_legendConfig->setTitle( QString( "Well Measurement: \n" ) + measurementKind() );
        m_legendConfig->setMappingMode( RimRegularLegendConfig::CATEGORY_INTEGER );
        return true;
    }
    else
    {
        std::vector<QString> selectedMeasurementKinds;
        selectedMeasurementKinds.push_back( m_measurementKind );
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
            m_legendConfig->setTitle( QString( "Well Measurement: \n" ) + selectedMeasurementKinds[0] );
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
void RimWellMeasurementInView::updateLegendRangesTextAndVisibility( RiuViewer* nativeOrOverrideViewer,
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
bool RimWellMeasurementInView::hasMeasurementKindForWell( const RimWellPath*                      wellPath,
                                                          const RimWellPathCollection*            wellPathCollection,
                                                          const std::vector<RimWellMeasurement*>& measurements,
                                                          const QString&                          measurementKind )
{
    for ( auto measurement : measurements )
    {
        if ( measurement->kind() == measurementKind )
        {
            RimWellPath* measurementWellPath = wellPathCollection->tryFindMatchingWellPath( measurement->wellName() );
            if ( wellPath && wellPath == measurementWellPath )
            {
                return true;
            }
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RimWellMeasurementInView::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;
    if ( fieldNeedingOptions == &m_wells )
    {
        RimWellPathCollection* wellPathCollection = RimTools::wellPathCollection();
        if ( wellPathCollection )
        {
            std::vector<RimWellMeasurement*> measurements = wellPathCollection->measurementCollection()->measurements();

            // Find wells with a given measurement.
            std::set<QString> wellsWithMeasurementKind;
            for ( auto well : wellPathCollection->wellPaths )
            {
                if ( hasMeasurementKindForWell( well, wellPathCollection, measurements, m_measurementKind ) )
                    wellsWithMeasurementKind.insert( well->name() );
            }

            for ( auto wellName : wellsWithMeasurementKind )
            {
                options.push_back( caf::PdmOptionItemInfo( wellName, wellName ) );
            }
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellMeasurementInView::measurementKind() const
{
    return m_measurementKind;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellMeasurementInView::setMeasurementKind( const QString& measurementKind )
{
    m_measurementKind = measurementKind;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellMeasurementInView::hasCategoryResult() const
{
    return !RimWellMeasurement::kindHasValue( measurementKind() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellMeasurementInView::isWellChecked( const QString& wellName ) const
{
    return std::find( m_wells.v().begin(), m_wells.v().end(), wellName ) != m_wells.v().end();
}
