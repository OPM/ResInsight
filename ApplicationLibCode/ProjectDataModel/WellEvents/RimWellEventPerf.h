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
/// Perforation event - defines when a perforation interval opens or closes
///
//==================================================================================================
class RimWellEventPerf : public RimWellEvent
{
    CAF_PDM_HEADER_INIT;

public:
    enum class State
    {
        OPEN,
        SHUT
    };

    RimWellEventPerf();
    ~RimWellEventPerf() override;

    // Getters
    double startMD() const;
    double endMD() const;
    double diameter() const;
    double skinFactor() const;
    State  state() const;

    // Setters
    void setStartMD( double md );
    void setEndMD( double md );
    void setDiameter( double diameter );
    void setSkinFactor( double skinFactor );
    void setState( State state );

    // Override from RimWellEvent
    EventType eventType() const override;
    QString   generateScheduleKeyword( const QString& wellName ) const override;

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;

private:
    caf::PdmField<double>              m_startMD;
    caf::PdmField<double>              m_endMD;
    caf::PdmField<double>              m_diameter;
    caf::PdmField<double>              m_skinFactor;
    caf::PdmField<caf::AppEnum<State>> m_state;
};

namespace caf
{
template <>
void caf::AppEnum<RimWellEventPerf::State>::setUp();
} // namespace caf
