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
#include "RiaFieldHandleTools.h"
#include "RiaFontCache.h"
#include "RiaPreferences.h"
#include "RiaQDateTimeTools.h"

#include "RimSummaryMultiPlot.h"
#include "RimSummaryPlot.h"

#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiDateEditor.h"
#include "cafPdmUiLineEditor.h"
#include "cafPdmUiTimeEditor.h"
#include "cafPdmUiTreeAttributes.h"

#include "cvfAssert.h"

#include "qwt_date.h"

namespace caf
{
template <>
void caf::AppEnum<RimSummaryTimeAxisProperties::TimeModeType>::setUp()
{
    addItem( RimSummaryTimeAxisProperties::DATE, "DATE", "Date" );
    addItem( RimSummaryTimeAxisProperties::TIME_FROM_SIMULATION_START, "TIME_FROM_SIMULATION_START", "Time From Simulation Start" );

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

template <>
void RimSummaryTimeAxisProperties::TickmarkTypeEnum::setUp()
{
    addItem( RimSummaryTimeAxisProperties::TickmarkType::TICKMARK_COUNT, "COUNT", "Count" );
    addItem( RimSummaryTimeAxisProperties::TickmarkType::TICKMARK_CUSTOM, "CUSTOM", "Custom" );
    setDefault( RimSummaryTimeAxisProperties::TickmarkType::TICKMARK_COUNT );
}

template <>
void RimSummaryTimeAxisProperties::TickmarkIntervalEnum::setUp()
{
    addItem( RimSummaryTimeAxisProperties::TickmarkInterval::MINUTES, "MINUTES", "Minutes" );
    addItem( RimSummaryTimeAxisProperties::TickmarkInterval::HOURS, "HOURS", "Hours" );
    addItem( RimSummaryTimeAxisProperties::TickmarkInterval::DAYS, "DAYS", "Days" );
    addItem( RimSummaryTimeAxisProperties::TickmarkInterval::MONTHS, "MONTHS", "Months" );
    addItem( RimSummaryTimeAxisProperties::TickmarkInterval::YEARS, "YEARS", "Years" );
    setDefault( RimSummaryTimeAxisProperties::TickmarkInterval::YEARS );
}

} // namespace caf

CAF_PDM_SOURCE_INIT( RimSummaryTimeAxisProperties, "SummaryTimeAxisProperties" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryTimeAxisProperties::RimSummaryTimeAxisProperties()
    : requestLoadDataAndUpdate( this )
{
    CAF_PDM_InitObject( "Time Axis", ":/BottomAxis16x16.png" );

    CAF_PDM_InitField( &m_isActive, "Active", true, "Active" );
    m_isActive.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &showTitle, "ShowTitle", false, "Show Title    " );
    CAF_PDM_InitField( &title, "Title", QString( "Time" ), "Title          " );

    CAF_PDM_InitField( &m_isAutoZoom, "AutoZoom", true, "Set Range Automatically" );
    CAF_PDM_InitFieldNoDefault( &m_timeMode, "TimeMode", "Time Mode" );
    CAF_PDM_InitFieldNoDefault( &m_timeUnit, "TimeUnit", "Time Unit" );

    CAF_PDM_InitFieldNoDefault( &m_visibleDateRangeMax, "VisibleDateRangeMax", "Max Date" );
    m_visibleDateRangeMax.uiCapability()->setUiEditorTypeName( caf::PdmUiDateEditor::uiEditorTypeName() );
    CAF_PDM_InitFieldNoDefault( &m_visibleDateRangeMin, "VisibleDateRangeMin", "Min Date" );
    m_visibleDateRangeMin.uiCapability()->setUiEditorTypeName( caf::PdmUiDateEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_visibleTimeRangeMax, "VisibleTimeRangeMax", "MaxTime" );
    m_visibleTimeRangeMax.uiCapability()->setUiEditorTypeName( caf::PdmUiTimeEditor::uiEditorTypeName() );
    m_visibleTimeRangeMax.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitFieldNoDefault( &m_visibleTimeRangeMin, "VisibleTimeRangeMin", "Min Time" );
    m_visibleTimeRangeMin.uiCapability()->setUiEditorTypeName( caf::PdmUiTimeEditor::uiEditorTypeName() );
    m_visibleTimeRangeMin.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitFieldNoDefault( &m_visibleTimeSinceStartRangeMax, "VisibleTimeModeRangeMax", "Max" );
    m_visibleTimeSinceStartRangeMax.uiCapability()->setUiEditorTypeName( caf::PdmUiLineEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_visibleTimeSinceStartRangeMin, "VisibleTimeModeRangeMin", "Min" );
    m_visibleTimeSinceStartRangeMin.uiCapability()->setUiEditorTypeName( caf::PdmUiLineEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_titlePositionEnum, "TitlePosition", "Title Position" );
    CAF_PDM_InitFieldNoDefault( &m_titleFontSize, "FontSize", "Font Size" );
    CAF_PDM_InitFieldNoDefault( &m_valuesFontSize, "ValuesFontSize", "Font Size" );

    CAF_PDM_InitField( &m_automaticDateComponents, "AutoDate", true, "Automatic Date/Time Labels" );
    CAF_PDM_InitFieldNoDefault( &m_dateComponents, "DateComponents", "Set Date Label" );
    CAF_PDM_InitFieldNoDefault( &m_timeComponents, "TimeComponents", "Set Time Label" );

    CAF_PDM_InitFieldNoDefault( &m_dateFormat, "DateFormat", "Date Label Format" );
    m_dateFormat.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );
    m_dateFormat = RiaPreferences::current()->dateFormat();

