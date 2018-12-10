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
#pragma once

#include "RiaEclipseUnitTools.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmUiGroup.h"

class RimNonDarcyPerforationParameters : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum NonDarcyFlowEnum
    {
        NON_DARCY_NONE,
        NON_DARCY_COMPUTED,
        NON_DARCY_USER_DEFINED,
    };

    RimNonDarcyPerforationParameters();
    ~RimNonDarcyPerforationParameters() override;

    NonDarcyFlowEnum nonDarcyFlowType() const;
    double           userDefinedDFactor() const;
    double           gridPermeabilityScalingFactor() const;
    double           wellRadius() const;
    double           relativeGasDensity() const;
    double           gasViscosity() const;
    double           inertialCoefficientBeta0() const;
    double           permeabilityScalingFactor() const;
    double           porosityScalingFactor() const;

protected:
    void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;

private:
    caf::PdmField<caf::AppEnum<NonDarcyFlowEnum>> m_nonDarcyFlowType;
    caf::PdmField<double>                         m_userDefinedDFactor;

    caf::PdmField<double> m_gridPermeabilityScalingFactor;
    caf::PdmField<double> m_wellRadius;
    caf::PdmField<double> m_relativeGasDensity;
    caf::PdmField<double> m_gasViscosity;

    caf::PdmField<double> m_inertialCoefficientBeta0;
    caf::PdmField<double> m_permeabilityScalingFactor;
    caf::PdmField<double> m_porosityScalingFactor;
};
