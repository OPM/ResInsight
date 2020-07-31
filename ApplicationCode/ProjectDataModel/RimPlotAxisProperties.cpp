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

#include "RiaApplication.h"
#include "RiaDefines.h"
#include "RiaFontCache.h"
#include "RiaPreferences.h"
#include "RigStatisticsCalculator.h"

#include "RimPlot.h"
#include "RimPlotAxisAnnotation.h"

#include "cafPdmUiSliderEditor.h"

#include "cvfVector2.h"

#include <cmath>

#include <qwt_plot_curve.h>

// clang-format off
namespace caf
{
template<>
void caf::AppEnum<RimPlotAxisProperties::NumberFormatType>::setUp()
{
    addItem(RimPlotAxisProperties::NUMBER_FORMAT_AUTO,       "NUMBER_FORMAT_AUTO",       "Auto");
    addItem(RimPlotAxisProperties::NUMBER_FORMAT_DECIMAL,    "NUMBER_FORMAT_DECIMAL",    "Decimal");
    addItem(RimPlotAxisProperties::NUMBER_FORMAT_SCIENTIFIC, "NUMBER_FORMAT_SCIENTIFIC", "Scientific");

    setDefault(RimPlotAxisProperties::NUMBER_FORMAT_AUTO);
}
} // namespace caf

CAF_PDM_SOURCE_INIT(RimPlotAxisProperties, "SummaryYAxisProperties");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlotAxisProperties::RimPlotAxisProperties()
    : settingsChanged(this)
    , logarithmicChanged(this)
    , stackingChanged(this)
    , stackingColorsChanged(this)
    , m_enableTitleTextSettings(true)
    , m_isRangeSettingsEnabled(true)
{
    CAF_PDM_InitObject("Axis Properties", ":/LeftAxis16x16.png", "", "");

    CAF_PDM_InitField(&m_isActive, "Active", true, "Active", "", "", "");
    m_isActive.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_name, "Name", "Name", "", "", "");
    m_name.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&isAutoTitle, "AutoTitle", true, "Auto Title", "", "", "");
    
    CAF_PDM_InitField(&m_displayLongName,   "DisplayLongName",  true,   "   Names", "", "", "");
    CAF_PDM_InitField(&m_displayShortName,  "DisplayShortName", false,  "   Acronyms", "", "", "");
    CAF_PDM_InitField(&m_displayUnitText,   "DisplayUnitText",  true,   "   Units", "", "", "");

    CAF_PDM_InitFieldNoDefault(&customTitle,        "CustomTitle",      "Title", "", "", "");

    CAF_PDM_InitField(&visibleRangeMax, "VisibleRangeMax", RiaDefines::maximumDefaultValuePlot(), "Max", "", "", "");
    CAF_PDM_InitField(&visibleRangeMin, "VisibleRangeMin", RiaDefines::minimumDefaultValuePlot(), "Min", "", "", "");

    CAF_PDM_InitFieldNoDefault(&numberFormat,   "NumberFormat",         "Number Format", "", "", "");
    CAF_PDM_InitField(&numberOfDecimals,        "Decimals", 2,          "Number of Decimals", "", "", "");
    CAF_PDM_InitField(&scaleFactor,             "ScaleFactor", 1.0,     "Scale Factor", "", "", "");

    numberOfDecimals.uiCapability()->setUiEditorTypeName(caf::PdmUiSliderEditor::uiEditorTypeName());

    CAF_PDM_InitField(&m_isAutoZoom, "AutoZoom", true, "Set Range Automatically", "", "", "");
    CAF_PDM_InitField(&isLogarithmicScaleEnabled, "LogarithmicScale", false, "Logarithmic Scale", "", "", "");
    CAF_PDM_InitField(&m_isAxisInverted, "AxisInverted", false, "Invert Axis", "", "", "");
    CAF_PDM_InitField(&m_stackCurves, "StackCurves", false, "Stack Curves", "", "", "");
    CAF_PDM_InitField(&m_stackWithPhaseColors, "StackPhaseColors", false, "  with Phase Colors", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_titlePositionEnum, "TitlePosition", "Title Position", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_titleFontSize, "TitleDeltaFontSize", "Font Size", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_valuesFontSize, "ValueDeltaFontSize", "Font Size", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_annotations, "Annotations", "", "", "", "");

    m_annotations.uiCapability()->setUiHidden(true);
