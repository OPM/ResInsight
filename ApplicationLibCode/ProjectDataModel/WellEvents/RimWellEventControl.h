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
/// Well control event - defines production/injection control changes (WCONPROD/WCONINJE)
///
//==================================================================================================
class RimWellEventControl : public RimWellEvent
{
    CAF_PDM_HEADER_INIT;

public:
    enum class ControlMode
    {
        ORAT, // Oil rate control
        WRAT, // Water rate control
        GRAT, // Gas rate control
        LRAT, // Liquid rate control
        RESV, // Reservoir volume rate control
        BHP, // Bottom hole pressure control
        THP // Tubing head pressure control
    };

    enum class WellStatus
    {
        OPEN,
        SHUT,
        STOP
    };

    RimWellEventControl();
    ~RimWellEventControl() override;

    // Getters
    ControlMode controlMode() const;
    WellStatus  wellStatus() const;
    double      controlValue() const;
    double      bhpLimit() const;
    double      thpLimit() const;
    double      oilRate() const;
    double      waterRate() const;
    double      gasRate() const;
    double      liquidRate() const;
    bool        isProducer() const;

    // Setters
    void setControlMode( ControlMode mode );
    void setWellStatus( WellStatus status );
    void setControlValue( double value );
    void setBhpLimit( double limit );
    void setThpLimit( double limit );
    void setOilRate( double rate );
    void setWaterRate( double rate );
    void setGasRate( double rate );
    void setLiquidRate( double rate );
    void setIsProducer( bool isProducer );

    // Override from RimWellEvent
    EventType eventType() const override;
    QString   generateScheduleKeyword( const QString& wellName ) const override;

    // Generate specific keywords
    QString generateWCONPROD( const QString& wellName ) const;
    QString generateWCONINJE( const QString& wellName, const QString& injectorType ) const;

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;

private:
    caf::PdmField<caf::AppEnum<ControlMode>> m_controlMode;
    caf::PdmField<caf::AppEnum<WellStatus>>  m_wellStatus;
    caf::PdmField<double>                    m_controlValue;
    caf::PdmField<double>                    m_bhpLimit;
    caf::PdmField<double>                    m_thpLimit;
    caf::PdmField<double>                    m_oilRate;
    caf::PdmField<double>                    m_waterRate;
    caf::PdmField<double>                    m_gasRate;
    caf::PdmField<double>                    m_liquidRate;
    caf::PdmField<bool>                      m_isProducer;
};

namespace caf
{
template <>
void caf::AppEnum<RimWellEventControl::ControlMode>::setUp();

template <>
void caf::AppEnum<RimWellEventControl::WellStatus>::setUp();
} // namespace caf
