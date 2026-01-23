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
/// Well type event - defines when a well changes type (producer/injector)
///
//==================================================================================================
class RimWellEventType : public RimWellEvent
{
    CAF_PDM_HEADER_INIT;

public:
    enum class WellType
    {
        OIL_PRODUCER,
        GAS_PRODUCER,
        WATER_PRODUCER,
        WATER_INJECTOR,
        GAS_INJECTOR
    };

    RimWellEventType();
    ~RimWellEventType() override;

    // Getters
    WellType wellType() const;

    // Setters
    void setWellType( WellType type );

    // Helper methods
    bool isProducer() const;
    bool isInjector() const;

    // Override from RimWellEvent
    EventType eventType() const override;
    QString   generateScheduleKeyword( const QString& wellName ) const override;

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;

private:
    caf::PdmField<caf::AppEnum<WellType>> m_wellType;
};

namespace caf
{
template <>
void caf::AppEnum<RimWellEventType::WellType>::setUp();
} // namespace caf
