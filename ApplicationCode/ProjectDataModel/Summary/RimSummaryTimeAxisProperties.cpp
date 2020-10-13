/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016 Statoil ASA
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

#include "RimSummaryTimeAxisProperties.h"

#include "RiaApplication.h"
#include "RiaFontCache.h"
#include "RiaPreferences.h"

#include "RimSummaryPlot.h"

#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiDateEditor.h"
#include "cafPdmUiLineEditor.h"
#include "cafPdmUiTimeEditor.h"
#include "cvfAssert.h"
#include "qwt_date.h"

namespace caf
{
template <>
void caf::AppEnum<RimSummaryTimeAxisProperties::TimeModeType>::setUp()
{
    addItem( RimSummaryTimeAxisProperties::DATE, "DATE", "Date" );
    addItem( RimSummaryTimeAxisProperties::TIME_FROM_SIMULATION_START,
             "TIME_FROM_SIMULATION_START",
             "Time From Simulation Start" );

    setDefault( RimSummaryTimeAxisProperties::DATE );
}

template <>
void caf::AppEnum<RimSummaryTimeAxisProperties::TimeUnitType>::setUp()
{
    addItem( RimSummaryTimeAxisProperties::SECONDS, "SECONDS", "Seconds" );
    addItem( RimSummaryTimeAxisProperties::MINUTES, "MINUTES", "Minutes" );
    addItem( RimSummaryTimeAxisProperties::HOURS, "HOURS", "Hours" );
    addItem( RimSummaryTimeAxisProperties::DAYS, "DAYS ", "Days" );
    addItem( RimSummaryTimeAxisProperties::MONTHS, "MONTHS", "Months" );
    addItem( RimSummaryTimeAxisProperties::YEARS, "YEARS", "Years" );

    setDefault( RimSummaryTimeAxisProperties::YEARS );
}

} // namespace caf

