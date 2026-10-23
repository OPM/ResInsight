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

#include <optional>
#include <utility>

//==================================================================================================
///
/// Segment event - defines changes to segment (tubing/liner) diameter and roughness
///
//==================================================================================================
class RimWellEventSegment : public RimWellEvent
{
    CAF_PDM_HEADER_INIT;

public:
    RimWellEventSegment();
    ~RimWellEventSegment() override;

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

    // Segmentation. Fixed length cannot be combined with min/max length.
    std::optional<double> fixedSegmentLength() const;
    std::optional<double> minSegmentLength() const;
    std::optional<double> maxSegmentLength() const;
    void                  setFixedSegmentLength( std::optional<double> length );
    void                  setMinSegmentLength( std::optional<double> length );
    void                  setMaxSegmentLength( std::optional<double> length );

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

    caf::PdmField<std::pair<bool, double>> m_fixedSegmentLength;
    caf::PdmField<std::pair<bool, double>> m_minSegmentLength;
    caf::PdmField<std::pair<bool, double>> m_maxSegmentLength;
};
