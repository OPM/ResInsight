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

class RimWellPathAicdParameters : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    RimWellPathAicdParameters();
    ~RimWellPathAicdParameters();
    bool isValid() const;

protected:
    void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    void defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute) override;

private:
    std::set<const caf::PdmField<QString>*> stringFieldsWithNoValidDefault() const;
    void setUnitLabels();
    bool isMetric() const;
private:
    // Required parameters
    caf::PdmField<QString>  m_strengthOfAICD;
    caf::PdmField<QString>  m_densityOfCalibrationFluid;
    caf::PdmField<QString>  m_viscosityOfCalibrationFluid;
    caf::PdmField<QString>  m_volumeFlowRateExponent;
    caf::PdmField<QString>  m_viscosityFunctionExponent;

    // Additional parameters
    caf::PdmField<bool>    m_deviceOpen;
    caf::PdmField<double>  m_lengthOfAICD;

    caf::PdmField<QString> m_criticalWaterInLiquidFractionEmulsions;
    caf::PdmField<QString> m_emulsionViscosityTransitionRegion;
    caf::PdmField<QString> m_maxRatioOfEmulsionVisc;
    caf::PdmField<QString> m_maxFlowRate;
    caf::PdmField<QString> m_exponentOilFractionInDensityMixCalc;
    caf::PdmField<QString> m_exponentWaterFractionInDensityMixCalc;
    caf::PdmField<QString> m_exponentGasFractionInDensityMixCalc;
    caf::PdmField<QString> m_exponentOilFractionInViscosityMixCalc;
    caf::PdmField<QString> m_exponentWaterFractionInViscosityMixCalc;
    caf::PdmField<QString> m_exponentGasFractionInViscosityMixCalc;

};

