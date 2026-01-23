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

#include "cafPdmField.h"

//==================================================================================================
///
/// Tubing event - defines changes to tubing/liner diameter and roughness
///
//==================================================================================================
class RimWellEventTubing : public RimWellEvent
{
    CAF_PDM_HEADER_INIT;

public:
    RimWellEventTubing();
    ~RimWellEventTubing() override;

    // Getters
    double startMD() const;
    double endMD() const;
    double innerDiameter() const;
    double roughness() const;

    // Setters
    void setStartMD( double md );
    void setEndMD( double md );
    void setInnerDiameter( double diameter );
    void setRoughness( double roughness );

    // Override from RimWellEvent
    EventType eventType() const override;
    QString   generateScheduleKeyword( const QString& wellName ) const override;

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;

private:
    caf::PdmField<double> m_startMD;
    caf::PdmField<double> m_endMD;
    caf::PdmField<double> m_innerDiameter;
    caf::PdmField<double> m_roughness;
};
