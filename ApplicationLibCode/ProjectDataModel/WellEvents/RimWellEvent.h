/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025-     Equinor ASA
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

#include "cafAppEnum.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

#include <QDateTime>
#include <QString>

class RimWellPath;

//==================================================================================================
///
/// Base class for well events in a timeline-based event system
///
//==================================================================================================
class RimWellEvent : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum class EventType
    {
        PERF,
        VALVE,
        TUBING,
        WSTATE,
        WTYPE,
        WCONTROL,
        KEYWORD,
        SCHEDULE_KEYWORD
    };

    RimWellEvent();
    ~RimWellEvent() override;

    QDateTime eventDate() const;
    void      setEventDate( const QDateTime& date );

    RimWellPath* wellPath() const;
    void         setWellPath( RimWellPath* wellPath );
    QString      wellName() const; // Convenience method to get well name

    virtual EventType eventType() const                                        = 0;
    virtual QString   generateScheduleKeyword( const QString& wellName ) const = 0;

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

protected:
    caf::PdmField<QDateTime>       m_eventDate;
    caf::PdmPtrField<RimWellPath*> m_wellPath;
};