CAF_PDM_SOURCE_INIT( RimSummaryTimeAxisProperties, "SummaryTimeAxisProperties" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryTimeAxisProperties::RimSummaryTimeAxisProperties()
{
    CAF_PDM_InitObject( "Time Axis", ":/BottomAxis16x16.png", "", "" );

    CAF_PDM_InitField( &m_isActive, "Active", true, "Active", "", "", "" );
    m_isActive.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &showTitle, "ShowTitle", false, "Show Title    ", "", "", "" );
    CAF_PDM_InitField( &title, "Title", QString( "Time" ), "Title          ", "", "", "" );

    CAF_PDM_InitField( &m_isAutoZoom, "AutoZoom", true, "Set Range Automatically", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_timeMode, "TimeMode", "Time Mode", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_timeUnit, "TimeUnit", "Time Unit", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_visibleDateRangeMax, "VisibleDateRangeMax", "Max Date", "", "", "" );
    m_visibleDateRangeMax.uiCapability()->setUiEditorTypeName( caf::PdmUiDateEditor::uiEditorTypeName() );
    CAF_PDM_InitFieldNoDefault( &m_visibleDateRangeMin, "VisibleDateRangeMin", "Min Date", "", "", "" );
    m_visibleDateRangeMin.uiCapability()->setUiEditorTypeName( caf::PdmUiDateEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_visibleTimeRangeMax, "VisibleTimeRangeMax", "MaxTime", "", "", "" );
    m_visibleTimeRangeMax.uiCapability()->setUiEditorTypeName( caf::PdmUiTimeEditor::uiEditorTypeName() );
    m_visibleTimeRangeMax.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitFieldNoDefault( &m_visibleTimeRangeMin, "VisibleTimeRangeMin", "Min Time", "", "", "" );
    m_visibleTimeRangeMin.uiCapability()->setUiEditorTypeName( caf::PdmUiTimeEditor::uiEditorTypeName() );
    m_visibleTimeRangeMin.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitFieldNoDefault( &m_visibleTimeSinceStartRangeMax, "VisibleTimeModeRangeMax", "Max", "", "", "" );
    m_visibleTimeSinceStartRangeMax.uiCapability()->setUiEditorTypeName( caf::PdmUiLineEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_visibleTimeSinceStartRangeMin, "VisibleTimeModeRangeMin", "Min", "", "", "" );
    m_visibleTimeSinceStartRangeMin.uiCapability()->setUiEditorTypeName( caf::PdmUiLineEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_titlePositionEnum, "TitlePosition", "Title Position", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_titleFontSize, "FontSize", "Font Size", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_valuesFontSize, "ValuesFontSize", "Font Size", "", "", "" );

    CAF_PDM_InitField( &m_automaticDateComponents, "AutoDate", true, "Automatic Date/Time Labels", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_dateComponents, "DateComponents", "Set Date Label", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_timeComponents, "TimeComponents", "Set Time Label", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_dateFormat, "DateFormat", "Date Label Format", "", "", "" );
    m_dateFormat.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );
    m_dateFormat = RiaApplication::instance()->preferences()->dateFormat();

    CAF_PDM_InitFieldNoDefault( &m_timeFormat, "TimeFormat", "Time Label Format", "", "", "" );
    m_timeFormat.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );
    m_timeFormat = RiaApplication::instance()->preferences()->timeFormat();

    CAF_PDM_InitFieldNoDefault( &m_majorTickmarkCount, "MajorTickmarkCount", "Major Tickmark Count", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_annotations, "Annotations", "", "", "", "" );

    m_annotations.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_visibleDateTimeRangeMax_OBSOLETE, "VisibleRangeMax", "Max", "", "", "" );
    m_visibleDateTimeRangeMax_OBSOLETE.uiCapability()->setUiEditorTypeName( caf::PdmUiLineEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_visibleDateTimeRangeMin_OBSOLETE, "VisibleRangeMin", "Min", "", "", "" );
    m_visibleDateTimeRangeMin_OBSOLETE.uiCapability()->setUiEditorTypeName( caf::PdmUiLineEditor::uiEditorTypeName() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlotAxisPropertiesInterface::AxisTitlePositionType RimSummaryTimeAxisProperties::titlePosition() const
{
    return m_titlePositionEnum();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimSummaryTimeAxisProperties::titleFontSize() const
{
    return caf::FontTools::absolutePointSize( plotFontSize(), m_titleFontSize() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimSummaryTimeAxisProperties::valuesFontSize() const
{
    return caf::FontTools::absolutePointSize( plotFontSize(), m_valuesFontSize() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::FontTools::FontSize RimSummaryTimeAxisProperties::plotFontSize() const
{
    return RiaPreferences::current()->defaultPlotFontSize();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimSummaryTimeAxisProperties::visibleRangeMin() const
{
    if ( m_timeMode() == DATE )
    {
        return QwtDate::toDouble( visibleDateTimeMin() );
    }
    else
        return m_visibleTimeSinceStartRangeMin();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimSummaryTimeAxisProperties::visibleRangeMax() const
{
    if ( m_timeMode() == DATE )
    {
        return QwtDate::toDouble( visibleDateTimeMax() );
    }
    else
        return m_visibleTimeSinceStartRangeMax();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryTimeAxisProperties::setVisibleRangeMin( double value )
{
    if ( m_timeMode() == DATE )
    {
        QDateTime dateTime              = QwtDate::toDateTime( value );
        m_visibleTimeSinceStartRangeMin = fromDateToDisplayTime( dateTime );
        setVisibleDateTimeMin( dateTime );
    }
    else
    {
        m_visibleTimeSinceStartRangeMin = value;
        QDateTime dateTime              = fromDisplayTimeToDate( value );
        setVisibleDateTimeMin( dateTime );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryTimeAxisProperties::setVisibleRangeMax( double value )
{
    if ( m_timeMode() == DATE )
    {
        QDateTime dateTime              = QwtDate::toDateTime( value );
        m_visibleTimeSinceStartRangeMax = fromDateToDisplayTime( dateTime );
        setVisibleDateTimeMax( dateTime );
    }
    else
    {
        m_visibleTimeSinceStartRangeMax = value;
        QDateTime dateTime              = fromDisplayTimeToDate( value );
        setVisibleDateTimeMax( dateTime );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryTimeAxisProperties::isAutoZoom() const
{
    return m_isAutoZoom;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryTimeAxisProperties::setAutoZoom( bool enableAutoZoom )
{
    m_isAutoZoom = enableAutoZoom;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryTimeAxisProperties::updateTimeVisibleRange()
{
    m_visibleTimeSinceStartRangeMax = fromDateToDisplayTime( visibleDateTimeMax() );
    m_visibleTimeSinceStartRangeMin = fromDateToDisplayTime( visibleDateTimeMin() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryTimeAxisProperties::updateDateVisibleRange()
{
    setVisibleDateTimeMin( fromDisplayTimeToDate( m_visibleTimeSinceStartRangeMin() ) );
    setVisibleDateTimeMax( fromDisplayTimeToDate( m_visibleTimeSinceStartRangeMax() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QDateTime RimSummaryTimeAxisProperties::fromDisplayTimeToDate( double displayTime )
{
    RimSummaryPlot* rimSummaryPlot = nullptr;
    this->firstAncestorOrThisOfType( rimSummaryPlot );
    time_t startOfSimulation = rimSummaryPlot->firstTimeStepOfFirstCurve();

    time_t    secsSinceSimulationStart = displayTime / fromTimeTToDisplayUnitScale();
    QDateTime date;
    date.setTime_t( startOfSimulation + secsSinceSimulationStart );

    return date;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimSummaryTimeAxisProperties::fromDateToDisplayTime( const QDateTime& displayTime )
{
    time_t secsSinceEpoc = displayTime.toTime_t();

    RimSummaryPlot* rimSummaryPlot = nullptr;
    this->firstAncestorOrThisOfType( rimSummaryPlot );
    time_t startOfSimulation = rimSummaryPlot->firstTimeStepOfFirstCurve();

    return fromTimeTToDisplayUnitScale() * ( secsSinceEpoc - startOfSimulation );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryTimeAxisProperties::isActive() const
{
    return m_isActive;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QDateTime RimSummaryTimeAxisProperties::visibleDateTimeMin() const
{
    return RiaQDateTimeTools::createUtcDateTime( m_visibleDateRangeMin(), m_visibleTimeRangeMin() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QDateTime RimSummaryTimeAxisProperties::visibleDateTimeMax() const
{
    return RiaQDateTimeTools::createUtcDateTime( m_visibleDateRangeMax(), m_visibleTimeRangeMax() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryTimeAxisProperties::setVisibleDateTimeMin( const QDateTime& dateTime )
{
    m_visibleDateRangeMin = dateTime.date();
    m_visibleTimeRangeMin = dateTime.time();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryTimeAxisProperties::setVisibleDateTimeMax( const QDateTime& dateTime )
{
    m_visibleDateRangeMax = dateTime.date();
    m_visibleTimeRangeMax = dateTime.time();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryTimeAxisProperties::LegendTickmarkCount RimSummaryTimeAxisProperties::majorTickmarkCount() const
{
    return m_majorTickmarkCount();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryTimeAxisProperties::setMajorTickmarkCount( LegendTickmarkCount count )
{
    m_majorTickmarkCount = count;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RimSummaryTimeAxisProperties::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;
    *useOptionsOnly = true;

    if ( fieldNeedingOptions == &m_dateFormat )
    {
        for ( auto dateFormat : RiaQDateTimeTools::supportedDateFormats() )
        {
            QDate   exampleDate = QDate( 2019, 8, 16 );
            QString fullDateFormat =
                RiaQDateTimeTools::dateFormatString( dateFormat,
                                                     dateComponents( RiaQDateTimeTools::DATE_FORMAT_YEAR_MONTH_DAY ) );
            QString uiText = QString( "%1 (%2)" ).arg( fullDateFormat ).arg( exampleDate.toString( fullDateFormat ) );
            uiText.replace( "AP", "AM/PM" );
            options.push_back( caf::PdmOptionItemInfo( uiText, QVariant::fromValue( dateFormat ) ) );
        }
    }
    else if ( fieldNeedingOptions == &m_timeFormat )
    {
        for ( auto timeFormat : RiaQDateTimeTools::supportedTimeFormats() )
        {
            QTime   exampleTime = QTime( 15, 48, 22 );
            QString timeFormatString =
                RiaQDateTimeTools::timeFormatString( timeFormat,
                                                     timeComponents(
                                                         RiaQDateTimeTools::TimeFormatComponents::TIME_FORMAT_HOUR_MINUTE_SECOND ) );
            QString uiText = QString( "%1 (%2)" ).arg( timeFormatString ).arg( exampleTime.toString( timeFormatString ) );
            uiText.replace( "AP", "AM/PM" );
            options.push_back( caf::PdmOptionItemInfo( uiText, QVariant::fromValue( timeFormat ) ) );
        }
    }
    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimSummaryTimeAxisProperties::objectToggleField()
{
    return &m_isActive;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryTimeAxisProperties::TimeModeType RimSummaryTimeAxisProperties::timeMode() const
{
    return m_timeMode();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryTimeAxisProperties::setTimeMode( TimeModeType val )
{
    m_timeMode = val;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimSummaryTimeAxisProperties::fromTimeTToDisplayUnitScale()
{
    double scale = 1.0;
    switch ( m_timeUnit() )
    {
        case SECONDS:
            break;
        case MINUTES:
            scale = 1.0 / 60.0;
            break;
        case HOURS:
            scale = 1.0 / ( 60.0 * 60.0 );
            break;
        case DAYS:
            scale = 1.0 / ( 60.0 * 60.0 * 24.0 );
            break;
        case YEARS:
            scale = 1.0 / 31556952.0;
            break;
        default:
            CVF_ASSERT( false );
            break;
    }

    return scale;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimSummaryTimeAxisProperties::fromDaysToDisplayUnitScale()
{
    double scale = 1.0;
    switch ( m_timeUnit() )
    {
        case SECONDS:
            scale = 60.0 * 60.0 * 24.0;
            break;
        case MINUTES:
            scale = 60.0 * 24.0;
            break;
        case HOURS:
            scale = 24.0;
            break;
        case DAYS:
            break;
        case YEARS:
            scale = 1.0 / 365.2425;
            break;
        default:
            CVF_ASSERT( false );
            break;
    }

    return scale;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaQDateTimeTools::DateFormatComponents
    RimSummaryTimeAxisProperties::dateComponents( RiaQDateTimeTools::DateFormatComponents fallback ) const
{
    if ( m_automaticDateComponents() ) return fallback;

    RiaQDateTimeTools::DateFormatComponents components = m_dateComponents();

    return components;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaQDateTimeTools::TimeFormatComponents
    RimSummaryTimeAxisProperties::timeComponents( RiaQDateTimeTools::TimeFormatComponents fallback ) const
{
    if ( m_automaticDateComponents() ) return fallback;

    RiaQDateTimeTools::TimeFormatComponents components = m_timeComponents();

    return components;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPlotAxisAnnotation*> RimSummaryTimeAxisProperties::annotations() const
{
    return m_annotations.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryTimeAxisProperties::appendAnnotation( RimPlotAxisAnnotation* annotation )
{
    m_annotations.push_back( annotation );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString& RimSummaryTimeAxisProperties::dateFormat() const
{
    return m_dateFormat();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString& RimSummaryTimeAxisProperties::timeFormat() const
{
    return m_timeFormat();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryTimeAxisProperties::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiGroup& titleGroup = *( uiOrdering.addNewGroup( "Axis Title" ) );
    titleGroup.add( &showTitle );
    titleGroup.add( &title );
    title.uiCapability()->setUiReadOnly( !showTitle() );
    if ( showTitle() )
    {
        titleGroup.add( &m_titlePositionEnum );
        titleGroup.add( &m_titleFontSize );
    }

    caf::PdmUiGroup* timeGroup = uiOrdering.addNewGroup( "Time Values" );
    timeGroup->add( &m_timeMode );
    if ( m_timeMode() == DATE )
    {
        timeGroup->add( &m_visibleDateRangeMax, true );
        timeGroup->add( &m_visibleTimeRangeMax, false );
        timeGroup->add( &m_visibleDateRangeMin, true );
        timeGroup->add( &m_visibleTimeRangeMin, false );
        timeGroup->add( &m_automaticDateComponents );
        if ( !m_automaticDateComponents() )
        {
            timeGroup->add( &m_dateComponents );
            timeGroup->add( &m_timeComponents );
        }
    }
    else
    {
        timeGroup->add( &m_timeUnit );
        timeGroup->add( &m_visibleTimeSinceStartRangeMax );
        timeGroup->add( &m_visibleTimeSinceStartRangeMin );
    }
    timeGroup->add( &m_valuesFontSize );
    if ( m_timeMode() == DATE )
    {
        caf::PdmUiGroup* advancedGroup = timeGroup->addNewGroup( "Date/Time Label Format" );
        advancedGroup->setCollapsedByDefault( true );
        if ( m_automaticDateComponents() || m_dateComponents() != RiaQDateTimeTools::DATE_FORMAT_NONE )
        {
            advancedGroup->add( &m_dateFormat );
        }
        if ( m_automaticDateComponents() || m_timeComponents() != RiaQDateTimeTools::TimeFormatComponents::TIME_FORMAT_NONE )
        {
            advancedGroup->add( &m_timeFormat );
        }
    }

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryTimeAxisProperties::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                     const QVariant&            oldValue,
                                                     const QVariant&            newValue )
{
    RimSummaryPlot* rimSummaryPlot = nullptr;
    this->firstAncestorOrThisOfType( rimSummaryPlot );
    if ( !rimSummaryPlot ) return;

    if ( changedField == &m_visibleDateRangeMax )
    {
        QDate test = newValue.toDate();
        if ( !test.isValid() )
        {
            m_visibleDateRangeMax = oldValue.toDate();
        }

        updateTimeVisibleRange();
        m_isAutoZoom = false;
    }
    else if ( changedField == &m_visibleTimeRangeMax )
    {
        QTime test = newValue.toTime();
        if ( !test.isValid() )
        {
            m_visibleTimeRangeMax = oldValue.toTime();
        }

        updateTimeVisibleRange();
        m_isAutoZoom = false;
    }
    else if ( changedField == &m_visibleDateRangeMin )
    {
        QDate test = newValue.toDate();
        if ( !test.isValid() )
        {
            m_visibleDateRangeMin = oldValue.toDate();
        }

        updateTimeVisibleRange();
        m_isAutoZoom = false;
    }
    else if ( changedField == &m_visibleTimeRangeMin )
    {
        QTime test = newValue.toTime();
        if ( !test.isValid() )
        {
            m_visibleTimeRangeMin = oldValue.toTime();
        }

        updateTimeVisibleRange();
        m_isAutoZoom = false;
    }
    else if ( changedField == &m_visibleTimeSinceStartRangeMin || changedField == &m_visibleTimeSinceStartRangeMax )
    {
        updateDateVisibleRange();
        m_isAutoZoom = false;
    }
    else if ( changedField == &m_timeMode )
    {
        rimSummaryPlot->loadDataAndUpdate();
    }
    else if ( changedField == &m_timeUnit )
    {
        updateTimeVisibleRange(); // Use the stored max min dates to update the visible time range to new unit
        rimSummaryPlot->loadDataAndUpdate();
        updateDateVisibleRange();
    }
    else if ( changedField == &m_dateFormat || changedField == &m_timeFormat )
    {
        updateTimeVisibleRange(); // Use the stored max min dates to update the visible time range to new unit
        rimSummaryPlot->loadDataAndUpdate();
        updateDateVisibleRange();
    }

    rimSummaryPlot->updateAxes();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryTimeAxisProperties::initAfterRead()
{
    QDateTime maxDateTime = m_visibleDateTimeRangeMax_OBSOLETE();
    QDateTime minDateTime = m_visibleDateTimeRangeMin_OBSOLETE();
    if ( maxDateTime.isValid() )
    {
        m_visibleDateRangeMax = maxDateTime.date();
        m_visibleTimeRangeMax = maxDateTime.time();
    }
    if ( minDateTime.isValid() )
    {
        m_visibleDateRangeMin = minDateTime.date();
        m_visibleTimeRangeMin = minDateTime.time();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryTimeAxisProperties::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                          QString                    uiConfigName,
                                                          caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_visibleDateRangeMin || field == &m_visibleDateRangeMax )
    {
        auto dateAttrib = dynamic_cast<caf::PdmUiDateEditorAttribute*>( attribute );
        if ( dateAttrib )
        {
            dateAttrib->dateFormat =
                RiaQDateTimeTools::dateFormatString( m_dateFormat(), RiaQDateTimeTools::DATE_FORMAT_YEAR_MONTH_DAY );
        }
    }
    else if ( field == &m_visibleTimeRangeMin || field == &m_visibleTimeRangeMax )
    {
        auto timeAttrib = dynamic_cast<caf::PdmUiTimeEditorAttribute*>( attribute );
        if ( timeAttrib )
        {
            timeAttrib->timeFormat =
                RiaQDateTimeTools::timeFormatString( m_timeFormat(),
                                                     RiaQDateTimeTools::TimeFormatComponents::TIME_FORMAT_HOUR_MINUTE_SECOND );
        }
    }
}
