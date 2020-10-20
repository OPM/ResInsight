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
#include "cafPdmUiDoubleValueEditor.h"
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
    CAF_PDM_InitObject( "Well Measurement", ":/WellMeasurement16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_measurementKind, "MeasurementKind", "Measurement Kind", "", "", "" );
    m_measurementKind.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_legendConfig, "LegendDefinition", "Color Legend", "", "", "" );
    m_legendConfig = new RimRegularLegendConfig();
    m_legendConfig.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_wells, "Wells", "Wells", "", "", "" );
    m_wells.uiCapability()->setAutoAddingOptionFromValue( false );
    m_wells.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );
    m_wells.xmlCapability()->disableIO();

    // The m_wells field does not serialize in a suitable format, so we work around it by
    // serializing to a pipe-delimited string.
    CAF_PDM_InitFieldNoDefault( &m_wellsSerialized, "WellsSerialized", "WellsSerialized", "", "", "" );
    m_wellsSerialized.uiCapability()->setUiHidden( true );

    // Keep track of the wells which has a given measurement in order to automatically select
    // new wells when they appear in new measurements
    CAF_PDM_InitFieldNoDefault( &m_availableWellsSerialized, "AvailableWellsSerialized", "AvailableWellsSerialized", "", "", "" );
    // m_availableWellsSerialized.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_lowerBound, "LowerBound", -HUGE_VAL, "Min", "", "", "" );
    m_lowerBound.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_upperBound, "UpperBound", HUGE_VAL, "Max", "", "", "" );
    m_upperBound.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_qualityFilter, "QualityFilter", "Quality Filter", "", "", "" );
    m_qualityFilter.uiCapability()->setAutoAddingOptionFromValue( false );
    m_qualityFilter.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_radiusScaleFactor, "RadiusScaleFactor", 2.5, "Radius Scale", "", "", "" );
    m_radiusScaleFactor.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleValueEditor::uiEditorTypeName() );

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

    if ( !hasCategoryResult() )
    {
        caf::PdmUiGroup& filterGroup = *( uiOrdering.addNewGroup( "Value Filter Settings" ) );
        filterGroup.add( &m_lowerBound );
        filterGroup.add( &m_upperBound );
    }

    uiOrdering.add( &m_wells );
    uiOrdering.add( &m_qualityFilter );
    uiOrdering.add( &m_radiusScaleFactor );

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
        if ( myAttr )
        {
            myAttr->m_minimum = m_minimumResultValue;
            myAttr->m_maximum = m_maximumResultValue;
        }
    }

    if ( field == &m_radiusScaleFactor )
    {
        caf::PdmUiDoubleValueEditorAttribute* uiDoubleValueEditorAttr =
            dynamic_cast<caf::PdmUiDoubleValueEditorAttribute*>( attribute );
        if ( uiDoubleValueEditorAttr )
        {
            uiDoubleValueEditorAttr->m_decimals  = 2;
            uiDoubleValueEditorAttr->m_validator = new QDoubleValidator( 0.001, 100.0, 2 );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellMeasurementInView::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                 const QVariant&            oldValue,
                                                 const QVariant&            newValue )
{
    if ( changedField == &m_wells )
    {
        m_wellsSerialized = convertToSerializableString( m_wells.v() );
    }

    updateLegendData();
    RimGridView* rimGridView = nullptr;
    this->firstAncestorOrThisOfTypeAsserted( rimGridView );
    rimGridView->scheduleCreateDisplayModelAndRedraw();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellMeasurementInView::initAfterRead()
{
    m_wells = convertFromSerializableString( m_wellsSerialized );
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
        m_legendConfig->setMappingMode( RimRegularLegendConfig::MappingType::CATEGORY_INTEGER );
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

            selectNewWells( wellsWithMeasurementKind );
            setAvailableWells( wellsWithMeasurementKind );
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

        m_wellsSerialized = convertToSerializableString( m_wells.v() );
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellMeasurementInView::convertToSerializableString( const std::vector<QString>& strings )
{
    QStringList stringList;
    for ( const auto& string : strings )
    {
        stringList.push_back( string );
    }
    return stringList.join( '|' );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RimWellMeasurementInView::convertFromSerializableString( const QString& string )
{
    QStringList stringList = string.split( '|' );
    return std::vector<QString>( stringList.begin(), stringList.end() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellMeasurementInView::selectNewWells( const std::set<QString>& wells )
{
    // Check if there are new wells on the measurement kind
    std::set<QString> currentAvailableWells = getAvailableWells();
    std::set<QString> newWells;
    std::set_difference( wells.begin(),
                         wells.end(),
                         currentAvailableWells.begin(),
                         currentAvailableWells.end(),
                         std::inserter( newWells, newWells.end() ) );

    // Select the new wells
    for ( const QString& newWell : newWells )
    {
        m_wells.v().push_back( newWell );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellMeasurementInView::setAvailableWells( const std::set<QString>& wells )
{
    std::vector<QString> v( wells.begin(), wells.end() );
    m_availableWellsSerialized = convertToSerializableString( v );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<QString> RimWellMeasurementInView::getAvailableWells() const
{
    std::vector<QString> v = convertFromSerializableString( m_availableWellsSerialized );
    std::set<QString>    s( v.begin(), v.end() );
    return s;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellMeasurementInView::radiusScaleFactor() const
{
    return m_radiusScaleFactor;
}
