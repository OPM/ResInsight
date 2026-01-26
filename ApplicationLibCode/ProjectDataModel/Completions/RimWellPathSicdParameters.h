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

#include <array>

enum SICDParameters
{
    SICD_STRENGTH = 0,
    SICD_CALIBRATION_DENSITY,
    SICD_CALIBRATION_VISCOSITY,
    SICD_EML_CRT,
    SICD_EML_TRANS,
    SICD_EML_MAX,
    SICD_MAX_CALIB_RATE,
    SICD_NUM_PARAMS
};

class RimWellPathSicdParameters : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimWellPathSicdParameters();
    ~RimWellPathSicdParameters() override;
    bool isValid() const;

    void                                setValue( SICDParameters parameter, double value );
    std::array<double, SICD_NUM_PARAMS> doubleValues() const;

    bool isOpen() const;

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

private:
    std::array<caf::PdmField<std::optional<double>>, SICD_NUM_PARAMS> m_sicdParameterFields;

    caf::PdmField<bool> m_deviceOpen;
};
