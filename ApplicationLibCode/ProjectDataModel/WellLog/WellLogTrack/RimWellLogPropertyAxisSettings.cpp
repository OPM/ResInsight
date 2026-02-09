/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RimWellLogPropertyAxisSettings.h"

#include "RimWellLogTrack.h"

#include "cafPdmUiDoubleValueEditor.h"

#define RI_LOGPLOTTRACK_MINX_DEFAULT -10.0
#define RI_LOGPLOTTRACK_MAXX_DEFAULT 100.0

CAF_PDM_SOURCE_INIT( RimWellLogPropertyAxisSettings, "RimWellLogPropertyAxisSettings" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogPropertyAxisSettings::RimWellLogPropertyAxisSettings()
{
    CAF_PDM_InitObject( "Property Axis Settings" );

    CAF_PDM_InitField( &m_isEnabled, "IsEnabled", true, "Show Axis" );

    CAF_PDM_InitField( &m_visibleRangeMin, "VisibleRangeMin", RI_LOGPLOTTRACK_MINX_DEFAULT, "Min" );
    m_visibleRangeMin.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleValueEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_visibleRangeMax, "VisibleRangeMax", RI_LOGPLOTTRACK_MAXX_DEFAULT, "Max" );
    m_visibleRangeMax.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleValueEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_isAutoScaleEnabled, "IsAutoScaleEnabled", true, "Auto Scale" );
    m_isAutoScaleEnabled.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_isLogarithmicScaleEnabled, "IsLogarithmicScaleEnabled", false, "Logarithmic Scale" );
    CAF_PDM_InitField( &m_isAxisInverted, "IsAxisInverted", false, "Invert Axis Range" );

    CAF_PDM_InitFieldNoDefault( &m_gridVisibility, "GridVisibility", "Show Grid Lines" );

    CAF_PDM_InitField( &m_minAndMaxTicksOnly, "MinAndMaxTicksOnly", false, "Show Ticks at Min and Max" );
    CAF_PDM_InitField( &m_explicitTickIntervals, "ExplicitTickIntervals", false, "Manually Set Tick Intervals" );

    CAF_PDM_InitField( &m_majorTickInterval, "MajorTickInterval", 0.0, "Major Tick Interval" );
    m_majorTickInterval.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_minorTickInterval, "MinorTickInterval", 0.0, "Minor Tick Interval" );
    m_minorTickInterval.uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogPropertyAxisSettings::~RimWellLogPropertyAxisSettings()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellLogPropertyAxisSettings::isEnabled() const
{
    return m_isEnabled();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPropertyAxisSettings::setEnabled( bool enable )
{
    m_isEnabled = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellLogPropertyAxisSettings::visibleRangeMin() const
{
    return m_visibleRangeMin();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellLogPropertyAxisSettings::visibleRangeMax() const
{
    return m_visibleRangeMax();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPropertyAxisSettings::setVisibleRange( double minValue, double maxValue )
{
    m_visibleRangeMin = minValue;
    m_visibleRangeMax = maxValue;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellLogPropertyAxisSettings::isAutoScaleEnabled() const
{
    return m_isAutoScaleEnabled();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPropertyAxisSettings::setAutoScaleEnabled( bool enabled )
{
    m_isAutoScaleEnabled = enabled;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellLogPropertyAxisSettings::isLogarithmicScaleEnabled() const
{
    return m_isLogarithmicScaleEnabled();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPropertyAxisSettings::setLogarithmicScaleEnabled( bool enabled )
{
    m_isLogarithmicScaleEnabled = enabled;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellLogPropertyAxisSettings::isAxisInverted() const
{
    return m_isAxisInverted();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPropertyAxisSettings::setAxisInverted( bool inverted )
{
    m_isAxisInverted = inverted;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDepthTrackPlot::AxisGridVisibility RimWellLogPropertyAxisSettings::gridVisibility() const
{
    return m_gridVisibility();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPropertyAxisSettings::setGridVisibility( RimDepthTrackPlot::AxisGridVisibility visibility )
{
    m_gridVisibility = visibility;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellLogPropertyAxisSettings::isMinAndMaxTicksOnly() const
{
    return m_minAndMaxTicksOnly();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPropertyAxisSettings::setMinAndMaxTicksOnly( bool enable )
{
    m_minAndMaxTicksOnly = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellLogPropertyAxisSettings::isExplicitTickIntervals() const
{
    return m_explicitTickIntervals();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPropertyAxisSettings::setExplicitTickIntervals( bool enable )
{
    m_explicitTickIntervals = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellLogPropertyAxisSettings::majorTickInterval() const
{
    return m_majorTickInterval();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellLogPropertyAxisSettings::minorTickInterval() const
{
    return m_minorTickInterval();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPropertyAxisSettings::setTickIntervals( double majorInterval, double minorInterval )
{
    m_majorTickInterval     = majorInterval;
    m_minorTickInterval     = minorInterval;
    m_explicitTickIntervals = true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPropertyAxisSettings::uiOrdering( const QString& uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiGroup* propertyAxisGroup = uiOrdering.addNewGroup( "Property Axis" );

    propertyAxisGroup->add( &m_isEnabled );
    propertyAxisGroup->add( &m_isLogarithmicScaleEnabled );
    propertyAxisGroup->add( &m_isAxisInverted );
    propertyAxisGroup->add( &m_visibleRangeMin );
    propertyAxisGroup->add( &m_visibleRangeMax );

    propertyAxisGroup->add( &m_gridVisibility );

    propertyAxisGroup->add( &m_explicitTickIntervals );
    if ( m_explicitTickIntervals() )
    {
        m_majorTickInterval.uiCapability()->setUiHidden( false );
        m_minorTickInterval.uiCapability()->setUiHidden( false );
        propertyAxisGroup->add( &m_majorTickInterval );
        propertyAxisGroup->add( &m_minorTickInterval );
    }
    else
    {
        m_majorTickInterval.uiCapability()->setUiHidden( true );
        m_minorTickInterval.uiCapability()->setUiHidden( true );
    }

    propertyAxisGroup->add( &m_minAndMaxTicksOnly );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPropertyAxisSettings::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    auto track = firstAncestorOrThisOfType<RimWellLogTrack>();
    if ( track )
    {
        if ( changedField == &m_isEnabled )
        {
            track->updateAxes();
            track->updateConnectedEditors();
        }
        else if ( changedField == &m_isLogarithmicScaleEnabled || changedField == &m_isAxisInverted )
        {
            track->loadDataAndUpdate();
            track->updateConnectedEditors();
        }
        else if ( changedField == &m_visibleRangeMin || changedField == &m_visibleRangeMax )
        {
            m_isAutoScaleEnabled = false;
            track->updatePlotWidgetFromAxisRanges();
            track->updateConnectedEditors();
        }
        else if ( changedField == &m_gridVisibility || changedField == &m_explicitTickIntervals || changedField == &m_majorTickInterval ||
                  changedField == &m_minorTickInterval || changedField == &m_minAndMaxTicksOnly )
        {
            track->updatePropertyValueAxisAndGridTickIntervals();
            track->updateConnectedEditors();
        }
    }
}
