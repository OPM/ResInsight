/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-2018 Statoil ASA
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

#include "RimPlotAxisProperties.h"

#include "RiaDefines.h"
#include "RiaPreferences.h"

#include "RimPlotAxisAnnotation.h"

#include "cafPdmUiSliderEditor.h"

#include "cvfVector2.h"

#include <cmath>

namespace caf
{
template <>
void caf::AppEnum<RimPlotAxisProperties::NumberFormatType>::setUp()
{
    addItem( RimPlotAxisProperties::NUMBER_FORMAT_AUTO, "NUMBER_FORMAT_AUTO", "Auto" );
    addItem( RimPlotAxisProperties::NUMBER_FORMAT_DECIMAL, "NUMBER_FORMAT_DECIMAL", "Decimal" );
    addItem( RimPlotAxisProperties::NUMBER_FORMAT_SCIENTIFIC, "NUMBER_FORMAT_SCIENTIFIC", "Scientific" );

    setDefault( RimPlotAxisProperties::NUMBER_FORMAT_AUTO );
}
} // namespace caf

CAF_PDM_SOURCE_INIT( RimPlotAxisProperties, "SummaryYAxisProperties" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlotAxisProperties::RimPlotAxisProperties()
    : settingsChanged( this )
    , logarithmicChanged( this )
    , axisPositionChanged( this )
    , m_enableTitleTextSettings( true )
    , m_isRangeSettingsEnabled( true )
{
    CAF_PDM_InitObject( "Axis Properties", ":/LeftAxis16x16.png" );

    CAF_PDM_InitField( &m_isActive, "Active", true, "Active" );
    m_isActive.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_name, "Name", "Name" );
    m_name.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &isAutoTitle, "AutoTitle", true, "Auto Title" );

    CAF_PDM_InitField( &m_displayLongName, "DisplayLongName", true, "   Names" );
    CAF_PDM_InitField( &m_displayShortName, "DisplayShortName", false, "   Acronyms" );
    CAF_PDM_InitField( &m_displayUnitText, "DisplayUnitText", true, "   Units" );

    CAF_PDM_InitFieldNoDefault( &customTitle, "CustomTitle", "Title" );

    CAF_PDM_InitField( &m_visibleRangeMax, "VisibleRangeMax", RiaDefines::maximumDefaultValuePlot(), "Max" );
    CAF_PDM_InitField( &m_visibleRangeMin, "VisibleRangeMin", RiaDefines::minimumDefaultValuePlot(), "Min" );

    CAF_PDM_InitFieldNoDefault( &numberFormat, "NumberFormat", "Number Format" );
    CAF_PDM_InitField( &numberOfDecimals, "Decimals", 2, "Number of Decimals" );
    CAF_PDM_InitField( &scaleFactor, "ScaleFactor", 1.0, "Scale Factor" );

    CAF_PDM_InitField( &m_isAutoZoom, "AutoZoom", true, "Set Range Automatically" );
    CAF_PDM_InitField( &m_isLogarithmicScaleEnabled, "LogarithmicScale", false, "Logarithmic Scale" );
    CAF_PDM_InitField( &m_isAxisInverted, "AxisInverted", false, "Invert Axis" );

    auto defaultPlotAxis = caf::AppEnum<RiaDefines::PlotAxis>( RiaDefines::PlotAxis::PLOT_AXIS_LEFT );
    CAF_PDM_InitField( &m_plotAxis, "PlotAxis", defaultPlotAxis, "Plot Axis" );
    CAF_PDM_InitField( &m_plotAxisIndex, "PlotAxisIndex", 0, "Plot Axis Index" );

    CAF_PDM_InitFieldNoDefault( &m_titlePositionEnum, "TitlePosition", "Title Position" );

    CAF_PDM_InitFieldNoDefault( &m_titleFontSize, "TitleDeltaFontSize", "Font Size" );
    CAF_PDM_InitFieldNoDefault( &m_valuesFontSize, "ValueDeltaFontSize", "Font Size" );

    CAF_PDM_InitFieldNoDefault( &m_annotations, "Annotations", "" );
    m_annotations.uiCapability()->setUiTreeHidden( true );

    updateOptionSensitivity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPlotAxisProperties::isDeletable() const
{
    // The default axes (which have index 0) are not deletable
    return m_plotAxisIndex != 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotAxisProperties::setEnableTitleTextSettings( bool enable )
{
    m_enableTitleTextSettings = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotAxisProperties::enableRangeSettings( bool enable )
{
    m_isRangeSettingsEnabled = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimPlotAxisProperties::userDescriptionField()
{
    return &m_name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RimPlotAxisProperties::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;
    *useOptionsOnly = true;

    if ( fieldNeedingOptions == &scaleFactor )
    {
        for ( int exp = -12; exp <= 12; exp += 3 )
        {
            QString uiText = exp == 0 ? "1" : QString( "10 ^ %1" ).arg( exp );
            double  value  = std::pow( 10, exp );

            options.push_back( caf::PdmOptionItemInfo( uiText, value ) );
        }
    }
    else if ( fieldNeedingOptions == &m_titleFontSize || fieldNeedingOptions == &m_valuesFontSize )
    {
        options = caf::FontTools::relativeSizeValueOptions( RiaPreferences::current()->defaultPlotFontSize() );
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotAxisProperties::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    if ( m_enableTitleTextSettings )
    {
        caf::PdmUiGroup* titleTextGroup = uiOrdering.addNewGroup( "Title Text" );

        titleTextGroup->add( &isAutoTitle );

        if ( isAutoTitle() )
        {
            titleTextGroup->add( &m_displayLongName );
            titleTextGroup->add( &m_displayShortName );
            titleTextGroup->add( &m_displayUnitText );

            customTitle.uiCapability()->setUiReadOnly( true );
        }
        else
        {
            titleTextGroup->add( &customTitle );
            customTitle.uiCapability()->setUiReadOnly( false );
        }
    }

    {
        caf::PdmUiGroup* titleGroup = uiOrdering.addNewGroup( "Title Layout" );
        titleGroup->add( &m_titlePositionEnum );
        titleGroup->add( &m_titleFontSize );
    }

    caf::PdmUiGroup& scaleGroup = *( uiOrdering.addNewGroup( "Axis Values" ) );
    if ( m_isRangeSettingsEnabled )
    {
        scaleGroup.add( &m_isLogarithmicScaleEnabled );
        scaleGroup.add( &m_isAxisInverted );
    }
    scaleGroup.add( &numberFormat );

    if ( numberFormat() != NUMBER_FORMAT_AUTO )
    {
        scaleGroup.add( &numberOfDecimals );
    }
    scaleGroup.add( &scaleFactor );
    if ( m_isRangeSettingsEnabled )
    {
        scaleGroup.add( &m_visibleRangeMin );
        scaleGroup.add( &m_visibleRangeMax );
    }
    scaleGroup.add( &m_valuesFontSize );

    scaleGroup.add( &m_plotAxis );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotAxisProperties::setNameAndAxis( const QString& name, RiaDefines::PlotAxis axis, int axisIndex )
{
    m_name          = name;
    m_plotAxis      = axis;
    m_plotAxisIndex = axisIndex;

    if ( axis == RiaDefines::PlotAxis::PLOT_AXIS_LEFT ) this->setUiIconFromResourceString( ":/LeftAxis16x16.png" );
    if ( axis == RiaDefines::PlotAxis::PLOT_AXIS_RIGHT ) this->setUiIconFromResourceString( ":/RightAxis16x16.png" );
    if ( axis == RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM ) this->setUiIconFromResourceString( ":/BottomAxis16x16.png" );
    if ( axis == RiaDefines::PlotAxis::PLOT_AXIS_TOP ) this->setUiIconFromResourceString( ":/TopAxis16x16.png" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlotAxisPropertiesInterface::AxisTitlePositionType RimPlotAxisProperties::titlePosition() const
{
    return m_titlePositionEnum();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimPlotAxisProperties::titleFontSize() const
{
    return caf::FontTools::absolutePointSize( plotFontSize(), m_titleFontSize() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimPlotAxisProperties::valuesFontSize() const
{
    return caf::FontTools::absolutePointSize( plotFontSize(), m_valuesFontSize() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString& RimPlotAxisProperties::name() const
{
    return m_name();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotAxis RimPlotAxisProperties::plotAxisType() const
{
    return RiuPlotAxis( m_plotAxis.value(), m_plotAxisIndex );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPlotAxisProperties::useAutoTitle() const
{
    return isAutoTitle();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPlotAxisProperties::showDescription() const
{
    return m_displayLongName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPlotAxisProperties::showAcronym() const
{
    return m_displayShortName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPlotAxisProperties::showUnitText() const
{
    return m_displayUnitText();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPlotAxisProperties::isAutoZoom() const
{
    return m_isAutoZoom();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotAxisProperties::setAutoZoom( bool enableAutoZoom )
{
    m_isAutoZoom = enableAutoZoom;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPlotAxisProperties::isAxisInverted() const
{
    return m_isAxisInverted();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPlotAxisAnnotation*> RimPlotAxisProperties::annotations() const
{
    return m_annotations.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotAxisProperties::appendAnnotation( RimPlotAxisAnnotation* annotation )
{
    m_annotations.push_back( annotation );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotAxisProperties::removeAllAnnotations()
{
    m_annotations.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotAxisProperties::setAxisInverted( bool inverted )
{
    m_isAxisInverted = inverted;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPlotAxisProperties::isActive() const
{
    return m_isActive;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimPlotAxisProperties::visibleRangeMin() const
{
    return m_visibleRangeMin;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimPlotAxisProperties::visibleRangeMax() const
{
    return m_visibleRangeMax;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotAxisProperties::setVisibleRangeMin( double value )
{
    m_visibleRangeMin = value;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotAxisProperties::setVisibleRangeMax( double value )
{
    m_visibleRangeMax = value;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotAxisProperties::showAnnotationObjectsInProjectTree()
{
    m_annotations.uiCapability()->setUiTreeChildrenHidden( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotAxisProperties::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                              const QVariant&            oldValue,
                                              const QVariant&            newValue )
{
    if ( changedField == &isAutoTitle )
    {
        updateOptionSensitivity();
    }
    else if ( changedField == &m_visibleRangeMax )
    {
        if ( m_visibleRangeMin > m_visibleRangeMax ) m_visibleRangeMax = oldValue.toDouble();

        m_isAutoZoom = false;
    }
    else if ( changedField == &m_visibleRangeMin )
    {
        if ( m_visibleRangeMin > m_visibleRangeMax ) m_visibleRangeMin = oldValue.toDouble();

        m_isAutoZoom = false;
    }

    if ( changedField == &m_isLogarithmicScaleEnabled )
    {
        logarithmicChanged.send( m_isLogarithmicScaleEnabled() );
    }
    else if ( changedField == &m_plotAxis )
    {
        RiuPlotAxis oldPlotAxis = RiuPlotAxis( (RiaDefines::PlotAxis)oldValue.toInt(), m_plotAxisIndex );
        axisPositionChanged.send( this, oldPlotAxis, plotAxisType() );
    }
    else
    {
        settingsChanged.send();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotAxisProperties::updateOptionSensitivity()
{
    customTitle.uiCapability()->setUiReadOnly( isAutoTitle );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::FontTools::FontSize RimPlotAxisProperties::plotFontSize() const
{
    return RiaPreferences::current()->defaultPlotFontSize();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotAxisProperties::initAfterRead()
{
    updateOptionSensitivity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimPlotAxisProperties::objectToggleField()
{
    return &m_isActive;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPlotAxisProperties::isLogarithmicScaleEnabled() const
{
    return m_isLogarithmicScaleEnabled;
}
