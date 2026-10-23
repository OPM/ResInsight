/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
//  A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "RimWellEvent.h"

//==================================================================================================
///
/// A date that must appear in the generated schedule, even when no other event falls on it.
///
/// The event itself produces no keyword: it only makes sure a DATES keyword (and an optional
/// comment) is emitted for its timestamp, e.g. to force a summary report at that date.
///
//==================================================================================================
class RimWellEventInsertDate : public RimWellEvent
{
    CAF_PDM_HEADER_INIT;

public:
    RimWellEventInsertDate();
    ~RimWellEventInsertDate() override;

    EventType eventType() const override;
    QString   generateScheduleKeyword( const QString& wellName ) const override;

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;
};
