/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021 Equinor ASA
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

#include "RimMultipleLocations.h"

#include "cafPdmUiDoubleValueEditor.h"
#include "cafPdmUiListEditor.h"

#include <cmath>

CAF_PDM_SOURCE_INIT( RimMultipleLocations, "RimMultipleLocations" );

namespace caf
{
template <>
void AppEnum<RimMultipleLocations::LocationType>::setUp()
{
    addItem( RimMultipleLocations::LocationType::COUNT, "COUNT", "Start/End/Number" );
    addItem( RimMultipleLocations::LocationType::SPACING, "SPACING", "Start/End/Spacing" );
    addItem( RimMultipleLocations::LocationType::CUSTOM, "CUSTOM", "User Specification" );
    setDefault( RimMultipleLocations::LocationType::COUNT );
}
} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMultipleLocations::RimMultipleLocations()
{
    CAF_PDM_InitObject( "RimMultipleLocations", ":/FishBoneGroup16x16.png", "", "" );

    CAF_PDM_InitField( &m_locationType,
                       "LocationMode",
                       caf::AppEnum<LocationType>( LocationType::COUNT ),
                       "Location Defined By",
                       "",
                       "",
                       "" );
    CAF_PDM_InitField( &m_rangeStart, "RangeStart", 100.0, "Start MD", "", "", "" );
    m_rangeStart.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleValueEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_rangeEnd, "RangeEnd", 250.0, "End MD", "", "", "" );
    m_rangeEnd.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleValueEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_rangeSpacing, "Spacing", "Spacing", "", "", "" );
    m_rangeSpacing.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleValueEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_minimumMD, "MinimumMD", "Minimum MD", "", "", "" );
    m_minimumMD.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleValueEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_maximumMD, "MaximumMD", "Maximum MD", "", "", "" );
    m_maximumMD.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleValueEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_rangeCount, "RangeValveCount", 13, "Number of Items", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_locations, "Locations", "Measured Depths", "", "", "" );
    m_locations.uiCapability()->setUiEditorTypeName( caf::PdmUiListEditor::uiEditorTypeName() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultipleLocations::setRange( double minimumMD, double maximumMD )
{
    m_minimumMD = minimumMD;
    m_maximumMD = maximumMD;

    m_rangeStart = minimumMD;
    m_rangeEnd   = maximumMD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultipleLocations::updateRangesAndLocations()
{
    double existingRangeStart = m_rangeStart();
    double existingRangeEnd   = m_rangeEnd();
    m_rangeStart              = std::clamp( m_rangeStart(), minimumMD(), maximumMD() );
    m_rangeEnd                = std::clamp( m_rangeEnd(), minimumMD(), maximumMD() );
    if ( existingRangeStart != m_rangeStart() || existingRangeEnd != m_rangeEnd() )
    {
        computeRangesAndLocations();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimMultipleLocations::measuredDepth( size_t valveIndex ) const
{
    return m_locations()[valveIndex];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimMultipleLocations::rangeStart() const
{
    return m_rangeStart;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimMultipleLocations::rangeEnd() const
{
    return m_rangeEnd;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RimMultipleLocations::locations() const
{
    return m_locations();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultipleLocations::setLocationType( LocationType locationType )
{
    m_locationType = locationType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultipleLocations::computeRangesAndLocations()
{
    if ( m_locationType == LocationType::COUNT )
    {
        int divisor = 1;
        if ( m_rangeCount > 2 ) divisor = m_rangeCount - 1;

        m_rangeSpacing = std::abs( m_rangeStart - m_rangeEnd ) / divisor;
        if ( m_rangeSpacing < minimumSpacingMeters() )
        {
            m_rangeSpacing = minimumSpacingMeters();
            m_rangeCount   = rangeCountFromSpacing();
        }
    }
    else if ( m_locationType == LocationType::SPACING )
    {
        m_rangeCount = rangeCountFromSpacing();
    }

    if ( m_locationType == LocationType::COUNT || m_locationType == LocationType::SPACING )
    {
        std::vector<double> validMeasuredDepths;
        for ( auto md : locationsFromStartSpacingAndCount( m_rangeStart(), m_rangeSpacing, m_rangeCount ) )
        {
            validMeasuredDepths.push_back( md );
        }

        m_locations = validMeasuredDepths;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultipleLocations::initFields( LocationType               locationType,
                                       double                     rangeStart,
                                       double                     rangeEnd,
                                       double                     valveSpacing,
                                       int                        valveCount,
                                       const std::vector<double>& locationOfValves )
{
    if ( locationType != LocationType::UNDEFINED )
    {
        m_locationType = locationType;
    }
    if ( rangeStart != std::numeric_limits<double>::infinity() )
    {
        m_rangeStart = rangeStart;
    }
    if ( rangeEnd != std::numeric_limits<double>::infinity() )
    {
        m_rangeEnd = rangeEnd;
    }
    if ( valveSpacing != std::numeric_limits<double>::infinity() )
    {
        m_rangeSpacing = valveSpacing;
    }
    if ( valveCount != -1 )
    {
        m_rangeCount = valveCount;
    }
    if ( !locationOfValves.empty() )
    {
        m_locations = locationOfValves;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultipleLocations::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    {
        m_locations.uiCapability()->setUiName( "Measured Depths" );
        m_rangeStart.uiCapability()->setUiName( "Start MD" );
        m_rangeEnd.uiCapability()->setUiName( "End MD" );
        m_rangeSpacing.uiCapability()->setUiName( "Spacing" );
    }

    {
        uiOrdering.add( &m_locationType );
        if ( m_locationType() != LocationType::CUSTOM )
        {
            uiOrdering.add( &m_rangeStart );
            uiOrdering.add( &m_rangeEnd );

            if ( m_locationType() == LocationType::COUNT )
            {
                uiOrdering.add( &m_rangeCount );
                uiOrdering.add( &m_rangeSpacing );
            }
            else if ( m_locationType() == LocationType::SPACING )
            {
                uiOrdering.add( &m_rangeSpacing );
                uiOrdering.add( &m_rangeCount );
            }
        }

        uiOrdering.add( &m_locations );
    }

    if ( m_locationType() == LocationType::CUSTOM )
    {
        m_locations.uiCapability()->setUiReadOnly( false );

        m_rangeSpacing.uiCapability()->setUiReadOnly( true );
        m_rangeCount.uiCapability()->setUiReadOnly( true );
        m_rangeStart.uiCapability()->setUiReadOnly( true );
        m_rangeEnd.uiCapability()->setUiReadOnly( true );
    }
    else
    {
        m_locations.uiCapability()->setUiReadOnly( true );

        m_rangeSpacing.uiCapability()->setUiReadOnly( false );
        m_rangeCount.uiCapability()->setUiReadOnly( false );
        m_rangeStart.uiCapability()->setUiReadOnly( false );
        m_rangeEnd.uiCapability()->setUiReadOnly( false );

        if ( m_locationType() == LocationType::COUNT )
        {
            m_rangeSpacing.uiCapability()->setUiReadOnly( true );
            m_rangeCount.uiCapability()->setUiReadOnly( false );
        }
        else
        {
            m_rangeSpacing.uiCapability()->setUiReadOnly( false );
            m_rangeCount.uiCapability()->setUiReadOnly( true );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMultipleLocations::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                             const QVariant&            oldValue,
                                             const QVariant&            newValue )
{
    bool recomputeLocations = false;

    if ( changedField == &m_locationType )
    {
        if ( m_locationType == LocationType::COUNT || m_locationType == LocationType::SPACING )
        {
            recomputeLocations = true;
        }
    }

    if ( changedField == &m_rangeStart || changedField == &m_rangeEnd || changedField == &m_rangeCount ||
         changedField == &m_rangeSpacing )
    {
        recomputeLocations = true;
        m_rangeStart       = std::clamp( m_rangeStart(), minimumMD(), maximumMD() );
        m_rangeEnd         = std::clamp( m_rangeEnd(), minimumMD(), maximumMD() );
    }

    if ( changedField == &m_rangeSpacing )
    {
        double minimumDistanceMeter = minimumSpacingMeters();

        m_rangeSpacing =
            std::clamp( m_rangeSpacing(), minimumDistanceMeter, std::max( m_rangeSpacing(), minimumDistanceMeter ) );
    }

    if ( recomputeLocations )
    {
        computeRangesAndLocations();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimMultipleLocations::rangeCountFromSpacing() const
{
    int rangeCount = ( std::fabs( m_rangeStart - m_rangeEnd ) / m_rangeSpacing ) + 1;

    if ( rangeCount < 1 )
    {
        rangeCount = 1;
    }
    return rangeCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimMultipleLocations::minimumSpacingMeters() const
{
    // Minimum distance between fishbones is 13.0m
    // Use 10.0m to allow for some flexibility
    return 10.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimMultipleLocations::minimumMD() const
{
    return m_rangeStart();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimMultipleLocations::maximumMD() const
{
    return m_rangeEnd();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimMultipleLocations::locationsFromStartSpacingAndCount( double start, double spacing, size_t count )
{
    std::vector<double> measuredDepths;

    for ( size_t i = 0; i < count; i++ )
    {
        measuredDepths.push_back( start + spacing * i );
    }

    return measuredDepths;
}
