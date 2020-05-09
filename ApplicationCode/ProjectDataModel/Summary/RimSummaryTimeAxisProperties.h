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

#pragma once

#include "RiaQDateTimeTools.h"
#include "RimPlotAxisPropertiesInterface.h"

#include "cafAppEnum.h"
#include "cafFontTools.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

#include <QDateTime>
#include <QString>

//==================================================================================================
///
///
//==================================================================================================
class RimSummaryTimeAxisProperties : public caf::PdmObject, public RimPlotAxisPropertiesInterface
{
    CAF_PDM_HEADER_INIT;

public:
    enum TimeModeType
    {
        DATE,
        TIME_FROM_SIMULATION_START
    };

    enum TimeUnitType
    {
        SECONDS,
        MINUTES,
        HOURS,
        DAYS,
        MONTHS,
        YEARS
    };

    typedef caf::AppEnum<RiaQDateTimeTools::DateFormatComponents> DateFormatEnum;
    typedef caf::AppEnum<RiaQDateTimeTools::TimeFormatComponents> TimeFormatEnum;

public:
    RimSummaryTimeAxisProperties();

    caf::PdmField<QString> title;
    caf::PdmField<bool>    showTitle;

    AxisTitlePositionType titlePosition() const override;
    int                   titleFontSize() const override;
    int                   valuesFontSize() const override;
    TimeModeType          timeMode() const;
    void                  setTimeMode( TimeModeType val );
    double                fromTimeTToDisplayUnitScale();
    double                fromDaysToDisplayUnitScale();

    RiaQDateTimeTools::DateFormatComponents
        dateComponents( RiaQDateTimeTools::DateFormatComponents fallback = RiaQDateTimeTools::DATE_FORMAT_UNSPECIFIED ) const;
    RiaQDateTimeTools::TimeFormatComponents
        timeComponents( RiaQDateTimeTools::TimeFormatComponents fallback =
                            RiaQDateTimeTools::TimeFormatComponents::TIME_FORMAT_UNSPECIFIED ) const;

    const QString& dateFormat() const;
    const QString& timeFormat() const;

    double visibleRangeMin() const;
    double visibleRangeMax() const;

    void setVisibleRangeMin( double value );
    void setVisibleRangeMax( double value );

    bool isAutoZoom() const;
    void setAutoZoom( bool enableAutoZoom );

    bool isActive() const;

    QDateTime visibleDateTimeMin() const;
    QDateTime visibleDateTimeMax() const;

    void setVisibleDateTimeMin( const QDateTime& dateTime );
    void setVisibleDateTimeMax( const QDateTime& dateTime );

    LegendTickmarkCount majorTickmarkCount() const;
    void                setMajorTickmarkCount( LegendTickmarkCount count );

protected:
    void                          fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;
    caf::PdmFieldHandle*          objectToggleField() override;
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void                          initAfterRead() override;
    void                          defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                         QString                    uiConfigName,
                                                         caf::PdmUiEditorAttribute* attribute ) override;

    double                   fromDateToDisplayTime( const QDateTime& displayTime );
    QDateTime                fromDisplayTimeToDate( double displayTime );
    void                     updateTimeVisibleRange();
    void                     updateDateVisibleRange();
    caf::FontTools::FontSize plotFontSize() const;

private:
    caf::PdmField<caf::AppEnum<TimeModeType>> m_timeMode;
    caf::PdmField<caf::AppEnum<TimeUnitType>> m_timeUnit;

    caf::PdmField<bool>  m_isActive;
    caf::PdmField<QDate> m_visibleDateRangeMin;
    caf::PdmField<QDate> m_visibleDateRangeMax;
    caf::PdmField<QTime> m_visibleTimeRangeMin;
    caf::PdmField<QTime> m_visibleTimeRangeMax;

    caf::PdmField<double> m_visibleTimeSinceStartRangeMin;
    caf::PdmField<double> m_visibleTimeSinceStartRangeMax;
    caf::PdmField<bool>   m_isAutoZoom;

    caf::PdmField<caf::FontTools::RelativeSizeEnum> m_titleFontSize;
    caf::PdmField<caf::FontTools::RelativeSizeEnum> m_valuesFontSize;

    caf::PdmField<caf::AppEnum<AxisTitlePositionType>> m_titlePositionEnum;
    caf::PdmField<bool>                                m_automaticDateComponents;
    caf::PdmField<DateFormatEnum>                      m_dateComponents;
    caf::PdmField<TimeFormatEnum>                      m_timeComponents;
    caf::PdmField<QString>                             m_dateFormat;
    caf::PdmField<QString>                             m_timeFormat;
    caf::PdmField<LegendTickmarkCountEnum>             m_majorTickmarkCount;

    caf::PdmField<QDateTime> m_visibleDateTimeRangeMin_OBSOLETE;
    caf::PdmField<QDateTime> m_visibleDateTimeRangeMax_OBSOLETE;
};
