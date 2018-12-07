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

#include <limits>

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

    CAF_PDM_InitField(&m_deviceOpen, "DeviceOpen", true, "Device Open?", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_aicdParameterFields[AICD_STRENGTH], "StrengthAICD", "Strength of AICD", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_aicdParameterFields[AICD_DENSITY_CALIB_FLUID], "DensityCalibrationFluid", "Calibration Fluid Density (kg/m^3)", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_aicdParameterFields[AICD_VISCOSITY_CALIB_FLUID], "ViscosityCalibrationFluid", "Calibration Fluid Viscosity (cP)", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_aicdParameterFields[AICD_VOL_FLOW_EXP], "VolumeFlowRateExponent", "Volume Flow Rate Exponent", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_aicdParameterFields[AICD_VISOSITY_FUNC_EXP], "ViscosityFunctionExponent", "Viscosity Function Exponent", "", "", "");
    
    CAF_PDM_InitField(&m_aicdParameterFields[AICD_LENGTH], "LengthAICD", QString("12.0"), "Length of AICD", "", "", "");
    CAF_PDM_InitField(&m_aicdParameterFields[AICD_CRITICAL_WATER_IN_LIQUID_FRAC], "CriticalWaterLiquidFractionEmul", QString("1*"), "Critical Water in Liquid Fraction for emulsions", "", "", "");
    CAF_PDM_InitField(&m_aicdParameterFields[AICD_EMULSION_VISC_TRANS_REGION], "ViscosityTransitionRegionEmul", QString("1*"), "Emulsion Viscosity Transition Region", "", "", "");
    CAF_PDM_InitField(&m_aicdParameterFields[AICD_MAX_RATIO_EMULSION_VISC], "MaxRatioOfEmulsionVisc", QString("1*"), "Max Ratio of Emulsion to Continuous Viscosity", "", "", "");
    CAF_PDM_InitField(&m_aicdParameterFields[AICD_MAX_FLOW_RATE], "MaxFlowRate", QString("1*"), "Max Flow Rate for AICD Device (m^3 / day)", "", "", "");
    CAF_PDM_InitField(&m_aicdParameterFields[AICD_EXP_OIL_FRAC_DENSITY], "ExponentOilDensity", QString("1*"), "Density Exponent of Oil Fraction", "", "", "");
    CAF_PDM_InitField(&m_aicdParameterFields[AICD_EXP_WATER_FRAC_DENSITY], "ExponentWaterDensity", QString("1*"), "Density Exponent of Water Fraction", "", "", "");
    CAF_PDM_InitField(&m_aicdParameterFields[AICD_EXP_GAS_FRAC_DENSITY], "ExponentGasDensity", QString("1*"), "Density Exponent of Gas Fraction", "", "", "");
    CAF_PDM_InitField(&m_aicdParameterFields[AICD_EXP_OIL_FRAC_VISCOSITY], "ExponentOilViscosity", QString("1*"), "Viscosity Exponent of Oil Fraction", "", "", "");
    CAF_PDM_InitField(&m_aicdParameterFields[AICD_EXP_WATER_FRAC_VISCOSITY], "ExponentWaterViscosity", QString("1*"), "Viscosity Exponent of Water Fraction", "", "", "");
    CAF_PDM_InitField(&m_aicdParameterFields[AICD_EXP_GAS_FRAC_VISCOSITY], "ExponentGasViscosity", QString("1*"), "Viscosity Exponent of Gas Fraction", "", "", "");

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
bool RimWellPathAicdParameters::isOpen() const
{
    return m_deviceOpen;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::array<double, AICD_NUM_PARAMS> RimWellPathAicdParameters::doubleValues() const
{
    std::array<double, AICD_NUM_PARAMS> doubleValues;
    for (int i = 0; i < (int)AICD_NUM_PARAMS; ++i)
    {
        NumericStringValidator validator(nullptr);
        QString stringValue = m_aicdParameterFields[(AICDParameters)i].value();
        bool ok = true;
        double doubleValue = stringValue.toDouble(&ok);
        if (ok)
        {
            doubleValues[i] = doubleValue;
        }
        else
        {
            doubleValues[i] = std::numeric_limits<double>::infinity();
        }
    }
    return doubleValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathAicdParameters::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    caf::PdmUiGroup* requiredGroup = uiOrdering.addNewGroup("Required Parameters");
    requiredGroup->add(&m_deviceOpen);
    for (int i = 0; i < (int)AICD_NUM_REQ_PARAMS; ++i)
    {
        requiredGroup->add(&m_aicdParameterFields[(AICDParameters) i]);
    }
    
    caf::PdmUiGroup* additionalGroup = uiOrdering.addNewGroup("Additional Parameters");
    for (int i = (int)AICD_LENGTH; i < (int)AICD_NUM_PARAMS; ++i)
    {
        additionalGroup->add(&m_aicdParameterFields[(AICDParameters) i]);
    }
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
    std::set<const caf::PdmField<QString>*> fields;
    for (int i = 0; i < (int)AICD_NUM_REQ_PARAMS; ++i)
    {
        fields.insert(&m_aicdParameterFields[(AICDParameters) i]);
    }
    return fields;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathAicdParameters::setUnitLabels()
{
    if (isMetric())
    {
        m_aicdParameterFields[AICD_DENSITY_CALIB_FLUID].uiCapability()->setUiName("Calibration Fluid Density (kg / m ^ 3)");
        m_aicdParameterFields[AICD_LENGTH].uiCapability()->setUiName("Length of AICD (m)");
        m_aicdParameterFields[AICD_MAX_FLOW_RATE].uiCapability()->setUiName("Max Flow Rate for AICD Device(m ^ 3 / day)");
    }
    else
    {
        m_aicdParameterFields[AICD_DENSITY_CALIB_FLUID].uiCapability()->setUiName("Calibration Fluid Density (lb / ft ^3)");
        m_aicdParameterFields[AICD_LENGTH].uiCapability()->setUiName("Length of AICD (ft)");
        m_aicdParameterFields[AICD_MAX_FLOW_RATE].uiCapability()->setUiName("Max Flow Rate for AICD Device(ft ^ 3 / day)");
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
