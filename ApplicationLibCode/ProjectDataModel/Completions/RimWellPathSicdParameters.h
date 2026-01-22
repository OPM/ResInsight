/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026    Equinor ASA
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

#include "cafPdmBase.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

class RimWellPathSicdParameters : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimWellPathSicdParameters();
    ~RimWellPathSicdParameters() override;
    bool isValid() const;

    bool isOpen() const;

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

private:
    caf::PdmField<double>                m_strength;
    caf::PdmField<double>                m_length;
    caf::PdmField<std::optional<double>> m_calibrationDensity;
    caf::PdmField<std::optional<double>> m_calibrationViscosity;
    caf::PdmField<std::optional<double>> m_emlCrt;
    caf::PdmField<std::optional<double>> m_emlTrans;
    caf::PdmField<std::optional<double>> m_emlMax;
    caf::PdmField<std::optional<int>>    m_scaleFactorType;
    caf::PdmField<std::optional<double>> m_maxCalibRate;
    caf::PdmField<bool>                  m_deviceOpen;
};
