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
#include "RimWellPathAicdParameters.h"

#include "RimWellPath.h"

#include "cafPdmUiDoubleValueEditor.h"
#include "cafPdmUiGroup.h"
#include "cafPdmUiLineEditor.h"

#include <QDoubleValidator>

CAF_PDM_SOURCE_INIT(RimWellPathAicdParameters, "WellPathAicdParameters");

class NumericStringValidator : public QDoubleValidator
{
public:
    NumericStringValidator(const QString& defaultValue)
        : m_defaultValue(defaultValue), QDoubleValidator(nullptr)
    {}

    void fixup(QString& input) const override
    {
        input = m_defaultValue;
    }

    State validate(QString& input, int& pos) const override
    {
        if (input == m_defaultValue)
        {
            return QValidator::Acceptable;
        }

        return QDoubleValidator::validate(input, pos);
    }
private:
    QString m_defaultValue;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathAicdParameters::RimWellPathAicdParameters()
{
    // clang-format off
    CAF_PDM_InitObject("RimWellPathAicdParameters", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_strengthOfAICD, "StrengthAICD", "Strength of AICD", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_densityOfCalibrationFluid, "DensityCalibrationFluid", "Calibration Fluid Density (kg/m^3)", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_viscosityOfCalibrationFluid, "ViscosityCalibrationFluid", "Calibration Fluid Viscosity (cP)", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_volumeFlowRateExponent, "VolumeFlowRateExponent", "Volume Flow Rate Exponent", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_viscosityFunctionExponent, "ViscosityFunctionExponent", "Viscosity Function Exponent", "", "", "");
    CAF_PDM_InitField(&m_deviceOpen, "DeviceOpen", true, "Device Open?", "", "", "");
    
    CAF_PDM_InitField(&m_lengthOfAICD, "LengthOfAICD", 12.0, "Length of AICD (m)", "", "", "");
    m_lengthOfAICD.uiCapability()->setUiEditorTypeName(caf::PdmUiDoubleValueEditor::uiEditorTypeName());
    
    CAF_PDM_InitField(&m_criticalWaterInLiquidFractionEmulsions, "CriticalWaterLiquidFractionEmul", QString("1*"), "Critical Water in Liquid Fraction for emulsions", "", "", "");
    CAF_PDM_InitField(&m_emulsionViscosityTransitionRegion, "ViscosityTransitionRegionEmul", QString("1*"), "Emulsion Viscosity Transition Region", "", "", "");
    CAF_PDM_InitField(&m_maxRatioOfEmulsionVisc, "MaxRatioOfEmulsionVisc", QString("1*"), "Max Ratio of Emulsion to Continuous Viscosity", "", "", "");
    CAF_PDM_InitField(&m_maxFlowRate, "MaxFlowRate", QString("1*"), "Max Flow Rate for AICD Device (m^3 / day)", "", "", "");
    CAF_PDM_InitField(&m_exponentOilFractionInDensityMixCalc, "ExponentOilDensity", QString("1*"), "Density Exponent of Oil Fraction", "", "", "");
    CAF_PDM_InitField(&m_exponentWaterFractionInDensityMixCalc, "ExponentWaterDensity", QString("1*"), "Density Exponent of Water Fraction", "", "", "");
    CAF_PDM_InitField(&m_exponentGasFractionInDensityMixCalc, "ExponentGasDensity", QString("1*"), "Density Exponent of Gas Fraction", "", "", "");
    CAF_PDM_InitField(&m_exponentOilFractionInViscosityMixCalc, "ExponentOilViscosity", QString("1*"), "Viscosity Exponent of Oil Fraction", "", "", "");
    CAF_PDM_InitField(&m_exponentWaterFractionInViscosityMixCalc, "ExponentWaterViscosity", QString("1*"), "Viscosity Exponent of Water Fraction", "", "", "");
    CAF_PDM_InitField(&m_exponentGasFractionInViscosityMixCalc, "ExponentGasViscosity", QString("1*"), "Viscosity Exponent of Gas Fraction", "", "", "");

    std::vector<caf::PdmFieldHandle*> allFields;
    this->fields(allFields);
    for (caf::PdmFieldHandle* field : allFields)
    {
        caf::PdmField<QString>* stringField = dynamic_cast<caf::PdmField<QString>*>(field);
        if (stringField)
        {
            stringField->uiCapability()->setUiEditorTypeName(caf::PdmUiLineEditor::uiEditorTypeName());
        }
    }

    // clang-format on
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathAicdParameters::~RimWellPathAicdParameters() {}

//--------------------------------------------------------------------------------------------------
/// Requires that all the required parameters are set and a proper value
//--------------------------------------------------------------------------------------------------
bool RimWellPathAicdParameters::isValid() const
{
    for (const caf::PdmField<QString>* stringField : stringFieldsWithNoValidDefault())
    {
        if (stringField->value().isEmpty()) return false;
        bool ok = true;
        stringField->value().toDouble(&ok);
        if (!ok) return false;
    }
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathAicdParameters::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    caf::PdmUiGroup* requiredGroup = uiOrdering.addNewGroup("Required Parameters");
    requiredGroup->add(&m_strengthOfAICD);
    requiredGroup->add(&m_densityOfCalibrationFluid);
    requiredGroup->add(&m_viscosityOfCalibrationFluid);
    requiredGroup->add(&m_volumeFlowRateExponent);
    requiredGroup->add(&m_viscosityFunctionExponent);

    caf::PdmUiGroup* additionalGroup = uiOrdering.addNewGroup("Additional Parameters");
    additionalGroup->add(&m_deviceOpen);
    additionalGroup->add(&m_lengthOfAICD);
    additionalGroup->add(&m_criticalWaterInLiquidFractionEmulsions);
    additionalGroup->add(&m_emulsionViscosityTransitionRegion);
    additionalGroup->add(&m_maxRatioOfEmulsionVisc);
    additionalGroup->add(&m_maxFlowRate);
    additionalGroup->add(&m_exponentOilFractionInDensityMixCalc);
    additionalGroup->add(&m_exponentWaterFractionInDensityMixCalc);
    additionalGroup->add(&m_exponentGasFractionInDensityMixCalc);
    additionalGroup->add(&m_exponentOilFractionInViscosityMixCalc);
    additionalGroup->add(&m_exponentWaterFractionInViscosityMixCalc);
    additionalGroup->add(&m_exponentGasFractionInViscosityMixCalc);

    additionalGroup->setCollapsedByDefault(true);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathAicdParameters::defineEditorAttribute(const caf::PdmFieldHandle* field,
                                                      QString                    uiConfigName,
                                                      caf::PdmUiEditorAttribute* attribute)
{
    const caf::PdmField<QString>* stringField = dynamic_cast<const caf::PdmField<QString>*>(field);
    caf::PdmUiLineEditorAttribute* lineEditorAttr = dynamic_cast<caf::PdmUiLineEditorAttribute*>(attribute);
    if (stringField && lineEditorAttr)
    {
        if (stringFieldsWithNoValidDefault().count(stringField))
        {
            lineEditorAttr->validator = new NumericStringValidator("");
        }
        else
        {
            lineEditorAttr->validator = new NumericStringValidator("1*");
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<const caf::PdmField<QString>*> RimWellPathAicdParameters::stringFieldsWithNoValidDefault() const
{
    std::set<const caf::PdmField<QString>*> fields = {&m_strengthOfAICD,
                                                      &m_densityOfCalibrationFluid,
                                                      &m_viscosityOfCalibrationFluid,
                                                      &m_volumeFlowRateExponent,
                                                      &m_viscosityFunctionExponent};
    return fields;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathAicdParameters::setUnitLabels()
{
    if (isMetric())
    {
        m_densityOfCalibrationFluid.uiCapability()->setUiName("Calibration Fluid Density (kg / m ^ 3)");
        m_lengthOfAICD.uiCapability()->setUiName("Length of AICD (m)");
        m_maxFlowRate.uiCapability()->setUiName("Max Flow Rate for AICD Device(m ^ 3 / day)");
    }
    else
    {
        m_densityOfCalibrationFluid.uiCapability()->setUiName("Calibration Fluid Density (lb / ft ^3)");
        m_lengthOfAICD.uiCapability()->setUiName("Length of AICD (ft)");
        m_maxFlowRate.uiCapability()->setUiName("Max Flow Rate for AICD Device(ft ^ 3 / day)");
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellPathAicdParameters::isMetric() const
{
    bool         metric = false;
    RimWellPath* wellPath;
    firstAncestorOrThisOfType(wellPath);
    if (wellPath)
    {
        if (wellPath->unitSystem() == RiaEclipseUnitTools::UNITS_METRIC)
        {
            metric = true;
        }
    }
    return metric;
}
