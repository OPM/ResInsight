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

#include "RiaDateTimeDefines.h"

#include "RimPlotAxisPropertiesInterface.h"
#include "RimTimeAxisAnnotation.h"

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
class RimSummaryTimeAxisProperties : public RimPlotAxisPropertiesInterface
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

    typedef caf::AppEnum<RiaDefines::DateFormatComponents> DateFormatEnum;
    typedef caf::AppEnum<RiaDefines::TimeFormatComponents> TimeFormatEnum;

public:
    RimSummaryTimeAxisProperties();

    caf::PdmField<QString> title;
    caf::PdmField<bool>    showTitle;

    RiuPlotAxis           plotAxisType() const override;
    AxisTitlePositionType titlePosition() const override;
    int                   titleFontSize() const override;
    int                   valuesFontSize() const override;
    TimeModeType          timeMode() const;
    void                  setTimeMode( TimeModeType val );
    double                fromTimeTToDisplayUnitScale();
    double                fromDaysToDisplayUnitScale();

    RiaDefines::DateFormatComponents dateComponents(
        RiaDefines::DateFormatComponents fallback = RiaDefines::DateFormatComponents::DATE_FORMAT_UNSPECIFIED ) const;
    RiaDefines::TimeFormatComponents timeComponents(
        RiaDefines::TimeFormatComponents fallback = RiaDefines::TimeFormatComponents::TIME_FORMAT_UNSPECIFIED ) const;

    std::vector<RimPlotAxisAnnotation*> annotations() const override;
    void                                appendAnnotation( RimPlotAxisAnnotation* annotation ) override;
    void                                removeAllAnnotations() override;

    const QString& dateFormat() const;
    const QString& timeFormat() const;

    double visibleRangeMin() const override;
    double visibleRangeMax() const override;

    void setVisibleRangeMin( double value ) override;
    void setVisibleRangeMax( double value ) override;

    bool isAutoZoom() const override;
    void setAutoZoom( bool enableAutoZoom ) override;

    bool isActive() const override;

    QDateTime visibleDateTimeMin() const;
    QDateTime visibleDateTimeMax() const;

    void setVisibleDateTimeMin( const QDateTime& dateTime );
    void setVisibleDateTimeMax( const QDateTime& dateTime );

    LegendTickmarkCount majorTickmarkCount() const;
    void                setMajorTickmarkCount( LegendTickmarkCount count );

    const QString& name() const override;

protected:
    void                          fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;
    caf::PdmFieldHandle*          objectToggleField() override;
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
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
    caf::PdmChildArrayField<RimPlotAxisAnnotation*>    m_annotations;
};
