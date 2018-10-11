/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018     Equinor ASA
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
#include "RimNonDarcyPerforationParameters.h"

#include "RimWellPath.h"

#include "cafPdmUiObjectEditorHandle.h"

#include <limits>

namespace caf
{
template<>
void caf::AppEnum<RimNonDarcyPerforationParameters::NonDarcyFlowEnum>::setUp()
{
    addItem(RimNonDarcyPerforationParameters::NON_DARCY_NONE, "None", "None");
    addItem(RimNonDarcyPerforationParameters::NON_DARCY_COMPUTED, "Computed", "Compute D-factor");
    addItem(RimNonDarcyPerforationParameters::NON_DARCY_USER_DEFINED, "UserDefined", "User Defined D-factor");

    setDefault(RimNonDarcyPerforationParameters::NON_DARCY_NONE);
}
} // namespace caf

CAF_PDM_SOURCE_INIT(RimNonDarcyPerforationParameters, "RimNonDarcyPerforationParameters");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimNonDarcyPerforationParameters::RimNonDarcyPerforationParameters()
{
    CAF_PDM_InitObject("NonDarcyPerforationParameters", ":/CompletionsSymbol16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_nonDarcyFlowType, "NonDarcyFlowType", "Non-Darcy Flow", "", "", "");

    CAF_PDM_InitField(&m_userDefinedDFactor, "UserDefinedDFactor", 1.0, "D Factor", "", "", "");

    CAF_PDM_InitField(&m_unitConstant,
                      "UnitConstant",
                      1.0,
                      "<html> Unit Constant (&alpha;)</html>",
                      "",
                      "<html>Unit:[cP*Day*m<sup>2</sup>/(Forch*mD*Sm<sup>3</sup>)]</html>",
                      "");

    CAF_PDM_InitField(&m_inertialCoefficient,
                      "InertialCoefficient",
                      0.006083236,
                      "<html>Inertial Coefficient (&beta;)</html> [Forch. unit]",
                      "",
                      "",
                      "");

    CAF_PDM_InitField(&m_gridPermeabilityScalingFactor,
                      "GridPermeabilityScalingFactor",
                      0.0,
                      "<html>Grid Permeability Scaling Factor (K<sub>r</sub>) [0..1]</html>",
                      "",
                      "",
                      "");

    CAF_PDM_InitField(&m_wellRadius, "WellRadius", 0.15, "<html>Well Radius (r<sub>w</sub>)</html> [m]", "", "", "");

    CAF_PDM_InitField(&m_relativeGasDensity,
                      "RelativeGasDensity",
                      0.8,
                      "<html>Relative Gas Density (&gamma;)</html>",
                      "",
                      "Relative density of gas at surface conditions with respect to air at STP",
                      "");

    CAF_PDM_InitField(&m_gasViscosity,
                      "GasViscosity",
                      0.02,
                      "<html>Gas Viscosity (&mu;)</html> [cP]",
                      "",
                      "Gas viscosity at bottom hole pressure",
                      "");

    CAF_PDM_InitField(&m_inertialCoefficientBeta0,
                      "InertialCoefficientBeta0",
                      883.90,
                      "<html>Inertial Coefficient (&beta;<sub>0</sub>)</html> [Forch. unit]",
                      "",
                      "",
                      "");
    CAF_PDM_InitField(
        &m_permeabilityScalingFactor, "PermeabilityScalingFactor", -1.1045, "Permeability Scaling Factor (B)", "", "", "");

    CAF_PDM_InitField(&m_porosityScalingFactor, "PorosityScalingFactor", 0.0, "Porosity Scaling Factor (C)", "", "", "");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimNonDarcyPerforationParameters::~RimNonDarcyPerforationParameters() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimNonDarcyPerforationParameters::NonDarcyFlowEnum RimNonDarcyPerforationParameters::nonDarcyFlowType() const
{
    return m_nonDarcyFlowType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimNonDarcyPerforationParameters::userDefinedDFactor() const
{
    return m_userDefinedDFactor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimNonDarcyPerforationParameters::unitConstant() const
{
    return m_unitConstant;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimNonDarcyPerforationParameters::inertialCoefficient() const
{
    return m_inertialCoefficient;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimNonDarcyPerforationParameters::gridPermeabilityScalingFactor() const
{
    return m_gridPermeabilityScalingFactor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimNonDarcyPerforationParameters::wellRadius() const
{
    return m_wellRadius;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimNonDarcyPerforationParameters::relativeGasDensity() const
{
    return m_relativeGasDensity;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimNonDarcyPerforationParameters::gasViscosity() const
{
    return m_gasViscosity;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimNonDarcyPerforationParameters::inertialCoefficientBeta0() const
{
    return m_inertialCoefficientBeta0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimNonDarcyPerforationParameters::permeabilityScalingFactor() const
{
    return m_permeabilityScalingFactor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimNonDarcyPerforationParameters::porosityScalingFactor() const
{
    return m_porosityScalingFactor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimNonDarcyPerforationParameters::fieldChangedByUi(const caf::PdmFieldHandle* changedField,
                                                        const QVariant&            oldValue,
                                                        const QVariant&            newValue)
{
    if (changedField == &m_nonDarcyFlowType)
    {
        caf::PdmUiObjectEditorHandle::updateUiAllObjectEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimNonDarcyPerforationParameters::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    auto nonDarcyFlowGroup = uiOrdering.addNewGroup("Non-Darcy Flow");
    nonDarcyFlowGroup->add(&m_nonDarcyFlowType);

    if (m_nonDarcyFlowType == NON_DARCY_USER_DEFINED)
    {
        nonDarcyFlowGroup->add(&m_userDefinedDFactor);
    }
    else if (m_nonDarcyFlowType == NON_DARCY_COMPUTED)
    {
        {
            auto group = nonDarcyFlowGroup->addNewGroup("Parameters");
            group->add(&m_unitConstant);
            group->add(&m_inertialCoefficient);
            group->add(&m_gridPermeabilityScalingFactor);
            group->add(&m_wellRadius);
            group->add(&m_relativeGasDensity);
            group->add(&m_gasViscosity);
        }
        {
            auto group = nonDarcyFlowGroup->addNewGroup("<html>&beta; Factor</html>");
            group->add(&m_inertialCoefficientBeta0);
            group->add(&m_permeabilityScalingFactor);
            group->add(&m_porosityScalingFactor);
        }
    }
    uiOrdering.skipRemainingFields(true);
}
