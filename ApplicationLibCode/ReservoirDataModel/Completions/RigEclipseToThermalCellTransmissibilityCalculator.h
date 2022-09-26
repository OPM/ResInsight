/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022     Statoil ASA
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

#include "RigEclipseToStimPlanCellTransmissibilityCalculator.h"

#include "cvfMatrix4.h"
#include "cvfObject.h"

#include "RimThermalFractureTemplate.h"

#include <vector>

class QString;

class RimEclipseCase;
class RigFractureCell;
class RigResultAccessor;
class RimFracture;

//==================================================================================================
///
///  Calculator used to compute the intersection areas between one Thermal RigFractureCell and Eclipse cells
///  Both active and inactive Eclipse cells are included. The transmissibility value for inactive cells are set to zero.
///  Eclipse reservoir cells open for flow is defined by reservoirCellIndicesOpenForFlow
///
//==================================================================================================
class RigEclipseToThermalCellTransmissibilityCalculator : public RigEclipseToStimPlanCellTransmissibilityCalculator
{
public:
    explicit RigEclipseToThermalCellTransmissibilityCalculator(
        const RimEclipseCase*                              caseToApply,
        cvf::Mat4d                                         fractureTransform,
        double                                             skinFactor,
        double                                             cDarcy,
        const RigFractureCell&                             stimPlanCell,
        const RimFracture*                                 fracture,
        RimThermalFractureTemplate::FilterCakePressureDrop filterCakePressureDrop,
        double                                             injectvityDecline,
        double                                             filterCakeMobility,
        double                                             viscosity,
        double                                             relativePermeability );

protected:
    double calculateTransmissibility( const cvf::Vec3d& transmissibilityVector, double fractureArea ) override;

    RimThermalFractureTemplate::FilterCakePressureDrop m_filterCakePressureDrop;
    double                                             m_injectivityDecline;
    double                                             m_filterCakeMobility;
    double                                             m_viscosity;
    double                                             m_relativePermeability;
};
