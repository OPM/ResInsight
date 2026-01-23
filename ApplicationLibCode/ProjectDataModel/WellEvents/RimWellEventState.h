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

#include "RimWellEvent.h"

#include "cafAppEnum.h"
#include "cafPdmField.h"

//==================================================================================================
///
/// Well state event - defines when a well changes state (OPEN/SHUT/STOP)
///
//==================================================================================================
class RimWellEventState : public RimWellEvent
{
    CAF_PDM_HEADER_INIT;

public:
    enum class WellState
    {
        OPEN,
        SHUT,
        STOP
    };

    RimWellEventState();
    ~RimWellEventState() override;

    // Getters
    WellState wellState() const;

    // Setters
    void setWellState( WellState state );

    // Override from RimWellEvent
    EventType eventType() const override;
    QString   generateScheduleKeyword( const QString& wellName ) const override;

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;

private:
    caf::PdmField<caf::AppEnum<WellState>> m_wellState;
};

namespace caf
{
template <>
void caf::AppEnum<RimWellEventState::WellState>::setUp();
} // namespace caf
