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
#include "cafPdmPtrField.h"

class RimValveTemplate;

//==================================================================================================
///
/// Valve event - defines when a valve opens or closes
///
//==================================================================================================
class RimWellEventValve : public RimWellEvent
{
    CAF_PDM_HEADER_INIT;

public:
    enum class State
    {
        OPEN,
        SHUT
    };

    enum class ValveType
    {
        ICV,
        ICD,
        AICD
    };

    RimWellEventValve();
    ~RimWellEventValve() override;

    // Getters
    double            measuredDepth() const;
    ValveType         valveType() const;
    State             state() const;
    double            flowCoefficient() const;
    double            area() const;
    RimValveTemplate* valveTemplate() const;

    // Setters
    void setMeasuredDepth( double md );
    void setValveType( ValveType type );
    void setState( State state );
    void setFlowCoefficient( double coefficient );
    void setArea( double area );
    void setValveTemplate( RimValveTemplate* valveTemplate );

    // Override from RimWellEvent
    EventType eventType() const override;
    QString   generateScheduleKeyword( const QString& wellName ) const override;

protected:
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void                          defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

private:
    caf::PdmField<double>                  m_measuredDepth;
    caf::PdmField<caf::AppEnum<ValveType>> m_valveType;
    caf::PdmField<caf::AppEnum<State>>     m_state;
    caf::PdmField<double>                  m_flowCoefficient;
    caf::PdmField<double>                  m_area;
    caf::PdmPtrField<RimValveTemplate*>    m_valveTemplate;
};

namespace caf
{
template <>
void caf::AppEnum<RimWellEventValve::State>::setUp();

template <>
void caf::AppEnum<RimWellEventValve::ValveType>::setUp();
} // namespace caf
