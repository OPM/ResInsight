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
class RimWelspecsData : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimWelspecsData();
    ~RimWelspecsData() override;

public:
    caf::PdmField<QString>                m_wellname;
    caf::PdmField<QString>                m_groupname;
    caf::PdmField<int>                    m_I;
    caf::PdmField<int>                    m_J;
    caf::PdmField<std::optional<double>>  m_bhpDepth;
    caf::PdmField<QString>                m_phase;
    caf::PdmField<std::optional<double>>  m_drainageRadius;
    caf::PdmField<std::optional<QString>> m_inflowEquation;
    caf::PdmField<std::optional<QString>> m_autoShutIn;
    caf::PdmField<std::optional<QString>> m_crossFlow;
    caf::PdmField<std::optional<int>>     m_pvtTableNum;
    caf::PdmField<std::optional<QString>> m_hydrostaticDensCalc;
    caf::PdmField<std::optional<int>>     m_fipRegion;
};
