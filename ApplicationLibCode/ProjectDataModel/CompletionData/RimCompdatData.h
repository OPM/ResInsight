/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 Equinor ASA
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

#include "cafPdmField.h"
#include "cafPdmObject.h"

#include <optional>

#include <QString>

//==================================================================================================
///
///
//==================================================================================================
class RimCompdatData : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimCompdatData();
    ~RimCompdatData() override;

public:
    caf::PdmField<QString>                m_wellname;
    caf::PdmField<int>                    m_I;
    caf::PdmField<int>                    m_J;
    caf::PdmField<int>                    m_upperK;
    caf::PdmField<int>                    m_lowerK;
    caf::PdmField<QString>                m_openShutFlag;
    caf::PdmField<std::optional<int>>     m_satTableNum;
    caf::PdmField<std::optional<double>>  m_transmissibility;
    caf::PdmField<std::optional<double>>  m_diameter;
    caf::PdmField<std::optional<double>>  m_kh;
    caf::PdmField<std::optional<double>>  m_skinFactor;
    caf::PdmField<std::optional<double>>  m_dFactor;
    caf::PdmField<std::optional<QString>> m_direction;
};