//     m_annotations.uiCapability()->setUiTreeChildrenHidden(true);

    updateOptionSensitivity();
}
// clang-format on

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
        scaleGroup.add( &isLogarithmicScaleEnabled );
        scaleGroup.add( &m_isAxisInverted );
    }
    scaleGroup.add( &m_stackCurves );
    if ( m_stackCurves )
    {
        scaleGroup.add( &m_stackWithPhaseColors );
    }
    scaleGroup.add( &numberFormat );

    if ( numberFormat() != NUMBER_FORMAT_AUTO )
    {
        scaleGroup.add( &numberOfDecimals );
    }
    scaleGroup.add( &scaleFactor );
    if ( m_isRangeSettingsEnabled )
    {
        scaleGroup.add( &visibleRangeMin );
        scaleGroup.add( &visibleRangeMax );
    }
    scaleGroup.add( &m_valuesFontSize );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotAxisProperties::setNameAndAxis( const QString& name, QwtPlot::Axis axis )
{
    m_name = name;
    m_axis = axis;

    if ( axis == QwtPlot::yRight ) this->setUiIconFromResourceString( ":/RightAxis16x16.png" );
    if ( axis == QwtPlot::xBottom ) this->setUiIconFromResourceString( ":/BottomAxis16x16.png" );
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
QwtPlot::Axis RimPlotAxisProperties::qwtPlotAxisType() const
{
    return m_axis;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimPlotAxisProperties::name() const
{
    return m_name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::PlotAxis RimPlotAxisProperties::plotAxisType() const
{
    if ( m_axis == QwtPlot::yRight ) return RiaDefines::PlotAxis::PLOT_AXIS_RIGHT;
    if ( m_axis == QwtPlot::xBottom ) return RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM;

    return RiaDefines::PlotAxis::PLOT_AXIS_LEFT;
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
void RimPlotAxisProperties::setAxisInverted( bool inverted )
{
    m_isAxisInverted = inverted;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPlotAxisProperties::stackCurves() const
{
    return m_stackCurves();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPlotAxisProperties::stackWithPhaseColors() const
{
    return m_stackWithPhaseColors;
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
void RimPlotAxisProperties::setInvertedAxis( bool enable )
{
    m_isAxisInverted = enable;
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
    else if ( changedField == &visibleRangeMax )
    {
        if ( visibleRangeMin > visibleRangeMax ) visibleRangeMax = oldValue.toDouble();

        m_isAutoZoom = false;
    }
    else if ( changedField == &visibleRangeMin )
    {
        if ( visibleRangeMin > visibleRangeMax ) visibleRangeMin = oldValue.toDouble();

        m_isAutoZoom = false;
    }

    if ( changedField == &isLogarithmicScaleEnabled )
    {
        logarithmicChanged.send( isLogarithmicScaleEnabled() );
    }
    else if ( changedField == &m_stackCurves )
    {
        stackingChanged.send( m_stackCurves() );
    }
    else if ( changedField == &m_stackWithPhaseColors )
    {
        stackingColorsChanged.send( m_stackWithPhaseColors() );
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
RimPlotAxisLogRangeCalculator::RimPlotAxisLogRangeCalculator( QwtPlot::Axis                           axis,
                                                              const std::vector<const QwtPlotCurve*>& qwtCurves )
    : m_axis( axis )
    , m_curves( qwtCurves )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotAxisLogRangeCalculator::computeAxisRange( double* minPositive, double* max ) const
{
    double minPosValue = HUGE_VAL;
    double maxValue    = -HUGE_VAL;

    for ( const QwtPlotCurve* curve : m_curves )
    {
        double minPosCurveValue = HUGE_VAL;
        double maxCurveValue    = -HUGE_VAL;

        if ( curveValueRange( curve, &minPosCurveValue, &maxCurveValue ) )
        {
            if ( minPosCurveValue < minPosValue )
            {
                CVF_ASSERT( minPosCurveValue > 0.0 );
                minPosValue = minPosCurveValue;
            }

            if ( maxCurveValue > maxValue )
            {
                maxValue = maxCurveValue;
            }
        }
    }

    if ( minPosValue == HUGE_VAL )
    {
        minPosValue = RiaDefines::minimumDefaultLogValuePlot();
        maxValue    = RiaDefines::maximumDefaultValuePlot();
    }

    *minPositive = minPosValue;
    *max         = maxValue;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPlotAxisLogRangeCalculator::curveValueRange( const QwtPlotCurve* qwtCurve, double* minPositive, double* max ) const
{
    if ( !qwtCurve ) return false;

    if ( qwtCurve->data()->size() < 1 )
    {
        return false;
    }

    float minPosF = std::numeric_limits<float>::infinity();
    float maxF    = -std::numeric_limits<float>::infinity();

    int axisValueIndex = 0;
    if ( m_axis == QwtPlot::yLeft || m_axis == QwtPlot::yRight )
    {
        axisValueIndex = 1;
    }

    for ( size_t i = 0; i < qwtCurve->dataSize(); ++i )
    {
        QPointF    sample = qwtCurve->sample( (int)i );
        cvf::Vec2f vec( sample.x(), sample.y() );
        float      value = vec[axisValueIndex];
        if ( value == HUGE_VALF ) continue;

        maxF = std::max( maxF, value );
        if ( value > 0.0f && value < minPosF )
        {
            minPosF = value;
        }
    }

    *minPositive = minPosF;
    *max         = maxF;

    return true;
}
