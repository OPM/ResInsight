/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 -    equinor ASA
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
#include "cafPdmObject.h"
#include "cafPdmField.h"

#include <array>

enum AICDParameters
{
    AICD_STRENGTH = 0,
    AICD_DENSITY_CALIB_FLUID,
    AICD_VISCOSITY_CALIB_FLUID,
    AICD_VOL_FLOW_EXP,
    AICD_VISOSITY_FUNC_EXP,
    AICD_NUM_REQ_PARAMS,
    AICD_CRITICAL_WATER_IN_LIQUID_FRAC = AICD_NUM_REQ_PARAMS,
    AICD_EMULSION_VISC_TRANS_REGION,
    AICD_MAX_RATIO_EMULSION_VISC,
    AICD_MAX_FLOW_RATE,
    AICD_EXP_OIL_FRAC_DENSITY,
    AICD_EXP_WATER_FRAC_DENSITY,
    AICD_EXP_GAS_FRAC_DENSITY,
    AICD_EXP_OIL_FRAC_VISCOSITY,
    AICD_EXP_WATER_FRAC_VISCOSITY,
    AICD_EXP_GAS_FRAC_VISCOSITY,
    AICD_NUM_PARAMS
};

class RimWellPathAicdParameters : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    RimWellPathAicdParameters();
    ~RimWellPathAicdParameters();
    bool isValid() const;

    bool isOpen() const;
    std::array<double, AICD_NUM_PARAMS> doubleValues() const;

protected:
    void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    void defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute) override;

private:
    std::set<const caf::PdmField<QString>*> stringFieldsWithNoValidDefault() const;
    void setUnitLabels();
    bool isMetric() const;
private:
    caf::PdmField<bool> m_deviceOpen;

    std::array<caf::PdmField<QString>, AICD_NUM_PARAMS> m_aicdParameterFields;
};