    CAF_PDM_InitFieldNoDefault( &m_timeFormat, "TimeFormat", "Time Label Format" );
    m_timeFormat.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );
    m_timeFormat = RiaPreferences::current()->timeFormat();

    CAF_PDM_InitFieldNoDefault( &m_tickmarkType, "TickmarkType", "Tickmark Type" );
    m_tickmarkType.uiCapability()->enableAutoValueSupport( true );

    CAF_PDM_InitFieldNoDefault( &m_tickmarkInterval, "TickmarkInterval", "Tickmark Interval" );
    m_tickmarkInterval.uiCapability()->enableAutoValueSupport( true );

    CAF_PDM_InitField( &m_tickmarkIntervalStep, "TickmarkIntervalStep", 1, "Tickmark Interval Step" );

    CAF_PDM_InitFieldNoDefault( &m_majorTickmarkCount, "MajorTickmarkCount", "Major Tickmark Count" );
    m_majorTickmarkCount.uiCapability()->enableAutoValueSupport( true );

    CAF_PDM_InitFieldNoDefault( &m_annotations, "Annotations", "" );
    m_annotations.uiCapability()->setUiTreeChildrenHidden( true );
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
RiuPlotAxis RimSummaryTimeAxisProperties::plotAxis() const
{
    return RiuPlotAxis::defaultBottom();
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
void RimSummaryTimeAxisProperties::defineObjectEditorAttribute( QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    auto summaryMultiPlot = firstAncestorOfType<RimSummaryMultiPlot>();
    if ( summaryMultiPlot && summaryMultiPlot->isTimeAxisLinked() )
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
    RimSummaryPlot* rimSummaryPlot    = firstAncestorOrThisOfType<RimSummaryPlot>();
    time_t          startOfSimulation = rimSummaryPlot->firstTimeStepOfFirstCurve();

    time_t    secsSinceSimulationStart = displayTime / fromTimeTToDisplayUnitScale();
    QDateTime date                     = RiaQDateTimeTools::fromTime_t( startOfSimulation + secsSinceSimulationStart );

    return date;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimSummaryTimeAxisProperties::fromDateToDisplayTime( const QDateTime& displayTime )
{
    time_t secsSinceEpoc = displayTime.toSecsSinceEpoch();

    RimSummaryPlot* rimSummaryPlot    = firstAncestorOrThisOfType<RimSummaryPlot>();
    time_t          startOfSimulation = rimSummaryPlot->firstTimeStepOfFirstCurve();

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
void RimSummaryTimeAxisProperties::setTickmarkInterval( TickmarkInterval interval )
{
    m_tickmarkInterval = interval;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryTimeAxisProperties::setTickmarkIntervalStep( int step )
{
    m_tickmarkIntervalStep = step;
}

//--------------------------------------------------------------------------------------------------
/// Create evenly distributed tickmarks between min and max with rounded values dependent on
/// the currently active tickmark step.
//--------------------------------------------------------------------------------------------------
QList<double> RimSummaryTimeAxisProperties::createTickmarkList( const QDateTime& minDateTime, const QDateTime& maxDateTime ) const
{
    const auto& [tickmarkInterval, tickmarkStep] = tickmarkIntervalAndStep();

    // Ensure no infinite loop
    if ( tickmarkStep < 1 ) return {};

    // Convert from list of QDateTime items to double values for Qwt
    auto toDoubleList = []( const QList<QDateTime>& dateTimeList )
    {
        QList<double> output;
        for ( const auto& elm : dateTimeList )
        {
            output.push_back( QwtDate::toDouble( elm ) );
        }
        return output;
    };

    QList<QDateTime> dateTimeList;
    if ( tickmarkInterval == RimSummaryTimeAxisProperties::TickmarkInterval::YEARS )
    {
        dateTimeList = { QDateTime( QDate( minDateTime.date().year(), 1, 1 ), QTime( 0, 0 ), Qt::UTC ) };
        while ( dateTimeList.back() < maxDateTime )
        {
            const auto nextYear = dateTimeList.back().addYears( tickmarkStep );
            dateTimeList.push_back( nextYear );
        }
    }
    else if ( tickmarkInterval == RimSummaryTimeAxisProperties::TickmarkInterval::MONTHS )
    {
        dateTimeList = { QDateTime( QDate( minDateTime.date().year(), 1, 1 ), QTime( 0, 0 ), Qt::UTC ) };
        while ( dateTimeList.back() < maxDateTime )
        {
            const auto nextMonth = dateTimeList.back().addMonths( tickmarkStep );
            dateTimeList.push_back( nextMonth );
        }
    }
    else if ( tickmarkInterval == RimSummaryTimeAxisProperties::TickmarkInterval::DAYS )
    {
        dateTimeList = {
            QDateTime( QDate( minDateTime.date().year(), minDateTime.date().month(), minDateTime.date().day() ), QTime( 0, 0 ), Qt::UTC ) };
        while ( dateTimeList.back() < maxDateTime )
        {
            const auto nextDay = dateTimeList.back().addDays( tickmarkStep );
            dateTimeList.push_back( nextDay );
        }
    }
    else if ( tickmarkInterval == RimSummaryTimeAxisProperties::TickmarkInterval::HOURS )
    {
        dateTimeList = { QDateTime( QDate( minDateTime.date().year(), minDateTime.date().month(), minDateTime.date().day() ),
                                    QTime( minDateTime.time().hour(), 0 ),
                                    Qt::UTC ) };
        while ( dateTimeList.back() < maxDateTime )
        {
            const auto nextHour = dateTimeList.back().addSecs( static_cast<qint64>( tickmarkStep * 60 * 60 ) );
            dateTimeList.push_back( nextHour );
        }
    }
    else if ( tickmarkInterval == RimSummaryTimeAxisProperties::TickmarkInterval::MINUTES )
    {
        dateTimeList = { QDateTime( QDate( minDateTime.date().year(), minDateTime.date().month(), minDateTime.date().day() ),
                                    QTime( minDateTime.time().hour(), minDateTime.time().minute() ),
                                    Qt::UTC ) };
        while ( dateTimeList.back() < maxDateTime )
        {
            const auto nextMinute = dateTimeList.back().addSecs( static_cast<qint64>( tickmarkStep * 60 ) );
            dateTimeList.push_back( nextMinute );
        }
    }
    else
    {
        CAF_ASSERT( "Tickmark interval type not handled!" );
    }

    return toDoubleList( dateTimeList );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimSummaryTimeAxisProperties::getTickmarkIntervalDouble()
{
    const auto& [tickmarkInterval, tickmarkStep] = tickmarkIntervalAndStep();

    QDateTime initialInterval = QDateTime::fromMSecsSinceEpoch( 0 );
    if ( tickmarkInterval == RimSummaryTimeAxisProperties::TickmarkInterval::YEARS )
    {
        auto interval = initialInterval.addYears( tickmarkStep );
        return QwtDate::toDouble( interval );
    }
    if ( tickmarkInterval == RimSummaryTimeAxisProperties::TickmarkInterval::MONTHS )
    {
        auto interval = initialInterval.addMonths( tickmarkStep );
        return QwtDate::toDouble( interval );
    }
    if ( tickmarkInterval == RimSummaryTimeAxisProperties::TickmarkInterval::DAYS )
    {
        auto interval = initialInterval.addDays( static_cast<qint64>( tickmarkStep ) );
        return QwtDate::toDouble( interval );
    }
    if ( tickmarkInterval == RimSummaryTimeAxisProperties::TickmarkInterval::HOURS )
    {
        const qint64 secs     = static_cast<qint64>( tickmarkStep ) * 60 * 60;
        auto         interval = initialInterval.addSecs( secs );
        return QwtDate::toDouble( interval );
    }
    if ( tickmarkInterval == RimSummaryTimeAxisProperties::TickmarkInterval::MINUTES )
    {
        const qint64 secs     = static_cast<qint64>( tickmarkStep ) * 60;
        auto         interval = initialInterval.addSecs( secs );
        return QwtDate::toDouble( interval );
    }

    CAF_ASSERT( "Tickmark interval type not handled!" );
    return 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryTimeAxisProperties::TickmarkType RimSummaryTimeAxisProperties::tickmarkType() const
{
    return m_tickmarkType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<RimSummaryTimeAxisProperties::TickmarkInterval, int> RimSummaryTimeAxisProperties::tickmarkIntervalAndStep() const
{
    return std::pair<TickmarkInterval, int>( m_tickmarkInterval(), m_tickmarkIntervalStep() );
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
void RimSummaryTimeAxisProperties::setAutoValueForMajorTickmarkCount( LegendTickmarkCount count, bool notifyFieldChanged )
{
    auto enumValue = static_cast<std::underlying_type_t<LegendTickmarkCount>>( count );

    m_majorTickmarkCount.uiCapability()->setAutoValue( enumValue, notifyFieldChanged );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryTimeAxisProperties::enableAutoValueForMajorTickmarkCount( bool enable )
{
    m_majorTickmarkCount.uiCapability()->enableAutoValue( enable );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString RimSummaryTimeAxisProperties::objectName() const
{
    return title();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString RimSummaryTimeAxisProperties::axisTitleText() const
{
    return title();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimSummaryTimeAxisProperties::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_dateFormat )
    {
        for ( auto dateFormat : RiaQDateTimeTools::supportedDateFormats() )
        {
            QDate   exampleDate = QDate( 2019, 8, 16 );
            QString fullDateFormat =
                RiaQDateTimeTools::dateFormatString( dateFormat,
                                                     dateComponents( RiaDefines::DateFormatComponents::DATE_FORMAT_YEAR_MONTH_DAY ) );
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
                                                     timeComponents( RiaDefines::TimeFormatComponents::TIME_FORMAT_HOUR_MINUTE_SECOND ) );
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
/// https://www.unitconverters.net/time-converter.html
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
        case MONTHS:
            scale = 3.805175038E-7;
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
/// https://www.unitconverters.net/time-converter.html
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
        case MONTHS:
            scale = 1.0 / 30.416666667;
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
RiaDefines::DateFormatComponents RimSummaryTimeAxisProperties::dateComponents( RiaDefines::DateFormatComponents fallback ) const
{
    if ( m_automaticDateComponents() ) return fallback;

    RiaDefines::DateFormatComponents components = m_dateComponents();

    return components;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::TimeFormatComponents RimSummaryTimeAxisProperties::timeComponents( RiaDefines::TimeFormatComponents fallback ) const
{
    if ( m_automaticDateComponents() ) return fallback;

    RiaDefines::TimeFormatComponents components = m_timeComponents();

    return components;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPlotAxisAnnotation*> RimSummaryTimeAxisProperties::annotations() const
{
    return m_annotations.childrenByType();
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
void RimSummaryTimeAxisProperties::removeAllAnnotations()
{
    m_annotations.deleteChildren();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryTimeAxisProperties::removeAnnotation( RimTimeAxisAnnotation* annotation )
{
    m_annotations.removeChild( annotation );
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
        timeGroup->add( &m_visibleDateRangeMax );
        timeGroup->appendToRow( &m_visibleTimeRangeMax );
        timeGroup->add( &m_visibleDateRangeMin );
        timeGroup->appendToRow( &m_visibleTimeRangeMin );
    }
    else
    {
        timeGroup->add( &m_timeUnit );
        timeGroup->add( &m_visibleTimeSinceStartRangeMax );
        timeGroup->add( &m_visibleTimeSinceStartRangeMin );
    }

    caf::PdmUiGroup* tickmarkDistributionGroup = timeGroup->addNewGroup( "Tickmark Distribution" );
    tickmarkDistributionGroup->add( &m_tickmarkType );
    m_tickmarkType.uiCapability()->setUiReadOnly( m_timeMode() == TIME_FROM_SIMULATION_START );
    if ( m_tickmarkType() == TickmarkType::TICKMARK_COUNT )
    {
        tickmarkDistributionGroup->add( &m_majorTickmarkCount );
    }
    if ( m_tickmarkType() == TickmarkType::TICKMARK_CUSTOM && m_timeMode() == DATE )
    {
        tickmarkDistributionGroup->add( &m_tickmarkInterval );
        tickmarkDistributionGroup->add( &m_tickmarkIntervalStep );
    }

    caf::PdmUiGroup* tickmarkLabelGroup = timeGroup->addNewGroup( "Tickmark Label Format" );
    tickmarkLabelGroup->setCollapsedByDefault();
    tickmarkLabelGroup->add( &m_valuesFontSize );
    if ( m_timeMode() == DATE )
    {
        tickmarkLabelGroup->add( &m_automaticDateComponents );
        if ( !m_automaticDateComponents() )
        {
            tickmarkLabelGroup->add( &m_dateComponents );
            tickmarkLabelGroup->add( &m_timeComponents );
        }
        if ( m_automaticDateComponents() || m_dateComponents() != RiaDefines::DateFormatComponents::DATE_FORMAT_NONE )
        {
            tickmarkLabelGroup->add( &m_dateFormat );
        }
        if ( m_automaticDateComponents() || m_timeComponents() != RiaDefines::TimeFormatComponents::TIME_FORMAT_NONE )
        {
            tickmarkLabelGroup->add( &m_timeFormat );
        }
    }

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryTimeAxisProperties::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimSummaryPlot* rimSummaryPlot = firstAncestorOrThisOfType<RimSummaryPlot>();
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
        if ( m_timeMode() == TimeModeType::TIME_FROM_SIMULATION_START )
        {
            m_tickmarkType = TickmarkType::TICKMARK_COUNT;
        }
        // Changing this setting requires a full update of the plot
        requestLoadDataAndUpdate.send();
        return;
    }
    else if ( changedField == &m_timeUnit || changedField == &m_dateFormat || changedField == &m_timeFormat )
    {
        // Changing these settings requires a full update of the plot
        requestLoadDataAndUpdate.send();
        return;
    }
    else if ( changedField == &m_tickmarkType && m_tickmarkType == TickmarkType::TICKMARK_CUSTOM )
    {
        m_isAutoZoom = false;
    }

    settingsChanged.send();
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
                RiaQDateTimeTools::dateFormatString( m_dateFormat(), RiaDefines::DateFormatComponents::DATE_FORMAT_YEAR_MONTH_DAY );
        }
    }
    else if ( field == &m_visibleTimeRangeMin || field == &m_visibleTimeRangeMax )
    {
        auto timeAttrib = dynamic_cast<caf::PdmUiTimeEditorAttribute*>( attribute );
        if ( timeAttrib )
        {
            timeAttrib->timeFormat =
                RiaQDateTimeTools::timeFormatString( m_timeFormat(), RiaDefines::TimeFormatComponents::TIME_FORMAT_HOUR_MINUTE_SECOND );
        }
    }
}
