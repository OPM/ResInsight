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

#include "cafAppEnum.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

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

    void   setTime( time_t time );
    void   setTimeRange( time_t startTime, time_t endTime );
    QColor color() const override;

protected:
    virtual void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

private:
    caf::PdmField<time_t> m_time;
    caf::PdmField<time_t> m_startTime;
    caf::PdmField<time_t> m_endTime;
};
