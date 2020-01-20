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
#include "cafPdmUiDoubleSliderEditor.h"
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

    CAF_PDM_InitField( &m_lowerBound, "LowerBound", -HUGE_VAL, "Min", "", "", "" );
    m_lowerBound.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_upperBound, "UpperBound", HUGE_VAL, "Max", "", "", "" );
    m_upperBound.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_qualityFilter, "QualityFilter", "Quality Filter", "", "", "" );
    m_qualityFilter.uiCapability()->setAutoAddingOptionFromValue( false );
    m_qualityFilter.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );

    this->setName( "Well Measurement" );

    m_minimumResultValue = cvf::UNDEFINED_DOUBLE;
    m_maximumResultValue = cvf::UNDEFINED_DOUBLE;
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
void RimWellMeasurementInView::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    RimCheckableNamedObject::defineUiOrdering( uiConfigName, uiOrdering );
    uiOrdering.add( &m_wells );

    if ( !hasCategoryResult() )
    {
        caf::PdmUiGroup& filterGroup = *( uiOrdering.addNewGroup( "Value Filter Settings" ) );
        filterGroup.add( &m_lowerBound );
        filterGroup.add( &m_upperBound );
    }

    uiOrdering.add( &m_qualityFilter );

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellMeasurementInView::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                      QString                    uiConfigName,
                                                      caf::PdmUiEditorAttribute* attribute )
{
    if ( m_minimumResultValue == cvf::UNDEFINED_DOUBLE || m_maximumResultValue == cvf::UNDEFINED_DOUBLE )
    {
        return;
    }

    if ( field == &m_lowerBound || field == &m_upperBound )
    {
        caf::PdmUiDoubleSliderEditorAttribute* myAttr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute );
        if ( !myAttr )
        {
            return;
        }

        myAttr->m_minimum = m_minimumResultValue;
        myAttr->m_maximum = m_maximumResultValue;
    }
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
void RimWellMeasurementInView::rangeValues( double* lowerBound, double* upperBound ) const
{
    *lowerBound = m_lowerBound;
    *upperBound = m_upperBound;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<int> RimWellMeasurementInView::qualityFilter() const
{
    return m_qualityFilter;
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

        m_minimumResultValue    = HUGE_VAL;
        m_maximumResultValue    = -HUGE_VAL;
        double posClosestToZero = HUGE_VAL;
        double negClosestToZero = -HUGE_VAL;

        for ( auto& measurement : wellMeasurements )
        {
            m_minimumResultValue = std::min( measurement->value(), m_minimumResultValue );
            m_maximumResultValue = std::max( measurement->value(), m_maximumResultValue );
        }

        if ( m_minimumResultValue != HUGE_VAL )
        {
            // Refresh filter slider if lower or upper boundary was undefined
            if ( std::isinf( m_lowerBound ) )
            {
                m_lowerBound = m_minimumResultValue;
            }

            if ( std::isinf( m_upperBound ) )
            {
                m_upperBound = m_maximumResultValue;
            }

            m_legendConfig->setTitle( QString( "Well Measurement: \n" ) + selectedMeasurementKinds[0] );
            m_legendConfig->setAutomaticRanges( m_minimumResultValue,
                                                m_maximumResultValue,
                                                m_minimumResultValue,
                                                m_maximumResultValue );
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
            for ( const auto& well : wellPathCollection->wellPaths )
            {
                if ( hasMeasurementKindForWell( well, wellPathCollection, measurements, m_measurementKind ) )
                    wellsWithMeasurementKind.insert( well->name() );
            }

            for ( const auto& wellName : wellsWithMeasurementKind )
            {
                options.push_back( caf::PdmOptionItemInfo( wellName, wellName ) );
            }
        }
    }
    else if ( fieldNeedingOptions == &m_qualityFilter )
    {
        RimWellPathCollection* wellPathCollection = RimTools::wellPathCollection();
        if ( wellPathCollection )
        {
            std::vector<RimWellMeasurement*> measurements = wellPathCollection->measurementCollection()->measurements();

            // Find possible quality values for a given measurement kind
            std::set<int> qualityValues;
            for ( const auto& measurement : measurements )
            {
                if ( measurement->kind() == m_measurementKind ) qualityValues.insert( measurement->quality() );
            }

            for ( const auto& quality : qualityValues )
            {
                options.push_back( caf::PdmOptionItemInfo( QString::number( quality ), quality ) );
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellMeasurementInView::setAllWellsSelected()
{
    RimWellPathCollection* wellPathCollection = RimTools::wellPathCollection();
    if ( wellPathCollection )
    {
        std::vector<RimWellMeasurement*> measurements = wellPathCollection->measurementCollection()->measurements();

        // Find wells with a given measurement.
        std::set<QString> wellsWithMeasurementKind;
        for ( const auto& well : wellPathCollection->wellPaths )
        {
            if ( hasMeasurementKindForWell( well, wellPathCollection, measurements, m_measurementKind ) )
                wellsWithMeasurementKind.insert( well->name() );
        }

        for ( const auto& wellName : wellsWithMeasurementKind )
        {
            m_wells.v().push_back( wellName );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellMeasurementInView::setAllQualitiesSelected()
{
    RimWellPathCollection* wellPathCollection = RimTools::wellPathCollection();
    if ( wellPathCollection )
    {
        std::vector<RimWellMeasurement*> measurements = wellPathCollection->measurementCollection()->measurements();

        // Find possible quality values for a given measurement kind
        std::set<int> qualityValues;
        for ( const auto& measurement : measurements )
        {
            if ( measurement->kind() == m_measurementKind ) qualityValues.insert( measurement->quality() );
        }

        for ( const auto& quality : qualityValues )
        {
            m_qualityFilter.v().push_back( quality );
        }
    }
}
