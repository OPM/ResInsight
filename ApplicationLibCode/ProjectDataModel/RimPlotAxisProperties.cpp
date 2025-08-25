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
#include "RiaFieldHandleTools.h"
#include "RiaPlotDefines.h"
#include "RiaPreferences.h"

#include "RimPlotAxisAnnotation.h"
#include "RimSummaryMultiPlot.h"

#include "cafPdmUiSliderEditor.h"
#include "cafPdmUiTreeAttributes.h"

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
    : logarithmicChanged( this )
    , axisPositionChanged( this )
    , m_enableTitleTextSettings( true )
    , m_enableTitleLayoutSettings( true )
    , m_isRangeSettingsEnabled( true )
    , m_isAlwaysRequired( false )
{
    CAF_PDM_InitObject( "Axis Properties", ":/LeftAxis16x16.png" );

    CAF_PDM_InitField( &m_isActive, "Active", true, "Active" );
    m_isActive.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_objectName, "Name", "Name" );
    m_objectName.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_axisTitle, "AxisTitle", "Axis Title" );
    m_objectName.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitField( &isAutoTitle, "AutoTitle", true, "Auto Title" );

    CAF_PDM_InitField( &m_displayLongName, "DisplayLongName", true, "   Names" );

    CAF_PDM_InitField( &m_displayShortName, "DisplayShortName", false, "   Acronyms" );
    CAF_PDM_InitField( &m_displayUnitText, "DisplayUnitText", true, "   Units" );

    CAF_PDM_InitFieldNoDefault( &m_customTitle, "CustomTitle", "Title" );

    CAF_PDM_InitField( &m_visibleRangeMax, "VisibleRangeMax", RiaDefines::maximumDefaultValuePlot(), "Max" );
    CAF_PDM_InitField( &m_visibleRangeMin, "VisibleRangeMin", RiaDefines::minimumDefaultValuePlot(), "Min" );

    CAF_PDM_InitFieldNoDefault( &m_numberFormat, "NumberFormat", "Number Format" );
    CAF_PDM_InitField( &m_numberOfDecimals, "Decimals", 2, "Number of Decimals" );
    CAF_PDM_InitField( &m_scaleFactor, "ScaleFactor", 1.0, "Scale Factor" );

    CAF_PDM_InitField( &m_isAutoZoom, "AutoZoom", true, "Set Range Automatically" );
    CAF_PDM_InitField( &m_isLogarithmicScaleEnabled, "LogarithmicScale", false, "Logarithmic Scale" );
    CAF_PDM_InitField( &m_isAxisInverted, "AxisInverted", false, "Invert Axis" );
    CAF_PDM_InitField( &m_showNumbers, "ShowNumbers", true, "Show Numbers" );

    auto defaultPlotAxis = caf::AppEnum<RiaDefines::PlotAxis>( RiaDefines::PlotAxis::PLOT_AXIS_LEFT );
    CAF_PDM_InitField( &m_plotAxis, "PlotAxis", defaultPlotAxis, "Plot Axis" );
    CAF_PDM_InitField( &m_plotAxisIndex, "PlotAxisIndex", 0, "Plot Axis Index" );

    CAF_PDM_InitFieldNoDefault( &m_titlePositionEnum, "TitlePosition", "Title Position" );

    CAF_PDM_InitFieldNoDefault( &m_titleFontSize, "TitleDeltaFontSize", "Font Size" );
    CAF_PDM_InitFieldNoDefault( &m_valuesFontSize, "ValueDeltaFontSize", "Font Size" );

    CAF_PDM_InitFieldNoDefault( &m_annotations, "Annotations", "" );

    CAF_PDM_InitFieldNoDefault( &m_majorTickmarkCount, "MajorTickmarkCount", "Major Tickmark Count" );

    updateOptionSensitivity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotAxisProperties::configureForBasicUse()
{
    setEnableTitleTextSettings( false );

    m_isLogarithmicScaleEnabled.uiCapability()->setUiHidden( true );
    m_isAxisInverted.uiCapability()->setUiHidden( true );
    m_showNumbers.uiCapability()->setUiHidden( true );
    m_majorTickmarkCount.uiCapability()->setUiHidden( true );
    m_plotAxis.uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotAxisProperties::configureForHistogramUse()
{
    setEnableTitleTextSettings( false );
    setEnableTitleLayoutSettings( false );

    m_isLogarithmicScaleEnabled.uiCapability()->setUiHidden( true );
    m_isAxisInverted.uiCapability()->setUiHidden( true );
    m_showNumbers.uiCapability()->setUiHidden( true );
    m_plotAxis.uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotAxisProperties::setAlwaysRequired( bool enable )
{
    m_isAlwaysRequired = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPlotAxisProperties::isDeletable() const
{
    return !m_isAlwaysRequired;
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
void RimPlotAxisProperties::setEnableTitleLayoutSettings( bool enable )
{
    m_enableTitleLayoutSettings = enable;
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
void RimPlotAxisProperties::setNameForUnusedAxis()
{
    QString name = "Unused " + m_plotAxis().text();

    m_objectName = name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimPlotAxisProperties::userDescriptionField()
{
    return &m_objectName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimPlotAxisProperties::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_scaleFactor )
    {
        for ( int exp = -12; exp <= 12; exp += 3 )
        {
            QString uiText = exp == 0 ? "1" : QString( "10 ^ %1" ).arg( exp );
            double  value  = std::pow( 10, exp );

            options.push_back( caf::PdmOptionItemInfo( uiText, value ) );
        }
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

            m_customTitle.uiCapability()->setUiReadOnly( true );
        }
        else
        {
            titleTextGroup->add( &m_customTitle );
            m_customTitle.uiCapability()->setUiReadOnly( false );
        }
    }

    if ( m_enableTitleLayoutSettings )
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
        scaleGroup.add( &m_showNumbers );
    }
    scaleGroup.add( &m_numberFormat );

    if ( m_numberFormat() != NUMBER_FORMAT_AUTO )
    {
        scaleGroup.add( &m_numberOfDecimals );
    }
    scaleGroup.add( &m_scaleFactor );
    if ( m_isRangeSettingsEnabled )
    {
        scaleGroup.add( &m_visibleRangeMin );
        scaleGroup.add( &m_visibleRangeMax );
    }

    scaleGroup.add( &m_valuesFontSize );
    scaleGroup.add( &m_majorTickmarkCount );
    scaleGroup.add( &m_plotAxis );
    m_plotAxis.uiCapability()->setUiReadOnly( m_isAlwaysRequired );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotAxisProperties::setNameAndAxis( const QString& objectName, const QString& axistTitle, RiaDefines::PlotAxis axis, int axisIndex )
{
    m_objectName    = objectName;
    m_axisTitle     = axistTitle;
    m_plotAxis      = axis;
    m_plotAxisIndex = axisIndex;

    if ( axis == RiaDefines::PlotAxis::PLOT_AXIS_LEFT ) setUiIconFromResourceString( ":/LeftAxis16x16.png" );
    if ( axis == RiaDefines::PlotAxis::PLOT_AXIS_RIGHT ) setUiIconFromResourceString( ":/RightAxis16x16.png" );
    if ( axis == RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM ) setUiIconFromResourceString( ":/BottomAxis16x16.png" );
    if ( axis == RiaDefines::PlotAxis::PLOT_AXIS_TOP ) setUiIconFromResourceString( ":/TopAxis16x16.png" );
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
QString RimPlotAxisProperties::customTitle() const
{
    return m_customTitle();
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
const QString RimPlotAxisProperties::objectName() const
{
    return m_objectName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString RimPlotAxisProperties::axisTitleText() const
{
    return m_axisTitle;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotAxis RimPlotAxisProperties::plotAxis() const
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
void RimPlotAxisProperties::setShowDescription( bool enable )
{
    m_displayLongName = enable;
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
void RimPlotAxisProperties::setShowAcronym( bool enable )
{
    m_displayShortName = enable;
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
void RimPlotAxisProperties::setShowUnitText( bool enable )
{
    m_displayUnitText = enable;
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
    return m_annotations.childrenByType();
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
    m_annotations.deleteChildren();
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
bool RimPlotAxisProperties::showNumbers() const
{
    return m_showNumbers;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotAxisProperties::setShowNumbers( bool enable )
{
    m_showNumbers = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlotAxisProperties::NumberFormatType RimPlotAxisProperties::numberFormat() const
{
    return m_numberFormat();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimPlotAxisProperties::decimalCount() const
{
    return m_numberOfDecimals();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimPlotAxisProperties::scaleFactor() const
{
    return m_scaleFactor();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotAxisProperties::setVisible( bool visible )
{
    m_isActive = visible;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotAxisProperties::enableAutoValueForScaleFactor( bool enable )
{
    m_scaleFactor.uiCapability()->enableAutoValue( enable );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotAxisProperties::computeAndSetAutoValueForScaleFactor()
{
    auto maxAbsValue = std::max( std::fabs( visibleRangeMax() ), std::fabs( visibleRangeMin() ) );

    double scaleFactor = 1.0;

    if ( maxAbsValue < 1.0 && maxAbsValue > 1e-6 )
    {
        // Do not use scale factor for small values above 1e-6
        scaleFactor = 1.0;
    }
    else if ( maxAbsValue > 1.0 && maxAbsValue < 1e6 )
    {
        // Do not use scale factor for values above 1 and below 1e-6
        scaleFactor = 1.0;
    }
    else
    {
        int exponent = std::floor( std::log10( maxAbsValue ) );
        if ( exponent > 0 )
        {
            while ( exponent > -20 && ( exponent % 3 ) != 0 )
            {
                exponent--;
            }
        }
        else
        {
            while ( exponent < 1 && ( exponent % 3 ) != 0 )
            {
                exponent++;
            }
        }

        scaleFactor = std::pow( 10, exponent );
    }

    m_scaleFactor.uiCapability()->setAutoValue( scaleFactor );
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
void RimPlotAxisProperties::enableAutoValueMinMax( bool enable )
{
    m_visibleRangeMin.uiCapability()->enableAutoValueSupport( enable );
    m_visibleRangeMax.uiCapability()->enableAutoValueSupport( enable );

    if ( enable )
    {
        m_visibleRangeMin.uiCapability()->enableAutoValue( enable );
        m_visibleRangeMax.uiCapability()->enableAutoValue( enable );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotAxisProperties::setAutoValueVisibleRangeMin( double value )
{
    // Do not notify editors, as this causes recursive updates
    bool notifyFieldChanged = false;
    m_visibleRangeMin.uiCapability()->setAutoValue( value, notifyFieldChanged );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotAxisProperties::setAutoValueVisibleRangeMax( double value )
{
    // Do not notify editors, as this causes recursive updates
    bool notifyFieldChanged = false;
    m_visibleRangeMax.uiCapability()->setAutoValue( value, notifyFieldChanged );
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
void RimPlotAxisProperties::setAutoZoomIfNoCustomRangeIsSet()
{
    if ( !m_visibleRangeMax.uiCapability()->isAutoValueEnabled() ) return;
    if ( !m_visibleRangeMin.uiCapability()->isAutoValueEnabled() ) return;

    setAutoZoom( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlotAxisPropertiesInterface::LegendTickmarkCount RimPlotAxisProperties::majorTickmarkCount() const
{
    return m_majorTickmarkCount();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotAxisProperties::setMajorTickmarkCount( LegendTickmarkCount count )
{
    m_majorTickmarkCount = count;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotAxisProperties::setAutoValueForMajorTickmarkCount( LegendTickmarkCount count, bool notifyFieldChanged )
{
    auto enumValue = static_cast<std::underlying_type_t<LegendTickmarkCount>>( count );

    m_majorTickmarkCount.uiCapability()->setAutoValue( enumValue, notifyFieldChanged );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotAxisProperties::enableAutoValueForMajorTickmarkCount( bool enable )
{
    m_majorTickmarkCount.uiCapability()->enableAutoValue( enable );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotAxisProperties::enableAutoValueForAllFields( bool enable )
{
    enableAutoValueForMajorTickmarkCount( enable );
    enableAutoValueForScaleFactor( enable );
    enableAutoValueMinMax( enable );

    m_displayLongName.uiCapability()->enableAutoValueSupport( enable );
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
void RimPlotAxisProperties::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
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
        axisPositionChanged.send( this, oldPlotAxis, plotAxis() );
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
    m_customTitle.uiCapability()->setUiReadOnly( isAutoTitle );
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
void RimPlotAxisProperties::defineObjectEditorAttribute( QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    auto summaryMultiPlot = firstAncestorOfType<RimSummaryMultiPlot>();

    if ( summaryMultiPlot && summaryMultiPlot->isSubPlotAxesLinked() )
    {
        auto* treeItemAttribute = dynamic_cast<caf::PdmUiTreeViewItemAttribute*>( attribute );
        if ( treeItemAttribute )
        {
            treeItemAttribute->tags.clear();
            auto tag  = caf::PdmUiTreeViewItemAttribute::createTag();
            tag->icon = caf::IconProvider( ":/chain.png" );

            treeItemAttribute->tags.push_back( std::move( tag ) );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotAxisProperties::initAfterRead()
{
    updateOptionSensitivity();
    m_isAlwaysRequired = m_plotAxisIndex == 0;
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
