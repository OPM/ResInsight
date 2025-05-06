/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Equinor ASA
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

#include "RimPlotAxisAnnotation.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"

#include "cvfColor3.h"

#include <QString>

//==================================================================================================
///
///
//==================================================================================================
class RimTimeAxisAnnotation : public RimPlotAxisAnnotation
{
    CAF_PDM_HEADER_INIT;

public:
    RimTimeAxisAnnotation();

    static RimTimeAxisAnnotation*
        createTimeAnnotation( time_t time, const cvf::Color3f& color, const QString& dateTimeFormatString = QString() );

    static RimTimeAxisAnnotation* createTimeRangeAnnotation( time_t              startTime,
                                                             time_t              endTime,
                                                             const cvf::Color3f& color,
                                                             const QString&      dateTimeFormatString = QString() );

private:
    void setTime( time_t time, const QString& dateTimeFormatString = QString() );
    void setTimeRange( time_t startTime, time_t endTime, const QString& dateTimeFormatString = QString() );

    void setDefaultColor();

    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void initAfterRead() override;

    static QColor defaultColor( AnnotationType annotationType );
};
